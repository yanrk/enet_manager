/********************************************************
 * Description : enet manager implement
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2021 - 2022
 ********************************************************/

#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "enet_channel.h"
#include "enet_connection.h"
#include "enet_manager_impl.h"
#include "log/log.h"

EnetManagerImpl::EnetManagerImpl()
    : m_running(false)
    , m_enet_service(nullptr)
    , m_enet_thread_count(0)
{

}

EnetManagerImpl::~EnetManagerImpl()
{
    exit();
}

bool EnetManagerImpl::init(EnetServiceBase * enet_service, const char * host, unsigned short * port_array, std::size_t port_count)
{
    exit();

    do
    {
        if (nullptr == enet_service)
        {
            RUN_LOG_ERR("enet manager init failure while invalid parameters");
            break;
        }

        if (nullptr == port_array && 0 != port_count)
        {
            RUN_LOG_ERR("enet manager init failure while invalid parameters");
            break;
        }

        if (0 != enet_initialize())
        {
            RUN_LOG_ERR("enet manager init failure while enet initialize");
            break;
        }

        m_running = true;

        m_enet_service = enet_service;

        if (port_count > 0)
        {
            if (nullptr == host)
            {
                host = "0.0.0.0";
            }

            ENetAddress address;
            enet_address_set_host(&address, host);

            std::size_t port_index = 0;
            while (port_index < port_count)
            {
                address.port = port_array[port_index++];
                ENetHost * server = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT, 0, 0);
                if (nullptr == server)
                {
                    RUN_LOG_ERR("enet manager init failure while enet server create");
                    break;
                }

                std::thread enet_thread = std::thread(&EnetManagerImpl::handle_event, this, server, nullptr);
                if (!enet_thread.joinable())
                {
                    RUN_LOG_ERR("enet manager init failure while event thread create");
                    break;
                }

                enet_thread.detach();
            }

            if (port_index < port_count)
            {
                break;
            }
        }

        return (true);
    } while (false);

    exit();

    return (false);
}

void EnetManagerImpl::exit()
{
    if (m_running)
    {
        m_running = false;

        if (m_enet_thread_count > 0)
        {
            std::mutex mutex;
            std::unique_lock<std::mutex> locker(mutex);
            std::condition_variable condition;

            while (m_enet_thread_count > 0)
            {
                condition.wait_for(locker, std::chrono::milliseconds(1));
            }
        }

        enet_deinitialize();

        m_enet_service = nullptr;
    }
}

std::shared_ptr<EnetConnection> EnetManagerImpl::create_connection(const std::string & host, unsigned short port, const void * identity, const char * bind_ip, unsigned short bind_port)
{
    ENetHost * client = nullptr;
    ENetPeer * enet_peer = nullptr;
    EnetConnection * enet_connection = nullptr;

    do
    {
        client = enet_host_create(nullptr, 1, ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT, 0, 0);
        if (nullptr == client)
        {
            RUN_LOG_ERR("create connection failure while enet client create");
            break;
        }

        if (0 != bind_port)
        {
            if (nullptr == bind_ip)
            {
                bind_ip = "0.0.0.0";
            }

            ENetAddress address;
            enet_address_set_host_ip(&address, bind_ip);
            address.port = bind_port;
            enet_socket_bind(client->socket, &address);
        }

        ENetAddress address;
        enet_address_set_host(&address, host.c_str());
        address.port = port;

        enet_peer = enet_host_connect(client, &address, ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT, 0);
        if (nullptr == enet_peer)
        {
            RUN_LOG_ERR("create connection failure while enet peer create");
            break;
        }

        ENetEvent enet_event;
        if (enet_host_service(client, &enet_event, 5000) <= 0)
        {
            RUN_LOG_ERR("create connection failure while enet peer connect timeout");
            break;
        }

        if (ENET_EVENT_TYPE_CONNECT != enet_event.type)
        {
            RUN_LOG_ERR("create connection failure while enet peer connect failure");
            break;
        }

        enet_socket_get_address(client->socket, &client->address);

        std::shared_ptr<EnetConnection> enet_connection = std::make_shared<EnetConnection>(m_enet_service, client, enet_peer);
        if (!enet_connection)
        {
            RUN_LOG_ERR("create connection failure while enet connection create");
            break;
        }

        if (!enet_connection->on_connect(identity))
        {
            RUN_LOG_ERR("create connection failure while user preprocess failure");
            break;
        }

        enet_peer->data = enet_connection.get();

        std::thread enet_thread = std::thread(&EnetManagerImpl::handle_event, this, client, enet_connection);
        if (!enet_thread.joinable())
        {
            RUN_LOG_ERR("create connection failure while event thread create");
            m_enet_service->on_connection_close(enet_connection);
            break;
        }

        enet_thread.detach();

        return (enet_connection);
    } while (false);

    if (nullptr != enet_connection)
    {
        delete enet_connection;
    }

    if (nullptr != enet_peer)
    {
        enet_peer_reset(enet_peer);
    }

    if (nullptr != client)
    {
        enet_host_destroy(client);
    }

    return (nullptr);
}

void EnetManagerImpl::handle_event(ENetHost * host, std::shared_ptr<EnetConnection> connection)
{
    ++m_enet_thread_count;

    bool client = true;
    bool working = true;
    ENetEvent enet_event;
    std::map<void *, std::shared_ptr<EnetConnection>> enet_connection_map;

    if (!!connection)
    {
        enet_connection_map[connection.get()] = connection;
    }
    else
    {
        client = false;
    }

    while (m_running && working)
    {
        if (enet_host_service(host, &enet_event, 1) > 0)
        {
            switch (enet_event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                {
                    std::shared_ptr<EnetConnection> enet_connection = std::make_shared<EnetConnection>(m_enet_service, host, enet_event.peer);
                    if (!!enet_connection)
                    {
                        if (enet_connection->on_accept(host->address.port))
                        {
                            enet_event.peer->data = enet_connection.get();
                            enet_connection_map[enet_event.peer->data] = enet_connection;
                        }
                    }
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE:
                {
                    EnetConnection * enet_connection = reinterpret_cast<EnetConnection *>(enet_event.peer->data);
                    if (!!enet_connection)
                    {
                        enet_connection->on_recv(enet_event.channelID, enet_event.packet->data, enet_event.packet->dataLength);
                    }
                    enet_packet_destroy(enet_event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                {
                    EnetConnection * enet_connection = reinterpret_cast<EnetConnection *>(enet_event.peer->data);
                    if (!!enet_connection)
                    {
                        enet_connection->on_close();
                        enet_connection_map.erase(enet_event.peer->data);
                        enet_event.peer->data = nullptr;
                    }
                    if (client)
                    {
                        working = false;
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }

    enet_host_destroy(host);

    --m_enet_thread_count;
}

/********************************************************
 * Description : enet connection
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2021 - 2022
 ********************************************************/

#include "enet_channel.h"
#include "enet_connection.h"

EnetConnectionBase::EnetConnectionBase()
{

}

EnetConnectionBase::~EnetConnectionBase()
{

}

EnetConnection::EnetConnection(std::map<void *, std::shared_ptr<EnetConnection>> * enet_connection_map, EnetServiceBase * enet_service, ENetHost * enet_host, ENetPeer * enet_peer, EnetPacketList * enet_packets)
    : m_enet_connection_map(enet_connection_map)
    , m_enet_service(enet_service)
    , m_enet_host(enet_host)
    , m_enet_peer(enet_peer)
    , m_enet_packets(enet_packets)
    , m_enet_channels()
    , m_user_data(nullptr)
    , m_closed(false)
{

}

EnetConnection::~EnetConnection()
{

}

void EnetConnection::set_user_data(void * user_data)
{
    m_user_data = user_data;
}

void * EnetConnection::get_user_data()
{
    return (m_user_data);
}

void EnetConnection::get_host_address(std::string & ip, unsigned short & port)
{
    if (m_closed)
    {
        return;
    }

    char buffer[128] = { 0x0 };
    enet_address_get_host_ip(&m_enet_host->address, buffer, sizeof(buffer) - 1);
    ip = buffer;
    port = m_enet_host->address.port;
}

void EnetConnection::get_peer_address(std::string & ip, unsigned short & port)
{
    if (m_closed)
    {
        return;
    }

    char buffer[128] = { 0x0 };
    enet_address_get_host_ip(&m_enet_peer->address, buffer, sizeof(buffer) - 1);
    ip = buffer;
    port = m_enet_peer->address.port;
}

EnetChannelSharedPtr EnetConnection::open_channel()
{
    if (m_closed)
    {
        return (nullptr);
    }

    for (std::size_t index = 0; index < sizeof(m_enet_channels) / sizeof(m_enet_channels[0]); ++index)
    {
        enet_uint8 channel = static_cast<enet_uint8>(index);
        std::shared_ptr<EnetChannel> enet_channel = m_enet_channels[channel];
        if (!!enet_channel)
        {
            continue;
        }

        enet_channel = std::make_shared<EnetChannel>(shared_from_this(), m_enet_service, channel);
        if (!enet_channel)
        {
            return (nullptr);
        }

        m_enet_channels[channel] = enet_channel;
        if (!m_enet_service->on_channel_open(shared_from_this(), enet_channel))
        {
            enet_channel->close();
            return (nullptr);
        }

        return (enet_channel);
    }

    return (nullptr);
}

void EnetConnection::close()
{
    if (m_closed)
    {
        return;
    }

    m_enet_peer->data = nullptr;

    enet_peer_disconnect(m_enet_peer, 0);

    on_close();
}

bool EnetConnection::send(enet_uint8 channel, const void * data, std::size_t len, bool reliable)
{
    if (m_closed)
    {
        return (false);
    }

    ENetPacket * enet_packet = enet_packet_create(data, len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    if (nullptr == enet_packet)
    {
        return (false);
    }

    EnetPacket packet = { 0x0 };
    packet.peer = m_enet_peer;
    packet.packet = enet_packet;
    packet.channel = channel;

    {
        std::lock_guard<std::mutex> locker(m_enet_packets->mutex);

        m_enet_packets->packets.push_back(packet);
    }

    return (true);
}

bool EnetConnection::on_connect(const void * identity)
{
    if (!m_enet_service->on_connection_connect(shared_from_this(), identity))
    {
        return (false);
    }

    m_enet_peer->data = this;

    if (nullptr != m_enet_connection_map)
    {
        (*m_enet_connection_map)[this] = shared_from_this();
    }

    return (true);
}

bool EnetConnection::on_accept(unsigned short listener_port)
{
    if (!m_enet_service->on_connection_accept(shared_from_this(), listener_port))
    {
        return (false);
    }

    m_enet_peer->data = this;

    if (nullptr != m_enet_connection_map)
    {
        (*m_enet_connection_map)[this] = shared_from_this();
    }

    return (true);
}

void EnetConnection::on_close()
{
    if (m_closed)
    {
        return;
    }

    m_closed = true;

    for (std::size_t index = 0; index < sizeof(m_enet_channels) / sizeof(m_enet_channels[0]); ++index)
    {
        enet_uint8 channel = static_cast<enet_uint8>(index);
        std::shared_ptr<EnetChannel> enet_channel = m_enet_channels[channel];
        if (!!enet_channel)
        {
            enet_channel->close();
        }
    }

    m_enet_service->on_connection_close(shared_from_this());

    m_enet_peer->data = nullptr;

    if (nullptr != m_enet_connection_map)
    {
        m_enet_connection_map->erase(this);
    }
}

bool EnetConnection::on_recv(enet_uint8 channel, const void * data, std::size_t len)
{
    if (m_closed)
    {
        return (false);
    }

    if (channel >= sizeof(m_enet_channels) / sizeof(m_enet_channels[0]))
    {
        return (false);
    }

    std::shared_ptr<EnetChannel> enet_channel = m_enet_channels[channel];
    if (!enet_channel)
    {
        enet_channel = std::make_shared<EnetChannel>(shared_from_this(), m_enet_service, channel);
        if (!enet_channel)
        {
            return (false);
        }

        m_enet_channels[channel] = enet_channel;
        if (!m_enet_service->on_channel_open(shared_from_this(), enet_channel))
        {
            enet_channel->close();
            return (false);
        }
    }

    if (enet_channel->closed())
    {
        return (false);
    }

    return (m_enet_service->on_channel_recv(shared_from_this(), enet_channel, data, len));
}

bool EnetConnection::closed() const
{
    return (m_closed);
}

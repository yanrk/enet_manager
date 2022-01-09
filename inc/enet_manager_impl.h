/********************************************************
 * Description : enet manager implement
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2021 - 2022
 ********************************************************/

#ifndef ENET_MANAGER_IMPLEMENT_H
#define ENET_MANAGER_IMPLEMENT_H


#include <atomic>
#include "enet/enet.h"
#include "enet_packet.h"
#include "enet_manager.h"

class EnetConnection;

class EnetManagerImpl
{
public:
    EnetManagerImpl();
    ~EnetManagerImpl();

public:
    EnetManagerImpl(const EnetManagerImpl &) = delete;
    EnetManagerImpl(EnetManagerImpl &&) = delete;
    EnetManagerImpl & operator = (const EnetManagerImpl &) = delete;
    EnetManagerImpl & operator = (EnetManagerImpl &&) = delete;

public:
    bool init(EnetServiceBase * enet_service, const char * host, unsigned short * port_array, std::size_t port_count);
    void exit();

public:
    std::shared_ptr<EnetConnection> create_connection(const std::string & host, unsigned short port, const void * identity, const char * bind_ip, unsigned short bind_port);

private:
    void handle_event(ENetHost * enet_host, EnetPacketList * enet_packets, std::shared_ptr<EnetConnection> connection);

private:
    volatile bool                               m_running;
    EnetServiceBase                           * m_enet_service;
    std::atomic<uint32_t>                       m_enet_thread_count;
};


#endif // ENET_MANAGER_IMPLEMENT_H

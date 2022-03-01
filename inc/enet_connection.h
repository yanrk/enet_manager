/********************************************************
 * Description : enet connection
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2021 - 2022
 ********************************************************/

#ifndef ENET_CONNECTION_H
#define ENET_CONNECTION_H


#include <map>
#include "enet/enet.h"
#include "enet_packet.h"
#include "enet_manager.h"

class EnetChannel;

class EnetConnection : public EnetConnectionBase, public std::enable_shared_from_this<EnetConnection>
{
public:
    EnetConnection(std::map<void *, std::shared_ptr<EnetConnection>> * enet_connection_map, EnetServiceBase * enet_service, ENetHost * enet_host, ENetPeer * enet_peer, EnetPacketList * enet_packets);
    virtual ~EnetConnection() override;

public:
    EnetConnection(const EnetConnection &) = delete;
    EnetConnection(EnetConnection &&) = delete;
    EnetConnection & operator = (const EnetConnection &) = delete;
    EnetConnection & operator = (EnetConnection &&) = delete;

public:
    virtual void set_user_data(void * user_data) override;
    virtual void * get_user_data() override;

public:
    virtual void get_host_address(std::string & ip, unsigned short & port) override;
    virtual void get_peer_address(std::string & ip, unsigned short & port) override;

public:
    virtual EnetChannelSharedPtr open_channel() override;

public:
    virtual void close() override;

public:
    bool send(enet_uint8 channel, const void * data, std::size_t len, bool reliable);

public:
    bool on_connect(const void * identity);
    bool on_accept(unsigned short listener_port);
    bool on_recv(enet_uint8 channel, const void * data, std::size_t len);
    void on_close();

public:
    bool closed() const;

private:
    std::map<void *, std::shared_ptr<EnetConnection>> * m_enet_connection_map;
    EnetServiceBase                                   * m_enet_service;
    ENetHost                                          * m_enet_host;
    ENetPeer                                          * m_enet_peer;
    EnetPacketList                                    * m_enet_packets;
    std::shared_ptr<EnetChannel>                        m_enet_channels[ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT];
    void                                              * m_user_data;
    bool                                                m_closed;
};


#endif // ENET_CONNECTION_H

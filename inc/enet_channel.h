/********************************************************
 * Description : enet channel
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2021 - 2022
 ********************************************************/

#ifndef ENET_CHANNEL_H
#define ENET_CHANNEL_H


#include "enet/enet.h"
#include "enet_manager.h"

class EnetConnection;

class EnetChannel : public EnetChannelBase, public std::enable_shared_from_this<EnetChannel>
{
public:
    EnetChannel(std::weak_ptr<EnetConnection> enet_connection, EnetServiceBase * enet_service, enet_uint8 channel_id);
    virtual ~EnetChannel() override;

public:
    EnetChannel(const EnetChannel &) = delete;
    EnetChannel(EnetChannel &&) = delete;
    EnetChannel & operator = (const EnetChannel &) = delete;
    EnetChannel & operator = (EnetChannel &&) = delete;

public:
    virtual void set_user_data(void * user_data) override;
    virtual void * get_user_data() override;

public:
    virtual bool send_data(const void * data, std::size_t len, bool reliable) override;

public:
    virtual void close() override;
    virtual bool reopen() override;

public:
    bool closed() const;

private:
    std::weak_ptr<EnetConnection>                   m_enet_connection;
    EnetServiceBase                               * m_enet_service;
    enet_uint8                                      m_channel_id;
    void                                          * m_user_data;
    bool                                            m_closed;
};


#endif // ENET_CHANNEL_H

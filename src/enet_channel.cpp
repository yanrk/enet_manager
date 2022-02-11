/********************************************************
 * Description : enet channel
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2021 - 2022
 ********************************************************/

#include "enet_channel.h"
#include "enet_connection.h"

EnetChannelBase::EnetChannelBase()
{

}

EnetChannelBase::~EnetChannelBase()
{

}

EnetChannel::EnetChannel(std::weak_ptr<EnetConnection> enet_connection, EnetServiceBase * enet_service, enet_uint8 channel_id)
    : m_enet_connection(enet_connection)
    , m_enet_service(enet_service)
    , m_channel_id(channel_id)
    , m_user_data(nullptr)
    , m_closed(false)
{

}

EnetChannel::~EnetChannel()
{

}

void EnetChannel::set_user_data(void * user_data)
{
    m_user_data = user_data;
}

void * EnetChannel::get_user_data()
{
    return (m_user_data);
}

bool EnetChannel::send_data(const void * data, std::size_t len, bool reliable)
{
    if (m_closed)
    {
        return (false);
    }

    std::shared_ptr<EnetConnection> enet_connection = m_enet_connection.lock();
    if (!enet_connection)
    {
        return (false);
    }

    return (enet_connection->send(m_channel_id, data, len, reliable));
}

void EnetChannel::close()
{
    if (m_closed)
    {
        return;
    }

    std::shared_ptr<EnetConnection> enet_connection = m_enet_connection.lock();
    if (!enet_connection)
    {
        return;
    }

    m_closed = true;

    m_enet_service->on_channel_close(enet_connection, shared_from_this());
}

bool EnetChannel::reopen()
{
    if (!m_closed)
    {
        return (false);
    }

    std::shared_ptr<EnetConnection> enet_connection = m_enet_connection.lock();
    if (!enet_connection)
    {
        return (false);
    }

    if (enet_connection->closed())
    {
        return (false);
    }

    m_closed = false;

    m_enet_service->on_channel_open(enet_connection, shared_from_this());

    return (true);
}

std::shared_ptr<EnetConnectionBase> EnetChannel::get_connection()
{
    return (m_enet_connection.lock());
}

bool EnetChannel::closed() const
{
    return (m_closed);
}

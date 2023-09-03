/********************************************************
 * Description : enet manager class
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2021 - 2022
 ********************************************************/

#include "enet_channel.h"
#include "enet_connection.h"
#include "enet_manager_impl.h"

EnetManager::EnetManager()
    : m_manager_impl(nullptr)
{

}

EnetManager::~EnetManager()
{
    exit();
}

bool EnetManager::init(EnetServiceBase * enet_service, const char * host, unsigned short * port_array, std::size_t port_count, bool port_any_valid)
{
    m_manager_impl = new EnetManagerImpl;
    if (nullptr == m_manager_impl)
    {
        return (false);
    }

    if (m_manager_impl->init(enet_service, host, port_array, port_count, port_any_valid))
    {
        return (true);
    }

    delete m_manager_impl;
    m_manager_impl = nullptr;

    return (false);
}

void EnetManager::exit()
{
    if (nullptr != m_manager_impl)
    {
        m_manager_impl->exit();
        delete m_manager_impl;
        m_manager_impl = nullptr;
    }
}

EnetConnectionSharedPtr EnetManager::create_connection(const std::string & host, unsigned short port, const void * identity, const char * bind_ip, unsigned short bind_port)
{
    if (nullptr != m_manager_impl)
    {
        return (m_manager_impl->create_connection(host, port, identity, bind_ip, bind_port));
    }
    return (nullptr);
}

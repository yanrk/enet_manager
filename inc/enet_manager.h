/********************************************************
 * Description : enet manager
 * Author      : ryan
 * Email       : ryan@rayvision.com
 * Version     : 1.0
 * History     :
 * Copyright(C): 2021-2022
 ********************************************************/

#ifndef ENET_MANAGER_H
#define ENET_MANAGER_H


#include <string>
#include <memory>

#ifdef _MSC_VER
    #define ENET_MANAGER_CDECL             __cdecl
    #define ENET_MANAGER_STDCALL           __stdcall
    #ifdef EXPORT_ENET_MANAGER_DLL
        #define ENET_MANAGER_API           __declspec(dllexport)
    #else
        #ifdef USE_ENET_MANAGER_DLL
            #define ENET_MANAGER_API       __declspec(dllimport)
        #else
            #define ENET_MANAGER_API
        #endif // USE_ENET_MANAGER_DLL
    #endif // EXPORT_ENET_MANAGER_DLL
#else
    #define ENET_MANAGER_CDECL
    #define ENET_MANAGER_STDCALL
    #define ENET_MANAGER_API
#endif // _MSC_VER

class ENET_MANAGER_API EnetChannelBase
{
public:
    EnetChannelBase();
    virtual ~EnetChannelBase();

public:
    virtual void set_user_data(void * user_data) = 0;
    virtual void * get_user_data() = 0;

public:
    virtual bool send_data(const void * data, std::size_t len, bool reliable = true) = 0;

public:
    virtual void close() = 0;
    virtual bool reopen() = 0;
};

typedef std::shared_ptr<EnetChannelBase> EnetChannelSharedPtr;
typedef std::weak_ptr<EnetChannelBase> EnetChannelWeakPtr;

class ENET_MANAGER_API EnetConnectionBase
{
public:
    EnetConnectionBase();
    virtual ~EnetConnectionBase();

public:
    virtual void set_user_data(void * user_data) = 0;
    virtual void * get_user_data() = 0;

public:
    virtual void get_host_address(std::string & ip, unsigned short & port) = 0;
    virtual void get_peer_address(std::string & ip, unsigned short & port) = 0;

public:
    virtual EnetChannelSharedPtr open_channel() = 0;

public:
    virtual void close() = 0;
};

typedef std::shared_ptr<EnetConnectionBase> EnetConnectionSharedPtr;
typedef std::weak_ptr<EnetConnectionBase> EnetConnectionWeakPtr;

class ENET_MANAGER_API EnetServiceBase
{
public:
    EnetServiceBase();
    virtual ~EnetServiceBase();

public:
    virtual bool on_connection_connect(EnetConnectionSharedPtr connection, const void * identity) = 0;
    virtual bool on_connection_accept(EnetConnectionSharedPtr connection, unsigned short listener_port) = 0;
    virtual void on_connection_close(EnetConnectionSharedPtr connection) = 0;

public:
    virtual bool on_channel_open(EnetConnectionSharedPtr connection, EnetChannelSharedPtr channel) = 0;
    virtual bool on_channel_recv(EnetConnectionSharedPtr connection, EnetChannelSharedPtr channel, const void * data, std::size_t len) = 0;
    virtual void on_channel_close(EnetConnectionSharedPtr connection, EnetChannelSharedPtr channel) = 0;
};

class EnetManagerImpl;

class ENET_MANAGER_API EnetManager
{
public:
    EnetManager();
    ~EnetManager();

public:
    EnetManager(const EnetManager &) = delete;
    EnetManager(EnetManager &&) = delete;
    EnetManager & operator = (const EnetManager &) = delete;
    EnetManager & operator = (EnetManager &&) = delete;

public:
    bool init(EnetServiceBase * enet_service, const char * host = nullptr, unsigned short * port_array = nullptr, std::size_t port_count = 0);
    void exit();

public:
    EnetConnectionSharedPtr create_connection(const std::string & host, unsigned short port, const void * identity = nullptr, const char * bind_ip = "0.0.0.0", unsigned short bind_port = 0);

private:
    EnetManagerImpl                               * m_manager_impl;
};


#endif // ENET_MANAGER_H

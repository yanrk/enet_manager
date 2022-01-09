/********************************************************
 * Description : enet packet
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2021 - 2022
 ********************************************************/

#ifndef ENET_PACKET_H
#define ENET_PACKET_H


#include <list>
#include <mutex>
#include "enet/enet.h"

struct EnetPacket
{
    ENetPeer                  * peer;
    ENetPacket                * packet;
    enet_uint8                  channel;
};

struct EnetPacketList
{
    std::mutex                  mutex;
    std::list<EnetPacket>       packets;
};


#endif // ENET_PACKET_H

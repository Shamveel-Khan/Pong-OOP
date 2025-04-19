#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "network.h"
#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std;

// Structure for game state
typedef struct
{
    float x;
    float y;
    float serverPaddle;
    float clientPaddle;
} state;

typedef struct
{
    bool pause;
} pauseState;

typedef struct
{
    int height;
    int width;
} size;

int networkInitialize(NetworkMode mode, const char *address, ENetHost **host, ENetPeer **peer)
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return 1;
    }
    ENetAddress addr;
    if (mode == MODE_SERVER)
    {
        addr.host = ENET_HOST_ANY;
        addr.port = 1234;
        *host = enet_host_create(&addr, 32, 2, 0, 0);

        if (!*host)
        {
            fprintf(stderr, "Failed to create server.\n");
            return 1;
        }

        printf("Server waiting for client...\n");
        if (networkWaitForConnect(*host, 20000) <= 0)
        {
            fprintf(stderr, "No client connected within timeout.\n");
            return 1;
        }
    }
    else
    { // Client
        addr.port = 1234;
        enet_address_set_host_ip(&addr, address);
        *host = enet_host_create(NULL, 1, 2, 0, 0);

        if (!*host)
        {
            fprintf(stderr, "Failed to create client.\n");
            return 1;
        }

        *peer = enet_host_connect(*host, &addr, 2, 0);
        if (!*peer)
        {
            fprintf(stderr, "Failed to connect to server.\n");
            return 1;
        }

        printf("Client waiting for server response...\n");
        if (networkWaitForConnect(*host, 5000) <= 0)
        {
            fprintf(stderr, "Connection failed.\n");
            return 1;
        }
    }

    return 0;
}

void networkShutdown(ENetHost *host)
{
    if (host)
    {
        enet_host_destroy(host);
    }
    enet_deinitialize();
}

int networkSendState(ENetHost *host, ENetPeer *peer, float x, float y, float serverPaddle, float clientPaddle)
{
    state sts;
    sts.x = x;
    sts.y = y;
    sts.serverPaddle = serverPaddle;
    sts.clientPaddle = clientPaddle;
    ENetPacket *gts = enet_packet_create(&sts, sizeof(state), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    if (peer)
    {
        enet_peer_send(peer, 0, gts);
    }
    else
    {
        enet_host_broadcast(host, 0, gts);
    }
    enet_host_flush(host);
    return 0;
}

int networkReceiveState(ENetHost *host, float *x, float *y, float *serverPaddle, float *clientPaddle)
{
    ENetEvent event;
    while (enet_host_service(host, &event, 5) > 0)
    {
        if (event.type == ENET_EVENT_TYPE_RECEIVE)
        {
            if (event.packet->dataLength == sizeof(state))
            {
                state *Sdata = (state *)event.packet->data;
                *x = Sdata->x;
                *y = Sdata->y;
                *serverPaddle = Sdata->serverPaddle;
                *clientPaddle = Sdata->clientPaddle;
            }
            enet_packet_destroy(event.packet);
        }
    }
    return 0;
}

void networkProcessEvents(ENetHost *host)
{
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0)
    {
        if (event.type == ENET_EVENT_TYPE_CONNECT)
        {
            printf("Server: A client connected.\n");
        }
        else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
        {
            printf("A client disconnected.\n");
        }
    }
}

int networkWaitForConnect(ENetHost *host, int timeout)
{
    ENetEvent event;
    int result = enet_host_service(host, &event, timeout);
    if (result > 0)
    {
        if (event.type == ENET_EVENT_TYPE_CONNECT)
            return 1;
        if (event.type == ENET_EVENT_TYPE_DISCONNECT)
            return -1;
    }
    return 0;
}

void sendPause(ENetHost *host, ENetPeer *peer, bool state)
{
    pauseState p;
    p.pause = state;
    ENetPacket *pauseMessage = enet_packet_create(&p, sizeof(pauseState), ENET_PACKET_FLAG_RELIABLE);
    if (peer)
    {
        enet_peer_send(peer, 1, pauseMessage);
        cout << "Sent pause from client as: " << state << endl;
    }
    else
    {
        enet_host_broadcast(host, 1, pauseMessage);
        cout << "Sent pause from server as: " << state << endl;
    }
    enet_host_flush(host);
}

bool receivePause(ENetHost *host, bool curr)
{
    ENetEvent receivePause;
    while (enet_host_service(host, &receivePause, 5))
    {
        if (receivePause.type == ENET_EVENT_TYPE_RECEIVE && receivePause.packet->dataLength == sizeof(pauseState))
        {
            pauseState *p2 = (pauseState *)receivePause.packet->data;
            bool ans = (*p2).pause;
            cout << "Received Pause as: " << ans << endl;
            return ans;
        }
        enet_packet_destroy(receivePause.packet);
    }
    return curr;
}
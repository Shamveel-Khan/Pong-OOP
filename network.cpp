#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "network.h"
#include <enet/enet.h>
#include <stdio.h>
#include <string.h>

// Structure for game state
typedef struct {
    float x;
    float y;
    float serverPaddle;
    float clientPaddle;
} state;

typedef struct {
    int height;
    int width;
} size;

int networkInitialize(NetworkMode mode, const char* address, ENetHost** host, ENetPeer** peer, int* sh1, int* sw1, int* sh2, int* sw2) {
    if (enet_initialize() != 0) {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return 1;
    }
    ENetAddress addr;
    if (mode == MODE_SERVER) {
        addr.host = ENET_HOST_ANY;
        addr.port = 1234;
        *host = enet_host_create(&addr, 32, 2, 0, 0);

        if (!*host) {
            fprintf(stderr, "Failed to create server.\n");
            return 1;
        }

        printf("Server waiting for client...\n");
        if (networkWaitForConnect(*host, 5000) <= 0) {
            fprintf(stderr, "No client connected within timeout.\n");
            return 1;
        }

        // Send size to client
        size serverSize = { *sh1, *sw1 };
        ENetPacket* sizePacket = enet_packet_create(&serverSize, sizeof(size), ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(*host, 0, sizePacket);
        enet_host_flush(*host);

        // Receive client's size
        ENetEvent event;
        if (enet_host_service(*host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
            if (event.packet->dataLength == sizeof(size)) {
                size* clientSize = (size*)event.packet->data;
                *sh2 = clientSize->height;
                *sw2 = clientSize->width;
            }
            enet_packet_destroy(event.packet);
        }
    } else { // Client
        addr.port = 1234;
        enet_address_set_host_ip(&addr, address);
        *host = enet_host_create(NULL, 1, 2, 0, 0);

        if (!*host) {
            fprintf(stderr, "Failed to create client.\n");
            return 1;
        }

        *peer = enet_host_connect(*host, &addr, 2, 0);
        if (!*peer) {
            fprintf(stderr, "Failed to connect to server.\n");
            return 1;
        }

        printf("Client waiting for server response...\n");
        if (networkWaitForConnect(*host, 5000) <= 0) {
            fprintf(stderr, "Connection failed.\n");
            return 1;
        }

        // Receive server's size
        ENetEvent event;
        if (enet_host_service(*host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
            if (event.packet->dataLength == sizeof(size)) {
                size* serverSize = (size*)event.packet->data;
                *sh1 = serverSize->height;
                *sw1 = serverSize->width;
            }
            enet_packet_destroy(event.packet);
        }

        // Send client's size to server
        size clientSize = { *sh2, *sw2 };
        ENetPacket* sizePacket = enet_packet_create(&clientSize, sizeof(size), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(*peer, 0, sizePacket);
        enet_host_flush(*host);
    }

    return 0;
}


void networkShutdown(ENetHost* host) {
    if (host) {
        enet_host_destroy(host);
    }
    enet_deinitialize();
}

int networkSendState(ENetHost* host, ENetPeer* peer, float x, float y, float serverPaddle, float clientPaddle) {
    state sts;
    sts.x = x;
    sts.y = y;
    sts.serverPaddle = serverPaddle;
    sts.clientPaddle = clientPaddle;
    ENetPacket* gts = enet_packet_create(&sts, sizeof(state), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    if(peer) {
        enet_peer_send(peer, 0, gts);
    }
    else {
        enet_host_broadcast(host, 0, gts);
    }
    enet_host_flush(host);
    return 0;
}

int networkReceiveState(ENetHost* host, float* x, float* y, float* serverPaddle, float* clientPaddle) {
    ENetEvent event;
    while (enet_host_service(host, &event, 5) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            if (event.packet->dataLength == sizeof(state)) {
                state* Sdata = (state*) event.packet->data;
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

void networkProcessEvents(ENetHost* host) {
    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            printf("Server: A client connected.\n");
        } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            printf("A client disconnected.\n");
        }
    }
}

int networkWaitForConnect(ENetHost* host, int timeout) {
    ENetEvent event;
    int result = enet_host_service(host, &event, timeout);
    if (result > 0) {
        if (event.type == ENET_EVENT_TYPE_CONNECT) return 1;
        if (event.type == ENET_EVENT_TYPE_DISCONNECT) return -1;
    }
    return 0;
}
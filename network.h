#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>   // For size_t
#include <stdint.h>   // For uint32_t and uint8_t

// Forward declarations to hide full ENet definitions:
typedef struct _ENetAddress ENetAddress;
typedef struct _ENetHost    ENetHost;
typedef struct _ENetPeer    ENetPeer;
typedef struct _ENetEvent   ENetEvent;
typedef struct _ENetPacket  ENetPacket;

// Mode enumeration:
typedef enum {
    MODE_SERVER,
    MODE_CLIENT
} NetworkMode;

#ifdef __cplusplus
extern "C" {
#endif

// One-to-one wrapper function declarations (camelCase wrappers)
int networkInitialize(NetworkMode mode, const char* address, ENetHost** host, ENetPeer** peer);
void networkShutdown(ENetHost* host);
int networkSendState(ENetHost* host, ENetPeer* peer, float x, float y, float serverPaddle, float clientPaddle);
int networkReceiveState(ENetHost* host, float* x, float* y, float* serverPaddle, float* clientPaddle);

// New function: Wait for a connection event (only used in client mode).
// Returns 1 if connected, -1 if a disconnect event occurs, or 0 if timeout/no event.
int networkWaitForConnect(ENetHost* host, int timeout);
void networkProcessEvents(ENetHost* host);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_H

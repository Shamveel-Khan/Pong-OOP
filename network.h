#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>  
#include <stdint.h>  

typedef struct _ENetAddress ENetAddress;
typedef struct _ENetHost    ENetHost;
typedef struct _ENetPeer    ENetPeer;
typedef struct _ENetEvent   ENetEvent;
typedef struct _ENetPacket  ENetPacket;

typedef enum {
    MODE_SERVER,
    MODE_CLIENT
} NetworkMode;

#ifdef __cplusplus
extern "C" {
#endif

int networkInitialize(NetworkMode mode, const char* address, ENetHost** host, ENetPeer** peer);
void networkShutdown(ENetHost* host);
int networkSendState(ENetHost* host, ENetPeer* peer, float x, float y, float serverPaddle, float clientPaddle);
int networkReceiveState(ENetHost* host, float* x, float* y, float* serverPaddle, float* clientPaddle);

int networkWaitForConnect(ENetHost* host, int timeout);
void networkProcessEvents(ENetHost* host);
void sendPause(ENetHost *host, ENetPeer *peer, bool state);
bool receivePause(ENetHost *host, bool curr);
int networkReceiveScores(ENetHost *host, int* sl,int* sr);
int networkSendScore(ENetHost *host, ENetPeer *peer, int sl,int sr);
int receiveTime(ENetHost *host, int currentTime);
void sendTime(ENetHost *host, int time);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_H
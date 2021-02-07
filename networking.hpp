#pragma once
#include "enet.h"
#include "player.hpp"

#define ENETPORT 7777
#define MAX_CLIENTS 8

#define GAMESERVER_PORT     7777
#define GAMESERVER_TICKRATE 30

// Initialize server
extern ENetAddress address;
extern ENetHost *server;
extern ENetEvent event;
extern int event_status;

extern ENetHost *client;
extern ENetPeer *peer;
extern int enet_event_status;

extern bool is_server;
extern bool is_client;

int init_server();
int init_client(const char *ip = "10.0.0.52");
void client_publish(const Player& player);
void update_networking(Player& player);


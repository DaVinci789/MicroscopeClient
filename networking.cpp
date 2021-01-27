#include "networking.hpp"
#include "common.hpp"
#include "player.hpp"

int init_server() {
    if (enet_initialize() != 0) {
        std::cout << "failure to init enet" << std::endl;
        return -1;
    }

    address.host = ENET_HOST_ANY;
    address.port = ENETPORT;
    server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);
    if (server == NULL) {
        std::cout << "failure to create server host" << std::endl;
        return -1;
    }
    is_server = true;
    return 0;
}

int init_client(const char *ip) {
    if (enet_initialize() != 0) {
        std::cout << "failure to init enet" << std::endl;
        return -1;
    }
    client = enet_host_create(NULL, 1, 2, 0, 0);
    if (client == NULL) {
        std::cout << "failure to create client" << std::endl;
        return -1;
    }
    enet_address_set_host(&address, ip);
    address.port = GAMESERVER_PORT;
    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL) {
        std::cout << "no avaliable peers for initiating a connection" << std::endl;
        return -1;
    }
    enet_event_status = enet_host_service(client, &event, 1000);
    is_client = true;
    return 0;
}

void client_publish(const Player& player) {
    Vector2 position = GetScreenToWorld2D(GetMousePosition(), player.camera);
    ENetPacket *packet = enet_packet_create(&position, sizeof(position), ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(client);
}

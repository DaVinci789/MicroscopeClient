#pragma once
#include "common.hpp"
#include "card.hpp"

struct Drawer {
    bool open;
    Rectangle body_rect;
    std::vector<Card>* cards;
};

Drawer init_drawer(std::vector<Card>* cards = NULL);
void update_drawer(Drawer& drawer);
void draw_drawer(const Drawer& drawer, Camera2D camera);

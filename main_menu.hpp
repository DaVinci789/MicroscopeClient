#pragma once
#include "common.hpp"

struct MainMenu {
    bool visible;
    Rectangle body_rect;
    Button start_game;
    Button settings;
    Button load_game;
    Button close_button;
};

MainMenu init_menu(bool visible = false);
void update_menu(MainMenu& menu, Vector2 mouse_position, bool& new_game, bool& opened_save_file, char *opened_file = NULL);
void draw_menu(MainMenu& menu);

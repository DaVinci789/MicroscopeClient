#pragma once
#include "card.hpp"
#include "palette.hpp"
#include "project.hpp"
#include "drawer.hpp"

enum PlayerState {
    HOVERING, // Just looking, but still able to move cards around and such
    READONLY,
    WRITING,
    BIGPICTUREWRITING,
    FOCUSWRITING,
    PALETTEWRITING,
    SCENECARDSELECTING,
    DRAWERCARDSELECTING,
    GRABBING,
    SELECTING, // Click and dragging on the background
};

enum WhichEdit {
    NAME,
    BODY,
};

struct KeyBinds {
    KeyboardKey move_up;
    KeyboardKey move_down;
    KeyboardKey move_left;
    KeyboardKey move_right;
};

struct Player {
    bool quit;
    bool mouse_held;
    Rectangle player_rect;
    Vector2 mouse_position;

    Camera2D camera;
    int camera_move_speed;
    int zoom_level;
    double camera_zoom_target;
    Vector2 camera_target;

    KeyBinds binds;
    PlayerState state;
    WhichEdit editing;
    PaletteType palette_edit_type;

    Vector2 hold_origin;
    Vector2 hold_diff;
    Rectangle selection_rec;

    Card *selected_card;
    Vector2 offset;
    bool resizing_card;
};

Player init_player();
void player_update_camera(Player &player, bool allow_key_scroll = true);
void player_write_update(Player& player);
void player_resize_chosen_card(Player& player);
void player_hover_update(Player& player, std::vector<Card>& cards, Palette& palette, Project &project, Drawer& drawer);
void player_grabbing_update(Player& player, std::vector<Card>& cards);
void player_write_big_picture_update(Player &player, Project &project);
void player_write_palette_update(Player& player, Palette &palette);
void player_write_focus_update(Player& player, Project& project);
void player_select_scene_card_update(Player& player, std::vector<Card>& cards);
void player_drawer_select_card_update(Player& player, Drawer& drawer);

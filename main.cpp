#include <raylib.h>
#ifdef __linux__
#include <signal.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>

#define ENET_IMPLEMENTATION
#include "enet.h"

#include "common.hpp"
#include "player.hpp"
#include "card.hpp"
#include "palette.hpp"
#include "networking.hpp"

ENetAddress address;
ENetHost *server = NULL;
ENetEvent event;
int event_status;

ENetHost *client = NULL;
ENetPeer *peer = NULL;
int enet_event_status;

bool is_server = false;
bool is_client = false;

Vector2 previous_mouse_position = {0};

Font application_font_small;
Font application_font_regular;
Font application_font_large;

Texture2D spritesheet;

Texture generate_grid() {
    auto data = std::vector<char>();
    int gridsize = GRIDSIZE;
    for (int y = 0; y < gridsize * 2; y++) {
        for (int x = 0; x < gridsize * 2; x++) {
            auto current_color = RAYWHITE;
            if ((x % gridsize == 0 || x % gridsize == gridsize - 1) && (y % gridsize == 0 || y % gridsize == gridsize - 1)) current_color = BLUE;
            data.push_back(current_color.r);
            data.push_back(current_color.g);
            data.push_back(current_color.b);
            data.push_back(current_color.a);
        }
    }
    Image image = {data.data(), gridsize * 2, gridsize * 2, 1, UNCOMPRESSED_R8G8B8A8};
    return LoadTextureFromImage(image);
}

volatile int signal_handler = 1;
void signal_func(int dummy) {
    signal_handler = 0;
}

int main(void) {
    Defer {if (is_server || is_client) enet_deinitialize();};
    SetTraceLogLevel(LOG_INFO);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(800, 600, "Microscope RPG");
    MaximizeWindow();
    SetExitKey(-1);
    Defer {CloseWindow();};

    spritesheet = LoadTexture("assets/spritesheet.png");
    Defer {UnloadTexture(spritesheet);};

    auto logo = LoadImage("assets/logo.png");
    Defer {UnloadImage(logo);};

    SetWindowIcon(logo);

    application_font_small  = LoadFontEx("assets/monogram_extended.ttf", FONTSIZE_SMALL, NULL, 256);
    application_font_regular = LoadFontEx("assets/monogram_extended.ttf", FONTSIZE_REGULAR, NULL, 256);
    application_font_large  = LoadFontEx("assets/monogram_extended.ttf", FONTSIZE_LARGE, NULL, 256);
    Defer {UnloadFont(application_font_small);};
    Defer {UnloadFont(application_font_regular);};
    Defer {UnloadFont(application_font_large);};

    Project current_project = init_project("New Game");
    Palette palette = init_palette();

    Player player = init_player();
    auto cards = std::vector<Card>();

    Texture grid_texture = generate_grid();

    #ifdef __linux__
    signal(SIGINT, signal_func);
    #endif

    while (!WindowShouldClose() && signal_handler) {
        if (is_server) {
            event_status = enet_host_service(server, &event, 0);
            if (event_status > 0) {
                switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("A new client connected from %x:%u.\n",
                           event.peer -> address.host,
                           event.peer -> address.port);
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    printf("%s\n", event.packet->data);
                    break;
                }
            }
        } else if (is_client) {
            client_publish();
        }

        // Update Game
        /// How I write update function sigs:
        /// first param is the player struct followed by everything the player can interact with while in that state.
        switch (player.state) {
        case READONLY:
            break;
        case WRITING:
            player_write_update(player);
            player_resize_chosen_card(player);
            break;
        case FOCUSWRITING:
            player_write_focus_update(player, current_project);
            break;
        case PALETTEWRITING:
            player_write_palette_update(player, palette);
            break;
        case HOVERING:
            player_hover_update(player, cards, palette, current_project);
            // HACK: This is here because if we enter the focus writing state, we want to keep it "purple" to signify that it's been selected.
            update_button_hover(current_project.focus, GetMousePosition());
            break;
        case GRABBING:
            player_grabbing_update(player, cards);
            break;
        case SELECTING:
            break;
        default:
            break;
        }
        previous_mouse_position = GetMousePosition();

        // Tween camera
        player.camera.target = lerp<Vector2>(floor(player.camera.target), floor(player.camera_target), 0.2);
        player.camera.zoom = lerp<float>(player.camera.zoom, player.camera_zoom_target, 0.2);

        update_cards(cards);
        update_palette(palette);
        update_project(current_project);

        BeginDrawing();
        BeginMode2D(player.camera);
        ClearBackground(RAYWHITE);
        // Draw Background Grid
        auto src = (Rectangle) {-100000, -100000, 200000, 200000};
        auto dst = src;
        DrawTexturePro(grid_texture, src, dst, (Vector2){0, 0}, 0, RAYWHITE);
        // Draw Description Lines and Big Picture
        DrawLineEx((Vector2) {0, -100000}, (Vector2) {0, 100000}, 3.0, WHITE);
        DrawLineEx((Vector2) {-100000, 0}, (Vector2) {100000, 0}, 3.0, WHITE);
        DrawRectangle(1, 1, MeasureTextEx(application_font_regular, current_project.big_picture.c_str(), FONTSIZE_REGULAR, 1.0).x + 16, MeasureTextEx(application_font_regular, current_project.big_picture.c_str(), FONTSIZE_REGULAR, 1.0).y, WHITE);
        DrawTextEx(application_font_regular, current_project.big_picture.c_str(), {8, -1}, FONTSIZE_REGULAR, 1.0, BLACK);

        Defer {
            for (auto &card: cards) {
                card.drawn = false;
            }
        };
        int current_depth = 0;
        // Depth sorting for cards
        while (true) {
            bool every_card_undrawn = true;
            for (auto &card: cards) {
                if (card.drawn) {
                    continue;
                } else {
                    every_card_undrawn = false;
                    if (card.depth == current_depth) {
                        draw(card, player.camera);
                    }
                }
            }
            if (every_card_undrawn) {
                break;
            }
            current_depth += 1;
        }

        EndMode2D();

        // Draw Player Select Rectangle
        if (player.state == GRABBING) {
            DrawRectangleLinesEx((Rectangle){
                    player.selection_rec.x,
                    player.selection_rec.y,
                    player.selection_rec.width,
                    player.selection_rec.height
            }, 5.0, BLUE);

        }

        // Draw project (really just the focus for now 1/22/2021)
        draw(current_project);
        // Draw palette
        draw(palette);

        switch (player.state) {
        case WRITING:
            DrawText("Writing", 0, GetScreenHeight() - 16, 16, BLACK);
            break;
        case HOVERING:
            DrawText("Hovering", 0, GetScreenHeight() - 16, 16, BLACK);
            break;
        case GRABBING:
            DrawText("Grabbing", 0, GetScreenHeight() - 16, 16, BLACK);
            break;
        case FOCUSWRITING:
            DrawText("Focuswriting", 0, GetScreenHeight() - 16, 16, BLACK);
            break;
        case PALETTEWRITING:
            DrawText("Palettewriting", 0, GetScreenHeight() - 16, 16, BLACK);
            break;
        }

        // Draw player cursor over everything.
        player.player_rect.x = GetMousePosition().x;
        player.player_rect.y = GetMousePosition().y;
        DrawRectangleRec(player.player_rect, BLUE);

        EndDrawing();
    }

    return 0;
}

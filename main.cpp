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
#include "main_menu.hpp"
#include "player.hpp"
#include "card.hpp"
#include "card_pool.hpp"
#include "palette.hpp"
#include "drawer.hpp"
#include "serialization.hpp"

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
Shader darken_shader;

Texture generate_grid() {
    auto data = std::vector<char>();
    int gridsize = GRIDSIZE;
    for (int y = 0; y < gridsize * 2; y++) {
        for (int x = 0; x < gridsize * 2; x++) {
            auto current_color = WHITE;
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

    long spritesheet_modtime = GetFileModTime("assets/spritesheet.png");
    spritesheet = LoadTexture("assets/spritesheet.png");
    Defer {UnloadTexture(spritesheet);};

    auto logo = LoadImage("assets/logo.png");
    Defer {UnloadImage(logo);};

    SetWindowIcon(logo);

    // File loading stuff
    Font font_base = LoadFontEx("assets/monogram_extended.ttf", FONTSIZE_BASE, NULL, 256);
    application_font_small  = font_base;
    application_font_regular = font_base;
    application_font_large  = font_base;
    Defer {UnloadFont(font_base);}; // Idk if this leaks memory

    darken_shader = LoadShader(0, "assets/darken.fs");
    int darken_loc = GetShaderLocation(darken_shader, "darkness_mod");
    float value = 2.0;
    SetShaderValue(darken_shader, darken_loc, &value, UNIFORM_FLOAT);
    Defer {UnloadShader(darken_shader);};

    // Component stuff

    MainMenu main_menu = init_menu(true);
 
    Project current_project = init_project("Scratch Buffer");
    Palette palette = init_palette();

    Player player = init_player();
    auto cards = std::vector<Card>();//FileExists("save.json") ? load_cards() : std::vector<Card>();
    if (FileExists("save.json")) {
        load_cards(cards);
    }

    Texture grid_texture = generate_grid();
    Drawer drawer = init_drawer();

    bool win_focus = IsWindowFocused();
    bool last_win_focus = win_focus;

    #ifdef __linux__
    signal(SIGINT, signal_func);
    #endif
    Vector2 external_data = {-1000, -1000};

    while (!WindowShouldClose() && signal_handler && !player.quit) {
        win_focus = IsWindowFocused();
        if (win_focus != last_win_focus) {
            SetTargetFPS(win_focus ? 60 : 10);
        }
        last_win_focus = win_focus;

        if (is_server) {
            event_status = enet_host_service(server, &event, 0);
            if (event_status > 0) {
                switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("A new client connected from %x:%u.\n",
                           event.peer -> address.host,
                           event.peer -> address.port);
                    break;
                case ENET_EVENT_TYPE_RECEIVE: {
                    Vector2 vector_data = * ((Vector2*) event.packet->data);
		    print(vector_data);
                    external_data = vector_data;
		    enet_packet_destroy(event.packet);
                    break;
					      }
		case ENET_EVENT_TYPE_DISCONNECT:
		    printf("Player disconnected.\n");
		    event.peer->data = NULL;
		    break;
                }
            } else {
		print(123);
	    }
            Vector2 position = GetScreenToWorld2D(GetMousePosition(), player.camera);
            ENetPacket *packet = enet_packet_create(&position, sizeof(position), ENET_PACKET_FLAG_UNSEQUENCED);
            enet_host_broadcast(server, 0, packet);
        } else if (is_client) {
            event_status = enet_host_service(client, &event, 0);
            if (event_status > 0) {
                switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    Vector2 vector_data = * ((Vector2*) event.packet->data);
                    print(vector_data);
                    external_data = vector_data;
                    enet_packet_destroy(event.packet);
                }
                default:
                    break;
                }
            }
            client_publish(player);
        }

        if (main_menu.visible) {
            update_cards(cards);
            update_palette(palette);
            update_project(current_project);
            char opened_file[256] = {0};
            bool file_changed = false;
            bool new_game = false;
            update_menu(main_menu, GetMousePosition(), new_game, file_changed, opened_file);
            if (file_changed) {
                cards.clear();
                update_cards(cards);
                load_cards(cards, opened_file);
            } else if (new_game) {
                cards.clear();
            }
            goto draw;
        }

        // Update Game
        /// How I write update function sigs:
        /// first param is the player struct followed by everything the player can interact with while in that state.
        switch (player.state) {
        case READONLY:
            break;
        case HOVERING:
            player_hover_update(player, cards, palette, current_project, drawer, main_menu);
            // HACK: This is here because if we enter the focus writing state, we want to keep it "purple" to signify that it's been selected.
            update_button_hover(current_project.focus, GetMousePosition());
            break;
        case WRITING:
            player_update_camera(player, false);
            player_write_update(player);
            player_resize_chosen_card(player);
            break;
        case BIGPICTUREWRITING:
            player_write_big_picture_update(player, current_project);
            break;
        case FOCUSWRITING:
            player_write_focus_update(player, current_project);
            break;
        case PALETTEWRITING:
            player_write_palette_update(player, palette);
            break;
        case SCENECARDSELECTING:
            player_update_camera(player, true);
            player_select_scene_card_update(player, cards);
            break;
        case DRAWERCARDSELECTING:
            player_update_camera(player, true);
            player_drawer_select_card_update(player, drawer, cards);
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
        update_drawer(drawer);

    draw:
        BeginDrawing();
        BeginMode2D(player.camera);
        ClearBackground(RAYWHITE);
        // Draw Background Grid
        auto src = (Rectangle) {-100000, -100000, 200000, 200000};
        auto dst = src;
        DrawTexturePro(grid_texture, src, dst, (Vector2){0, 0}, 0, WHITE);
        // Draw Description Lines and Big Picture
        DrawLineEx((Vector2) {0, -100000}, (Vector2) {0, 100000}, 3.0, SKYBLUE);
        DrawLineEx((Vector2) {-100000, 0}, (Vector2) {100000, 0}, 3.0, SKYBLUE);
        DrawRectangle(1, 1, MeasureTextEx(application_font_regular, current_project.big_picture.c_str(), FONTSIZE_REGULAR, 1.0).x + 16, MeasureTextEx(application_font_regular, current_project.big_picture.c_str(), FONTSIZE_REGULAR, 1.0).y, SKYBLUE);
        DrawTextEx(application_font_regular, current_project.big_picture.c_str(), {8, -1}, FONTSIZE_REGULAR, 1.0, BLACK);

        // Depth sorting for cards
        // TODO use std stuff
        int current_depth = smallest_depth(cards);
        while (true) {
            bool every_card_undrawn = true;
            for (auto &card: cards) {
                if (card.drawn) {
                    continue;
                } else {
                    every_card_undrawn = false;
                    if (card.depth == current_depth) {
                        if (card.type != player.card_focus && player.is_card_type_focus) BeginShaderMode(darken_shader);
                        draw(card, player.camera);
                        if (card.type != player.card_focus && player.is_card_type_focus) EndShaderMode();
                    }
                }
            }
            if (every_card_undrawn) {
                break;
            }
            current_depth += 1;
        }
        for (auto &card: cards) {
            card.drawn = false;
        }

        DrawRectangleRec((Rectangle) {external_data.x, external_data.y, 10, 10}, RED);

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
        if (main_menu.visible) draw_menu(main_menu);

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

        if (drawer.open) draw_drawer(drawer, player.camera);

        // Draw player cursor over everything.
        player.player_rect.x = GetMousePosition().x;
        player.player_rect.y = GetMousePosition().y;
        DrawRectangleRec(player.player_rect, BLUE);
        DrawFPS(0, 0);
        EndDrawing();

        if (spritesheet_modtime != GetFileModTime("assets/spritesheet.png")) {
            spritesheet = LoadTexture("assets/spritesheet.png");
            spritesheet_modtime = GetFileModTime("assets/spritesheet.png");
        }
    }
    save_cards(cards);

    return 0;
}

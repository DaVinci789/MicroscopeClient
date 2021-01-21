#include <raylib.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "common.hpp"

#define GRIDSIZE 32

template<typename T>
T clamp(T value, T lower, T upper) {
    if (value < lower) return lower;
    else if (value > upper) return upper;
    else return value;
}

template<typename T>
T lerp(T a, T b, float f)
{
    return a + f * (b - a);
}

template<>
Vector2 lerp<Vector2>(Vector2 v1, Vector2 v2, float amount)
{
    Vector2 result = { 0 };

    result.x = v1.x + amount*(v2.x - v1.x);
    result.y = v1.y + amount*(v2.y - v1.y);

    return result;
}

bool operator==(Color lh, Color rh) {
    return lh.r == rh.r && lh.g == rh.g && lh.b == rh.b && lh.a == rh.a;
}

Vector2 Vector2Scale(Vector2 v, float scale)
{
    Vector2 result = { v.x*scale, v.y*scale };
    return result;
}

Vector2 operator-(Vector2 v1, Vector2 v2) {
    Vector2 result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

Vector2 operator+(Vector2 v1, Vector2 v2) {
    Vector2 result = { v1.x + v2.x, v1.y + v2.y };
    return result;
}

bool collide(Rectangle rect1, Rectangle rect2) {
    return CheckCollisionRecs(rect1, rect2);
}

void print(Vector2 vector) {
    std::cout << "(" << vector.x << " " << vector.y << ")" << std::endl;
}

void print(int n) {
    std::cout << n << std::endl;
}

void print(Rectangle rect) {
    std::cout << "(" << rect.x << " " << rect.y << " " << rect.width << " " << rect.height << ")" << std::endl;
}

Vector2 GetWorldMousePosition(Camera2D camera) {
    auto position = camera.target;
    auto mouse_position = GetMousePosition();
    mouse_position.x -= (float) GetScreenWidth() / 2.0;
    mouse_position.y -= (float) GetScreenHeight() / 2.0;
    mouse_position.x /= camera.zoom;
    mouse_position.y /= camera.zoom;

    position.x += mouse_position.x;
    position.y += mouse_position.y;

    return position;
}

struct Project {
    std::string name;
};

Project init_project(std::string project_name) {
    Project project;
    project.name = project_name;
    return project;
}

enum CardType {
    LIGHT,
    DARK
};

struct Card {
    std::string name;
    std::string content;
    CardType type;
    bool deleted;
    bool grabbed;
    bool selected;

    Rectangle body_rect;
    Vector2 lock_target;
    Color color;

    Rectangle header_rec;
    Rectangle close_button_rec;
    Rectangle edit_button_rec;
    Rectangle type_toggle_rec;
    bool header_hover;
    bool close_hover;
    bool button_hover;
    bool type_toggle_hover;

    int depth;
    bool drawn;
};

Card init_card(std::string name, Rectangle body_rect) {
    float header_height = 20.0;
    Card card;
    card.name = name;
    card.content = "";
    card.type = LIGHT;
    card.deleted  = false;
    card.grabbed  = false;
    card.selected = false;
    card.body_rect = body_rect;
    card.lock_target = {body_rect.x, body_rect.y};
    card.color = WHITE;
    card.header_rec       = {card.body_rect.x, card.body_rect.y - header_height, card.body_rect.width - 30, header_height};
    card.close_button_rec = {card.body_rect.x + card.body_rect.width - 30, card.body_rect.y - header_height, 30, header_height};
    card.edit_button_rec  = {card.body_rect.x + card.body_rect.width - 50, card.body_rect.y + card.body_rect.height - 30, 50, 30};
    card.type_toggle_rec  = {card.body_rect.x, card.body_rect.height - 30, 50, 30};

    card.header_hover = false;
    card.close_hover  = false;
    card.button_hover = false;
    card.type_toggle_hover = false;
    card.depth = 0;
    card.drawn = false;
    return card;
}

Card* greatest_depth_and_furthest_along(std::vector<Card>& cards) {
    if (cards.size() == 0) {
        return NULL;
    }
    auto current_depth = 0;
    Card *current_card = &cards[0];
    for (auto &card: cards) {
        if (card.depth > current_depth) {
            current_card = &card;
            current_depth = card.depth;
        }
    }
    return current_card;
}


enum PlayerState {
    READONLY,
    WRITING,
    HOVERING, // Just looking, but still able to move cards around and such
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
    bool mouse_held;
    Rectangle player_rect;

    Camera2D camera;
    int camera_move_speed;
    double camera_zoom_target;
    Vector2 camera_target;

    KeyBinds binds;
    PlayerState state;
    WhichEdit editing;

    Vector2 hold_origin;
    Vector2 hold_diff;
    Rectangle selection_rec;

    Card *selected_card;
    Vector2 offset;
};

Player init_player() {
    Player player;
    player.mouse_held  = false;

    auto position = GetMousePosition();
    player.player_rect = {position.x, position.y, 10, 10};
    player.camera_move_speed = 20;
    player.camera = {0};
    player.camera_target = {0, 0};
    player.camera.target = player.camera_target;
    player.camera_zoom_target = 1.0f;
    player.camera.offset = (Vector2){ 800/2, 600/2 };
    player.camera.rotation = 0.0f;
    player.camera.zoom = 1.0f;

    player.binds = {
        KEY_W,
        KEY_S,
        KEY_A,
        KEY_D,
    };
    player.state = HOVERING;
    player.editing = BODY;

    player.hold_origin = {0};
    player.hold_diff = {0};

    player.selected_card = NULL;
    player.offset = {0, 0};
    return player;
}

void spawn_card(Player player, std::vector<Card>& cards) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    Rectangle to_draw = {position.x, position.y, 300, 200};
    Card the_card = init_card("New Card", to_draw);
    auto next_card = greatest_depth_and_furthest_along(cards);
    if (next_card) the_card.depth = next_card->depth + 1;
    else the_card.depth = 0;
    cards.push_back(the_card);
}

void player_write_update(Player& player) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        player.state = HOVERING;
        player.selected_card = NULL;
        return;
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (player.editing == NAME && player.selected_card->name.size() > 0) player.selected_card->name.pop_back();
        else if (player.selected_card->content.size() > 0)player.selected_card->content.pop_back();
        return;
    }
    // Typing
    if (player.selected_card) {
        auto char_pressed = GetCharPressed();
        if (char_pressed != 0) {
            if (player.editing == NAME) {
                player.selected_card->name += (char) char_pressed;
            } else {
                player.selected_card->content += (char) char_pressed;
            }
        }
    }
}

Vector2 lock_position_to_grid(Vector2 position) {
    return (Vector2) {
        (float) round(position.x / (float) GRIDSIZE) * (float) GRIDSIZE,
        (float) round(position.y / (float) GRIDSIZE) * (float) GRIDSIZE
    };
}

void player_hover_update(Player& player, std::vector<Card>& cards) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    player.player_rect.x = position.x;
    player.player_rect.y = position.y;

    // Mouse and Card Selection
    for (auto &card: cards) {
        // Drag Card?
        if (IsMouseButtonPressed(0) && collide(player.player_rect, {card.body_rect.x, card.body_rect.y - 30, card.body_rect.width, card.body_rect.height + 30})) {
            player.mouse_held = true;

            // If the game has found a card for the player to hold and the next card in the unsorted array of cards is lower than that card, skip it. 
            if (player.selected_card) {
                if (card.depth < player.selected_card->depth) continue;
            }
            card.grabbed = true;
            player.offset = {player.player_rect.x - card.body_rect.x, player.player_rect.y - card.body_rect.y};
            player.selected_card = &card;
        }
        // FIXME: The button logic shouldn't be in this function!
        card.header_hover      = collide(player.player_rect, card.header_rec);
        card.close_hover       = collide(player.player_rect, card.close_button_rec);
        card.button_hover      = collide(player.player_rect, card.edit_button_rec);
        card.type_toggle_hover = collide(player.player_rect, card.type_toggle_rec);
    }

    if (player.selected_card && IsMouseButtonPressed(0)) {
        auto deepest_card = greatest_depth_and_furthest_along(cards);
        player.selected_card->depth = deepest_card->depth + 1;
        // Card button clicked!
        if (player.selected_card->header_hover) {
            player.state = WRITING;
            player.editing = NAME;
            player.mouse_held = false;
            player.offset = {0 ,0};
        } else if (player.selected_card->close_hover) {
            player.selected_card->deleted = true;
            player.mouse_held = false;
            player.offset = {0 ,0};
        } else if (player.selected_card->button_hover) {
            player.state = WRITING;
            player.editing = BODY;
            player.mouse_held = false;
            player.offset = {0 ,0};
        } else if (player.selected_card->type_toggle_hover) {
            if (player.selected_card->color == WHITE) player.selected_card->color = BLACK;
            else player.selected_card->color = WHITE;
        }
    } else if (IsMouseButtonReleased(0) && player.selected_card) {
        auto position_to_lock_to = lock_position_to_grid((Vector2) {player.selected_card->body_rect.x, player.selected_card->body_rect.y});
        // player.selected_card->body_rect.x = position_to_lock_to.x;
        // player.selected_card->body_rect.y = position_to_lock_to.y;
        player.selected_card->lock_target = position_to_lock_to;
        player.selected_card->grabbed = false;
        player.mouse_held = false;
        player.offset = {0, 0};
        player.selected_card = NULL;
    } else if (IsMouseButtonPressed(0)) { // Player clicks on background
        player.state = GRABBING;
        player.hold_origin = GetMousePosition();
        return;
    }

    if (player.mouse_held && player.selected_card) {
        player.selected_card->body_rect.x = position.x - player.offset.x;
        player.selected_card->body_rect.y = position.y - player.offset.y;
    }

    // Key Processing
    /// Move Left/Right
    if (IsKeyDown(player.binds.move_right)) {
        player.camera_target.x += player.camera_move_speed;
    } else if (IsKeyDown(player.binds.move_left)) {
        player.camera_target.x -= player.camera_move_speed;
    }

    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        // Zoom In/Out
        if (IsKeyPressed(player.binds.move_up)) {
            player.camera_zoom_target += 0.5f;
        } else if (IsKeyPressed(player.binds.move_down)) {
            player.camera_zoom_target -= 0.5f;
        }
        player.camera_zoom_target = clamp<float>(player.camera_zoom_target, 0.5, 1.5);
    } else {
        // Move Up/Down
        if (IsKeyDown(player.binds.move_up)) {
            player.camera_target.y -= player.camera_move_speed;
        } else if (IsKeyDown(player.binds.move_down)) {
            player.camera_target.y += player.camera_move_speed;
        }
    }

    /// Spawn card
    if (IsKeyPressed(KEY_SPACE)) {
        spawn_card(player, cards);
    }
}

Vector2 previous_mouse_position = {0};

Vector2 get_mouse_delta(Camera2D camera) {
    Vector2 vec = GetMousePosition() - previous_mouse_position;
    // vec = Vector2Scale(vec, 1/camera.zoom);
    return vec;
}

void player_grabbing_update(Player& player, std::vector<Card>& cards) {
    if (IsMouseButtonReleased(0)) {
        player.hold_diff = {0, 0};
        player.selection_rec = {0};
        player.state = HOVERING;
        return;
    }
    player.hold_diff = player.hold_diff + get_mouse_delta(player.camera);
    player.selection_rec = {
        player.hold_origin.x,
        player.hold_origin.y,
        player.hold_diff.x,
        player.hold_diff.y,
    };
    if (player.hold_diff.x < 0) {
        player.selection_rec.x = player.hold_origin.x + player.hold_diff.x;
        player.selection_rec.width *= -1;
    }
    if (player.hold_diff.y < 0) {
        player.selection_rec.y = player.hold_origin.y + player.hold_diff.y;
        player.selection_rec.height *= -1;
    }

    // print(player.selection_rec);
    auto world_coords = GetScreenToWorld2D((Vector2) {player.selection_rec.x, player.selection_rec.y}, player.camera);
    auto world_size = Vector2Scale((Vector2) {player.selection_rec.width, player.selection_rec.height}, 1.0/player.camera.zoom);
    Rectangle selected_world_rect = {world_coords.x, world_coords.y, world_size.x, world_size.y};
    for (auto &card: cards) {
        card.selected = collide(card.body_rect, selected_world_rect);
    }
}

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

Font application_font;

void draw(Card &card, Camera2D camera, Player player) {
    Defer {card.drawn = true;};

    auto position = GetScreenToWorld2D(GetMousePosition(), camera);
    // Draw Header
    if (card.header_hover) {
        DrawRectangleRec(card.header_rec, PURPLE);
    } else {
        DrawRectangleRec(card.header_rec, GRAY);
    }
    auto card_title = std::vector<char>();
    for (auto &c: card.name) {
        card_title.push_back((char) c);
    }
    card_title.push_back((char) '\0');
    DrawText(card_title.data(), card.body_rect.x, card.body_rect.y - card.header_rec.height, 20, WHITE);
    /// Draw close button
    DrawRectangleRec(card.close_button_rec, RED);

    // Draw Body
    DrawRectangleRec(card.body_rect, card.color);
    /// Draw Card Border
    if (!card.selected) {
        DrawRectangleLinesEx(card.body_rect, 5.0, card.color == BLACK ? WHITE : BLACK);
    } else {
        DrawRectangleLinesEx(card.body_rect, 5.0, BLUE);
    }
    // Draw Card Content
    auto content = std::vector<char>();
    for (auto &c: card.content) {
        content.push_back((char) c);
    }
    content.push_back((char) '\0');

    card.body_rect.x += 5;
    card.body_rect.y += 5;
    card.body_rect.width -= 5;
    card.body_rect.height -= 5;
    DrawTextRec(application_font, content.data(), card.body_rect, 16, 1.0, true, card.color == BLACK ? WHITE : BLACK);
    card.body_rect.x -= 5;
    card.body_rect.y -= 5;
    card.body_rect.width += 5;
    card.body_rect.height += 5;

    // Draw Toggle Button
    if (card.type_toggle_hover) {
        DrawRectangleRec(card.type_toggle_rec, PURPLE);
    } else {
        DrawRectangleRec(card.type_toggle_rec, GRAY);
    }

    // Draw Edit Button
    if (card.button_hover) {
        DrawRectangleRec(card.edit_button_rec, PURPLE);
    } else {
        DrawRectangleRec(card.edit_button_rec, GRAY);
    }

    float vertical_offset = (card.edit_button_rec.height - MeasureTextEx(application_font, "Edit", 16.0, 1.0).y) / 2.0;
    float horizontal_offset = (card.edit_button_rec.width - MeasureTextEx(application_font, "Edit", 16.0, 1.0).x) / 2.0;
    card.edit_button_rec.x += vertical_offset - 3;
    card.edit_button_rec.y += vertical_offset;
    DrawTextRec(application_font, "Edit", card.edit_button_rec, 16.0, 1.0, false, WHITE);
    card.edit_button_rec.x -= vertical_offset - 3;
    card.edit_button_rec.y -= vertical_offset;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(800, 600, "Microscope RPG");
    MaximizeWindow();
    SetExitKey(-1);
    Defer {CloseWindow();};

    application_font = LoadFontEx("Courier For The People.ttf", 16, 0, 250);
    SetTextureFilter(application_font.texture, FILTER_BILINEAR);
    Defer {UnloadFont(application_font);};

    Project current_project = init_project("New Game");

    Player player = init_player();
    auto cards = std::vector<Card>();

    Texture grid_texture = generate_grid();

    while (!WindowShouldClose()) {
        // print(get_mouse_delta(player.camera));

        // Update Game
        switch (player.state) {
        case READONLY:
            break;
        case WRITING:
            player_write_update(player);
            break;
        case HOVERING:
            player_hover_update(player, cards);
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
        player.camera.target = lerp<Vector2>(player.camera.target, player.camera_target, 0.2);
        player.camera.zoom = lerp<float>(player.camera.zoom, player.camera_zoom_target, 0.2);
        // Tween cards
        for (auto &card: cards) {
            if (card.grabbed) continue;
            card.body_rect.x = lerp<float>(card.body_rect.x, card.lock_target.x, 0.2);
            card.body_rect.y = lerp<float>(card.body_rect.y, card.lock_target.y, 0.2);
        }

        // Cull deleted cards
        cards.erase(std::remove_if(cards.begin(), cards.end(), [] (auto &card) {return card.deleted;}), cards.end());

        /// Move subparts of cards
        for (auto& card: cards) {
            card.header_rec = {card.body_rect.x, card.body_rect.y - card.header_rec.height, card.body_rect.width - 30, card.header_rec.height};
            card.close_button_rec = {card.body_rect.x + card.body_rect.width - 30, card.body_rect.y - card.header_rec.height, 30, card.header_rec.height};
            card.edit_button_rec = {card.body_rect.x + card.body_rect.width - 50, card.body_rect.y + card.body_rect.height - 30, 50, 30};
            card.type_toggle_rec = {card.body_rect.x, card.body_rect.y + card.body_rect.height - 30, 50, 30};
        }
        BeginDrawing();
        BeginMode2D(player.camera);
        ClearBackground(RAYWHITE);
        // Draw Background Grid
        auto src = (Rectangle) {-100000, -100000, 200000, 200000};
        auto dst = src;
        DrawTexturePro(grid_texture, src, dst, (Vector2){0, 0}, 0, RAYWHITE);
        // Draw Description Lines and Project Name
        DrawLineEx((Vector2) {0, -100000}, (Vector2) {0, 100000}, 3.0, WHITE);
        DrawLineEx((Vector2) {-100000, 0}, (Vector2) {100000, 0}, 3.0, WHITE);
        DrawRectangle(0, 0, MeasureTextEx(GetFontDefault(), current_project.name.c_str(), 30, 1.0).x + 36, MeasureTextEx(GetFontDefault(), current_project.name.c_str(), 30, 1.0).y + 3, WHITE);
        DrawText(current_project.name.c_str(), 0, 0, 30, BLACK);

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
                        draw(card, player.camera, player);
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

        // Draw player cursor over everything.
        player.player_rect.x = GetMousePosition().x;
        player.player_rect.y = GetMousePosition().y;
        DrawRectangleRec(player.player_rect, BLUE);
        switch (player.state) {
        case WRITING:
            DrawText("Writing", 0, 0, 16, BLACK);
            break;
        case HOVERING:
            DrawText("Hovering", 0, 0, 16, BLACK);
            break;
        case GRABBING:
            DrawText("Grabbing", 0, 0, 16, BLACK);
            break;
        }
        EndDrawing();
    }

    return 0;
}

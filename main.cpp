#include <raylib.h>
#include <stdio.h>
#ifdef __linux__
#include <signal.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "common.hpp"

#define GRIDSIZE 16

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

Vector2 floor(Vector2 vector) {
    return (Vector2) {(float) floor(vector.x), (float) floor(vector.y)};
}

std::vector<char> to_c_str(std::string string) {
    auto the_string = std::vector<char>();
    for (auto &character: string) {
        the_string.push_back((char) character);
    }
    the_string.push_back((char) '\0');
    return the_string;
}

bool operator==(Color lh, Color rh) {
    return lh.r == rh.r && lh.g == rh.g && lh.b == rh.b && lh.a == rh.a;
}

Vector2 operator-(Vector2 v1, Vector2 v2) {
    Vector2 result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

Vector2 operator+(Vector2 v1, Vector2 v2) {
    Vector2 result = { v1.x + v2.x, v1.y + v2.y };
    return result;
}

Vector2 operator/(Vector2 v1, Vector2 v2) {
    Vector2 result = { v1.x / v2.x, v1.y / v2.y };
    return result;
}

Vector2 operator*(Vector2 v, float f) {
    return (Vector2) {v.x * f, v.y * f};
}

template<typename T>
T center(T large, T small) {
    return (large / small) / 2.0;
}

bool collide(Rectangle rect1, Rectangle rect2) {
    return CheckCollisionRecs(rect1, rect2);
}

void print(bool the_bool) {
    std::cout << (the_bool ? "true" : "false") << std::endl;
}

void print(Vector2 vector) {
    std::cout << "(" << vector.x << " " << vector.y << ")" << std::endl;
}

void print(int n) {
    std::cout << n << std::endl;
}

void print(float n) {
    std::cout << n << std::endl;
}

void print(Rectangle rect) {
    std::cout << "(" << rect.x << " " << rect.y << " " << rect.width << " " << rect.height << ")" << std::endl;
}

Vector2 get_world_mouse_position(Camera2D camera) {
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

Vector2 previous_mouse_position = {0};

Vector2 get_mouse_delta() {
    Vector2 vec = GetMousePosition() - previous_mouse_position;
    return vec;
}

Vector2 lock_position_to_grid(Vector2 position) {
    return (Vector2) {
        (float) round(position.x / (float) GRIDSIZE) * (float) GRIDSIZE,
        (float) round(position.y / (float) GRIDSIZE) * (float) GRIDSIZE
    };
}

#define FONTSIZE_SMALL 24
#define FONTSIZE_REGULAR 32 
#define FONTSIZE_LARGE 48

Font application_font_small;
Font application_font_regular;
Font application_font_large;

inline int get_font_size(Font *font) {
    if (font == &application_font_small) {
        return FONTSIZE_SMALL;
    } else if (font == &application_font_regular) {
        return FONTSIZE_REGULAR;
    } else {
        return FONTSIZE_LARGE;
    }
}

struct Button {
    Rectangle rect;
    bool hover;
    std::string text;
    Texture texture;
    Font *font;
};

Button init_button(Rectangle button_rect, std::string button_text = "", Texture texture = {0}) {
    Button button;
    button.rect = button_rect;
    button.hover = false;
    button.text = button_text;
    button.texture = texture;
    button.font = &application_font_regular;
    return button;
}

void update_button_hover(Button& button, Vector2 position) {
    button.hover = CheckCollisionPointRec(position, button.rect);
}

void draw(Button &button, Color depressed = GRAY, Color pressed = PURPLE) {
    DrawRectangleRec(button.rect, button.hover ? pressed : depressed);
}

struct Project {
    std::string big_picture;
    std::string last_focus_text;
    Button focus;
};

void update_project(Project& project) {
    // Update focus
    auto focus_text = to_c_str(project.focus.text);
    auto focus_width = MeasureTextEx(application_font_regular, focus_text.data(), 30, 1.0).x;
    project.focus.rect = {(float) (GetScreenWidth() / 2.0 - (float) (focus_width / 2.0)) - 16, 0, focus_width + 32, 30};
}

Project init_project(std::string project_name) {
    Project project;
    project.big_picture = project_name;

    update_project(project); // sets focus_rect
    project.focus.text = "No Focus Set";

    project.last_focus_text = "No Focus Set";
    return project;
}

void draw(Project& project) {
    auto focus_text = to_c_str(project.focus.text);
    auto focus_width = MeasureTextEx(application_font_regular, focus_text.data(), 30, 1.0).x;
    draw(project.focus);
    DrawTextEx(application_font_regular, focus_text.data(), {(float (GetScreenWidth() / 2.0 - (float) (focus_width / 2.0))), 0}, 30, 1.0, WHITE);
}

enum PaletteType {
    YES,
    NO,
};


struct Palette {
    bool open;
    std::vector<std::string> yes;
    std::vector<std::string> no;

    Rectangle palette_body_rec;

    Button open_button;
    Button yes_button;
    Button no_button;
};

void update_palette(Palette& palette) {
    if (!palette.open) {
        palette.open_button.rect = {(float) (GetScreenWidth() - 32.0), (float) (GetScreenHeight() / 2.0 - 16.0), 32, 32};
        return;
    }
    palette.open_button.rect = {(float) (GetScreenWidth() * 0.667 - 32.0), (float) (GetScreenHeight() / 2.0 - 16.0), 32, 32};
    palette.palette_body_rec = {(float) (GetScreenWidth() * 0.667), 0, (float) (GetScreenWidth() - 16.0), (float) (GetScreenHeight() - 16.0)};

    palette.yes_button.rect = {palette.palette_body_rec.x + 8, palette.palette_body_rec.y + (palette.yes.size() + 2) * MeasureTextEx(GetFontDefault(), "Yes", 30, 1.0).y, 30, 30};
    palette.no_button.rect  = {palette.palette_body_rec.x + 8, palette.palette_body_rec.y + (palette.yes.size() + palette.no.size() + 4) * MeasureTextEx(GetFontDefault(), "Yes", 30, 1.0).y, 30, 30};

    auto mouse_position = GetMousePosition();
    update_button_hover(palette.yes_button, mouse_position);
    update_button_hover(palette.no_button, mouse_position);
}

Palette init_palette() {
    Palette palette;
    palette.open = false;
    palette.yes = std::vector<std::string>();
    palette.no = std::vector<std::string>();

    palette.open_button.hover = false;
    palette.palette_body_rec = {0};
    // Run a single update step to initialize button rects
    update_palette(palette);
    return palette;
}

void toggle_palette(Palette& palette) {
    palette.open = !palette.open;
}

void add_palette_slot(Palette& palette, PaletteType type) {
    if (type == YES) {
        palette.yes.push_back(" ");
    } else {
        palette.no.push_back(" ");
    }
}

enum CardType {
    PERIOD,
    EVENT,
    SCENE,
    LEGACY,
};

enum Tone {
    LIGHT,
    DARK
};


enum FontSize {
    SMALL,
    REGULAR,
    LARGE
};

struct Card {
    std::string name;
    std::string content;
    std::string last_name;
    std::string last_content;
    FontSize fontsize;
    Font *font;
    CardType type;
    Tone tone;
    bool deleted;
    bool grabbed;
    bool selected;

    Rectangle body_rect;
    Vector2 lock_target;
    Color color;

    Rectangle header_rec;

    Button close_button;
    Button edit_button;
    Button tone_button;
    Button increase_font_button;
    Button decrease_font_button;

    int depth;
    bool drawn;
};

Card init_card(std::string name, Rectangle body_rect, CardType type = PERIOD) {
    float header_height = 20.0;
    Card card;
    card.name = name;
    card.content = "";
    card.last_name = name;
    card.last_content = "";
    card.fontsize = REGULAR;
    card.font = &application_font_regular;
    card.type = type;
    card.tone = LIGHT;
    card.deleted  = false;
    card.grabbed  = false;
    card.selected = false;
    card.body_rect = body_rect;
    card.lock_target = {body_rect.x, body_rect.y};
    card.color = WHITE;
    card.header_rec      = {card.body_rect.x, card.body_rect.y - header_height, card.body_rect.width - 30, header_height};
    card.close_button    = init_button({card.body_rect.x + card.body_rect.width - 30, card.body_rect.y - header_height, 30, header_height});
    card.edit_button = init_button({card.body_rect.x + card.body_rect.width - 50, card.body_rect.y + card.body_rect.height - 30, 50, 30});
    card.tone_button = init_button({card.body_rect.x, card.body_rect.height - 30, 50, 30});
    card.increase_font_button = init_button({card.body_rect.x + 10 + 50, card.body_rect.y - 30, 50, 30});
    card.decrease_font_button = init_button({card.body_rect.x + 10 + 100, card.body_rect.y - 30, 50, 30});

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

void update_cards(std::vector<Card>& cards) {
    cards.erase(std::remove_if(cards.begin(), cards.end(), [] (auto &card) {return card.deleted;}), cards.end());
    // Tween cards
    for (auto &card : cards) {
        if (card.grabbed)
            continue;
        if (!card.selected) {
            card.body_rect.x =
                lerp<float>(card.body_rect.x, card.lock_target.x, 0.2);
            card.body_rect.y =
                lerp<float>(card.body_rect.y, card.lock_target.y, 0.2);
            auto closeness = (Vector2){card.body_rect.x, card.body_rect.y} -
                             card.lock_target;
            if (!(closeness.x < 0.02 && closeness.y < 0.02))
              continue; // Basically make sure we finish moving before we resize
                        // the card.
            Vector2 corner_coords = {card.body_rect.x + card.body_rect.width,
                                     card.body_rect.y + card.body_rect.height};
            card.body_rect.width = lerp<float>(
                card.body_rect.width,
                lock_position_to_grid(corner_coords).x - card.body_rect.x, 0.2);
            card.body_rect.height = lerp<float>(
                card.body_rect.height,
                lock_position_to_grid(corner_coords).y - card.body_rect.y, 0.2);
        } else {
          card.body_rect.x = card.lock_target.x;
          card.body_rect.y = card.lock_target.y;
        }
    }

    /// Move subparts of cards
    for (auto &card : cards) {
        card.header_rec = {card.body_rect.x,
            card.body_rect.y - card.header_rec.height,
            card.body_rect.width - 30, card.header_rec.height};
        card.close_button.rect = {card.body_rect.x + card.body_rect.width - 30,
            card.body_rect.y - card.header_rec.height, 30,
            card.header_rec.height};
        card.edit_button.rect = {card.body_rect.x + card.body_rect.width - 50,
            card.body_rect.y + card.body_rect.height - 30, 50,
            30};
        card.tone_button.rect = {card.body_rect.x,
            card.body_rect.y + card.body_rect.height - 30, 50,
            30};
        card.increase_font_button.rect =
            {card.body_rect.x + 10 + 50, card.body_rect.y + card.body_rect.height - 30, 50, 30};
        card.decrease_font_button.rect = 
            {card.body_rect.x + 10 + 100, card.body_rect.y + card.body_rect.height - 30, 50, 30};
    }
}

enum PlayerState {
    HOVERING, // Just looking, but still able to move cards around and such
    READONLY,
    WRITING,
    FOCUSWRITING,
    PALETTEWRITING,
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

Player init_player() {
    Player player;
    player.mouse_held  = false;

    auto position = GetMousePosition();
    player.player_rect = {position.x, position.y, 10, 10};
    player.mouse_position = position;
    player.camera_move_speed = 20;
    player.camera = {0};
    player.camera_target = {0, 0};
    player.camera.target = player.camera_target;
    player.zoom_level = 2;
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
    player.resizing_card = false;
    return player;
}

void spawn_card(Player player, std::vector<Card>& cards, CardType type) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    Rectangle to_draw = {position.x, position.y, GRIDSIZE * 17, GRIDSIZE * 13};
    Card the_card = init_card("New Card", to_draw, type);
    auto next_card = greatest_depth_and_furthest_along(cards);
    if (next_card) the_card.depth = next_card->depth + 1;
    else the_card.depth = 0;
    cards.push_back(the_card);
}

void player_update_camera(Player &player, bool allow_key_scroll = true) {
    #define ZOOM_SIZE 5
    float zoom_levels[ZOOM_SIZE] = {0.5, 0.75, 1, 1.5, 2};

    /// Move Left/Right
    if (allow_key_scroll) {
        if (IsKeyDown(player.binds.move_right)) {
            player.camera_target.x += player.camera_move_speed * (1.0 / player.camera.zoom);
        } else if (IsKeyDown(player.binds.move_left)) {
            player.camera_target.x -= player.camera_move_speed * (1.0 / player.camera.zoom);
        }

        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) { // Zooming
            // Zoom In/Out
            if (IsKeyPressed(player.binds.move_up)) {
                player.zoom_level += 1;
            } else if (IsKeyPressed(player.binds.move_down)) {
                player.zoom_level -= 1;
            }
        } else {
            // Move Up/Down
            if (IsKeyDown(player.binds.move_up)) {
                player.camera_target.y -= player.camera_move_speed;
            } else if (IsKeyDown(player.binds.move_down)) {
                player.camera_target.y += player.camera_move_speed;
            }
        }
    }

    // Mouse wheel stuff
    auto wheel_move = GetMouseWheelMove();
    if (wheel_move != 0.0) {
        if (wheel_move > 0.0) {
            player.zoom_level += 1;
        } else {
            player.zoom_level -= 1;
        }
    }

    player.zoom_level = clamp<int>(player.zoom_level, 0, ZOOM_SIZE - 1);
    player.camera_zoom_target = zoom_levels[player.zoom_level];

    // Middle mouse button press movement
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
        auto mouse_move = get_mouse_delta() * (1.0 / player.camera.zoom);
        player.camera_target = player.camera_target - mouse_move;
    }

    #undef ZOOM_SIZE
}

void player_write_update(Player& player) {
    player_update_camera(player, false);
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (player.editing == NAME) {
            if (player.selected_card->name.empty()) player.selected_card->name = player.selected_card->last_name;
        }
        player.state = HOVERING;
        player.selected_card->grabbed = false;
        player.selected_card = NULL;
        return;
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (player.editing == NAME && player.selected_card->name.size() > 0) {
            player.selected_card->name.pop_back();
        }
        else if (player.editing == BODY && player.selected_card->content.size() > 0) {
            player.selected_card->content.pop_back();
        }
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

void player_resize_chosen_card(Player& player) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    Card *card = player.selected_card;
    if (!card) return;

    auto mouse_over_button = CheckCollisionPointRec(position, card->edit_button.rect);
    if (IsMouseButtonPressed(0) && mouse_over_button) {
        player.resizing_card = true;
    } else if (IsMouseButtonReleased(0)) {
        player.resizing_card = false;
    }

    if (!player.resizing_card) return;
    auto mouse_delta = get_mouse_delta() * (1.0 / player.camera.zoom);
    card->body_rect.width += mouse_delta.x;
    card->body_rect.height += mouse_delta.y;
    if (card->body_rect.width < 64) card->body_rect.width = 64;
    if (card->body_rect.height < 64) card->body_rect.height = 64;
}

// This is the main meat of the program.
void player_hover_update(Player& player, std::vector<Card>& cards, Palette& palette, Project &project) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    player.player_rect.x = position.x;
    player.player_rect.y = position.y;

    palette.open_button.hover = collide((Rectangle) {mouse_position.x, mouse_position.y, 10, 10}, palette.open_button.rect);

    // Mouse and Card Selection
    for (auto &card: cards) {
        if (palette.open_button.hover) break; // skip checking the cards if the players is hovering over the palette open thingie
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
        update_button_hover(card.close_button, position);
        update_button_hover(card.edit_button, position);
        update_button_hover(card.tone_button, position);
        update_button_hover(card.increase_font_button, position);
        update_button_hover(card.decrease_font_button, position);
    }

    if (IsMouseButtonPressed(0) && player.selected_card) {
        auto deepest_card = greatest_depth_and_furthest_along(cards);
        player.selected_card->depth = deepest_card->depth + 1;
        // Card button clicked!
        if (player.selected_card->close_button.hover) {
            player.selected_card->deleted = true;
            player.mouse_held = false;
            player.offset = {0 ,0};
        } else if (player.selected_card->edit_button.hover) {
            player.state = WRITING;
            player.editing = BODY;
            player.mouse_held = false;
            player.offset = {0 ,0};
        } else if (player.selected_card->tone_button.hover) {
            if (player.selected_card->color == WHITE) player.selected_card->color = BLACK;
            else player.selected_card->color = WHITE;
        } else if (player.selected_card->increase_font_button.hover) {
            FontSize *the_size = &player.selected_card->fontsize;
            switch (player.selected_card->fontsize) {
            case SMALL: 
                print(200);
                *the_size = REGULAR;
                player.selected_card->font = &application_font_regular;
                break;
            case REGULAR: 
                *the_size = LARGE;
                player.selected_card->font = &application_font_large;
                break;
            case LARGE: 
                *the_size = SMALL;
                player.selected_card->font = &application_font_small;
                break;
            }
        } else if (player.selected_card->decrease_font_button.hover) {
            FontSize *the_size = &player.selected_card->fontsize;
            switch (player.selected_card->fontsize) {
            case SMALL: 
                *the_size = LARGE;
                player.selected_card->font = &application_font_large;
                break;
            case REGULAR: 
                *the_size = SMALL;
                player.selected_card->font = &application_font_small;
                break;
            case LARGE: 
                *the_size = REGULAR;
                player.selected_card->font = &application_font_regular;
                break;
            }
        }
    } else if (IsMouseButtonPressed(0)) { // Player clicks, but is not on a card
        if (palette.open_button.hover) {
            toggle_palette(palette);
            return;
        } else if (palette.yes_button.hover) {
            player.state = PALETTEWRITING;
            player.palette_edit_type = YES;
            add_palette_slot(palette, YES);
            return;
        } else if (palette.no_button.hover) {
            player.state = PALETTEWRITING;
            player.palette_edit_type = NO;
            add_palette_slot(palette, NO);
            return;
        } else if (project.focus.hover) { // Focus clicked
            player.state = FOCUSWRITING;
            project.last_focus_text = project.focus.text;
            project.focus.text = "";
            return;
        } else {
            player.state = GRABBING; // Background drag
            player.hold_origin = GetMousePosition();
            return;
        }
    } else if (IsMouseButtonReleased(0) && player.selected_card) { // Player releases a card
        auto position_to_lock_to = lock_position_to_grid((Vector2) {player.selected_card->body_rect.x, player.selected_card->body_rect.y});
        player.selected_card->lock_target = position_to_lock_to;
        player.selected_card->grabbed = false;
        for (auto& card: cards) {
            if (!card.selected) continue;
            position_to_lock_to = lock_position_to_grid((Vector2) {card.body_rect.x, card.body_rect.y});
            card.lock_target = position_to_lock_to;
            card.selected = false;
        }

        player.mouse_held = false;
        player.offset = {0, 0};
        player.selected_card = NULL;
    } 

    // Move grabbed and selected cards.
    if (player.mouse_held && player.selected_card) {
        player.selected_card->body_rect.x = position.x - player.offset.x;
        player.selected_card->body_rect.y = position.y - player.offset.y;
        for (auto& card: cards) {
            if (card.selected) {
                auto mouse_delta = get_mouse_delta() * (1.0/player.camera.zoom);
                card.lock_target.x += mouse_delta.x;
                card.lock_target.y += mouse_delta.y;
            }
        }
    }

    // Key Processing
    player_update_camera(player);

    /// Spawn card
    if (IsKeyPressed(KEY_ONE)) {
        spawn_card(player, cards, PERIOD);
    } else if (IsKeyPressed(KEY_TWO)) {
        spawn_card(player, cards, EVENT);
    } else if (IsKeyPressed(KEY_THREE)) {
        spawn_card(player, cards, SCENE);
    } else if (IsKeyPressed(KEY_FOUR)) {
        spawn_card(player, cards, LEGACY);
    }

    // Delete cards
    if (IsKeyPressed(KEY_DELETE)) {
        for (auto &card: cards) {
            card.deleted = card.selected;
        }
    }
}

void player_grabbing_update(Player& player, std::vector<Card>& cards) {
    if (IsMouseButtonReleased(0)) {
        player.hold_diff = {0, 0};
        player.selection_rec = {0};
        player.state = HOVERING;
        return;
    }
    player.hold_diff = player.hold_diff + get_mouse_delta();
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
    auto world_size = (Vector2) {player.selection_rec.width, player.selection_rec.height} * (1.0/player.camera.zoom);
    Rectangle selected_world_rect = {world_coords.x, world_coords.y, world_size.x, world_size.y};
    for (auto &card: cards) {
        card.selected = collide(card.body_rect, selected_world_rect);
    }
}

void player_write_palette_update(Player& player, Palette &palette) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        player.state = HOVERING;
        return;
    }
    std::string& last_item = player.palette_edit_type == YES ? palette.yes.back() : palette.no.back();
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (last_item.size() > 1) last_item.pop_back();
        return;
    }
    auto char_pressed = GetCharPressed();
    if (char_pressed != 0) {
        last_item += (char) char_pressed;
    }
}

void player_write_focus_update(Player& player, Project& project) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (project.focus.text.empty()) project.focus.text = project.last_focus_text;
        player.state = HOVERING;
        return;
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (project.focus.text.size() > 0) project.focus.text.pop_back();
        return;
    }
    auto char_pressed = GetCharPressed();
    if (char_pressed != 0) project.focus.text += (char) char_pressed;
    return;
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


void draw(Card &card, Camera2D camera, Player player) {
    Defer {card.drawn = true;};

    auto position = GetScreenToWorld2D(GetMousePosition(), camera);
    // Draw Header
    DrawRectangleRec(card.header_rec, GRAY);
    /// Draw close button
    DrawRectangleRec(card.close_button.rect, RED);

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
    DrawTextRec(*card.font, content.data(), card.body_rect, get_font_size(card.font), 1.0, true, card.color == BLACK ? WHITE : BLACK);
    card.body_rect.x -= 5;
    card.body_rect.y -= 5;
    card.body_rect.width += 5;
    card.body_rect.height += 5;

    // Draw Toggle Button
    if (card.tone_button.hover) {
        DrawRectangleRec(card.tone_button.rect, PURPLE);
    } else {
        DrawRectangleRec(card.tone_button.rect, GRAY);
    }

    // Draw Edit Button
    if (card.edit_button.hover) {
        DrawRectangleRec(card.edit_button.rect, PURPLE);
    } else {
        DrawRectangleRec(card.edit_button.rect, GRAY);
    }

    // Draw Card text
    float vertical_offset = (card.edit_button.rect.height - MeasureTextEx(application_font_small, "Edit", FONTSIZE_SMALL, 1.0).y) / 2.0;
    float horizontal_offset = (card.edit_button.rect.width - MeasureTextEx(application_font_small, "Edit", FONTSIZE_SMALL, 1.0).x) / 2.0;
    card.edit_button.rect.x += vertical_offset - 3;
    card.edit_button.rect.y += vertical_offset;
    DrawTextRec(application_font_small, "Edit", card.edit_button.rect, FONTSIZE_SMALL, 1.0, false, WHITE);
    card.edit_button.rect.x -= vertical_offset - 3;
    card.edit_button.rect.y -= vertical_offset;

    // Draw Font increase buttons
    draw(card.increase_font_button, BLUE);
    draw(card.decrease_font_button, BROWN);
}

void draw(Palette& palette) {
    // draw Palette open button
    DrawRectangleRec(palette.open_button.rect, palette.open_button.hover ? PURPLE : GRAY);
    if (!palette.open) return;
    // Draw Palette Body
    DrawRectangleRec(palette.palette_body_rec, GRAY);

    DrawText("Palette", palette.palette_body_rec.x, palette.palette_body_rec.y, 30, WHITE);
    DrawText("Yes", palette.palette_body_rec.x, palette.palette_body_rec.y + MeasureTextEx(GetFontDefault(), "Yes", 30, 1.0).y, 30, WHITE);
    int text_index = 2;

    // HACK: We're doing a bit of update logic within this draw call where we poll the mouse location because it would be a bit unwieldy to split
    // that up between update/draw (something with a vector of rec/hover variables)
    // Incomplete: Need to add implimentation of palette buttons
    auto mouse_position = GetMousePosition();

    for (auto& text: palette.yes) {
        auto text_as_cstr = to_c_str(text);
        auto text_width = MeasureTextEx(GetFontDefault(), text_as_cstr.data(), 30, 1.0).x;
        DrawText(text_as_cstr.data(), palette.palette_body_rec.x + 8, palette.palette_body_rec.y + text_index * MeasureTextEx(GetFontDefault(), "Yes", 30, 1.0).y, 30, WHITE);

        Rectangle button_rect = {palette.palette_body_rec.x + text_width + 32, palette.palette_body_rec.y + text_index * MeasureTextEx(GetFontDefault(), "Yes", 30, 1.0).y, 30, 30};
        bool button_hover = CheckCollisionPointRec(mouse_position, button_rect);
        DrawRectangleRec(button_rect, button_hover ? PURPLE : RED);
        if (IsMouseButtonPressed(0) && button_hover) {
            auto position = std::find(palette.yes.begin(), palette.yes.end(), text);
            if (position != palette.yes.end()) palette.yes.erase(position);
        }

        text_index += 1;
    }

    // Draw Add button
    draw(palette.yes_button, GREEN, PURPLE);

    DrawText("No", palette.palette_body_rec.x, palette.palette_body_rec.y + (palette.yes.size() + 3) * MeasureTextEx(GetFontDefault(), "Yes", 30, 1.0).y, 30, WHITE);
    text_index += 2;
    for (auto& text: palette.no) {
        auto text_as_cstr = to_c_str(text);
        auto text_width = MeasureTextEx(GetFontDefault(), text_as_cstr.data(), 30, 1.0).x;
        DrawText(text_as_cstr.data(), palette.palette_body_rec.x + 8, palette.palette_body_rec.y + text_index * MeasureTextEx(GetFontDefault(), "Yes", 30, 1.0).y, 30, WHITE);

        Rectangle button_rect = {palette.palette_body_rec.x + text_width + 32, palette.palette_body_rec.y + text_index * MeasureTextEx(GetFontDefault(), "Yes", 30, 1.0).y, 30, 30};
        bool button_hover = CheckCollisionPointRec(mouse_position, button_rect);
        DrawRectangleRec(button_rect, button_hover ? PURPLE : RED);
        if (IsMouseButtonPressed(0) && button_hover) {
            auto position = std::find(palette.no.begin(), palette.no.end(), text);
            if (position != palette.no.end()) palette.no.erase(position);
        }

        text_index += 1;
    }
    draw(palette.no_button, GREEN, PURPLE);

}

volatile int signal_handler = 1;
void signal_func(int dummy) {
    signal_handler = 0;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(800, 600, "Microscope RPG");
    MaximizeWindow();
    SetExitKey(-1);
    Defer {CloseWindow();};

    SetWindowIcon(LoadImage("assets/logo.png"));

    application_font_small  = LoadFontEx("monogram_extended.ttf", FONTSIZE_SMALL, NULL, 250);
    application_font_regular = LoadFontEx("monogram_extended.ttf", FONTSIZE_REGULAR, NULL, 250);
    application_font_large  = LoadFontEx("monogram_extended.ttf", FONTSIZE_LARGE, NULL, 250);
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
        // Draw Description Lines and Project Name
        DrawLineEx((Vector2) {0, -100000}, (Vector2) {0, 100000}, 3.0, WHITE);
        DrawLineEx((Vector2) {-100000, 0}, (Vector2) {100000, 0}, 3.0, WHITE);
        DrawRectangle(1, 1, MeasureTextEx(application_font_regular, current_project.big_picture.c_str(), 30, 1.0).x + 16, MeasureTextEx(application_font_regular, current_project.big_picture.c_str(), 30, 1.0).y, WHITE);
        DrawTextEx(application_font_regular, current_project.big_picture.c_str(), {8, -1}, 30, 1.0, BLACK);

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

#include <cstring>
#include <stack>
#include "main_menu.hpp"
#include "common.hpp"
#include "card.hpp"
#include "tinyfiledialogs.h"


#define MENU_MARGIN 700
#define MENU_MARGIN_MIN 650
#define MENU_COLOR (Color) {173, 229, 235, 255}

MainMenu init_menu(bool visible) {
    MainMenu menu;
    menu.visible = visible;
    menu.body_rect = {0};
    menu.start_game  = init_button();
    menu.settings    = init_button();
    menu.load_game   = init_button();
    menu.close_button = init_button();
    return menu;
}

void update_menu(MainMenu& menu, Vector2 mouse_position, bool& new_game, bool& opened_save_file, char *opened_file) {
    menu.body_rect = {0, 0, GetScreenWidth() - MENU_MARGIN, GetScreenHeight() - MENU_MARGIN / 2};

    menu.body_rect.width = menu.body_rect.width > MENU_MARGIN_MIN ? menu.body_rect.width : MENU_MARGIN_MIN;
    menu.body_rect.height = menu.body_rect.height > MENU_MARGIN_MIN ? menu.body_rect.height : MENU_MARGIN_MIN;
    menu.body_rect.height /= 3;
    menu.body_rect.x = (GetScreenWidth() - menu.body_rect.width) / 2.0;
    menu.body_rect.y = (GetScreenHeight() - menu.body_rect.height) / 2.0;

    menu.close_button.rect = {menu.body_rect.width - 33 / 2, -(12 * 3) / 2, 11 * 3, 12 * 3};

    menu.start_game.rect = {0, 0, 68 * 3, 29 * 3};
    menu.load_game.rect  = {68 * 3 + 10, 0, 68 * 3, 29 * 3};
    menu.settings.rect   = {(68 * 3) * 2 + 20, 0, 68 * 3, 29 * 3};

    if (IsKeyPressed(KEY_ESCAPE)) menu.visible = false;
    if (IsMouseButtonPressed(0)) {
        if (menu.start_game.hover) {
            menu.visible = false;
            new_game = true;
            return;
        } else if (menu.load_game.hover) {
            char *result = NULL;
            result = tinyfd_openFileDialog("Open save file",
                                                NULL,
                                                0,
                                                NULL,
                                                NULL,
                                                0);
            if (result != NULL) strcpy(opened_file, result);
            opened_save_file = true;
            return;
        } else if (menu.settings.hover) {
            return;
        } else if (menu.close_button.hover) {
            menu.visible = false;
        }
    }
    return;
}

void DrawRectangle(const Vector2 transform, int x, int y, int w, int h, Color color) {
    DrawRectangle(x + transform.x, y + transform.y, w, h, color);
}

void DrawRectangle(const std::vector<Vector2>& stack, int x, int y, int w, int h, Color color) {
    Vector2 transform = {0};
    for (auto &vector: stack) {
        transform = transform + vector;
    }
    DrawRectangle(transform, x, y, w, h, color);
}

void DrawRectangleRec(const std::vector<Vector2>& stack, const Rectangle& rec, Color color) {
    Vector2 transform = {0};
    for (auto &vector: stack) {
        transform = transform + vector;
    }
    DrawRectangle(transform, rec.x, rec.y, rec.width, rec.height, color);
}

void DrawTextEx(const Vector2 transform, Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint) {
    DrawTextEx(font, text, position + transform, fontSize, spacing, tint);
}

void DrawTextEx(const std::vector<Vector2>& stack, Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint) {
    Vector2 transform = {0};
    for (auto &vector: stack) {
        transform = transform + vector;
    }
    DrawTextEx(transform, font, text, position, fontSize, spacing, tint);
}
void draw_card_body(const std::vector<Vector2>& stack, Rectangle rect, bool light) {
    Vector2 transform = {0};
    for (auto &vector: stack) {
        transform = transform + vector;
    }
    draw_card_body(rect + transform, light);
}

Vector2 get_transform (std::vector<Vector2> transform) {
    Vector2 returned = {0};
    for (const auto& vec: transform) {
        returned = returned + vec;
    }
    return returned;
}

void draw_menu(MainMenu& menu) {
    auto transform_stack = std::vector<Vector2>();
    DrawRectangleRec(menu.body_rect, MENU_COLOR);
    DrawRectangleLinesEx(menu.body_rect, 3.0, BLACK);

    transform_stack.push_back(to_vector(menu.body_rect));
    auto text_width = MeasureTextEx(application_font_regular, "Welcome to LILLIPUT!", 16 *4, 1.0).x;
    DrawTextEx(transform_stack, application_font_regular, "Welcome to LILLIPUT!", {(menu.body_rect.width - text_width) / 2.0, 0}, 16 * 4, 1.0, BLACK);
    draw_texture_rect_scaled(spritesheet, {32, 0, 11, 12}, to_vector(menu.close_button.rect) + get_transform(transform_stack));

    update_button_hover(transform_stack, menu.close_button, GetMousePosition());

    transform_stack.push_back({5, 16 * 4 + 30});
    auto width = (68 * 3 + 10 + (68 * 3) * 2 + 20);

    transform_stack.push_back({(menu.body_rect.width - width) / 2.0, 0});
    update_button_hover(transform_stack, menu.start_game, GetMousePosition());
    update_button_hover(transform_stack, menu.settings, GetMousePosition());
    update_button_hover(transform_stack, menu.load_game, GetMousePosition());

    if (menu.start_game.hover || menu.load_game.hover || menu.settings.hover) set_darkness_shader_amount(1.1);

    if (menu.start_game.hover) BeginShaderMode(darken_shader);
    draw_card_body(transform_stack, menu.start_game.rect, true);
    menu.start_game.rect.x += 12;
    menu.start_game.rect.y += 12;
    draw_texture_rect_scaled(spritesheet, {16, 57, 28, 21}, to_vector(menu.start_game.rect) + get_transform(transform_stack));
    draw_texture_rect_scaled(spritesheet, {0, 80, 16, 16}, to_vector(menu.start_game.rect) + get_transform(transform_stack) + (Vector2) {40 * 3, 9});
    if (menu.start_game.hover) EndShaderMode();

    if (menu.load_game.hover) BeginShaderMode(darken_shader);
    draw_card_body(transform_stack, menu.load_game.rect, true);
    menu.load_game.rect.x += 12;
    menu.load_game.rect.y += 12;
    draw_texture_rect_scaled(spritesheet, {44, 57, 28, 21}, to_vector(menu.load_game.rect) + get_transform(transform_stack));
    draw_texture_rect_scaled(spritesheet, {16, 80, 16, 16}, to_vector(menu.load_game.rect) + get_transform(transform_stack) + (Vector2) {40 * 3, 9});
    if (menu.load_game.hover) EndShaderMode();

    if (menu.settings.hover) BeginShaderMode(darken_shader);
    draw_card_body(transform_stack, menu.settings.rect, true);
    menu.settings.rect.x += 12;
    menu.settings.rect.y += 12;
    draw_texture_rect_scaled(spritesheet, {72, 57, 46, 15}, to_vector(menu.settings.rect) + get_transform(transform_stack));
    if (menu.settings.hover) EndShaderMode();

    if (menu.start_game.hover || menu.load_game.hover || menu.settings.hover) set_darkness_shader_amount();
}

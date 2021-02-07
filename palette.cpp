#include "palette.hpp"
#define PALETTECOLOR (Color) {104, 136, 152, 255}
#define PALETTEOUTLINE (Color) {75, 72, 101, 255}

void update_palette(Palette& palette) {
    if (!palette.open) {
        palette.open_button.rect = {(float) (GetScreenWidth() - 32.0) + 2, (float) (GetScreenHeight() / 2.0 - 150.0), 32, 150};
        return;
    }
    palette.open_button.rect = {(float) (GetScreenWidth() * 0.667 - 32.0) + 2, (float) (GetScreenHeight() / 2.0 - 150.0), 32, 150};
    palette.palette_body_rec = {(float) (GetScreenWidth() * 0.667), 0, (float) (GetScreenWidth() - 16.0), (float) (GetScreenHeight() - 16.0)};

    palette.yes_button.rect = {palette.palette_body_rec.x + 45, palette.palette_body_rec.y + MeasureTextEx(application_font_regular, "Yes", 30, 1.0).y, 30, 30};
    palette.no_button.rect  = {palette.palette_body_rec.x + 30, palette.palette_body_rec.y + (2 + 2 * palette.yes.size()) * (14 * 2) + 15, 30, 30};

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

void draw_palette_text(bool on, std::string text, Vector2 where) {
    Rectangle rect_part_1 = {43, 30, 3, 14};
    Rectangle rect_part_2 = {45, 30, 1, 14};
    Rectangle rect_part_3 = {46, 30, 3, 14};
    if (on) {
        rect_part_1 = {49, 30, 3, 14};
        rect_part_2 = {51, 30, 1, 14};
        rect_part_3 = {52, 30, 3, 14};
    }
    auto text_width = MeasureTextEx(application_font_regular, text.c_str(), FONTSIZE_REGULAR, 1.0).x;
    draw_texture_rect_scaled(spritesheet, rect_part_1, where);
    draw_texture_rect_scaled(spritesheet, rect_part_2, {where.x + 9, where.y}, {text_width / 3 - 2, 0});
    draw_texture_rect_scaled(spritesheet, rect_part_3, {where.x + text_width + 6, where.y});
    DrawTextEx(application_font_regular, text.c_str(), {floor(where.x), floor(where.y + 1)}, FONTSIZE_REGULAR, 1.0, on ? BLACK : WHITE);
}

void draw(Palette& palette) {
    // draw Palette open button
    //DrawRectangleRec(palette.open_button.rect, palette.open_button.hover ? PURPLE : GRAY);
    // HACK: Drawing this twice, so it's always on top
    draw_texture_rect_scaled(spritesheet, {109, 16, 6, 30}, {palette.open_button.rect.x, palette.open_button.rect.y - 0.01}, {0}, 5);
    if (!palette.open) return;
    // Draw Palette Body
    DrawRectangleLinesEx({palette.palette_body_rec.x - 5, palette.palette_body_rec.y, palette.palette_body_rec.width, palette.palette_body_rec.height + 5}, 15, PALETTEOUTLINE);
    DrawRectangleRec(palette.palette_body_rec, PALETTECOLOR);
    draw_texture_rect_scaled(spritesheet, {109, 16, 6, 30}, {palette.open_button.rect.x, palette.open_button.rect.y - 0.01}, {0}, 5);

    DrawTextEx(application_font_regular, "Palette", {palette.palette_body_rec.x, palette.palette_body_rec.y}, 30, 0.0, WHITE);
    DrawTextEx(application_font_regular, "Yes", {palette.palette_body_rec.x, palette.palette_body_rec.y + MeasureTextEx(application_font_regular, "Yes", 30, 1.0).y}, 30, 0.0, WHITE);
    int text_index = 2;

    // HACK: We're doing a bit of update logic within this draw call where we poll the mouse location because it would be a bit unwieldy to split
    // that up between update/draw (something with a vector of rec/hover variables)
    // Incomplete: Need to add implimentation of palette buttons
    auto mouse_position = GetMousePosition();

    for (auto& text: palette.yes) {
        auto text_as_cstr = to_c_str(text);
        auto text_width = MeasureTextEx(GetFontDefault(), text_as_cstr.data(), 30, 1.0).x;
        draw_palette_text(true, text, {palette.palette_body_rec.x + (18 * 3), palette.palette_body_rec.y + text_index * (14 * 2) + 12});

        const Rectangle button_rect = {palette.palette_body_rec.x + 15, 6 + palette.palette_body_rec.y + text_index * (14 * 2) + 12, 30, 30};
        bool button_hover = CheckCollisionPointRec(mouse_position, button_rect);
        // DrawRectangleRec(button_rect, button_hover ? PURPLE : RED);
        draw_texture_rect_scaled(spritesheet, {59, 27, 9, 10}, {button_rect.x, button_rect.y});
        if (IsMouseButtonPressed(0) && button_hover) {
            auto position = std::find(palette.yes.begin(), palette.yes.end(), text);
            if (position != palette.yes.end()) palette.yes.erase(position);
        }

        text_index += 2;
    }

    // Draw Add button
    //draw(palette.yes_button, GREEN, PURPLE);
    draw_texture_rect_scaled(spritesheet, {68, 27, 9, 10}, {palette.yes_button.rect.x, palette.yes_button.rect.y});

    DrawTextEx(application_font_regular, "No", {palette.palette_body_rec.x, palette.palette_body_rec.y + (2 + 2 * palette.yes.size()) * (14 * 2) + 15}, 30, 0.0, WHITE);
    text_index += 2;
    for (auto& text: palette.no) {
        auto text_as_cstr = to_c_str(text);
        auto text_width = MeasureTextEx(GetFontDefault(), text_as_cstr.data(), 30, 1.0).x;
        draw_palette_text(true, text, {palette.palette_body_rec.x + (18 * 3), palette.palette_body_rec.y + text_index * (14 * 2)});

        const Rectangle button_rect = {palette.palette_body_rec.x + 15, 6 + palette.palette_body_rec.y + text_index * (14 * 2), 30, 30};
        bool button_hover = CheckCollisionPointRec(mouse_position, button_rect);
        //DrawRectangleRec(button_rect, button_hover ? PURPLE : RED);
        draw_texture_rect_scaled(spritesheet, {59, 27, 9, 10}, {button_rect.x, button_rect.y});
        if (IsMouseButtonPressed(0) && button_hover) {
            auto position = std::find(palette.no.begin(), palette.no.end(), text);
            if (position != palette.no.end()) palette.no.erase(position);
        }

        text_index += 2;
    }
    //draw(palette.no_button, GREEN, PURPLE);
    draw_texture_rect_scaled(spritesheet, {68, 27, 9, 10}, {palette.no_button.rect.x, palette.no_button.rect.y});
}

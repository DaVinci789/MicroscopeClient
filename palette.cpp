#include "palette.hpp"

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

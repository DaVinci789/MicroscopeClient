#pragma once
#include "common.hpp"
#include <raylib.h>
#include <vector>
#include <string>

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

Palette init_palette();
void update_palette(Palette& palette);
void toggle_palette(Palette& palette);
void add_palette_slot(Palette& palette, PaletteType type);
void draw(Palette& palette);

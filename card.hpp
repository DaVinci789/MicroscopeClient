#pragma once
#include "common.hpp"

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
    Texture2D *textures;
    FontSize fontsize;
    Font *font;
    CardType type;
    Tone tone;
    bool deleted;
    bool grabbed;
    bool selected;
    bool hover;

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

Card init_card(std::string name, Rectangle body_rect, CardType type = PERIOD);
Card* greatest_depth_and_furthest_along(std::vector<Card>& cards);
void update_cards(std::vector<Card>& cards);
void draw(Card &card, Camera2D camera);

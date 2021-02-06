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
    std::vector<Card> cards_under;
    Card *parent;
    std::string id;
    int depth;

    Texture2D *textures;
    FontSize fontsize;
    Font *font;
    CardType type;
    Tone tone;
    bool deleted;
    bool grabbed;
    bool selected;
    bool hover;
    bool draw_resize;

    bool drawn;
    bool in_drawer;
    bool is_beginning;
    bool is_end;

    Rectangle body_rect;
    Vector2 lock_target;
    Vector2 saved_dimensions;

    Rectangle header_rec;

    Button close_button;
    Button edit_button;
    Button tone_button;
    Button increase_font_button;
    Button decrease_font_button;
    Button scene_insert_button;
    Button scene_remove_button;
    Button move_up_button;
    Button move_down_button;
    Button remove_from_drawer_button;
};

bool operator==(Card c1, Card c2);
Card init_card(std::string name, Rectangle body_rect, CardType type = PERIOD);
Card* greatest_depth_and_furthest_along(std::vector<Card>& cards);
void update_cards(std::vector<Card>& cards);
void draw_card_body(float x, float y, float width, float height, bool light);
void draw_card_body(Rectangle rect, bool light);
void draw_card_ui(Card &card, Camera2D camera);
void draw(Card &card, Camera2D camera);
void draw_resize_corner(const Card& card);
int smallest_depth(const std::vector<Card>& cards);

#include "card.hpp"
#include "common.hpp"
#include <random>

std::string get_uuid() {
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    std::string res;
    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}


Card init_card(std::string name, Rectangle body_rect, CardType type) {
    float header_height = 20.0;
    Card card;
    card.id = get_uuid();
    card.name = name;
    card.content = "";
    card.last_name = name;
    card.last_content = "";
    card.cards_under = std::vector<Card>();
    card.parent = NULL;
    card.textures = &spritesheet;
    card.fontsize = REGULAR;
    card.font = &application_font_regular;
    card.type = type;
    card.tone = LIGHT;
    card.deleted  = false;
    card.grabbed  = false;
    card.selected = false;
    card.hover = false;
    card.draw_resize = false;
    card.in_drawer = false;
    card.is_beginning = false;
    card.is_end = false;
    card.body_rect = body_rect;
    card.lock_target = {body_rect.x, body_rect.y};
    card.saved_dimensions = {0};
    card.header_rec   = {card.body_rect.x, card.body_rect.y - header_height, card.body_rect.width - 30, header_height};
    card.close_button = init_button();
    card.edit_button  = init_button();
    card.tone_button  = init_button();
    card.increase_font_button = init_button();
    card.decrease_font_button = init_button();
    card.scene_insert_button = init_button();
    card.scene_remove_button = init_button();
    card.move_up_button = init_button();
    card.move_down_button = init_button();
    card.remove_from_drawer_button = init_button();

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

int smallest_depth(const std::vector<Card>& cards) {
    int depth = 9999;
    for (auto &card: cards) {
        if (card.depth < depth) depth = card.depth;
    }
    return depth;
}

bool operator==(Card c1, Card c2) {
    return c1.id == c2.id;
}

void update_cards(std::vector<Card>& cards) {
    cards.erase(std::remove_if(cards.begin(), cards.end(), [] (const auto &card) {return card.deleted;}), cards.end());
    // Tween cards
    for (auto &card : cards) {
        if (card.parent) {
            // TODO: TWWWWWEEEN
            card.body_rect.x = card.parent->body_rect.x;
            card.body_rect.y = card.parent->body_rect.y;
            card.body_rect.width = 5;
            card.body_rect.height = 5;
            continue;
        }
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
    for (auto &card: cards) {
        /// Move subparts of cards
        card.header_rec = {card.body_rect.x,
            card.body_rect.y - card.header_rec.height,
            card.body_rect.width - 30, card.header_rec.height};
        card.close_button.rect = {card.body_rect.x + card.body_rect.width - 20,
            card.body_rect.y - 12, 33, 36};
        card.edit_button.rect = {card.body_rect.x + card.body_rect.width - 50,
            card.body_rect.y + card.body_rect.height - 47, 50,
            30};
        card.tone_button.rect = {card.edit_button.rect.x, card.edit_button.rect.y + 30, 50, 30};
        card.increase_font_button.rect =
            {card.edit_button.rect.x - 10 - 27, card.body_rect.y + card.body_rect.height - 36, 27, 27};
        card.decrease_font_button.rect = 
            {card.edit_button.rect.x - 10 - 56, card.body_rect.y + card.body_rect.height - 36, 27, 27};
        card.scene_insert_button.rect = {card.body_rect.x + card.body_rect.width - (17 * 3) / 2 - 9, card.body_rect.y + card.body_rect.height - 13 * 3 - 100, 17 * 3, 13 * 3};
        card.scene_remove_button.rect = {card.body_rect.x + card.body_rect.width - (17 * 3) / 2 - 15, card.body_rect.y + card.body_rect.height - 13 * 3 - 50, 17 * 3, 13 * 3};
    }
}

void draw_resize_corner(const Card& card) {
    draw_texture_rect_scaled(spritesheet, {18, 16, 7, 7}, {card.body_rect.x + card.body_rect.width - 35, card.body_rect.y + card.body_rect.height - 35});
}

void draw_card_ui(Card &card, Camera2D camera) {
    // Draw close button
    /// Draw close texture
    auto close_button_position = (Vector2) {-card.close_button.rect.x, -card.close_button.rect.y};
    DrawTextureTiled(*card.textures, (Rectangle) {32, 0, 11, 12}, (Rectangle) {0, 0, 33, 36}, close_button_position, 0.0, 3.0, WHITE);

    // Draw Card Edit Button
    /// Draw Button Base
    DrawTexturePro(*card.textures, (Rectangle) {43, 0, 7, 10}, (Rectangle) {card.edit_button.rect.x, card.edit_button.rect.y, 21, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {50, 0, 1, 10}, (Rectangle) {card.edit_button.rect.x + 21, card.edit_button.rect.y, 63, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {51, 0, 8, 10}, (Rectangle) {card.edit_button.rect.x + 84, card.edit_button.rect.y, 24, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {77, 0, 17, 5}, (Rectangle) {card.edit_button.rect.x + 27, card.edit_button.rect.y + 6, 51, 15}, (Vector2) {0, 0}, 0.0, WHITE);

    /// Draw Button Text
    DrawTexturePro(*card.textures, (Rectangle) {43, 0, 7, 10}, (Rectangle) {card.tone_button.rect.x, card.tone_button.rect.y, 21, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {50, 0, 1, 10}, (Rectangle) {card.tone_button.rect.x + 21, card.tone_button.rect.y, 63, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {51, 0, 8, 10}, (Rectangle) {card.tone_button.rect.x + 84, card.tone_button.rect.y, 24, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {77, 5, 17, 5}, (Rectangle) {card.tone_button.rect.x + 27, card.tone_button.rect.y + 6, 51, 15}, (Vector2) {0, 0}, 0.0, WHITE);

    // Draw Font increase buttons
    DrawTexturePro(*card.textures, (Rectangle) {59, 0, 9, 9}, (Rectangle) {card.increase_font_button.rect.x, card.increase_font_button.rect.y, 27, 27}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {68, 0, 9, 9}, (Rectangle) {card.decrease_font_button.rect.x, card.decrease_font_button.rect.y, 27, 27}, (Vector2) {0, 0}, 0.0, WHITE);
}

void draw_card_body(float x, float y, float width, float height, bool light) {
    // Draw Body
    DrawRectangleRec((Rectangle) {x + 3, y + 3, width - 3, height - 3}, light ? CARDWHITE : CARDBLACK);
    auto top_left     = (Vector2) {-x, -y};
    auto top_right    = (Vector2) {-x - width + 12, -y};
    auto bottom_left  = (Vector2) {-x, -y - height + 12};
    auto bottom_right = (Vector2) {-x - width + 12, -y -height + 12};
    if (light) {
        DrawTexturePro(spritesheet, (Rectangle) {0, 0, 4, 4}, (Rectangle) {0, 0, 12, 12}, top_left, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {12, 0, 4, 4}, (Rectangle) {0, 0, 12, 12}, top_right, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {0, 12, 4, 4}, (Rectangle) {0, 0, 12, 12}, bottom_left, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {12, 12, 4, 4}, (Rectangle) {0, 0, 12, 12}, bottom_right, 0.0, WHITE);

        // Horizontal bars
        DrawTexturePro(spritesheet, (Rectangle) {4, 0, 8, 3}, (Rectangle) {0, 0, (width - 24), 9}, (Vector2) {-x - 12, -y}, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {4, 13, 8, 3}, (Rectangle) {0, 0, (width - 24), 9}, (Vector2) {-x - 12, -y - height + 9}, 0.0, WHITE);

        // Vertical bars
        DrawTexturePro(spritesheet, (Rectangle) {0, 4, 3, 8}, (Rectangle) {0, 0, 9, height - 20}, (Vector2) {-x, -y - 9}, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {13, 4, 3, 8}, (Rectangle) {0, 0, 9, height - 20}, (Vector2) {-x - width + 9, -y - 9}, 0.0, WHITE);

    } else {
        DrawTexturePro(spritesheet, (Rectangle) {16, 0, 4, 4}, (Rectangle) {0, 0, 12, 12}, top_left, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {12 + 16, 0, 4, 4}, (Rectangle) {0, 0, 12, 12}, top_right, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {16, 12, 4, 4}, (Rectangle) {0, 0, 12, 12}, bottom_left, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {12 + 16, 12, 4, 4}, (Rectangle) {0, 0, 12, 12}, bottom_right, 0.0, WHITE);

        // Horizontal bars
        DrawTexturePro(spritesheet, (Rectangle) {4 + 16, 0, 8, 3}, (Rectangle) {0, 0, (width - 24), 9}, (Vector2) {-x - 12, -y}, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {4 + 16, 13, 8, 3}, (Rectangle) {0, 0, (width - 24), 9}, (Vector2) {-x - 12, -y - height + 9}, 0.0, WHITE);

        // Vertical bars
        DrawTexturePro(spritesheet, (Rectangle) {16, 4, 3, 8}, (Rectangle) {0, 0, 9, height - 20}, (Vector2) {-x, -y - 9}, 0.0, WHITE);
        DrawTexturePro(spritesheet, (Rectangle) {13 + 16, 4, 3, 8}, (Rectangle) {0, 0, 9, height - 20}, (Vector2) {-x - width + 9, -y - 9}, 0.0, WHITE);
    }
}

void draw_card_body(Rectangle rect, bool light) {
    draw_card_body(rect.x, rect.y, rect.width, rect.height, light);
}

void draw(Card &card, Camera2D camera) {
    if (card.parent != NULL) {
        card.drawn = true;
        return;
    }

    Defer {card.drawn = true;};

    auto position = GetScreenToWorld2D(GetMousePosition(), camera);

    draw_card_body(card.body_rect.x, card.body_rect.y, card.body_rect.width, card.body_rect.height, card.tone == LIGHT);
    // Draw Card Content
    auto content = std::vector<char>();
    for (auto &c: card.content) {
        content.push_back((char) c);
    }
    content.push_back((char) '\0');
    card.body_rect.x += 9;
    card.body_rect.y += 30;
    card.body_rect.width -= 21;
    card.body_rect.height -= 39;
    if (card.type != SCENE) {
        draw_text_rec_justified(*card.font, content.data(), card.body_rect, get_font_size(card.font), 0.25, true, card.tone == LIGHT ? BLACK : WHITE);
    } else {
        DrawTextRec(*card.font, content.data(), card.body_rect, get_font_size(card.font), 0.15, true, card.tone == LIGHT ? BLACK : WHITE);
    }
    card.body_rect.x -= 9;
    card.body_rect.y -= 30;
    card.body_rect.width += 21;
    card.body_rect.height += 39;

    // Draw Selection outline
    if (card.selected) {
        DrawRectangleLinesEx(card.body_rect, 5.0, BLUE);
    }

    // NOTE: Remember, we scale every pixel asset by 3x!
    // TODO: Maybe make it so we don't have to manually calculate `rect` offsets?
    switch (card.type) {
    case PERIOD: {
        Rectangle rect = {card.body_rect.x + (card.body_rect.width / 2 - 31), card.body_rect.y + 9, 21 * 3, 4 * 3};
        DrawTexturePro(*card.textures, (Rectangle) {94, 0, 21, 4}, rect, (Vector2) {0, 0}, 0.0, WHITE);
        if (card.is_beginning) {
            draw_texture_rect_scaled(*card.textures, {0, 48, 16, 16}, {card.body_rect.x + card.body_rect.width / 2 - ((16 * 3) / 2), card.body_rect.y + card.body_rect.height - 16 * 3 - 9});
        } else if (card.is_end) {
            draw_texture_rect_scaled(*card.textures, {0, 64, 16, 16}, {card.body_rect.x + card.body_rect.width / 2 - ((16 * 3) / 2), card.body_rect.y + card.body_rect.height - 16 * 3 - 9});
        }
        break;
    }
    case EVENT: {
        Rectangle rect = {card.body_rect.x + (card.body_rect.width / 2 - 28), card.body_rect.y + 9, 19 * 3, 4 * 3};
        DrawTexturePro(*card.textures, (Rectangle) {94, 4, 19, 4}, rect, (Vector2) {0, 0}, 0.0, WHITE);

        // Draw a tiny circle at the bottom if there are cards under this.

        if (!card.hover && card.cards_under.size() != 0) {
            draw_texture_rect_scaled(*card.textures, {102, 18, 6, 6}, {card.body_rect.x + card.body_rect.width / 2 - 9, card.body_rect.y + card.body_rect.height - 27});
        }
        break;
    }
    case SCENE: {
        Rectangle rect = {card.body_rect.x + (card.body_rect.width / 2 - 28), card.body_rect.y + 9, 19 * 3, 4 * 3};
        DrawTexturePro(*card.textures, (Rectangle) {94, 8, 19, 4}, rect, (Vector2) {0, 0}, 0.0, WHITE);
        break;
    }
    case LEGACY: {
        Rectangle rect = {card.body_rect.x + (card.body_rect.width / 2 - 34), card.body_rect.y + 9, 23 * 3, 4 * 3};
        DrawTexturePro(*card.textures, (Rectangle) {94, 12, 23, 4}, rect, (Vector2) {0, 0}, 0.0, WHITE);
        break;
    }
    }

    if (card.draw_resize) draw_resize_corner(card);

    if (card.in_drawer) {
        draw_texture_rect_scaled(*card.textures, {33, 36, 9, 10}, to_vector(card.move_up_button.rect));
        draw_texture_rect_scaled(*card.textures, {33, 46, 9, 10}, to_vector(card.move_down_button.rect));
        draw_texture_rect_scaled(*card.textures, {42, 44, 9, 10}, to_vector(card.remove_from_drawer_button.rect));
    }

    if (!card.hover) return; // Everything after this is only rendered when the card is hovered over by the player.
    // Reset card hover status
    Defer {card.hover = false;};
    //DrawRectangleRec(card.edit_button.rect, RED);

    draw_card_ui(card, camera);
    if (card.type == EVENT) {
        // draw(card.scene_insert_button, YELLOW);
        // draw(card.scene_remove_button, ORANGE);
        // Draw card peeking out if there's cards under this one
        if (!card.cards_under.empty()) {
            Vector2 card_back_pos = {card.body_rect.x + card.body_rect.width, card.body_rect.y + card.body_rect.height - (31 * 3) * 1.5 - 6};
            draw_texture_rect_scaled(*card.textures, {18, 23, 14, 31}, card_back_pos);

            auto text_width = MeasureTextEx(application_font_small, std::to_string(card.cards_under.size()).c_str(), 30.0, 1.0);
            auto offset_x = ((14 * 3) - text_width.x) / 2.0;
            auto offset_y = ((31 * 3) - text_width.y) / 2.0;
            DrawTextEx(application_font_small, std::to_string(card.cards_under.size()).c_str(), card_back_pos + (Vector2) {offset_x, offset_y}, 30.0, 1.0, BLACK);
        }

        // Draw In Arrow
        draw_texture_rect_scaled(*card.textures, {77, 37, 16, 13}, {card.body_rect.x + card.body_rect.width - (17 * 3) / 2 - 9, card.body_rect.y + card.body_rect.height - 13 * 3 - 100});
        // Draw Out Arrow
        draw_texture_rect_scaled(*card.textures, {93, 37, 16, 13}, {card.body_rect.x + card.body_rect.width - (17 * 3) / 2 - 15, card.body_rect.y + card.body_rect.height - 13 * 3 - 50});
    }
}

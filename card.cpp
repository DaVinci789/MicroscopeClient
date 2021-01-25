#include "card.hpp"
#include "common.hpp"

Card init_card(std::string name, Rectangle body_rect, CardType type) {
    float header_height = 20.0;
    Card card;
    card.name = name;
    card.content = "";
    card.last_name = name;
    card.last_content = "";
    card.textures = &spritesheet;
    card.fontsize = REGULAR;
    card.font = &application_font_regular;
    card.type = type;
    card.tone = LIGHT;
    card.deleted  = false;
    card.grabbed  = false;
    card.selected = false;
    card.hover = false;
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
    }
}

void draw(Card &card, Camera2D camera) {
    Defer {card.drawn = true;};

    auto position = GetScreenToWorld2D(GetMousePosition(), camera);

    // Draw Body
    DrawRectangleRec((Rectangle) {card.body_rect.x + 3, card.body_rect.y + 3, card.body_rect.width - 3, card.body_rect.height - 3}, CARDWHITE);
    auto top_left     = (Vector2) {-card.body_rect.x, -card.body_rect.y};
    auto top_right    = (Vector2) {-card.body_rect.x - card.body_rect.width + 12, -card.body_rect.y};
    auto bottom_left  = (Vector2) {-card.body_rect.x, -card.body_rect.y - card.body_rect.height + 12};
    auto bottom_right = (Vector2) {-card.body_rect.x - card.body_rect.width + 12, -card.body_rect.y -card.body_rect.height + 12};
    DrawTexturePro(*card.textures, (Rectangle) {0, 0, 4, 4}, (Rectangle) {0, 0, 12, 12}, top_left, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {12, 0, 4, 4}, (Rectangle) {0, 0, 12, 12}, top_right, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {0, 12, 4, 4}, (Rectangle) {0, 0, 12, 12}, bottom_left, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {12, 12, 4, 4}, (Rectangle) {0, 0, 12, 12}, bottom_right, 0.0, WHITE);

    // Horizontal bars
    DrawTexturePro(*card.textures, (Rectangle) {4, 0, 8, 3}, (Rectangle) {0, 0, (card.body_rect.width - 24), 9}, (Vector2) {-card.body_rect.x - 12, -card.body_rect.y}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {4, 13, 8, 3}, (Rectangle) {0, 0, (card.body_rect.width - 24), 9}, (Vector2) {-card.body_rect.x - 12, -card.body_rect.y - card.body_rect.height + 9}, 0.0, WHITE);

    // Vertical bars
    DrawTexturePro(*card.textures, (Rectangle) {0, 4, 3, 8}, (Rectangle) {0, 0, 9, card.body_rect.height - 20}, (Vector2) {-card.body_rect.x, -card.body_rect.y - 9}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {13, 4, 3, 8}, (Rectangle) {0, 0, 9, card.body_rect.height - 20}, (Vector2) {-card.body_rect.x - card.body_rect.width + 9, -card.body_rect.y - 9}, 0.0, WHITE);

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

    // Draw Selection outline
    if (card.selected) {
        DrawRectangleLinesEx(card.body_rect, 5.0, BLUE);
    }

    if (!card.hover) return;

    /// Draw close button
    // DrawRectangleRec(card.close_button.rect, RED);
    // Draw close texture
    auto close_button_position = (Vector2) {-card.close_button.rect.x, -card.close_button.rect.y};
    DrawTextureTiled(*card.textures, (Rectangle) {32, 0, 11, 12}, (Rectangle) {0, 0, 33, 36}, close_button_position, 0.0, 3.0, WHITE);

    // // Draw Toggle Button
    // if (card.tone_button.hover) {
    //     DrawRectangleRec(card.tone_button.rect, PURPLE);
    // } else {
    //     DrawRectangleRec(card.tone_button.rect, GRAY);
    // }

    // // Draw Edit Button
    // if (card.edit_button.hover) {
    //     DrawRectangleRec(card.edit_button.rect, PURPLE);
    // } else {
    //     DrawRectangleRec(card.edit_button.rect, GRAY);
    // }

    // Draw Card Edit Button
    // float vertical_offset = (card.edit_button.rect.height - MeasureTextEx(application_font_small, "Edit", FONTSIZE_SMALL, 1.0).y) / 2.0;
    // float horizontal_offset = (card.edit_button.rect.width - MeasureTextEx(application_font_small, "Edit", FONTSIZE_SMALL, 1.0).x) / 2.0;
    // card.edit_button.rect.x += vertical_offset - 3;
    // card.edit_button.rect.y += vertical_offset;
    // DrawTextRec(application_font_small, "Edit", card.edit_button.rect, FONTSIZE_SMALL, 1.0, false, WHITE);
    // card.edit_button.rect.x -= vertical_offset - 3;
    // card.edit_button.rect.y -= vertical_offset;
    // Draw Button Base
    DrawTexturePro(*card.textures, (Rectangle) {43, 0, 7, 10}, (Rectangle) {card.edit_button.rect.x, card.edit_button.rect.y, 21, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {50, 0, 1, 10}, (Rectangle) {card.edit_button.rect.x + 21, card.edit_button.rect.y, 63, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {51, 0, 8, 10}, (Rectangle) {card.edit_button.rect.x + 84, card.edit_button.rect.y, 24, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {77, 0, 17, 5}, (Rectangle) {card.edit_button.rect.x + 27, card.edit_button.rect.y + 6, 51, 15}, (Vector2) {0, 0}, 0.0, WHITE);

    // Draw Button Text
    DrawTexturePro(*card.textures, (Rectangle) {43, 0, 7, 10}, (Rectangle) {card.tone_button.rect.x, card.tone_button.rect.y, 21, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {50, 0, 1, 10}, (Rectangle) {card.tone_button.rect.x + 21, card.tone_button.rect.y, 63, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {51, 0, 8, 10}, (Rectangle) {card.tone_button.rect.x + 84, card.tone_button.rect.y, 24, 30}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {77, 5, 17, 5}, (Rectangle) {card.tone_button.rect.x + 27, card.tone_button.rect.y + 6, 51, 15}, (Vector2) {0, 0}, 0.0, WHITE);

    // Draw Font increase buttons
    // draw(card.increase_font_button, BLUE);
    // draw(card.decrease_font_button, BROWN);
    DrawTexturePro(*card.textures, (Rectangle) {59, 0, 9, 9}, (Rectangle) {card.increase_font_button.rect.x, card.increase_font_button.rect.y, 27, 27}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*card.textures, (Rectangle) {68, 0, 9, 9}, (Rectangle) {card.decrease_font_button.rect.x, card.decrease_font_button.rect.y, 27, 27}, (Vector2) {0, 0}, 0.0, WHITE);

    // Reset card hover status
    card.hover = false;
}

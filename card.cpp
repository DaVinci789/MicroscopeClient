#include "card.hpp"
#include "common.hpp"

Card init_card(std::string name, Rectangle body_rect, CardType type) {
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

void draw(Card &card, Camera2D camera) {
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

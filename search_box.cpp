#include "search_box.hpp"
#include "common.hpp"
#include "card.hpp"
#include <cstring>
#include <iterator>
#include <sstream>

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "fuzzy_match.hpp"

using fts::fuzzy_match;

SearchBox init_search_box() {
    SearchBox box;
    box.visible = false;
    box.backdrop = {0, 0, 30, 30};
    box.search_box = {30, 30, 30, 30};
    box.search = "";
    box.search_confirm = init_button();
    box.results = std::vector<Card>();
    box.close = init_button();
    return box;
}

void update_search_box(SearchBox& box, std::vector<Card>& cards) {
    if (!box.visible) return;
    for (auto& card: cards) card.selected = false;
    if (box.search.empty()) return;
    box.results.clear();
    std::copy_if(cards.begin(), cards.end(), std::back_inserter(box.results), [&](auto &card) {
        int amount = 0;
        bool result = fuzzy_match(to_c_str(box.search).data(), to_c_str(card.content).data(), amount);
        if (!card.cards_under.empty()) {
            std::copy_if(card.cards_under.begin(), card.cards_under.end(), std::back_inserter(box.results), [&](auto &card) {
                int amount_inner = 0;
                bool result_inner = fuzzy_match(to_c_str(box.search).data(), to_c_str(card.content).data(), amount_inner);
                return amount_inner > 0 && result_inner;
            });
        }
        return amount > 0 && result;
    });
    std::sort(box.results.begin(), box.results.end(), [&](auto &result1, auto &result2) {
        int score1 = 0;
        int score2 = 0;
        fuzzy_match(to_c_str(box.search).data(), to_c_str(result1.content).data(), score1);
        fuzzy_match(to_c_str(box.search).data(), to_c_str(result2.content).data(), score2);
        return score1 > score2;
    });

    for (auto& result: box.results) {
        result.selected = true;
    }
    for (auto& card: cards) {
        for (auto& result: box.results) {
            if (result == card) card = result;
        }
    }
}

std::string truncate(std::string str, size_t width, bool show_ellipsis=true) {
    if (str.length() > width) {
        if (show_ellipsis) {
            return str.substr(0, width) + "...";
        } else {
            return str.substr(0, width);
        }
    }
    return str;
}

void draw_search_box(SearchBox& box) {
    DrawRectangleRec(box.backdrop, RED);
    draw_text_bubble(true, box.search, to_vector(box.backdrop));
    int result_index = 1;
    for (auto& result: box.results) {
        Vector2 where = {0, (float) result_index * 60};
        auto text_width = MeasureTextEx(application_font_regular, truncate(result.content, 8).c_str(), FONTSIZE_REGULAR, 1.0).x;
        auto text_height = MeasureTextEx(application_font_regular, truncate(result.content, 8).c_str(), FONTSIZE_REGULAR, 1.0).y;
        draw_card_body((Rectangle) {where.x, where.y, text_width + 12, text_height + 9}, true);
        draw_card_body((Rectangle) {where.x + text_width + 12, where.y, 90, text_height + 9}, true);
        DrawTextEx(application_font_regular, truncate(result.content, 8).c_str(), {where.x + 9, where.y + 3}, FONTSIZE_REGULAR, 1.0, BLACK);
        switch (result.type) {
        case PERIOD:
            draw_texture_rect_scaled(spritesheet, {94, 0, 21, 4}, {where.x + text_width + 12 + 9, where.y + 15});
            break;
        case EVENT:
            draw_texture_rect_scaled(spritesheet, {94, 4, 19, 4}, {where.x + text_width + 12 + 9, where.y + 15});
            break;
        case SCENE:
            draw_texture_rect_scaled(spritesheet, {94, 8, 19, 4}, {where.x + text_width + 12 + 9, where.y + 15});
            break;
        case LEGACY:
            draw_texture_rect_scaled(spritesheet, {94, 12, 23, 4}, {where.x + text_width + 12 + 9, where.y + 15});
            break;
        }
        result_index += 1;
    }
}

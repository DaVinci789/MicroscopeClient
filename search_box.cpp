#include "search_box.hpp"
#include "common.hpp"
#include "card.hpp"
#include <cstring>
#include <sstream>

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

size_t levenshtein_distance(const char* s, size_t n, const char* t, size_t m) {
   ++n; ++m;
   size_t* d = new size_t[n * m];

   memset(d, 0, sizeof(size_t) * n * m);
   for (size_t i = 1, im = 0; i < m; ++i, ++im) {
      for (size_t j = 1, jn = 0; j < n; ++j, ++jn) {
         if (s[jn] == t[im]) {
            d[(i * n) + j] = d[((i - 1) * n) + (j - 1)];
         }
         else {
             d[(i * n) + j] = std::min(d[(i - 1) * n + j] + 1, /* A deletion. */
                                       std::min(d[i * n + (j - 1)] + 1, /* An insertion. */
                                     d[(i - 1) * n + (j - 1)] + 1)); /* A substitution. */
         }
      }
   }

   size_t r = d[n * m - 1];
   delete [] d;
   return r;
}

size_t leven(std::string s1, std::string s2) {
    return levenshtein_distance(to_c_str(s1).data(), s1.size(), to_c_str(s2).data(), s2.size());
}

void update_search_box(SearchBox& box, std::vector<Card>& cards) {
    if (!box.visible) return;
    for (auto& card: cards) card.selected = false;
    if (box.search.empty()) return;
    box.results.clear();
    std::copy_if(cards.begin(), cards.end(), std::back_inserter(box.results), [&](auto& card){
        if (!card.cards_under.empty()) {
            // @HACK: This is literally a copy of the function. Instead of a lambda, this should be a separate function.
            std::copy_if(card.cards_under.begin(), card.cards_under.end(),
                         std::back_inserter(box.results), [&](auto &under_card) {
                             std::string card_content_lower_inner;
                             for (auto &c : under_card.content) {
                                 card_content_lower_inner += std::tolower(c);
                             }
                             std::string box_search_lower_inner;
                             for (auto &c : box.search) {
                                 box_search_lower_inner += std::tolower(c);
                             }
                             return card_content_lower_inner.find(
                                 box_search_lower_inner) !=
                                 std::string::npos;
                         });
        }
        std::string card_content_lower;
        for (auto& c: card.content) {
            card_content_lower += std::tolower(c);
        }
        std::string box_search_lower;
        for (auto& c: box.search) {
            box_search_lower += std::tolower(c);
        }
        return card_content_lower.find(box_search_lower) != std::string::npos;
    });
    std::copy_if(cards.begin(), cards.end(), std::back_inserter(box.results), [&](auto& card){
        if (!card.cards_under.empty()) {
              std::copy_if(
                  card.cards_under.begin(), card.cards_under.end(), std::back_inserter(box.results),
                  [&](auto &card_inner) {
                      if (std::find(box.results.begin(), box.results.end(),
                                    card_inner) != box.results.end())
                          return false;
                      std::string card_content_lower_inner;
                      for (auto &c : card_inner.content) {
                          card_content_lower_inner += std::tolower(c);
                      }
                      std::string box_search_lower_inner;
                      for (auto &c : box.search) {
                          box_search_lower_inner += std::tolower(c);
                      }
                      std::istringstream words_inner(card_inner.content);
                      do {
                          std::string word;
                          words_inner >> word;
                          if (word.size() < 3)
                              continue;
                          if (leven(box_search_lower_inner, word) <= 2) {
                              return true;
                          }
                      } while (words_inner);
                      return false;
                  });
        }
        if (std::find(box.results.begin(), box.results.end(), card) != box.results.end()) return false;
        std::string card_content_lower;
        for (auto& c: card.content) {
            card_content_lower += std::tolower(c);
        }
        std::string box_search_lower;
        for (auto& c: box.search) {
            box_search_lower += std::tolower(c);
        }
        std::istringstream words(card.content);
        do {
            std::string word;
            words >> word;
            if (word.size() < 3) continue;
            if (leven(box_search_lower, word) <= 2) {
                return true;
            }
        } while (words);
        return false;
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
        DrawTextEx(application_font_regular, truncate(result.content, 8).c_str(), {where.x + 9, where.y + 3}, FONTSIZE_REGULAR, 1.0, BLACK);
        result_index += 1;
    }
}

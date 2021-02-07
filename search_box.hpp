#pragma once
#include "common.hpp"
#include "card.hpp"

struct SearchBox {
    bool visible;
    Rectangle backdrop;
    Rectangle search_box;
    std::string search;
    std::vector<Card> results;
    Button search_confirm;
    Button close;
};

    
SearchBox init_search_box();
void update_search_box(SearchBox& box, std::vector<Card>& cards);
void draw_search_box(SearchBox& box);

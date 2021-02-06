#pragma once
#include "card.hpp"

struct CardPool {
    Card *memory;
    Card *current_element;
    int max_elements;
    int active_cards;
};

Card* begin(CardPool& pool);
Card* end(CardPool& pool);
CardPool init_pool(int num_cards = 256);
bool add_card(CardPool& pool, const Card card);
void free_pool(CardPool &pool);
void cull(CardPool& pool);

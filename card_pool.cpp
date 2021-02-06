#include "card_pool.hpp"
#include "card.hpp"
#include <cstdlib>
#include <cstring>

Card* begin(CardPool& pool) {
    return pool.memory;
}

Card* end(CardPool& pool) {
    return pool.memory + pool.max_elements;
}

CardPool init_pool(int num_cards) {
    CardPool pool;
    pool.memory = (Card*) malloc(sizeof(Card) * num_cards);
    memset(pool.memory, 0xAB, sizeof(Card) * num_cards);
    pool.current_element = pool.memory;
    pool.max_elements = num_cards;
    pool.active_cards = 0;
    return pool;
}

bool add_card(CardPool& pool, const Card card) {
    pool.active_cards += 1;
    pool.current_element = pool.memory;
    while (*((unsigned char*) pool.current_element) != 0xAB) {
        pool.current_element++;
    }
    memcpy(pool.current_element, &card, sizeof(Card));
    return true;
}

void cull(CardPool& pool) {
    int current = 0;
    Card *current_card = pool.memory;
    while (current < pool.max_elements) {
        if ((*((unsigned char*) current_card) != 0xAB) && (current_card->deleted)) {
            memset(current_card, 0xAB, sizeof(Card));
            pool.active_cards -= 1;
        }
        current_card++;
        current += 1;
    }
}

void free_pool(CardPool &pool) {
    free(pool.memory);
}

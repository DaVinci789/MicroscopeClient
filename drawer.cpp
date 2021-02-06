#include "drawer.hpp"
#include "card.hpp"

Drawer init_drawer(std::vector<Card>* cards) {
    Drawer drawer;
    drawer.open = false;
    drawer.body_rect = {0,0, 300, 100000};
    drawer.cards = cards;
    return drawer;
}

void update_drawer(Drawer& drawer) {}

void draw_drawer(const Drawer& drawer, Camera2D camera) {
    DrawRectangleRec(drawer.body_rect, {249, 232, 202, 255});
    int card_index = 0;
    for (auto &card: *drawer.cards) {
        Defer {card_index += 1;};
        card.body_rect.x = 0;
        card.body_rect.y = (GRIDSIZE * 13) * card_index;
        card.body_rect.width = GRIDSIZE * 17;
        card.body_rect.height = GRIDSIZE * 13;
        card.parent = NULL;
        card.in_drawer = true;
        card.move_up_button.rect = {card.body_rect.x + card.body_rect.width - 66, card.body_rect.y + card.body_rect.height - 36 * 2, 27, 30};
        card.move_down_button.rect = {card.body_rect.x + card.body_rect.width - 66, card.body_rect.y + card.body_rect.height - 36, 27, 30};
        card.remove_from_drawer_button.rect = {card.body_rect.x + card.body_rect.width - 36, card.body_rect.y + card.body_rect.height - 36 - (36 / 2), 27, 30};
        draw(card, camera);
        // draw(card->move_up_button, {255, 203, 0, 125});
        // draw(card->move_down_button, {255, 203, 0, 125});
        // draw(card->remove_from_drawer_button, {255, 203, 0, 125});
    }
}

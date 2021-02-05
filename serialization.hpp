#pragma once
#include "common.hpp"
#include "card.hpp"

enum FileCheck {
    CARDTYPE,
    CARDPOSITION,
    CARDTONE,
    CARDCONTENT,
};

std::vector<Card> load_cards(const char *filename = "save.json");
void save_cards(const std::vector<Card>& cards, const char *savefile = "save.json");

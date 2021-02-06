#pragma once
#include "common.hpp"
#include "card.hpp"

enum FileCheck {
    CARDTYPE,
    CARDPOSITION,
    CARDTONE,
    CARDCONTENT,
};

void load_cards(std::vector<Card>& cards, const char *filename = "save.json");
void save_cards(const std::vector<Card>& cards, const char *savefile = "save.json");

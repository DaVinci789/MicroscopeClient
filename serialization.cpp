#include <fstream>
#include <ostream>
#include "json.hpp"
#include "common.hpp"
#include "card.hpp"
#include "serialization.hpp"

using json = nlohmann::json;

struct CardToFind {
    Card card;
    std::string id;
};

void load_cards(std::vector<Card>& cards, const char *filename) {
    //std::vector<Card> cards;
    std::ifstream file(filename);
    std::string content;
    Card current_card = init_card("", {0, 0, GRIDSIZE * 17, GRIDSIZE * 13});
    json j;
    file >> j;
    for (auto &card: j["cards"]) {
        Defer {current_card = init_card("", {0, 0, GRIDSIZE * 17, GRIDSIZE * 13});};
        current_card.id = card.at("id");
        current_card.type = card.at("type");
        current_card.tone = card.at("tone");
        current_card.body_rect.x = card.at("x");
        current_card.body_rect.y = card.at("y");
        current_card.lock_target.x = card.at("x");
        current_card.lock_target.y = card.at("y");
        current_card.body_rect.width = card.at("w");
        current_card.body_rect.height = card.at("h");
        current_card.content = card.at("content");
        if (card.count("cards_under") > 0) {
            for (auto &under_card_json: card.at("cards_under")) {
                auto under_card = init_card("", {0, 0, GRIDSIZE * 17, GRIDSIZE * 13});
                under_card.parent = &current_card;
                under_card.id = under_card_json.at("id");
                under_card.type = under_card_json.at("type");
                under_card.tone = under_card_json.at("tone");
                under_card.content = under_card_json.at("content");
                under_card.saved_dimensions.x = under_card_json.at("saved_dimensions_x");
                under_card.saved_dimensions.y = under_card_json.at("saved_dimensions_y");
                current_card.cards_under.push_back(std::move(under_card));
            }
        }
        cards.emplace_back(current_card);
    }
}

void save_cards(const std::vector<Card>& cards, const char *savefile) {
    std::ofstream file(savefile);
    json save_file;
    save_file["cards"] = {};
    int id = 0;
    for (auto &card: cards) {
        save_file["cards"][std::to_string(id)] = {
            {"id", card.id},
            {"type", card.type},
            {"tone", card.tone},
            {"x", card.body_rect.x},
            {"y", card.body_rect.y},
            {"w", card.body_rect.width},
            {"h", card.body_rect.height},
            {"content", card.content},
        };
        if (!card.cards_under.empty()) {
            save_file["cards"][std::to_string(id)]["cards_under"] = {};
            for (auto &under_card: card.cards_under) {
                save_file["cards"][std::to_string(id)]["cards_under"].push_back(
                    {
                        {"id", under_card.id},
                        {"type", under_card.type},
                        {"tone", under_card.tone},
                        {"content", under_card.content},
                        {"saved_dimensions_x", under_card.saved_dimensions.x},
                        {"saved_dimensions_y", under_card.saved_dimensions.y},
                    }
                    );
            }
        }
        id += 1;
    }
    file << save_file << std::endl;
}

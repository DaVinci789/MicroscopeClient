#include <fstream>
#include <ostream>
#include "json.hpp"
#include "common.hpp"
#include "card.hpp"
#include "serialization.hpp"

using json = nlohmann::json;
std::vector<Card> load_cards(const char *filename) {
    std::vector<Card> cards;
    std::ifstream file(filename);
    std::string content;
    Card current_card = init_card("", {0, 0, GRIDSIZE * 17, GRIDSIZE * 13});
    json j;
    file >> j;
    for (auto &card: j["cards"]) {
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
        cards.push_back(current_card);
        current_card = init_card("", {0, 0, GRIDSIZE * 17, GRIDSIZE * 13});
    }
    return cards;
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
        id += 1;
    }
    file << save_file << std::endl;
}

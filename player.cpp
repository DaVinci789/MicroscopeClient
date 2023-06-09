#include "card.hpp"
#include "common.hpp"
#include "player.hpp"
#include "drawer.hpp"
#include "networking.hpp"
#include "main_menu.hpp"
#include "search_box.hpp"

Player init_player() {
    Player player;
    player.quit = false;
    player.mouse_held  = false;

    auto position = GetMousePosition();
    player.player_rect = {position.x, position.y, 10, 10};
    player.mouse_position = position;

    player.card_focus = SCENE;
    player.is_card_type_focus = false;
    player.camera_move_speed = 20;
    player.camera = {0};
    player.camera_target = {0, 0};
    player.camera.target = player.camera_target;
    player.zoom_level = 3;
    player.camera_zoom_target = 1.0f;
    player.camera.offset = (Vector2){ 800/2, 600/2 };
    player.camera.rotation = 0.0f;
    player.camera.zoom = 1.0f;

    player.binds = {
        KEY_W,
        KEY_S,
        KEY_A,
        KEY_D,
    };
    player.state = HOVERING;
    player.editing = BODY;

    player.hold_origin = {0};
    player.hold_diff = {0};

    player.selected_card = NULL;
    player.offset = {0, 0};
    player.resizing_card = false;
    return player;
}

void spawn_card(Player player, std::vector<Card>& cards, CardType type) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    Rectangle to_draw = {position.x, position.y, GRIDSIZE * 17, GRIDSIZE * 13};
    Card the_card = init_card("New Card", to_draw, type);

    // center card on player cursor
    the_card.body_rect.x -= the_card.body_rect.width / 2;
    the_card.body_rect.y -= the_card.body_rect.height / 2;
    the_card.lock_target = {the_card.body_rect.x, the_card.body_rect.y};

    auto next_card = greatest_depth_and_furthest_along(cards);
    if (next_card) the_card.depth = next_card->depth + 1;
    else the_card.depth = 0;
    cards.push_back(the_card);
}

void player_update_camera(Player &player, bool allow_key_scroll) {
    #define ZOOM_SIZE 5
    float zoom_levels[ZOOM_SIZE] = {0.3, 0.5, 0.7, 1, 1.5};

    /// Move Left/Right
    if (allow_key_scroll) {
        if (IsKeyDown(player.binds.move_right)) {
            player.camera_target.x += player.camera_move_speed * (1.0 / player.camera.zoom);
        } else if (IsKeyDown(player.binds.move_left)) {
            player.camera_target.x -= player.camera_move_speed * (1.0 / player.camera.zoom);
        }

        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) { // Zooming
            // Zoom In/Out
            if (IsKeyPressed(player.binds.move_up)) {
                player.zoom_level += 1;
            } else if (IsKeyPressed(player.binds.move_down)) {
                player.zoom_level -= 1;
            }
        } else {
            // Move Up/Down
            if (IsKeyDown(player.binds.move_up)) {
                player.camera_target.y -= player.camera_move_speed;
            } else if (IsKeyDown(player.binds.move_down)) {
                player.camera_target.y += player.camera_move_speed;
            }
        }
    }

    // Mouse wheel stuff
    auto wheel_move = GetMouseWheelMove();
    if (wheel_move != 0.0) {
        if (wheel_move > 0.0) {
            player.zoom_level += 1;
        } else {
            player.zoom_level -= 1;
        }
    }

    player.zoom_level = clamp<int>(player.zoom_level, 0, ZOOM_SIZE - 1);
    player.camera_zoom_target = zoom_levels[player.zoom_level];

    // Middle mouse button press movement
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
        auto mouse_move = get_mouse_delta() * (1.0 / player.camera.zoom);
        player.camera_target = player.camera_target - mouse_move;
    }

    #undef ZOOM_SIZE
}

void player_write_update(Player& player) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (player.editing == NAME) {
            if (player.selected_card->name.empty()) player.selected_card->name = player.selected_card->last_name;
        }
        player.state = HOVERING;
        player.selected_card->grabbed = false;
        player.selected_card->selected = false;
        player.selected_card->draw_resize = false;
        player.selected_card = NULL;
        return;
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (player.editing == NAME && player.selected_card->name.size() > 0) {
            player.selected_card->name.pop_back();
        }
        else if (player.editing == BODY && player.selected_card->content.size() > 0) {
            player.selected_card->content.pop_back();
        }
        return;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        if (player.editing == NAME) return;
        player.selected_card->content += (char) '\n';
    }
    // Typing
    if (player.selected_card) {
        auto char_pressed = GetCharPressed();
        if (char_pressed != 0) {
            if (player.editing == NAME) {
                player.selected_card->name += (char) char_pressed;
            } else {
                player.selected_card->content += (char) char_pressed;
            }
        }
    }
}

void player_resize_chosen_card(Player& player) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    Card *card = player.selected_card;
    if (!card) return;

    auto mouse_over_button = CheckCollisionPointRec(position, card->edit_button.rect);
    if (IsMouseButtonPressed(0) && mouse_over_button) {
        player.resizing_card = true;
    } else if (IsMouseButtonReleased(0)) {
        player.resizing_card = false;
    }

    if (!player.resizing_card) return;
    auto mouse_delta = get_mouse_delta() * (1.0 / player.camera.zoom);
    card->body_rect.width += mouse_delta.x;
    card->body_rect.height += mouse_delta.y;
    if (card->body_rect.width < GRIDSIZE * 11) card->body_rect.width = GRIDSIZE * 11;
    if (card->body_rect.height < GRIDSIZE * 11) card->body_rect.height = GRIDSIZE * 11;
}

// This is the main meat of the program.
void player_hover_update(Player& player, std::vector<Card>& cards, Palette& palette, Project &project, Drawer& drawer, MainMenu &main_menu, SearchBox& search_box) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    player.player_rect.x = position.x;
    player.player_rect.y = position.y;

    palette.open_button.hover = collide((Rectangle) {mouse_position.x, mouse_position.y, 10, 10}, palette.open_button.rect);
    // update_button_hover(project.start_server, mouse_position);
    // update_button_hover(project.start_client, mouse_position);

    // We need to initialize this value to the first card because if it's null, we can't check if the player is over a card.
    Card *player_card_over = &cards[0]; // Card that the player is hovering over
    bool found_card = false;
    // Mouse and Card Selection
    for (auto &card: cards) {
        if (palette.open_button.hover) break; // skip checking the cards if the players is hovering over the palette open thingie
        if (card.parent != NULL) continue;
        if (CheckCollisionPointRec(position, card.body_rect)) {
            found_card = true;
            if (card.depth < player_card_over->depth && CheckCollisionPointRec(position, player_card_over->body_rect)) {
                continue;
            }
            player_card_over = &card;
        }
        update_button_hover(card.close_button, position);
        update_button_hover(card.edit_button, position);
        update_button_hover(card.tone_button, position);
        update_button_hover(card.increase_font_button, position);
        update_button_hover(card.decrease_font_button, position);
        if (card.type == EVENT) {
            update_button_hover(card.scene_insert_button, position);
            update_button_hover(card.scene_remove_button, position);
        }
    }
    if (!found_card) {
        player_card_over = NULL;
    }

    if (player_card_over != NULL) {
        player_card_over->hover = true;
    }

    if (IsMouseButtonPressed(0) && player_card_over != NULL) {
        player.mouse_held = true;
        player.selected_card = player_card_over;
        player.offset = {player.player_rect.x - player.selected_card->body_rect.x, player.player_rect.y - player.selected_card->body_rect.y};
        player_card_over->grabbed = true;
    }

    if (IsMouseButtonPressed(0) && player.selected_card) {
        auto deepest_card = greatest_depth_and_furthest_along(cards);
        player.selected_card->depth = deepest_card->depth + 1;
        // Card button clicked!
        if (player.selected_card->close_button.hover) {
            player.selected_card->deleted = true;
            player.mouse_held = false;
            player.offset = {0 ,0};
            return;
        } else if (player.selected_card->edit_button.hover) {
            player.state = WRITING;
            player.editing = BODY;
            player.mouse_held = false;
            player.selected_card->draw_resize = true;
            player.offset = {0 ,0};
        } else if (player.selected_card->tone_button.hover) {
            if (player.selected_card->tone == LIGHT) {
                player.selected_card->tone  = DARK;
            } else {
                player.selected_card->tone  = LIGHT;
            }
        } else if (player.selected_card->increase_font_button.hover) {
            FontSize *the_size = &player.selected_card->fontsize;
            switch (player.selected_card->fontsize) {
            case SMALL: 
                *the_size = REGULAR;
                player.selected_card->font = &application_font_regular;
                break;
            case REGULAR: 
                *the_size = LARGE;
                player.selected_card->font = &application_font_large;
                break;
            case LARGE: 
                *the_size = SMALL;
                player.selected_card->font = &application_font_small;
                break;
            }
        } else if (player.selected_card->decrease_font_button.hover) {
            FontSize *the_size = &player.selected_card->fontsize;
            switch (player.selected_card->fontsize) {
            case SMALL: 
                *the_size = LARGE;
                player.selected_card->font = &application_font_large;
                break;
            case REGULAR: 
                *the_size = SMALL;
                player.selected_card->font = &application_font_small;
                break;
            case LARGE: 
                *the_size = REGULAR;
                player.selected_card->font = &application_font_regular;
                break;
            }
        } else if (player.selected_card->scene_insert_button.hover) {
            player.state = SCENECARDSELECTING;
            player.card_focus = SCENE;
            player.is_card_type_focus = true;
            player.mouse_held = false;
            player.offset = {0, 0};
            return;
        } else if (player.selected_card->scene_remove_button.hover) {
            player.state = DRAWERCARDSELECTING;
            drawer.open = true;
            drawer.cards = &player.selected_card->cards_under;
            player.mouse_held = false;
            player.offset = {0, 0};
            return;
        }
    } else if (IsMouseButtonPressed(0)) { // Player clicks, but is not on a card
        if (palette.open_button.hover) {
            toggle_palette(palette);
            return;
        } else if (palette.yes_button.hover && palette.open) {
            player.state = PALETTEWRITING;
            player.palette_edit_type = YES;
            add_palette_slot(palette, YES);
            return;
        } else if (palette.no_button.hover && palette.open) {
            player.state = PALETTEWRITING;
            player.palette_edit_type = NO;
            add_palette_slot(palette, NO);
            return;
        } else if (project.focus.hover) { // Focus clicked
            player.state = FOCUSWRITING;
            project.last_focus_text = project.focus.text;
            project.focus.text = "";
            return;
        }
        // else if (project.start_server.hover) {
        //     print(200);
        //     init_server();
        //     return;
        // } else if (project.start_client.hover) {
        //     print(201);
        //     init_client();
        //     return;
        // }
        else {
            player.state = GRABBING; // Background drag
            player.hold_origin = GetMousePosition();
            return;
        }
    } else if (IsMouseButtonReleased(0) && player.selected_card) { // Player releases a card
        auto position_to_lock_to = lock_position_to_grid((Vector2) {player.selected_card->body_rect.x, player.selected_card->body_rect.y});
        player.selected_card->lock_target = position_to_lock_to;
        player.selected_card->grabbed = false;
        for (auto& card: cards) {
            if (!card.selected) continue;
            position_to_lock_to = lock_position_to_grid((Vector2) {card.body_rect.x, card.body_rect.y});
            card.lock_target = position_to_lock_to;
            card.selected = false;
        }

        player.mouse_held = false;
        player.offset = {0, 0};
        player.selected_card = NULL;
    } 

    // Move grabbed and selected cards.
    if (player.mouse_held && player.selected_card) {
        player.selected_card->body_rect.x = position.x - player.offset.x;
        player.selected_card->body_rect.y = position.y - player.offset.y;
        for (auto& card: cards) {
            if (card.selected) {
                auto mouse_delta = get_mouse_delta() * (1.0/player.camera.zoom);
                card.lock_target.x += mouse_delta.x;
                card.lock_target.y += mouse_delta.y;
            }
        }
    }

    // Key Processing
    player_update_camera(player);

    /// Spawn card
    if (IsKeyPressed(KEY_ONE)) {
        spawn_card(player, cards, PERIOD);
    } else if (IsKeyPressed(KEY_TWO)) {
        spawn_card(player, cards, EVENT);
    } else if (IsKeyPressed(KEY_THREE)) {
        spawn_card(player, cards, SCENE);
    } else if (IsKeyPressed(KEY_FOUR)) {
        spawn_card(player, cards, LEGACY);
    }

    if (IsKeyPressed(KEY_F9) && player_card_over != NULL) {
        player_card_over->is_beginning = !player_card_over->is_beginning;
    }
    if (IsKeyPressed(KEY_F10) && player_card_over != NULL) {
        player_card_over->is_end = !player_card_over->is_end;
    }

    // Change big picture
    /// @Incomplete: make this a button
    if (IsKeyDown(KEY_F11)) {
        print(200);
        project.last_big_picture = project.big_picture;
        project.big_picture = "";
        player.state = BIGPICTUREWRITING;
        return;
    }

    // Control Key Handling
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_Q)) {
            // @Incomplete: Warn user about quitting first!
            player.quit = true;
        } else if (IsKeyPressed(KEY_F)) {
            search_box.visible = true;
            player.state = SEARCHING;
            return;
        }
    }

    // Delete cards
    if (IsKeyPressed(KEY_DELETE)) {
        for (auto &card: cards) {
            card.deleted = card.selected;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        main_menu.visible = true;
    }

    if (IsKeyPressed(KEY_H)) {
        print(200);
        std::vector<Card> x_cards;
        std::copy_if(cards.begin(), cards.end(), std::back_inserter(x_cards), [](auto& card){return card.selected;});
        std::sort(x_cards.begin(), x_cards.end(), [](auto& card1, auto& card2) {
            return card1.body_rect.x < card2.body_rect.x;
        });
        float move = 0;
        for (auto &card: x_cards) {
            card.lock_target = {position.x + move, position.y};
            move += card.body_rect.width;
        }
        // This is so slow! We need to just use references or pointers or whatever instead.
        for (auto &card: cards) {
            for (auto &mod_card: x_cards) {
                if (card == mod_card) {
                    card = mod_card;
                }
            }
        }
    } else if (IsKeyPressed(KEY_V)) {
        std::vector<Card> y_cards;
        std::copy_if(cards.begin(), cards.end(), std::back_inserter(y_cards), [](auto& card){return card.selected;});
        std::sort(y_cards.begin(), y_cards.end(), [](auto& card1, auto& card2) {
            return card1.body_rect.y < card2.body_rect.y;
        });
        float move = 0;
        for (auto &card: y_cards) {
            card.lock_target = {position.x, position.y + move};
            move += card.body_rect.height;
        }
        for (auto &card: cards) {
            for (auto &mod_card: y_cards) {
                if (card == mod_card) {
                    card = mod_card;
                }
            }
        }
    } else if (IsKeyPressed(KEY_C)) {
        std::vector<Card> selected_cards;
        std::copy_if(cards.begin(), cards.end(), std::back_inserter(selected_cards), [](auto& card){return card.selected;});
        std::sort(selected_cards.begin(), selected_cards.end(), [](auto& card1, auto& card2) {
            return card1.depth < card2.depth;
        });
        for (auto &card: selected_cards) {
            card.lock_target = {position.x, position.y};
        }
        for (auto &card: cards) {
            for (auto &mod_card: selected_cards) {
                if (card == mod_card) {
                    card = mod_card;
                }
            }
        }
    }
}

void player_grabbing_update(Player& player, std::vector<Card>& cards) {
    if (IsMouseButtonReleased(0)) {
        player.hold_diff = {0, 0};
        player.selection_rec = {0};
        player.state = HOVERING;
        return;
    }
    player.hold_diff = player.hold_diff + get_mouse_delta();
    player.selection_rec = {
        player.hold_origin.x,
        player.hold_origin.y,
        player.hold_diff.x,
        player.hold_diff.y,
    };
    if (player.hold_diff.x < 0) {
        player.selection_rec.x = player.hold_origin.x + player.hold_diff.x;
        player.selection_rec.width *= -1;
    }
    if (player.hold_diff.y < 0) {
        player.selection_rec.y = player.hold_origin.y + player.hold_diff.y;
        player.selection_rec.height *= -1;
    }

    // print(player.selection_rec);
    auto world_coords = GetScreenToWorld2D((Vector2) {player.selection_rec.x, player.selection_rec.y}, player.camera);
    auto world_size = (Vector2) {player.selection_rec.width, player.selection_rec.height} * (1.0/player.camera.zoom);
    Rectangle selected_world_rect = {world_coords.x, world_coords.y, world_size.x, world_size.y};
    for (auto &card: cards) {
        card.selected = collide(card.body_rect, selected_world_rect);
    }
}

void player_write_palette_update(Player& player, Palette &palette) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        player.state = HOVERING;
        return;
    }
    if (IsMouseButtonPressed(0)) {
        player.state = HOVERING;
        return;
    }
    std::string& last_item = player.palette_edit_type == YES ? palette.yes.back() : palette.no.back();
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (last_item.size() > 1) last_item.pop_back();
        return;
    }
    auto char_pressed = GetCharPressed();
    if (char_pressed != 0) {
        last_item += (char) char_pressed;
    }
}

void player_write_focus_update(Player& player, Project& project) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (project.focus.text.empty()) project.focus.text = project.last_focus_text;
        player.state = HOVERING;
        return;
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (project.focus.text.size() > 0) project.focus.text.pop_back();
        return;
    }
    auto char_pressed = GetCharPressed();
    if (char_pressed != 0) project.focus.text += (char) char_pressed;
    return;
}

void player_write_big_picture_update(Player &player, Project &project) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (project.big_picture.empty()) project.big_picture = project.last_big_picture;
        player.state = HOVERING;
        return;
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (project.big_picture.size() > 0) project.big_picture.pop_back();
        return;
    }
    auto char_pressed = GetCharPressed();
    if (char_pressed != 0) project.big_picture += (char) char_pressed;
    return;
}

void player_select_scene_card_update(Player& player, std::vector<Card>& cards) {
    if (player.selected_card == NULL) return;
    if (IsKeyPressed(KEY_ESCAPE)) {
        player.selected_card = NULL;
        player.state = HOVERING;
        player.is_card_type_focus = false;
        return;
    }
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);

    if (IsMouseButtonPressed(0)) {
        Card *player_card_over = &cards[0]; // Card that the player is hovering over
        bool found_card = NULL;
        for (auto &card: cards) {
            if (CheckCollisionPointRec(position, card.body_rect)) {
                found_card = true;
                if (card.depth < player_card_over->depth && CheckCollisionPointRec(position, player_card_over->body_rect)) {
                    continue;
                }
                player_card_over = &card;
            }
        }

        if (!player_card_over) return;
        Defer {player.is_card_type_focus = false;};
        if (player_card_over->type != SCENE) {
            player.selected_card = NULL;
            player.state = HOVERING;
            return;
        }

        player_card_over->saved_dimensions.x = player_card_over->body_rect.width;
        player_card_over->saved_dimensions.y = player_card_over->body_rect.height;
        player_card_over->parent = player.selected_card;
        player.selected_card->cards_under.push_back(*player_card_over);
        player_card_over->deleted = true;
        player.selected_card = NULL;
        player.state = HOVERING;
    }

}

template <typename T>
void vec_move(std::vector<T>& v, size_t old_index, size_t new_index) {
    if (old_index > new_index) {
        std::rotate(v.rend() - old_index - 1, v.rend() - old_index, v.rend() - new_index);
    } else {
        std::rotate(v.begin() + old_index, v.begin() + old_index + 1, v.begin() + new_index + 1);
    }
}

void player_drawer_select_card_update(Player& player, Drawer& drawer, std::vector<Card>& cards) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        player.selected_card = NULL;
        player.state = HOVERING;
        drawer.open = false;
        drawer.cards = NULL;
        return;
    }

    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);

    Card *hovering_card = NULL;
    for (auto &card: *drawer.cards) {
        update_button_hover(card.move_up_button, mouse_position);
        update_button_hover(card.move_down_button, mouse_position);
        update_button_hover(card.remove_from_drawer_button, mouse_position);
        if (card.move_up_button.hover || card.move_down_button.hover || card.remove_from_drawer_button.hover) {
            hovering_card = &card;
        }
    }
    if (hovering_card == NULL) return;

    if (IsMouseButtonPressed(0)) {
        if (hovering_card->remove_from_drawer_button.hover) {
            hovering_card->in_drawer = false;
            hovering_card->parent = NULL;
            hovering_card->body_rect.width = hovering_card->saved_dimensions.x + GRIDSIZE / 2;
            hovering_card->body_rect.height = hovering_card->saved_dimensions.y + GRIDSIZE / 2;
            hovering_card->lock_target.x = player.selected_card->body_rect.x;
            hovering_card->lock_target.y = player.selected_card->body_rect.y;
            hovering_card->depth = player.selected_card->depth + 3;
            Card new_card = (*hovering_card);
            player.selected_card->cards_under.erase(std::remove(player.selected_card->cards_under.begin(), player.selected_card->cards_under.end(), *hovering_card), player.selected_card->cards_under.end());
            cards.push_back(new_card);
            return;
        } else if (hovering_card->move_up_button.hover) {
            size_t index = std::find(drawer.cards->begin(), drawer.cards->end(), *hovering_card) - drawer.cards->begin();
            if (index == 0) return;
            vec_move(*drawer.cards, index, index - 1);
        } else if (hovering_card->move_down_button.hover) {
            size_t index = std::find(drawer.cards->begin(), drawer.cards->end(), *hovering_card) - drawer.cards->begin();
            if (index == drawer.cards->size() - 1) return;
            vec_move(*drawer.cards, index, index + 1);
        }
    }
}

void player_search_update(Player& player, SearchBox& box) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        player.state = HOVERING;
        box.search.clear();
        box.visible = false;
        return;
    }
    
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (!box.search.empty()) box.search.pop_back();
    }
    auto char_pressed = GetCharPressed();
    if (char_pressed != 0) box.search += (char) char_pressed;
}

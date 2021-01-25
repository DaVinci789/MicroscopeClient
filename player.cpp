#include "card.hpp"
#include "common.hpp"
#include "player.hpp"
#include "networking.hpp"

Player init_player() {
    Player player;
    player.mouse_held  = false;

    auto position = GetMousePosition();
    player.player_rect = {position.x, position.y, 10, 10};
    player.mouse_position = position;
    player.camera_move_speed = 20;
    player.camera = {0};
    player.camera_target = {0, 0};
    player.camera.target = player.camera_target;
    player.zoom_level = 2;
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
    auto next_card = greatest_depth_and_furthest_along(cards);
    if (next_card) the_card.depth = next_card->depth + 1;
    else the_card.depth = 0;
    cards.push_back(the_card);
}

void player_update_camera(Player &player, bool allow_key_scroll) {
    #define ZOOM_SIZE 5
    float zoom_levels[ZOOM_SIZE] = {0.5, 0.75, 1, 1.5, 2};

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
    player_update_camera(player, false);
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (player.editing == NAME) {
            if (player.selected_card->name.empty()) player.selected_card->name = player.selected_card->last_name;
        }
        player.state = HOVERING;
        player.selected_card->grabbed = false;
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
    if (card->body_rect.width < 64) card->body_rect.width = 64;
    if (card->body_rect.height < 64) card->body_rect.height = 64;
}

// This is the main meat of the program.
void player_hover_update(Player& player, std::vector<Card>& cards, Palette& palette, Project &project) {
    auto mouse_position = GetMousePosition();
    auto position = GetScreenToWorld2D(mouse_position, player.camera);
    player.player_rect.x = position.x;
    player.player_rect.y = position.y;

    palette.open_button.hover = collide((Rectangle) {mouse_position.x, mouse_position.y, 10, 10}, palette.open_button.rect);
    update_button_hover(project.start_server, mouse_position);
    update_button_hover(project.start_client, mouse_position);

    // Mouse and Card Selection
    for (auto &card: cards) {
        if (palette.open_button.hover) break; // skip checking the cards if the players is hovering over the palette open thingie
        // Drag Card?
        if (IsMouseButtonPressed(0) && collide(player.player_rect, {card.body_rect.x, card.body_rect.y - 30, card.body_rect.width, card.body_rect.height + 30})) {
            player.mouse_held = true;

            // If the game has found a card for the player to hold and the next card in the unsorted array of cards is lower than that card, skip it. 
            if (player.selected_card) {
                if (card.depth < player.selected_card->depth) continue;
            }
            card.grabbed = true;
            player.offset = {player.player_rect.x - card.body_rect.x, player.player_rect.y - card.body_rect.y};
            player.selected_card = &card;
        }
        update_button_hover(card.close_button, position);
        update_button_hover(card.edit_button, position);
        update_button_hover(card.tone_button, position);
        update_button_hover(card.increase_font_button, position);
        update_button_hover(card.decrease_font_button, position);
    }

    if (IsMouseButtonPressed(0) && player.selected_card) {
        auto deepest_card = greatest_depth_and_furthest_along(cards);
        player.selected_card->depth = deepest_card->depth + 1;
        // Card button clicked!
        if (player.selected_card->close_button.hover) {
            player.selected_card->deleted = true;
            player.mouse_held = false;
            player.offset = {0 ,0};
        } else if (player.selected_card->edit_button.hover) {
            player.state = WRITING;
            player.editing = BODY;
            player.mouse_held = false;
            player.offset = {0 ,0};
        } else if (player.selected_card->tone_button.hover) {
            if (player.selected_card->color == WHITE) player.selected_card->color = BLACK;
            else player.selected_card->color = WHITE;
        } else if (player.selected_card->increase_font_button.hover) {
            FontSize *the_size = &player.selected_card->fontsize;
            switch (player.selected_card->fontsize) {
            case SMALL: 
                print(200);
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
        } else if (project.start_server.hover) {
            print(200);
            init_server();
            return;
        } else if (project.start_client.hover) {
            print(201);
            init_client();
            return;
        } else {
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

    // Delete cards
    if (IsKeyPressed(KEY_DELETE)) {
        for (auto &card: cards) {
            card.deleted = card.selected;
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

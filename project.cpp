#include "common.hpp"
#include "project.hpp"

void update_project(Project& project) {
    // Update focus
    auto focus_text = to_c_str(project.focus.text);
    auto focus_width = MeasureTextEx(application_font_regular, focus_text.data(), 36, 1.0).x;
    project.focus.rect = {(float) (GetScreenWidth() / 2.0 - (float) (focus_width / 2.0)) - 16, 0, focus_width + 32, 30};
    project.start_server.rect = {(float) (GetScreenWidth() - 32.0), 0, 32, 32};
    project.start_client.rect = {(float) (GetScreenWidth() - 64.0), 0, 32, 32};
}

Project init_project(std::string project_name) {
    Project project;
    project.big_picture = project_name;
    update_project(project); // sets focus_rect
    project.focus.text = "No Focus Set";
    project.last_focus_text = "No Focus Set";
    project.start_server = init_button({0});
    project.start_client = init_button({0});
    project.textures = &spritesheet;
    return project;
}

void draw(Project& project) {
    auto focus_text = to_c_str(project.focus.text);
    auto focus_width = MeasureTextEx(application_font_regular, focus_text.data(), 36, 1.0).x;
    float texture_y_offset = project.focus.hover ? 16 : 0;
    draw(project.focus);
    DrawTexturePro(*project.textures, (Rectangle) {0, 16 + texture_y_offset, 8, 16}, (Rectangle) {project.focus.rect.x - 8 * 3, 0, 8 * 3, 16 * 3}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*project.textures, (Rectangle) {8, 16 + texture_y_offset, 1, 16}, (Rectangle) {project.focus.rect.x, 0, focus_width + 24, 16 * 3}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTexturePro(*project.textures, (Rectangle) {9, 16 + texture_y_offset, 9, 16}, (Rectangle) {project.focus.rect.x + focus_width + 8 * 3, 0, 8 * 3, 16 * 3}, (Vector2) {0, 0}, 0.0, WHITE);
    DrawTextEx(application_font_regular, focus_text.data(), {(float (GetScreenWidth() / 2.0 - (float) (focus_width / 2.0))), 3.5}, 36, 1.0, BLACK);

    draw(project.start_server, ORANGE);
    draw(project.start_client, BLUE);
}

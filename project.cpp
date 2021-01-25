#include "common.hpp"
#include "project.hpp"

void update_project(Project& project) {
    // Update focus
    auto focus_text = to_c_str(project.focus.text);
    auto focus_width = MeasureTextEx(application_font_regular, focus_text.data(), 30, 1.0).x;
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
    return project;
}

void draw(Project& project) {
    auto focus_text = to_c_str(project.focus.text);
    auto focus_width = MeasureTextEx(application_font_regular, focus_text.data(), 30, 1.0).x;
    draw(project.focus);
    DrawTextEx(application_font_regular, focus_text.data(), {(float (GetScreenWidth() / 2.0 - (float) (focus_width / 2.0))), 0}, 30, 1.0, WHITE);

    draw(project.start_server, ORANGE);
    draw(project.start_client, BLUE);
}

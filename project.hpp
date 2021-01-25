#pragma once
#include "common.hpp"

struct Project {
    std::string big_picture;
    std::string last_focus_text;
    Button focus;
    Button start_server;
    Button start_client;
};

void update_project(Project& project);
Project init_project(std::string project_name);
void draw(Project& project);

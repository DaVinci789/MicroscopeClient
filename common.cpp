#include "common.hpp"

template<typename T>
T clamp(T value, T lower, T upper) {
    if (value < lower) return lower;
    else if (value > upper) return upper;
    else return value;
}

template int clamp<int>(int a, int b, int c);

template<typename T>
T lerp(T a, T b, float f)
{
    return a + f * (b - a);
}

template float lerp<float>(float a, float b, float c);

template<>
Vector2 lerp<Vector2>(Vector2 v1, Vector2 v2, float amount)
{
    Vector2 result = { 0 };

    result.x = v1.x + amount*(v2.x - v1.x);
    result.y = v1.y + amount*(v2.y - v1.y);

    return result;
}

Vector2 floor(Vector2 vector) {
    return (Vector2) {(float) floor(vector.x), (float) floor(vector.y)};
}

std::vector<char> to_c_str(std::string string) {
    auto the_string = std::vector<char>();
    for (auto &character: string) {
        the_string.push_back((char) character);
    }
    the_string.push_back((char) '\0');
    return the_string;
}

bool operator==(Color lh, Color rh) {
    return lh.r == rh.r && lh.g == rh.g && lh.b == rh.b && lh.a == rh.a;
}

Vector2 operator-(Vector2 v1, Vector2 v2) {
    Vector2 result = { v1.x - v2.x, v1.y - v2.y };
    return result;
}

Vector2 operator+(Vector2 v1, Vector2 v2) {
    Vector2 result = { v1.x + v2.x, v1.y + v2.y };
    return result;
}

Vector2 operator/(Vector2 v1, Vector2 v2) {
    Vector2 result = { v1.x / v2.x, v1.y / v2.y };
    return result;
}

Vector2 operator*(Vector2 v, float f) {
    return (Vector2) {v.x * f, v.y * f};
}

bool collide(Rectangle rect1, Rectangle rect2) {
    return CheckCollisionRecs(rect1, rect2);
}

void print(bool the_bool) {
    std::cout << (the_bool ? "true" : "false") << std::endl;
}

void print(Vector2 vector) {
    std::cout << "(" << vector.x << " " << vector.y << ")" << std::endl;
}

void print(int n) {
    std::cout << n << std::endl;
}

void print(float n) {
    std::cout << n << std::endl;
}

void print(Rectangle rect) {
    std::cout << "(" << rect.x << " " << rect.y << " " << rect.width << " " << rect.height << ")" << std::endl;
}

Vector2 get_mouse_delta() {
    Vector2 vec = GetMousePosition() - previous_mouse_position;
    return vec;
}

Vector2 lock_position_to_grid(Vector2 position) {
    return (Vector2) {
        (float) round(position.x / (float) GRIDSIZE) * (float) GRIDSIZE,
        (float) round(position.y / (float) GRIDSIZE) * (float) GRIDSIZE
    };
}

Vector2 get_world_mouse_position(Camera2D camera) {
    auto position = camera.target;
    auto mouse_position = GetMousePosition();
    mouse_position.x -= (float) GetScreenWidth() / 2.0;
    mouse_position.y -= (float) GetScreenHeight() / 2.0;
    mouse_position.x /= camera.zoom;
    mouse_position.y /= camera.zoom;

    position.x += mouse_position.x;
    position.y += mouse_position.y;

    return position;
}

Button init_button(Rectangle button_rect, std::string button_text, Texture texture) {
    Button button;
    button.rect = button_rect;
    button.hover = false;
    button.text = button_text;
    button.texture = texture;
    button.font = &application_font_regular;
    return button;
}

void update_button_hover(Button& button, Vector2 position) {
    button.hover = CheckCollisionPointRec(position, button.rect);
}

void draw(Button &button, Color depressed, Color pressed) {
    DrawRectangleRec(button.rect, button.hover ? pressed : depressed);
}

void draw_texture_rect_scaled(Texture2D texture, Rectangle texture_source, Vector2 where, Vector2 stretch, int scale) {
    DrawTexturePro(spritesheet, texture_source, {where.x, where.y, texture_source.width * scale + stretch.x * scale, texture_source.height * scale + stretch.y * scale}, {0, 0}, 0.0, WHITE);
}

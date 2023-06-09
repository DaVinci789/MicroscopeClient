#ifndef COMMON_H
#define COMMON_H
#include <raylib.h>

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>

#define CARDWHITE (Color) { 248, 248, 248, 255 }
#define CARDBLACK (Color) { 56, 56, 84, 255 }

template<typename F>
class defer_finalizer {
    F f;
    bool moved;
  public:
    template<typename T>
    defer_finalizer(T && f_) : f(std::forward<T>(f_)), moved(false) { }

    defer_finalizer(const defer_finalizer &) = delete;

    defer_finalizer(defer_finalizer && other) : f(std::move(other.f)), moved(other.moved) {
        other.moved = true;
    }

    ~defer_finalizer() {
        if (!moved) f();
    }
};

struct {
    template<typename F>
    defer_finalizer<F> operator<<(F && f) {
        return defer_finalizer<F>(std::forward<F>(f));
    }
} deferrer;

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define Defer auto TOKENPASTE2(__deferred_lambda_call, __COUNTER__) = deferrer << [&]

template<typename T>
T clamp(T value, T lower, T upper);

template<typename T>
T lerp(T a, T b, float f);

template<>
Vector2 lerp<Vector2>(Vector2 v1, Vector2 v2, float amount);

Vector2 floor(Vector2 vector);

std::vector<char> to_c_str(std::string string);

bool operator==(Color lh, Color rh);
Vector2 operator-(Vector2 v1, Vector2 v2);
Vector2 operator+(Vector2 v1, Vector2 v2);
Rectangle operator+(Rectangle r1, Vector2 v2);
Vector2 operator/(Vector2 v1, Vector2 v2);
Vector2 operator*(Vector2 v, float f);

Vector2 to_vector(Rectangle rect);

template<typename T>
T center(T large, T small) {
    return (large / small) / 2.0;
}

extern Texture2D spritesheet;
extern Shader darken_shader;

void set_darkness_shader_amount(float amount = 2.0);

bool collide(Rectangle rect1, Rectangle rect2);

void print(bool the_bool);
void print(Vector2 vector);
void print(int n);
void print(float n);
void print(Rectangle rect);

#define FONTSIZE_BASE 72.0
#define FONTSIZE_SMALL 24.0
#define FONTSIZE_REGULAR 32.0
#define FONTSIZE_LARGE 48.0

extern Font application_font_small;
extern Font application_font_regular;
extern Font application_font_large;

inline int get_font_size(Font *font) {
    if (font == &application_font_small) {
        return FONTSIZE_SMALL;
    } else if (font == &application_font_regular) {
        return FONTSIZE_REGULAR;
    } else {
        return FONTSIZE_LARGE;
    }
}

Vector2 get_world_mouse_position(Camera2D camera);

#define GRIDSIZE 16

extern Vector2 previous_mouse_position;

Vector2 get_mouse_delta();
Vector2 lock_position_to_grid(Vector2 position);

struct Button {
    Rectangle rect;
    bool hover;
    std::string text;
    Texture texture;
    Font *font;
};

Button init_button(Rectangle button_rect = {0}, std::string button_text = "", Texture texture = {0});
void update_button_hover(Button& button, Vector2 position);
void update_button_hover(const std::vector<Vector2>& transform_stack, Button& button, Vector2 position);
void draw(Button &button, Color depressed = GRAY, Color pressed = PURPLE);
void draw(const std::vector<Vector2>& transform_stack, Button &button, Color depressed = GRAY, Color pressed = PURPLE);

void draw_pixel_rect(Rectangle rec, float border_size = 3.0, Color fill = WHITE, Color border = BLACK);

void draw_texture_rect_scaled(Texture2D texture, Rectangle texture_source, Vector2 where, Vector2 stretch = {0}, int scale = 3);

void draw_text_rec_justified(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);
void draw_text_rec_ex_justified(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint);

void draw_text_bubble(bool on, std::string text, Vector2 where);
#endif

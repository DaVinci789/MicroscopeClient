#ifndef COMMON_H
#define COMMON_H
#include <raylib.h>

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
T clamp(T value, T lower, T upper) {
    if (value < lower) return lower;
    else if (value > upper) return upper;
    else return value;
}

template<typename T>
T lerp(T a, T b, float f)
{
    return a + f * (b - a);
}

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

template<typename T>
T center(T large, T small) {
    return (large / small) / 2.0;
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

#define GRIDSIZE 16

extern Vector2 previous_mouse_position;

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
#endif

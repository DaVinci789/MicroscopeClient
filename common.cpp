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

Vector2 to_vector(Rectangle rect) {
    return {rect.x, rect.y};
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

void draw_text_rec_justified(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint) {
    draw_text_rec_ex_justified(font, text, rec, fontSize, spacing, wordWrap, tint, 0, 0, WHITE, WHITE);
}

// Draw text using font inside rectangle limits with support for text selection
/// EDITED: Justifies text
void draw_text_rec_ex_justified(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint) {
    int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;            // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;     // Character quad scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    int textToDrawWidth = 0;

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetNextCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        int glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.chars[index].advanceX == 0)?
                         (int)(font.recs[index].width*scaleFactor + spacing):
                         (int)(font.chars[index].advanceX*scaleFactor + spacing);
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            textToDrawWidth = 0;
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) {
                endLine = i;
            }

            if ((textOffsetX + glyphWidth + 1) >= rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;

                state = !state;
            }
            else if (codepoint == '\n') {
                state = !state;
            }

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;

                // Get width of text we're going to render.
                for (int justify_index = i; justify_index < endLine; justify_index++) {
                    int _codepointByteCount = 0;
                    int _codepoint = GetNextCodepoint(&text[justify_index], &_codepointByteCount);
                    int _index = GetGlyphIndex(font, _codepoint);

                    int _glyphWidth = 0;
                    if (_codepoint != '\n')
                    {
                        _glyphWidth = (font.chars[_index].advanceX == 0)?
                            (int)(font.recs[_index].width*scaleFactor + spacing):
                            (int)(font.chars[_index].advanceX*scaleFactor + spacing);
                    }
                    textToDrawWidth += _glyphWidth;
                }
            }
        }
        else
        {
            if (codepoint == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth + 1) >= rec.width))
                {
                    textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                    textOffsetX = 0;
                }

                // When text overflows rectangle height limit, just stop drawing
                if ((textOffsetY + (int)(font.baseSize*scaleFactor)) > rec.height) break;

                // Draw selection background
                bool isGlyphSelected = false;
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
                {
                    DrawRectangleRec((Rectangle){ rec.x + textOffsetX - 1, rec.y + textOffsetY, (float)glyphWidth, (float)font.baseSize*scaleFactor }, selectBackTint);
                    isGlyphSelected = true;
                }

                // Draw current character glyph
                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint, (Vector2){ rec.x + textOffsetX + ((rec.width - textToDrawWidth) / 2.0), rec.y + textOffsetY }, fontSize, isGlyphSelected? selectTint : tint);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (int)((font.baseSize + font.baseSize/2)*scaleFactor);
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                selectStart += lastk - k;
                k = lastk;

                state = !state;
            }
        }
        textOffsetX += glyphWidth;
    }
}

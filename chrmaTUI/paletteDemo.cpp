#include "chrmaTUI.hpp"
#include <chrono>
#include <cmath>
#include <algorithm>

int main() {
    TUImanager tui;

    // Palette (from Waybar CSS)
    auto make = [](uint8_t r, uint8_t g, uint8_t b, uint8_t a) -> color { return color{r,g,b,a}; };
    const color DARK1 = make(0x29, 0x28, 0x31, 255);
    const color DARK2 = make(0x1e, 0x1e, 0x2e, 255);
    const color GRAY  = make(0x45, 0x47, 0x5a, 255);
    const color TEAL  = make(0x4a, 0x7a, 0x96, 255);
    const color PEACH = make(0xfb, 0xbb, 0xad, 255);
    const color PINK  = make(0xee, 0x86, 0x95, 255);

    auto lerp8 = [](uint8_t a, uint8_t b, float t) -> uint8_t {
        return static_cast<uint8_t>(a + (b - a) * t);
    };
    auto mix = [&](const color& a, const color& b, float t) -> color {
        return make(lerp8(a.r, b.r, t), lerp8(a.g, b.g, t), lerp8(a.b, b.b, t), lerp8(a.a, b.a, t));
    };

    auto startTime = std::chrono::high_resolution_clock::now();
    char c;
    while (!tui.windowShouldClose()) {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = now - startTime;
        float time = elapsed.count();

        const int rows = tui.rows;
        const int cols = tui.cols;

        // 1) Background vertical gradient animated over time
        for (int y = 0; y < rows; ++y) {
            float ty = rows > 1 ? static_cast<float>(y) / static_cast<float>(rows - 1) : 0.0f;
            float shift = std::sin(time * 0.5f + ty * 3.14159f) * 0.5f + 0.5f;
            color bg = mix(DARK1, DARK2, shift);
            for (int x = 0; x < cols; ++x) {
                tui.drawCharacter( {' ', bg, GRAY}, x, y);
            }
        }

        // 2) Accent stripes animate horizontally
        if (rows >= 5) {
            int topY = 1;
            int botY = rows - 2;
            for (int x = 0; x < cols; ++x) {
                float tx = cols > 1 ? static_cast<float>(x) / static_cast<float>(cols - 1) : 0.0f;
                float cycle = std::sin(time * 1.0f + tx * 6.28318f) * 0.5f + 0.5f;
                color topCol = mix(TEAL, PEACH, cycle);
                color botCol = mix(PEACH, PINK, cycle);
                tui.drawCharacter( {' ', topCol, topCol}, x, topY);
                tui.drawCharacter( {' ', botCol, botCol}, x, botY);
            }
        }

        // 3) Centered title text with per-letter vertical oscillation and gradient fg
        std::string title = "chrmaTUI <3 palette demo";
        int baseY = rows / 2;
        int amplitude = std::max(1, rows / 30); // small oscillation relative to screen
        float speed = 2.0f;      // oscillation speed
        float phase = 0.2f;      // phase shift per character (radians)
        int startX = cols / 2 - static_cast<int>(title.size()) / 2;
        for (size_t i = 0; i < title.size(); ++i) {
            int x = startX + static_cast<int>(i);
            if (x < 0 || x >= cols) continue;
            int y = baseY + static_cast<int>(std::round(std::sin(time * speed + static_cast<float>(i) * phase) * amplitude));
            if (y < 0 || y >= rows) continue;
            float t = title.size() > 1 ? static_cast<float>(i) / static_cast<float>(title.size() - 1) : 0.0f;
            color fg = mix(PEACH, PINK, t);
              tui.drawCharacter({ title[i], fg, GRAY}, x, y);
        }

        tui.drawString(
            "Press 'q' to quit", PEACH, {0,0,0,0},
            std::max(0, cols - 17),
            std::max(0, rows - 1)
        );

        // Render the frame
        tui.render();

    }

    // Restore cursor and clear
    std::cout << "\x1b[?25h\x1b[0m\x1b[2J\x1b[H" << std::flush;
    return 0;
}
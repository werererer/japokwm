#ifndef COLOR_H
#define COLOR_H

struct color {
    float red;
    float green;
    float blue;
    float alpha;
};

void color_to_wlr_color(float *dest_wlr_color, struct color color);

#define BLACK (struct color) {.red = 0.0f, .green = 0.0f, .blue = 0.0f, .alpha = 1.0f}
#define WHITE (struct color) {.red = 1.0f, .green = 1.0f, .blue = 1.0f, .alpha = 1.0f}
#define RED (struct color) {.red = 1.0f, .green = 0.0f, .blue = 0.0f, .alpha = 1.0f}
#define GREEN (struct color) {.red = 0.0f, .green = 1.0f, .blue = 0.0f, .alpha = 1.0f}
#define BLUE (struct color) {.red = 0.0f, .green = 0.0f, .blue = 1.0f, .alpha = 1.0f}

#endif /* COLOR_H */

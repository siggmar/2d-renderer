#ifndef R2D_H
#define R2D_H

#include <stdint.h>

typedef struct {
    int width;
    int height;
    uint32_t *pixel_buf;
} R2D_State;

typedef struct {
    int x, y;
} R2D_Point;

typedef struct {
    float x, y;
} R2D_vec2;

static inline uint32_t r2d_random_color();

static inline void r2d_clear_buffer(R2D_State *state, uint32_t color);

static inline void r2d_set_pixel(R2D_State *state, R2D_Point p, uint32_t color);

void r2d_draw_vline(R2D_State *state, R2D_Point p, int length, uint32_t color);
void r2d_draw_hline(R2D_State *state, R2D_Point p, int length, uint32_t color);
void r2d_draw_line(
    R2D_State *state, R2D_Point p1, R2D_Point p2, uint32_t color
);

void r2d_draw_rect(R2D_State *state, R2D_Point p, int w, int h, uint32_t color);
void r2d_draw_filled_rect(
    R2D_State *state, R2D_Point p, int w, int h, uint32_t color
);

void r2d_draw_circle(R2D_State *state, R2D_Point p, int r, uint32_t color);
void r2d_draw_filled_circle(
    R2D_State *state, R2D_Point p, int r, uint32_t color
);

// Basic colors

#define WHITE 0xFFFFFFFF
#define BLACK 0x000000FF
#define RED 0xFF0000FF
#define GREEN 0x00FF00FF
#define BLUE 0x0000FFFF
#define YELLOW 0xFFFF00FF
#define PURPLE 0xFF00FFFF
#define CYAN 0x00FFFFFF

#define DARKPRUPLE 0x5E145CFF // cool background

// Implementaion

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline uint32_t r2d_random_color() {
    return 0xFF000000 | (rand() & 0xFFFFFF);
}

// TODO make it faster, update only changed regions oder so
static inline void r2d_clear_buffer(R2D_State *state, uint32_t color) {
    if (color == 0) {
        memset(
            state->pixel_buf,
            0,
            state->width * state->height * sizeof(uint32_t)
        );
    }
    for (int i = 0; i < state->width * state->height; i++) {
        state->pixel_buf[i] = color;
    }
}

static inline void
r2d_set_pixel(R2D_State *state, R2D_Point p, uint32_t color) {
#ifndef R2D_NO_CLIPPING
    if (p.x < 0 || p.x >= state->width || p.y < 0 || p.y >= state->height) {
        return;
    }
#endif

    state->pixel_buf[p.y * state->width + p.x] = color;
}

void r2d_draw_vline(R2D_State *state, R2D_Point p, int length, uint32_t color) {

    if (length == 0) return;

    int x = p.x;
    if (x < 0 || x >= state->width) return;

    int y0 = p.y;
    int y1 = p.y + length;

    // normalize
    if (y0 > y1) {
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    // clip
    if (y0 < 0) y0 = 0;
    if (y1 > state->height) y1 = state->height;

    if (y0 >= y1) return;

    // draw
    for (int y = y0; y < y1; y++) {
        r2d_set_pixel(state, (R2D_Point){x, y}, color);
    }
}

void r2d_draw_hline(R2D_State *state, R2D_Point p, int length, uint32_t color) {
    if (length == 0) return;

    int y = p.y;
    if (y < 0 || y >= state->height) return;

    int x0 = p.x;
    int x1 = p.x + length;

    // normalize
    if (x0 > x1) {
        int tmp = x0;
        x0 = x1;
        x1 = tmp;
    }

    // clip
    if (x0 < 0) x0 = 0;
    if (x1 > state->height) x1 = state->height;

    if (x0 >= x1) return;

    // draw
    for (int x = x0; x < x1; x++) {
        r2d_set_pixel(state, (R2D_Point){x, y}, color);
    }
}

void r2d_draw_line(
    R2D_State *state, R2D_Point p1, R2D_Point p2, uint32_t color
) {
    // Versteh ich absolut nicht
    // y = m*x + b

    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;

    if (dx == 0) {
        r2d_draw_vline(state, p1, dy, color);
        return;
    }

    if (dy == 0) {
        r2d_draw_hline(state, p1, dx, color);
        return;
    }

    float m = (float)dy / (float)dx;
    float b = p1.y - m * p1.x;

    if (p1.x >= p2.x) {
        int tmp = p1.x;
        p1.x = p2.x;
        p2.x = tmp;
    }
    if (p1.y >= p2.y) {
        int tmp = p1.y;
        p1.y = p2.y;
        p2.y = tmp;
    }

    if (abs(dx) >= abs(dy)) {
        for (int x = p1.x; x <= p2.x; x++) {
            int y = round(m * x + b);
            R2D_Point n = {x, y};

            r2d_set_pixel(state, n, color);
        }
    } else {
        for (int y = p1.y; y <= p2.y; y++) {
            int x = round((y - b) / m);
            R2D_Point n = {x, y};

            r2d_set_pixel(state, n, color);
        }
    }
}

void r2d_draw_rect(
    R2D_State *state, R2D_Point p, int w, int h, uint32_t color
) {
    r2d_draw_vline(state, p, h, color); // left
    r2d_draw_hline(state, p, w, color); // top

    R2D_Point nv = {p.x + w - 1, p.y};
    R2D_Point nh = {p.x, p.y + h - 1};
    r2d_draw_vline(state, nv, h, color); // right
    r2d_draw_hline(state, nh, w, color); // bottom
}

void r2d_draw_filled_rect(
    R2D_State *state, R2D_Point p, int w, int h, uint32_t color
) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {

            R2D_Point n = {j + p.x, i + p.y};
            r2d_set_pixel(state, n, color);
        }
    }
}

void r2d_draw_circle(R2D_State *state, R2D_Point p, int r, uint32_t color) {
    // x² + y² = r²

    for (int angle = 0; angle < 360; angle++) {
        int x = p.x + r * cos(angle);
        int y = p.y + r * sin(angle);

        R2D_Point n = {x, y};

        r2d_set_pixel(state, n, color);
    }
}

#endif

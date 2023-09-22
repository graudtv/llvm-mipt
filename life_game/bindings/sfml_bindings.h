#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef char sf_bool_t;
typedef unsigned sf_color_t;

#define SF_COLOR(r, g, b, a) ((r) << 24 | (g) << 16 | (b) << 8 | (a))

enum sf_shape_t {
  SF_TRIANGLE_SHAPE = 3,
  SF_RECTANGLE_SHAPE = 4,
  SF_CIRCLE_SHAPE = 32
};

void sf_init_window(unsigned win_width, unsigned win_height, unsigned grid_width, unsigned grid_height);
sf_bool_t sf_handle_events();
void sf_draw_pixel(unsigned x, unsigned y, sf_color_t color, enum sf_shape_t shape);
void sf_clear(sf_color_t color);
void sf_display();

#ifdef __cplusplus
}
#endif

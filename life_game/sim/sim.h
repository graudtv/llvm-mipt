#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define SIM_SCREEN_WIDTH 200
#define SIM_SCREEN_HEIGHT 100

typedef unsigned sim_color_t;

#define SIM_RGBA(r, g, b, a) ((r) << 24 | (g) << 16 | (b) << 8 | (a))
#define SIM_RGB(r, g, b) SIM_RGBA(r, g, b, 255)

enum sim_shape_t {
  SIM_TRIANGLE_SHAPE = 3,
  SIM_RECTANGLE_SHAPE = 4,
  SIM_CIRCLE_SHAPE = 32
};

/* Clear hidden buffer */
void sim_clear(sim_color_t color);
/* Set pixel in hidden buffer */
void sim_set_pixel(unsigned x, unsigned y, sim_color_t color, enum sim_shape_t shape);
/* Swap buffers */
void sim_display();

#ifdef __cplusplus
}
#endif

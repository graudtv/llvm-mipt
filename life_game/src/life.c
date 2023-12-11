#include <sim.h>

#define MAP_WIDTH SIM_SCREEN_WIDTH
#define MAP_HEIGHT SIM_SCREEN_HEIGHT

#define BACKGROUND_COLOR SIM_RGB(128, 128, 128)
#define PIXEL_SHAPE SIM_CIRCLE_SHAPE
#define INITIAL_DENSITY_RATIO 5

#define ENABLE_STATIC_MODE 0

typedef struct {
  unsigned char is_alive[MAP_HEIGHT][MAP_WIDTH];
  unsigned char neighbour_count[MAP_HEIGHT][MAP_WIDTH];
} map_t;

void draw_map(const map_t *m) {
  // clang-format off
  const sim_color_t color_scheme[9] = {
    SIM_RGB(240, 223, 36),
    SIM_RGB(98, 240, 36),
    SIM_RGB(36, 240, 214),
    SIM_RGB(36, 194, 240),
    SIM_RGB(36, 50, 240),
    SIM_RGB(194, 236, 240),
    SIM_RGB(240, 36, 28),
    SIM_RGB(240, 60, 36),
    SIM_RGB(240, 122, 36),
  };
  // clang-format on
  for (int i = 0; i < MAP_HEIGHT; ++i)
    for (int j = 0; j < MAP_WIDTH; ++j)
      if (m->is_alive[i][j])
        sim_set_pixel(j, i, color_scheme[m->neighbour_count[i][j]],
                      PIXEL_SHAPE);
}

void update_neighbour_count(map_t *m) {
  for (int i = 0; i < MAP_HEIGHT; ++i)
    for (int j = 0; j < MAP_WIDTH; ++j) {
      unsigned left = (j + MAP_WIDTH - 1) % MAP_WIDTH;
      unsigned right = (j + 1) % MAP_WIDTH;
      unsigned top = (i + MAP_HEIGHT - 1) % MAP_HEIGHT;
      unsigned bottom = (i + 1) % MAP_HEIGHT;
      m->neighbour_count[i][j] =
          m->is_alive[top][left] + m->is_alive[top][j] +
          m->is_alive[top][right] + m->is_alive[i][left] +
          m->is_alive[i][right] + m->is_alive[bottom][left] +
          m->is_alive[bottom][j] + m->is_alive[bottom][right];
    }
}

void init_map(map_t *m) {
  for (int i = 0; i < MAP_HEIGHT; ++i)
    for (int j = 0; j < MAP_WIDTH; ++j)
      m->is_alive[i][j] = (unsigned)sim_rand() % INITIAL_DENSITY_RATIO == 0;
  update_neighbour_count(m);
}

void update_map(map_t *m) {
  for (int i = 0; i < MAP_HEIGHT; ++i)
    for (int j = 0; j < MAP_WIDTH; ++j) {
      m->is_alive[i][j] =
          (m->is_alive[i][j] && (m->neighbour_count[i][j] == 2)) ||
          (m->neighbour_count[i][j] == 3);
    }
  update_neighbour_count(m);
}

int main() {
  map_t map;
  init_map(&map);
  while (1) {
    sim_clear(BACKGROUND_COLOR);
    draw_map(&map);
    sim_display();
#if !ENABLE_STATIC_MODE
    update_map(&map);
#endif
  }
}

#include <sfml_bindings.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400
#define MAP_WIDTH 200
#define MAP_HEIGHT 100

#define BACKGROUND_COLOR SF_COLOR(128, 128, 128, 255)
#define PIXEL_SHAPE SF_CIRCLE_SHAPE
#define INITIAL_DENSITY_RATIO 5

#define ENABLE_STATIC_MODE 0

// clang-format off
sf_color_t color_scheme[9] = {
  SF_COLOR(240, 223, 36, 255),
  SF_COLOR(98, 240, 36, 255),
  SF_COLOR(36, 240, 214, 255),
  SF_COLOR(36, 194, 240, 255),
  SF_COLOR(36, 50, 240, 255),
  SF_COLOR(194, 236, 240, 255),
  SF_COLOR(240, 36, 28, 255),
  SF_COLOR(240, 60, 36, 255),
  SF_COLOR(240, 122, 36, 255),
};
// clang-format on

unsigned char map[MAP_WIDTH][MAP_HEIGHT];
unsigned char neighbour_count[MAP_WIDTH][MAP_HEIGHT];

int rand_int() {
  /* https://stackoverflow.com/a/3062783 */
  static int seed = 123456789;
  return seed = (1103515245 * seed + 12345) % (1 << 31);
}

void draw_map() {
  for (int i = 0; i < MAP_HEIGHT; ++i)
    for (int j = 0; j < MAP_WIDTH; ++j)
      if (map[i][j])
        sf_draw_pixel(j, i, color_scheme[neighbour_count[i][j]], PIXEL_SHAPE);
}

void update_neighbour_count() {
  for (int i = 0; i < MAP_HEIGHT; ++i)
    for (int j = 0; j < MAP_WIDTH; ++j) {
      unsigned left = (j + MAP_WIDTH - 1) % MAP_WIDTH;
      unsigned right = (j + 1) % MAP_WIDTH;
      unsigned top = (i + MAP_HEIGHT - 1) % MAP_HEIGHT;
      unsigned bottom = (i + 1) % MAP_HEIGHT;
      neighbour_count[i][j] = map[top][left] + map[top][j] + map[top][right] +
                              map[i][left] + map[i][right] + map[bottom][left] +
                              map[bottom][j] + map[bottom][right];
    }
}

void init_map() {
  for (int i = 0; i < MAP_HEIGHT; ++i)
    for (int j = 0; j < MAP_WIDTH; ++j)
      map[i][j] = rand_int() % INITIAL_DENSITY_RATIO == 0;
  update_neighbour_count();
}

void update_map() {
  for (int i = 0; i < MAP_HEIGHT; ++i)
    for (int j = 0; j < MAP_WIDTH; ++j) {
      map[i][j] = (map[i][j] && (neighbour_count[i][j] == 2)) ||
                  (map[i][j] && (neighbour_count[i][j] == 3)) ||
                  (!map[i][j] && (neighbour_count[i][j] == 3));
    }
  update_neighbour_count();
}

int main() {
  init_map();
  sf_init_window(WINDOW_WIDTH, WINDOW_HEIGHT, MAP_WIDTH, MAP_HEIGHT);
  while (sf_handle_events()) {
    sf_clear(BACKGROUND_COLOR);
    draw_map();
    sf_display();
#if !ENABLE_STATIC_MODE
    update_map();
#endif
  }
}

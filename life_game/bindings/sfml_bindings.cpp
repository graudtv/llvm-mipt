#include "sfml_bindings.h"
#include <SFML/Graphics.hpp>

static_assert(sizeof(sf_color_t) == 4, "type mismatch");

static sf::RenderWindow Window;

void sf_init_window(unsigned win_width, unsigned win_height,
                    unsigned grid_width, unsigned grid_height) {
  Window.create(sf::VideoMode{win_width, win_height}, "Demo Window");
  Window.setFramerateLimit(120);
  sf::View V{sf::FloatRect(0, 0, grid_width, grid_height)};
  Window.setView(V);
}

/* returns true if window is open */
sf_bool_t sf_handle_events() {
  sf::Event E;
  while (Window.pollEvent(E))
    if (E.type == sf::Event::Closed)
      Window.close();
  return Window.isOpen();
}

void sf_draw_pixel(unsigned x, unsigned y, sf_color_t color, sf_shape_t shape) {
  sf::CircleShape Pixel(0.5, shape);
  Pixel.setPosition(x, y);
  Pixel.setFillColor(sf::Color(color));
  Window.draw(Pixel);
}

void sf_clear(sf_color_t color) { Window.clear(sf::Color{color}); }
void sf_display() { Window.display(); }

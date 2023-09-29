#include "sim.h"
#include <SFML/Graphics.hpp>
#include <cstdlib> // exit()

#define SIM_WINDOW_WIDTH 800
#define SIM_WINDOW_HEIGHT 400

static_assert(sizeof(sim_color_t) == 4, "type mismatch");

static sf::RenderWindow Window;

static void init_window() {
  Window.create(sf::VideoMode{SIM_WINDOW_WIDTH, SIM_WINDOW_HEIGHT},
                "Demo Window");
  Window.setFramerateLimit(120);
  sf::View V{sf::FloatRect(0, 0, SIM_SCREEN_WIDTH, SIM_SCREEN_HEIGHT)};
  Window.setView(V);
}

static void handle_events() {
  sf::Event E;
  while (Window.pollEvent(E))
    if (E.type == sf::Event::Closed)
      exit(0);
}

void sim_set_pixel(unsigned x, unsigned y, sim_color_t color,
                   sim_shape_t shape) {
  sf::CircleShape Pixel(0.5, shape);
  Pixel.setPosition(x, y);
  Pixel.setFillColor(sf::Color(color));
  Window.draw(Pixel);
}

void sim_clear(sim_color_t color) {
  if (!Window.isOpen())
    init_window();
  Window.clear(sf::Color{color});
}

void sim_display() {
  Window.display();
  handle_events();
}

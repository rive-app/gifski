#include "gifski.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <cassert>

// build command:
// clang++ -std=c++11 main.cpp -L../target/release -lgifski -I..

void set_pixel(std::vector<unsigned char> &buffer, int x, int y, int width, const RGB8 &color)
{
  if (x < 0 || x >= width || y < 0 || y >= width)
  {
    return;
  }
  int index = (y * width + x) * 4;
  buffer[index] = color.r;
  buffer[index + 1] = color.g;
  buffer[index + 2] = color.b;
  buffer[index + 3] = 255; // Opaque.
}

void draw_line(std::vector<unsigned char> &buffer, int x0, int y0, int x1, int y1, int width, const RGB8 &color)
{
  int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (true)
  {
    set_pixel(buffer, x0, y0, width, color);
    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 >= dy)
    {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

void draw_triangle(std::vector<unsigned char> &buffer, int width, const RGB8 &color)
{
  int height = width; // Assuming square buffer for simplicity
  int x0 = width / 2, y0 = height / 10;
  int x1 = width / 10, y1 = height - height / 10;
  int x2 = width - width / 10, y2 = height - height / 10;

  // Draw triangle edges
  draw_line(buffer, x0, y0, x1, y1, width, color);
  draw_line(buffer, x1, y1, x2, y2, width, color);
  draw_line(buffer, x2, y2, x0, y0, width, color);

  // Simple scan-line fill
  for (int y = y1; y < y2; y++)
  {
    int minX = width, maxX = 0;
    for (int x = 0; x < width; x++)
    {
      int index = (y * width + x) * 4;
      if (buffer[index + 3] == 255)
      {
        if (x < minX)
        {
          minX = x;
        }
        if (x > maxX)
        {
          maxX = x;
        }
      }
    }
    if (maxX >= minX)
    {
      for (int x = minX; x <= maxX; x++)
      {
        set_pixel(buffer, x, y, width, color);
      }
    }
  }
}

int main()
{
  RGB8 *matte_color = new RGB8;
  matte_color->r = 255;
  matte_color->g = 0;
  matte_color->b = 170;
  GifskiSettings settings = {256, 256, 100, false, 0, matte_color};

  gifski *g = gifski_new(&settings);
  if (!g)
  {
    std::cerr << "Failed to initialize gifski encoder" << std::endl;
    return 1;
  }
  RGB8 *matte = gifski_get_matte(g);
  assert(matte->r == 255);
  assert(matte->g == 0);
  assert(matte->b == 170);
  gifski_set_file_output(g, "test_output.gif");

  std::vector<RGB8> colors = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}, {255, 0, 255}, {0, 255, 255}};
  for (int i = 0; i < colors.size(); i++)
  {
    std::vector<unsigned char> buffer(settings.width * settings.height * 4, 0); // Reset buffer for each frame
    draw_triangle(buffer, settings.width, colors[i]);
    gifski_add_frame_rgba(g, i, settings.width, settings.height, buffer.data(), i * 0.1);
  }

  gifski_finish(g);

  std::cout << "GIF created successfully with animated colors and matte." << std::endl;

  delete matte_color;
  return 0;
}
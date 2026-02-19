#pragma once
#include "geom.hpp"
#include <vector>

struct Color {
  unsigned char r, g, b;
};

void set_model(Vec3 pos, Vec3 rot, Vec3 scale);
void look_at(Vec3 eye, Vec3 target, Vec3 up);
void set_perspective(float near, float far, float aspect_ratio, float fov);
void rasterize(Vec4 v[3], Vec3 vn[3], std::vector<Color> &frame, int width, float hw,
               int height, float hh, const Color &basecolor);
Vec4 clip(const Vec3 &vertex);
void reset_z_buffer(size_t size);
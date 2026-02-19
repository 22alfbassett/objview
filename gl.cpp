#include "gl.hpp"
#include "Model.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

static Mat4 M = IDENTITY_MAT4, V = IDENTITY_MAT4, P = IDENTITY_MAT4,
            PVM = IDENTITY_MAT4;

static inline void recalculate_mvp() { PVM = P * V * M; }

void set_model(Vec3 pos, Vec3 rot, Vec3 scale) {
  float theta = rot.x;
  float phi = rot.y;
  float rho = rot.z;
  // clang-format off
  Mat4 Scale = {
    scale.x, 0, 0, 0,
    0, scale.y, 0, 0,
    0, 0, scale.z, 0,
    0, 0, 0, 1
  };
  Mat4 RotTheta = {
    cosf(theta), 0, sinf(theta), 0,
    0, 1, 0, 0,
    -sinf(theta), 0, cosf(theta), 0,
    0, 0, 0, 1,
  };
  Mat4 RotPhi = {
    cosf(phi), sinf(phi), 0, 0,
    -sinf(phi), cosf(phi), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  };
  Mat4 RotRho = {
    1, 0, 0, 0,
    0, cosf(rho), sinf(rho), 0,
    0, -sinf(rho), cosf(rho), 0,
    0, 0, 0, 1
  };
  Mat4 Pos = {
    1, 0, 0, pos.x,
    0, 1, 0, pos.y,
    0, 0, 1, pos.z,
    0, 0, 0, 1,
  };
  // clang-format on
  M = Pos * RotRho * RotPhi * RotTheta * Scale;
  recalculate_mvp();
}

void look_at(Vec3 eye, Vec3 target, Vec3 up) {}

void set_perspective(float near, float far, float aspect_ratio, float fov) {
  float f = tan(fov / 2);
  // clang-format off
  P = {
    1/(aspect_ratio * f), 0, 0, 0,
    0, 1/f, 0, 0,
    0, 0, -(far + near)/(far - near), -(2 * far * near)/(far - near),
    0, 0, -1, 0,
  };
  // clang-format on
  recalculate_mvp();
}

static float signed_triangle_area(Vec2 a, Vec2 b, Vec2 c) {
  return (b.x - a.x) * (c.y - b.y) - (c.x - b.x) * (b.y - a.y);
}

Vec4 clip(const Vec3 &vertex) {
  Vec4 v_extended = Vec4{vertex.x, vertex.y, vertex.z, 1};
  Vec4 clip_vec4 = PVM * v_extended;
  if (fabs(clip_vec4.w) < 1e-8f)
    clip_vec4.w = 1e-8f;
  return clip_vec4;
}

static std::vector<float> z_buffer;

void reset_z_buffer(size_t size) {
  if (z_buffer.size() != size)
    z_buffer.resize(size);

  std::fill(z_buffer.begin(), z_buffer.end(), 1.0f);
}

static Vec3 light_dir = Vec3{0, 1, 0.75}.n();
static Vec3 camera_pos = {0, 0, 0};
static float brightness = 1.0f;

void set_brightness(float intensity) { brightness = intensity; }

void rasterize(const Model &model, int face_idx, Vec4 v[3], Vec3 vn[3],
               Vec2 uv[3], std::vector<Color> &frame, int width, float hw,
               int height, float hh, const Color *override_color) {
  const Material &mat = model.mat(face_idx);
  for (const int &i : {0, 1, 2})
    vn[i] = (M * Vec4{vn[i].x, vn[i].y, vn[i].z, 0}).xyz().n();
  float inv_w[3] = {1.0f / v[0].w, 1.0f / v[1].w, 1.0f / v[2].w};
  Vec3 a = v[0].xyz() * inv_w[0];
  Vec3 b = v[1].xyz() * inv_w[1];
  Vec3 c = v[2].xyz() * inv_w[2];
  Vec3 n_over_w[3] = {vn[0] * inv_w[0], vn[1] * inv_w[1], vn[2] * inv_w[2]};
  Vec2 uv_over_w[3] = {uv[0] * inv_w[0], uv[1] * inv_w[1], uv[2] * inv_w[2]};
  Vec2 a_s = {hw + a.x * hw, hh + a.y * hh};
  Vec2 b_s = {hw + b.x * hw, hh + b.y * hh};
  Vec2 c_s = {hw + c.x * hw, hh + c.y * hh};
  float area = signed_triangle_area(a_s, b_s, c_s);
  // back-culling
  if (area <= 0)
    return;
  float inv_area = 1.0f / area;
  float xmin = std::min(a_s.x, std::min(b_s.x, c_s.x));
  float xmax = std::max(a_s.x, std::max(b_s.x, c_s.x));
  float ymin = std::min(a_s.y, std::min(b_s.y, c_s.y));
  float ymax = std::max(a_s.y, std::max(b_s.y, c_s.y));

  for (int x = std::max(0, static_cast<int>(xmin));
       x <= std::min(width - 1, static_cast<int>(xmax)); ++x) {
    for (int y = std::max(0, static_cast<int>(ymin));
         y <= std::min(height - 1, static_cast<int>(ymax)); ++y) {
      // clang-format off
          float alpha = signed_triangle_area(Vec2{x + 0.5f, y + 0.5f}, b_s, c_s) * inv_area;
          float beta = signed_triangle_area(a_s, Vec2{x + 0.5f, y + 0.5f}, c_s) * inv_area;
          float gamma = signed_triangle_area(a_s, b_s, Vec2{x + 0.5f, y + 0.5f}) * inv_area;
      // clang-format on
      if (alpha < 0 || beta < 0 || gamma < 0)
        continue;

      float inv_w_interp =
          alpha * inv_w[0] + beta * inv_w[1] + gamma * inv_w[2];
      float z = (alpha * a.z * inv_w[0] + beta * b.z * inv_w[1] +
                 gamma * c.z * inv_w[2]) /
                inv_w_interp;
      if (z <= -1 || z > z_buffer[y * width + x])
        continue;
      z_buffer[y * width + x] = z;

      Vec3 n =
          ((n_over_w[0] * alpha + n_over_w[1] * beta + n_over_w[2] * gamma) *
           (1. / inv_w_interp))
              .n();

      Vec2 uv_interp =
          (uv_over_w[0] * alpha + uv_over_w[1] * beta + uv_over_w[2] * gamma) *
          (1.f / inv_w_interp);

      Vec3 ka = mat.ka, kd = mat.kd, ks = mat.ks;
      float shininess = mat.Ns;
      Vec3 ambient = mat.ka;
      Vec3 diff = kd * std::max(0.0f, n * light_dir);

      Vec3 world_pos =
          ((v[0].xyz() * alpha * inv_w[0] + v[1].xyz() * beta * inv_w[1] +
            v[2].xyz() * gamma * inv_w[2]) *
           (1 / inv_w_interp));

      Vec3 view_dir = (camera_pos - world_pos).n();

      Vec3 reflect_dir = ((n * (2.f * (n * light_dir))) - light_dir).n();
      float spec_factor =
          powf(std::max(view_dir * reflect_dir, 0.0f), shininess);
      Vec3 spec = ks * spec_factor;

      Vec3 color_rgb = ambient + diff + spec;

      color_rgb.x = std::min(color_rgb.x, 1.0f);
      color_rgb.y = std::min(color_rgb.y, 1.0f);
      color_rgb.z = std::min(color_rgb.z, 1.0f);

      Color base;
      if (override_color) {
        base = *override_color;
      } else if (mat.has_texture) {
        int tx = std::clamp(int(uv_interp.x * mat.texture.width), 0,
                            mat.texture.width - 1);
        int ty = std::clamp(int((1.0f - uv_interp.y) * mat.texture.height), 0,
                            mat.texture.height - 1);
        base = mat.texture.pixels[ty * mat.texture.width + tx];
      } else {
        base = {
            (unsigned char)(mat.kd.x * 255),
            (unsigned char)(mat.kd.y * 255),
            (unsigned char)(mat.kd.z * 255),
        };
      }

      Color shaded = {(unsigned char)std::clamp(
                          (base.r * color_rgb.x * brightness), 0.f, 255.f),
                      (unsigned char)std::clamp(
                          (base.g * color_rgb.y * brightness), 0.f, 255.f),
                      (unsigned char)std::clamp(
                          (base.b * color_rgb.z * brightness), 0.f, 255.f)};

      frame[y * width + x] = shaded;
    }
  }
}
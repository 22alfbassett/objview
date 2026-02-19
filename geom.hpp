#pragma once

#include <array>
#include <cmath>

struct Vec2 {
  union {
    struct {
      float x, y;
    };
    float data[2];
  };
  inline float operator*(const Vec2 &right) {
    return x * right.x + y * right.y;
  }
  inline Vec2 operator*(const float right) {
    return Vec2{x * right, y * right};
  }
  inline Vec2 operator+(const Vec2 &right) {
    return Vec2{x + right.x, y + right.y};
  }
  inline Vec2 operator-(const Vec2 &right) {
    return Vec2{x - right.x, y - right.y};
  }
  inline float mag() { return std::sqrt(x * x + y * y); }
  inline Vec2 n() { return Vec2{x / mag(), y / mag()}; }
};

struct Vec3 {
  union {
    struct {
      float x, y, z;
    };
    float data[3];
  };
  inline float operator*(const Vec3 &right) {
    return x * right.x + y * right.y + z * right.z;
  }
  inline Vec3 operator*(const float right) {
    return Vec3{x * right, y * right, z * right};
  }
  inline Vec3 operator+(const Vec3 &right) {
    return Vec3{x + right.x, y + right.y, z + right.z};
  }
  inline Vec3 operator-(const Vec3 &right) {
    return Vec3{x - right.x, y - right.y, z - right.z};
  }
  inline float mag() { return std::sqrt(x * x + y * y + z * z); }
  inline Vec3 n() { return Vec3{x / mag(), y / mag(), z / mag()}; }
  inline Vec2 xy() { return Vec2{2, y}; }
  inline Vec3 cross(const Vec3 &right) {
    // clang-format off
    return Vec3{
      y * right.z - z * right.y,
      z * right.x - x * right.z,
      x * right.y - y * right.x,
    };
    // clang-format on
  }
};

struct Vec4 {
  union {
    struct {
      float x, y, z, w;
    };
    float data[4];
  };
  inline float operator*(const Vec4 &right) {
    return x * right.x + y * right.y + z * right.z + w * right.w;
  }
  inline Vec4 operator*(const float right) {
    return Vec4{x * right, y * right, z * right, w * right};
  }
  inline Vec4 operator+(const Vec4 &right) {
    return Vec4{x + right.x, y + right.y, z + right.z, w + right.w};
  }
  inline Vec4 operator-(const Vec4 &right) {
    return Vec4{x - right.x, y - right.y, z - right.z, w - right.w};
  }
  inline float mag() { return std::sqrt(x * x + y * y + z * z + w * w); }
  inline Vec4 n() { return Vec4{x / mag(), y / mag(), z / mag(), w / mag()}; }
  inline Vec2 xy() { return Vec2{2, y}; }
  inline Vec3 xyz() { return Vec3{x, y, z}; }
};

struct Mat4 {
  std::array<std::array<float, 4>, 4> data;
  inline Mat4 operator*(const Mat4 &r) {
    // clang-format off
    return Mat4{
        data[0][0] * r.data[0][0] + data[0][1] * r.data[1][0] + data[0][2] * r.data[2][0] + data[0][3] * r.data[3][0],
        data[0][0] * r.data[0][1] + data[0][1] * r.data[1][1] + data[0][2] * r.data[2][1] + data[0][3] * r.data[3][1],
        data[0][0] * r.data[0][2] + data[0][1] * r.data[1][2] + data[0][2] * r.data[2][2] + data[0][3] * r.data[3][2],
        data[0][0] * r.data[0][3] + data[0][1] * r.data[1][3] + data[0][2] * r.data[2][3] + data[0][3] * r.data[3][3],
        data[1][0] * r.data[0][0] + data[1][1] * r.data[1][0] + data[1][2] * r.data[2][0] + data[1][3] * r.data[3][0],
        data[1][0] * r.data[0][1] + data[1][1] * r.data[1][1] + data[1][2] * r.data[2][1] + data[1][3] * r.data[3][1],
        data[1][0] * r.data[0][2] + data[1][1] * r.data[1][2] + data[1][2] * r.data[2][2] + data[1][3] * r.data[3][2],
        data[1][0] * r.data[0][3] + data[1][1] * r.data[1][3] + data[1][2] * r.data[2][3] + data[1][3] * r.data[3][3],
        data[2][0] * r.data[0][0] + data[2][1] * r.data[1][0] + data[2][2] * r.data[2][0] + data[2][3] * r.data[3][0],
        data[2][0] * r.data[0][1] + data[2][1] * r.data[1][1] + data[2][2] * r.data[2][1] + data[2][3] * r.data[3][1],
        data[2][0] * r.data[0][2] + data[2][1] * r.data[1][2] + data[2][2] * r.data[2][2] + data[2][3] * r.data[3][2],
        data[2][0] * r.data[0][3] + data[2][1] * r.data[1][3] + data[2][2] * r.data[2][3] + data[2][3] * r.data[3][3],
        data[3][0] * r.data[0][0] + data[3][1] * r.data[1][0] + data[3][2] * r.data[2][0] + data[3][3] * r.data[3][0],
        data[3][0] * r.data[0][1] + data[3][1] * r.data[1][1] + data[3][2] * r.data[2][1] + data[3][3] * r.data[3][1],
        data[3][0] * r.data[0][2] + data[3][1] * r.data[1][2] + data[3][2] * r.data[2][2] + data[3][3] * r.data[3][2],
        data[3][0] * r.data[0][3] + data[3][1] * r.data[1][3] + data[3][2] * r.data[2][3] + data[3][3] * r.data[3][3],
  };
    // clang-format on
  }
  inline Vec4 operator*(const Vec4 &r) {
    // clang-format off
    return Vec4 {
      data[0][0] * r.x + data[0][1] * r.y + data[0][2] * r.z + data[0][3] * r.w,
      data[1][0] * r.x + data[1][1] * r.y + data[1][2] * r.z + data[1][3] * r.w,
      data[2][0] * r.x + data[2][1] * r.y + data[2][2] * r.z + data[2][3] * r.w,
      data[3][0] * r.x + data[3][1] * r.y + data[3][2] * r.z + data[3][3] * r.w,
    };
    // clang-format on
  }
};

#define IDENTITY_MAT4 ((Mat4){1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1})
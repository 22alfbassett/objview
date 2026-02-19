#pragma once
#include "geom.hpp"
#include <string>
#include <unordered_map>
#include <vector>

struct Color {
  unsigned char r, g, b;
};

struct Texture {
  int width;
  int height;
  std::vector<Color> pixels;
};

struct Face {
  std::array<int, 3> v;  // vertex indexes
  std::array<int, 3> vt; // texture indexes
  std::array<int, 3> vn; // normal indexes
  int material_id = -1;  // material indexes
};

struct Material {
  std::string name;
  Vec3 ka = {0.1f, 0.1f, 0.1f}; // ambient color
  Vec3 kd = {0.7f, 0.7f, 0.7f}; // diffuse color
  Vec3 ks = {0.2f, 0.2f, 0.2f}; // specular color
  float Ns = 32.f;     // specular exponent
  Texture texture;
  bool has_texture = false;
  std::string diffuse_map; // .mtl filename
};

class Model {
private:
  std::vector<Vec3> verts{};
  std::vector<Face> faces{};
  std::vector<Vec3> vert_normals{};
  std::vector<Vec2> vert_textures{};
  std::vector<Material> materials{};
  std::unordered_map<std::string, int> material_lookup;
  std::string directory;

public:
  Model(const std::string &filename);
  void load_mtl(const std::string &filename);
  void load_texture(Material *mat);
  int nverts() const { return verts.size(); };
  int nfaces() const { return faces.size(); };
  Vec3 &vert(const int i) { return verts[i]; };
  Vec3 &vert(const int iface, const int nth_vert) {
    return verts[faces[iface].v[nth_vert]];
  };
  const Vec3 &vert(const int iface, const int nth_vert) const {
    return verts[faces[iface].v[nth_vert]];
  };
  const Vec3 &vert_normal(const int iface, const int nth_vert) const {
    static Vec3 fb{0.0f, 0.0f, 1.0f};
    const Face &f = faces[iface];
    int idx = f.vn[nth_vert];
    if (idx < 0 || idx >= (int)vert_normals.size())
      return fb;
    return vert_normals[idx];
  };

  const Vec2 &vert_texture(const int iface, const int nth_vert) const {
    static Vec2 fb{0.0f, 0.0f};
    const Face &f = faces[iface];
    int idx = f.vt[nth_vert];
    if (idx < 0 || idx >= (int)vert_textures.size())
      return fb;
    return vert_textures[idx];
  }
  const Material &mat(const int face_idx) const {
    static Material default_mat;
    int id = faces[face_idx].material_id;
    if (id < 0 || id >= materials.size())
      return default_mat;
    return materials[id];
  }
};
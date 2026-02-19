#include "geom.hpp"
#include <string>
#include <vector>

struct Face {
  std::array<int, 3> v;  // vertex indexes
  std::array<int, 3> vt; // texture indexes
  std::array<int, 3> vn; // normal indexes
};

class Model {
private:
  std::vector<Vec3> verts{};
  std::vector<Face> faces{};
  std::vector<Vec3> vert_normals{};

public:
  Model(const std::string &filename);
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
    return vert_normals[faces[iface].v[nth_vert]];
  };
};
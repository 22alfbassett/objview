#include "Model.hpp"
#include <cmath>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void normalize_verts(Model &model) {
  float max_val = 1;
  for (int v = 0; v < model.nverts(); v++) {
    if (std::abs(model.vert(v).x) > max_val)
      max_val = std::abs(model.vert(v).x);
    if (std::abs(model.vert(v).y) > max_val)
      max_val = std::abs(model.vert(v).y);
    if (std::abs(model.vert(v).z) > max_val)
      max_val = std::abs(model.vert(v).z);
  }
  for (int v = 0; v < model.nverts(); v++) {
    model.vert(v).x /= max_val;
    model.vert(v).y /= max_val;
    model.vert(v).z /= max_val;
  }
}

std::vector<Face> parse_face(std::stringstream &ss) {
  std::vector<int> v_idx, vt_idx, vn_idx;
  std::string token;

  while (ss >> token) {
    std::stringstream ts(token);
    std::string part;

    int v = -1, vt = -1, vn = -1;

    if (std::getline(ts, part, '/'))
      v = std::stoi(part) - 1;

    if (std::getline(ts, part, '/') && !part.empty())
      vt = std::stoi(part) - 1;

    if (std::getline(ts, part, '/') && !part.empty())
      vn = std::stoi(part) - 1;

    v_idx.push_back(v);
    vt_idx.push_back(vt);
    vn_idx.push_back(vn);
  }

  std::vector<Face> triangles;

  for (size_t i = 1; i + 1 < v_idx.size(); i++) {
    Face f;
    f.v[0] = v_idx[0];
    f.v[1] = v_idx[i];
    f.v[2] = v_idx[i + 1];

    f.vt[0] = vt_idx[0];
    f.vt[1] = vt_idx[i];
    f.vt[2] = vt_idx[i + 1];

    f.vn[0] = vn_idx[0];
    f.vn[1] = vn_idx[i];
    f.vn[2] = vn_idx[i + 1];

    triangles.push_back(f);
  }

  return triangles;
}

static std::string file_extension(const std::string &path) {
  size_t dot = path.find_last_of('.');
  if (dot == std::string::npos)
    return "";
  std::string ext = path.substr(dot);
  for (char &c : ext)
    c = std::tolower(c);
  return ext;
}

Model::Model(const std::string &filename) {
  directory = filename.substr(0, filename.find_last_of("/\\") + 1);

  std::string ext = file_extension(filename);
  if (ext == ".stl")
    load_stl(filename);
  else
    load_obj(filename);

  if (!verts.empty())
    normalize_verts(*this);
}

void Model::load_obj(const std::string &filename) {
  std::ifstream file(filename);
  if (!file) {
    fprintf(stderr, "objview: %s: No such file or directory\n",
            filename.c_str());
    exit(1);
  }
  std::string line, tok;
  int current_material = -1;

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    ss >> tok;
    if (tok == "v") {
      float x, y, z;
      ss >> x >> y >> z;
      verts.push_back({x, y, z});
    } else if (tok == "vn") {
      float x, y, z;
      ss >> x >> y >> z;
      vert_normals.push_back({x, y, z});
    } else if (tok == "vt") {
      float x, y;
      ss >> x >> y;
      vert_textures.push_back({x, y});
    } else if (tok == "mtllib") {
      std::string mtl_file;
      ss >> mtl_file;
      load_mtl(directory + mtl_file);
    } else if (tok == "usemtl") {
      std::string name;
      ss >> name;
      if (material_lookup.count(name))
        current_material = material_lookup[name];
      else
        current_material = -1;

    } else if (tok == "f") {
      std::vector<Face> new_faces = parse_face(ss);
      for (Face &f : new_faces) {
        f.material_id = current_material;
        faces.push_back(f);
      }
    }
  }
}

void Model::load_stl(const std::string &filename) {
  // Detect ASCII vs binary: ASCII STL starts with "solid" followed by
  // a "facet" keyword on an early line
  bool is_ascii = false;
  {
    std::ifstream test(filename);
    std::string line;
    if (std::getline(test, line)) {
      size_t first = line.find_first_not_of(" \t");
      if (first != std::string::npos &&
          line.substr(first, 5) == "solid") {
        while (std::getline(test, line)) {
          first = line.find_first_not_of(" \t");
          if (first == std::string::npos)
            continue;
          std::string trimmed = line.substr(first);
          if (trimmed.rfind("facet", 0) == 0 ||
              trimmed.rfind("endsolid", 0) == 0)
            is_ascii = true;
          break;
        }
      }
    }
  }

  if (is_ascii) {
    std::ifstream file(filename);
    if (!file) {
      fprintf(stderr, "objview: %s: No such file or directory\n",
              filename.c_str());
      exit(1);
    }
    std::string line;
    std::getline(file, line); // skip "solid ..." line

    Vec3 normal{};
    int vert_count = 0;
    while (std::getline(file, line)) {
      std::stringstream ss(line);
      std::string tok;
      ss >> tok;
      if (tok == "facet") {
        ss >> tok; // "normal"
        ss >> normal.x >> normal.y >> normal.z;
      } else if (tok == "vertex") {
        float x, y, z;
        ss >> x >> y >> z;
        verts.push_back({x, y, z});
        vert_count++;
        if (vert_count == 3) {
          int base = verts.size() - 3;
          int ni = vert_normals.size();
          vert_normals.push_back(normal);
          faces.push_back(
              Face{{base, base + 1, base + 2}, {-1, -1, -1}, {ni, ni, ni}});
          vert_count = 0;
        }
      }
    }
  } else {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
      fprintf(stderr, "objview: %s: No such file or directory\n",
              filename.c_str());
      exit(1);
    }

    // Skip 80-byte header
    file.seekg(80);

    uint32_t num_triangles;
    file.read(reinterpret_cast<char *>(&num_triangles), 4);

    verts.reserve(num_triangles * 3);
    vert_normals.reserve(num_triangles);
    faces.reserve(num_triangles);

    for (uint32_t i = 0; i < num_triangles; i++) {
      float data[12]; // normal(3) + v0(3) + v1(3) + v2(3)
      file.read(reinterpret_cast<char *>(data), 48);
      uint16_t attr;
      file.read(reinterpret_cast<char *>(&attr), 2);

      if (!file) {
        fprintf(stderr, "objview: %s: unexpected end of STL data\n",
                filename.c_str());
        break;
      }

      int ni = vert_normals.size();
      vert_normals.push_back({data[0], data[1], data[2]});

      int base = verts.size();
      verts.push_back({data[3], data[4], data[5]});
      verts.push_back({data[6], data[7], data[8]});
      verts.push_back({data[9], data[10], data[11]});

      faces.push_back(
          Face{{base, base + 1, base + 2}, {-1, -1, -1}, {ni, ni, ni}});
    }
  }
}

void Model::load_mtl(const std::string &filename) {
  std::ifstream file(filename);
  if (!file) {
    fprintf(stderr, "failed to load mtl: %s\n", filename.c_str());
    return;
  }

  std::string line, tok;
  Material *current = nullptr;

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    ss >> tok;

    if (tok == "newmtl") {
      std::string name;
      ss >> name;

      materials.push_back(Material());
      current = &materials.back();
      current->name = name;
      material_lookup[name] = materials.size() - 1;
    } else if (tok == "Ka" && current) {
      ss >> current->ka.x >> current->ka.y >> current->ka.z;
    } else if (tok == "Kd" && current) {
      ss >> current->kd.x >> current->kd.y >> current->kd.z;
    } else if (tok == "Ks" && current) {
      ss >> current->ks.x >> current->ks.y >> current->ks.z;
    } else if (tok == "Ns" && current) {
      ss >> current->Ns;
    } else if (tok == "map_Kd" && current) {
      ss >> current->diffuse_map;

      load_texture(current);
    }
  }
}

void Model::load_texture(Material *mat) {
  int w, h, comp;
  std::string fullpath = directory + mat->diffuse_map;
  unsigned char *data = stbi_load(fullpath.c_str(), &w, &h, &comp, 0);
  if (!data) {
    fprintf(stderr, "failed to load texture: %s\n", fullpath.c_str());
    return;
  }

  mat->texture.width = w;
  mat->texture.height = h;
  mat->texture.pixels.resize(w * h);

  for (int i = 0; i < w * h; i++) {
    int idx = i * comp;
    unsigned char r = data[idx + 0];
    unsigned char g = comp > 1 ? data[idx + 1] : r;
    unsigned char b = comp > 2 ? data[idx + 2] : r;
    mat->texture.pixels[i] = {r, g, b};
  }

  stbi_image_free(data);
  mat->has_texture = true;
}

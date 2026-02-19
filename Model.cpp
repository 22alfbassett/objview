#include "Model.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

Face parse_face(std::stringstream &ss) {
  Face f;
  std::string token;
  int i = 0;
  while (i < 3 && ss >> token) {
    std::string part;
    std::stringstream ts(token);

    std::getline(ts, part, '/');
    f.v[i] = std::stoi(part) - 1;

    if (std::getline(ts, part, '/') && !part.empty())
      f.vt[i] = std::stoi(part) - 1;
    if (std::getline(ts, part, '/') && !part.empty())
      f.vn[i] = std::stoi(part) - 1;
    i++;
  }
  return f;
}

Model::Model(const std::string &filename) {
  directory = filename.substr(0, filename.find_last_of("/\\") + 1);

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
      Face f = parse_face(ss);
      f.material_id = current_material;
      faces.push_back(f);
    }
  }
  if (!verts.empty()) {
    normalize_verts(*this);
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

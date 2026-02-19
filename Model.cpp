#include "Model.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

static void normalize_verts(Model &model) {
  float max_val = 1;
  for (int v = 0; v < model.nverts(); v++) {
    if (std::abs(model.vert(v).x) > max_val)
      max_val = std::abs(model.vert(v).x);
    if (std::abs(model.vert(v).y) > max_val)
      max_val = std::abs(model.vert(v).y);
    if (std::abs(model.vert(v).x) > max_val)
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
  while (ss >> token) {
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
  std::ifstream file(filename);
  if (!file) {
    fprintf(stderr, "objview: %s: No such file or directory\n",
            filename.c_str());
    exit(1);
  }
  std::string line;
  std::string tok;

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
    } else if (tok == "f") {
      faces.push_back(parse_face(ss));
    }
  }
  if (!verts.empty()) {
    normalize_verts(*this);
  }
}
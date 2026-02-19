#include "Model.hpp"
#include "gl.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

struct termios original;

static unsigned short term_width = 0;
static unsigned short term_height = 0;

static unsigned render_width = 0;
static unsigned render_height = 0;

static bool g_use_fixed_color = false;
static Color g_fixed_color = (Color){0, 0, 0};

volatile sig_atomic_t resized = 0;

void cleanup() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
  write(STDOUT_FILENO, "\033[?25h\033[?1049l", 14);
}

void enable_raw() {
  tcgetattr(STDIN_FILENO, &original);
  atexit(cleanup);

  struct termios raw = original;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void handle_resize(int) { resized = 1; }

void update_size() {
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
    return;
  if (!w.ws_col || !w.ws_row)
    return;

  term_width = w.ws_col;
  term_height = w.ws_row;

  render_width = term_width;
  render_height = term_height * 2;

  resized = 0;
  write(STDOUT_FILENO, "\033[2J", 4);
}

static std::vector<Color> frame;
static std::vector<char> output;

static float x_model = 0, y_model = 0, z_model = -2;
static float theta_model = 0, rho_model = 0, phi_model = 0;

void render_model(const Model &m) {
  static unsigned int seed = time(NULL);
  srand(seed);

  if (!render_width || !render_height)
    return;

  float hw = render_width / 2.f, hh = render_height / 2.f;

  size_t needed = render_width * render_height;

  if (frame.size() != needed)
    frame.assign(needed, {0, 0, 0});
  else
    memset(frame.data(), 0, needed * sizeof(Color));

  set_model({x_model, y_model, z_model}, {theta_model, rho_model, phi_model},
            {1, 1, 1});

  set_perspective(0.1, 100, static_cast<float>(render_width) / render_height,
                  M_PI / 3);

  reset_z_buffer(needed);

  for (int f = 0; f < m.nfaces(); ++f) {
    Vec4 c[3];
    Vec3 vn[3];
    for (int i = 0; i < 3; ++i) {
      c[i] = clip(m.vert(f, i));
      vn[i] = m.vert_normal(f, i);
    }

    Color face_color;

    if (g_use_fixed_color) {
      face_color = g_fixed_color;
    } else {
      face_color = {(unsigned char)(rand() % 256),
                    (unsigned char)(rand() % 256),
                    (unsigned char)(rand() % 256)};
    }

    rasterize(c, vn, frame, render_width, hw, render_height, hh, face_color);
  }

  output.clear();
  output.reserve(term_width * term_height * 8);

  output.insert(output.end(), "\033[H", &"\033[H"[3]);

  Color last_fg{255, 255, 255};
  Color last_bg{255, 255, 255};
  bool first = true;

  for (unsigned y = 0; y < term_height; ++y) {
    for (unsigned x = 0; x < term_width; ++x) {

      int top_i = (render_height - 1 - y * 2) * render_width + x;
      int bottom_i = (render_height - 1 - (y * 2 + 1)) * render_width + x;

      Color fg = frame[top_i];
      Color bg = frame[bottom_i];

      if (first || memcmp(&fg, &last_fg, 3) != 0) {
        char buf[32];
        int len = sprintf(buf, "\033[38;2;%d;%d;%dm", fg.r, fg.g, fg.b);
        output.insert(output.end(), buf, buf + len);
        last_fg = fg;
      }

      if (first || memcmp(&bg, &last_bg, 3) != 0) {
        char buf[32];
        int len = sprintf(buf, "\033[48;2;%d;%d;%dm", bg.r, bg.g, bg.b);
        output.insert(output.end(), buf, buf + len);
        last_bg = bg;
      }

      output.insert(output.end(), "▀", &"▀"[3]);
      first = false;
    }
  }

  write(STDOUT_FILENO, output.data(), output.size());
}

int main(int argc, char *argv[]) {

  int target_fps = 60;
  bool auto_rotate = false;
  float rotation_speed = M_PI; // 1 rotation every 2 seconds
  float render_scale = 1.0f;
  float change_scale = 1.0f;

  static struct option long_options[] = {{"fps", required_argument, 0, 'f'},
                                         {"rotate", no_argument, 0, 'r'},
                                         {"speed", required_argument, 0, 's'},
                                         {"color", required_argument, 0, 'c'},
                                         {"help", no_argument, 0, 'h'},
                                         {"version", no_argument, 0, 'v'},
                                         {0, 0, 0, 0}};

  int opt;
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "f:rs:c:hv", long_options,
                            &option_index)) != -1) {

    switch (opt) {

    case 'f':
      target_fps = std::max(1, atoi(optarg));
      break;

    case 'r':
      auto_rotate = true;
      break;

    case 's':
      rotation_speed = strtof(optarg, nullptr);
      break;

    case 'h':
      printf("Usage: %s [options] <file.obj>\n\n"
             "Options:\n"
             "  -f, --fps N        Target FPS (default 60)\n"
             "  -r, --rotate       Start with auto rotation\n"
             "  -s, --speed N      Rotation speed (rad/sec)\n"
             "  -c  --color R,G,B  Change display color\n"
             "  -h, --help         Show this help\n"
             "  -v, --version      Show version\n\n"
             "Controls (runtime):\n"
             "  q  quit\n"
             "  t  toggle rotation\n"
             "  r  reset model\n"
             "  h  left\n"
             "  l  right\n"
             "  j  up\n"
             "  k  down\n"
             "  u  zoom in\n"
             "  i  zoom out\n"
             "  J  rotate left\n"
             "  L  rotate right\n"
             "  J  rotate up\n"
             "  K  rotate down\n"
             "  Y  rotate in\n"
             "  O  rotate out\n"
             "  U  double scaling\n"
             "  I  half scaling\n",
             argv[0]);
      return 0;

    case 'v':
      printf("objview v0.1\n");
      return 0;

    case 'c':

      int r, g, b;
      if (sscanf(optarg, "%d,%d,%d", &r, &g, &b) != 3) {
        fprintf(stderr, "--color format: R,G,B\n");
        return 1;
      }

      r = std::clamp(r, 0, 255);
      g = std::clamp(g, 0, 255);
      b = std::clamp(b, 0, 255);

      g_fixed_color = {(unsigned char)r, (unsigned char)g, (unsigned char)b};

      g_use_fixed_color = true;
      break;

    default:
      return 1;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Error: missing model file\n");
    fprintf(stderr, "Try '%s --help'\n", argv[0]);
    return 1;
  }

  const char *model_path = argv[optind];

  srand(time(NULL));
  Model m(model_path);

  struct sigaction sa{};
  sa.sa_handler = handle_resize;
  sigaction(SIGWINCH, &sa, NULL);

  write(STDOUT_FILENO, "\033[?1049h\033[?25l", 14);
  enable_raw();

  update_size();

  // apply scale after size update
  render_width = std::max(1u, (unsigned)(term_width * render_scale));
  render_height = std::max(1u, (unsigned)(term_height * 2 * render_scale));

  render_model(m);

  const int frame_time_us = 1000000 / target_fps;

  struct timespec last_time;
  clock_gettime(CLOCK_MONOTONIC, &last_time);

  while (1) {

    if (resized) {
      update_size();
      render_width = std::max(1u, (unsigned)(term_width * render_scale));
      render_height = std::max(1u, (unsigned)(term_height * 2 * render_scale));
      render_model(m);
    }

    struct timeval tv{};
    tv.tv_usec = frame_time_us;

    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    select(STDIN_FILENO + 1, &set, NULL, NULL, &tv);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    float dt = (now.tv_sec - last_time.tv_sec) +
               (now.tv_nsec - last_time.tv_nsec) / 1e9f;

    last_time = now;

    if (auto_rotate) {
      theta_model += rotation_speed * dt;
      render_model(m);
    }

    if (FD_ISSET(STDIN_FILENO, &set)) {
      char c;
      if (read(STDIN_FILENO, &c, 1) == 1) {

        if (c == 'q')
          break;

        else if (c == 't')
          auto_rotate = !auto_rotate;

        else if (c == 'r') {
          x_model = y_model = theta_model = rho_model = phi_model = 0;
          change_scale = 1;
          z_model = -2;
        }

        else if (c == 'h')
          x_model += 0.1 * change_scale;
        else if (c == 'l')
          x_model -= 0.1 * change_scale;
        else if (c == 'j')
          y_model -= 0.1 * change_scale;
        else if (c == 'k')
          y_model += 0.1 * change_scale;
        else if (c == 'u')
          z_model += 0.1 * change_scale;
        else if (c == 'i')
          z_model -= 0.1 * change_scale;
        else if (c == 'J')
          rho_model += M_PI / 10 * change_scale;
        else if (c == 'K')
          rho_model -= M_PI / 10 * change_scale;
        else if (c == 'O')
          phi_model += M_PI / 10 * change_scale;
        else if (c == 'Y')
          phi_model -= M_PI / 10 * change_scale;
        else if (c == 'H')
          theta_model += M_PI / 10 * change_scale;
        else if (c == 'L')
          theta_model -= M_PI / 10 * change_scale;
        else if (c == 'U')
          change_scale *= 2;
        else if (c == 'I')
          change_scale /= 2;

        if (!auto_rotate)
          render_model(m);
      }
    }
  }

  return 0;
}

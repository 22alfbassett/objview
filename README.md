# objview

A zero-dependency terminal-based .obj wavefront viewer. Outputs images in ANSI format.

To build:

```bash
make prod
```



Images:


![1771485927473](image/README/1771485927473.png)

![1771486559845](image/README/1771486559845.png)

![1771486912713](image/README/1771486912713.png)

Usage: `./objview [options] <file.obj>`

Options:

-f, --fps N        Target FPS (default 60)

-r, --rotate       Start with auto rotation

-s, --speed N      Rotation speed (rad/sec)

-c  --color R,G,B  Change display color

-h, --help         Show this help

-v, --version      Show version

Controls (runtime):

q  quit

t  toggle rotation

r  reset model

h  left

l  right

j  up

k  down

u  zoom in

i  zoom out

J  rotate left

L  rotate right

J  rotate up

K  rotate down

Y  rotate in

O  rotate out

U  double scaling

I  half scaling

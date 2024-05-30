
#define COL_BLACK               0
#define COL_BLUE                1
#define COL_GREEN               2
#define COL_CYAN                3
#define COL_RED                 4
#define COL_MAGENTA             5
#define COL_BROWN               6
#define COL_WHITE               7
#define COL_DARK_GREY           8
#define COL_LIGHT_BLUE          9
#define COL_LIGHT_GREEN        10
#define COL_LIGHT_CYAN         11
#define COL_LIGHT_RED          12
#define COL_LIGHT_MAGENTA      13
#define COL_YELLOW             14
#define COL_BRIGHT_WHITE       15

#define MONO_BASE_ADDRESS      0xB0000000
#define COLOUR_BASE_ADDRESS    0xB8000000

struct palette_str
{
    short black,
          blue,
          green,
          cyan,
          red,
          magenta,
          brown,
          white,
          dark_grey,
          light_blue,
          light_green,
          light_cyan,
          light_red,
          light_magenta,
          yellow,
          bright_white;
};

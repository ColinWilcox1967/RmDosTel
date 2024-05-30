#include "palette.h"

/* Used with colour modes - field values are just their namesake colours */

struct palette_str colour_mode_palette = {COL_BLACK, COL_BLUE, COL_GREEN,
                                          COL_CYAN, COL_RED, COL_MAGENTA,
                                          COL_BROWN, COL_WHITE, COL_YELLOW,
                                          COL_LIGHT_BLUE, COL_LIGHT_GREEN, COL_LIGHT_CYAN,
                                          COL_LIGHT_RED, COL_LIGHT_MAGENTA, COL_YELLOW,
                                          COL_BRIGHT_WHITE
                                    };

/* Used in Mono mode - colour's need to be re-assigned */

struct palette_str mono_mode_palette = { COL_BLACK, COL_GREEN, COL_BLUE,
                                         COL_CYAN, COL_BRIGHT_WHITE, COL_BLUE,
                                         COL_BLUE, COL_BRIGHT_WHITE, COL_DARK_GREY,
                                         COL_YELLOW, COL_YELLOW, COL_BLACK,
                                         COL_YELLOW, COL_YELLOW, COL_BLACK,
                                         COL_BRIGHT_WHITE
                                       };
struct palette_str palette;



void assign_palette ( struct palette_str active_palette )
{
    memcpy ( &palette.black, &active_palette.black, sizeof (struct palette_str));
}

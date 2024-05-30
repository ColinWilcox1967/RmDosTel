#define DEFAULT_TELNUM_FILENAME "RMDOSTEL.TXT"
#define DEFAULT_CONFIG_FILENAME "RMDOSTEL.CFG"

#define FILE_ATTR_LIST       ( _A_NORMAL | _A_SUBDIR | _A_ARCH | _A_HIDDEN | _A_SYSTEM )

#define RM_VERSION           "v2.0 ( 'BETA 9' )"

#define LEFT_MARKER         (char)16
#define RIGHT_MARKER        (char)17
#define OPTION_CHECKED      (char)251  /* Tick Symbol */

#define ID_STRING           "DOSTEL_2.0"

#define PRINTER_LPT1        0
#define PRINTER_LPT2        1
#define PRINTER_LPT3        2
#define PRINTER_LPT4        3

#define HELP_SWITCH         -1
#define CONFIG_SWITCH       -2
#define DATAFILE_SWITCH     -3
#define UNKNOWN_SWITCH      -999

#define BIOS_KEYBOARD           0x16
#define BIOS_KEYBOARD_GET_FLAGS 2

/* Stored in ROM BIOS Segment zero */

#define LOC_KEYBOARD_FLAGS      0x417

#define TRUE                ( 0 == 0)
#define FALSE               ( ! TRUE )

#define DEFAULT_PR_FILE    "DEFAULT.PR"

#define MAX_UNDELETES      10

#define MAX_NAME_SIZE     23
#define MAX_NUMBER_SIZE   30
#define MAX_PASSWORD_SIZE 8
#define MAX_BLANK_SIZE    16

#define BLANK_NAME        "                       "
#define BLANK_NUMBER      "                              "
#define BLANK_PASSWORD    "        "
#define BLANK_UNUSED      "                "

#define MAX_PHONE_ENTRIES    750

struct rmdostel_rec
{
    char name[MAX_NAME_SIZE + 1];
    char number[MAX_NUMBER_SIZE + 1];
    char password[MAX_PASSWORD_SIZE + 1];
    char unused[16];
};

struct telnum_str
{
    char name[ MAX_NAME_SIZE + 1 ];
    char number[ MAX_NUMBER_SIZE + 1 ];
    char password[ MAX_PASSWORD_SIZE + 1 ];
    int  search_match; /* found as part of last search */
    int marked; /* selected by user for use later perhaps */
};

struct telnum_str phone_list[ MAX_PHONE_ENTRIES ];


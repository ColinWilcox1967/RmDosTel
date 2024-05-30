#define PORTSIZE(x1,y1,x2,y2)    ((x2-x1+1)*(y2-y1+1)*2)
#define CHAR_POS(x,y)            ((y-1) * 160 + (2 * (x-1)))
#define ATTR_POS(x,y)            ((y-1) * 160 + (2 * (x-1))+1)
#define ATTR_BYTE(fg, bk)        (( ( bk << 4 ) | fg) & 127)

#define LOW_NIBBLE(ch)    ( ch & 15 )
#define HIGH_NIBBLE(ch)   ((ch >> 4) & 15)

#define BIOS_VIDEO            0x10
#define BIOS_SET_CURSOR_TYPE  1
#define BIOS_SET_CURSOR       2
#define BIOS_SET_DISPLAY_PAGE 5
#define BIOS_READ_CHARATTR    8
#define BIOS_WRITE_CHARATTR   9
#define BIOS_SET_VIDEO_MODE   0
#define BIOS_GET_VIDEO_MODE   0x0F

#define TL_SINGLE             (char)218
#define TR_SINGLE             (char)191
#define BL_SINGLE             (char)192
#define BR_SINGLE             (char)217
#define V_SINGLE              (char)179
#define H_SINGLE              (char)196

#define TL_DOUBLE             (char)201
#define TR_DOUBLE             (char)187
#define BL_DOUBLE             (char)200
#define BR_DOUBLE             (char)188
#define V_DOUBLE              (char)186
#define H_DOUBLE              (char)205


extern void SetColourPair ( short, short );
extern void erase_prompt ( void );
extern void WriteChar ( int, int, char );
extern void WriteString ( int, int, char *);
extern void ShowHeader ( int, int, int, char *);
extern void save_area ( int, int, int, int, char *);
extern void restore_area (int, int, int, int, char *);

extern void hide_cursor ( void );
extern void show_cursor ( void );
extern char *string ( char, int );
extern void text ( int, int, char *);
extern void centre_text ( int, char *);
extern void draw_shadow (int, int, int, int);
extern void draw_box (int, int, int, int );
extern void draw_window ( int, int, int, int );
extern void explode_window (int, int, int, int);
extern int  difference (int, int);


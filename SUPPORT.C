#include <stdio.h>
#include <string.h>
#include <bios.h>
#include <io.h>
#include <string.h>
#include "palette.h"
#include "config.h"
#include "keys.h"
#include "windows.h"
#include "dostel2.h"

extern unsigned char config_byte2;

extern char datafile[];
extern struct palette_str palette;
extern unsigned char far *video_base;
extern short current_foreground;
extern short current_background;
extern unsigned char config_byte3;
extern char trim_name[];
extern unsigned char initial_config_byte2;

extern void trim_filename (char [], int );

union REGS regs;

char n_str[255];

unsigned char keyboard_flags;

void save_keyboard_flags ()
{
    union REGS regs;

    regs.h.ah = BIOS_KEYBOARD_GET_FLAGS;
    int86 (BIOS_KEYBOARD, &regs, &regs);
    keyboard_flags = regs.h.al;
}

void restore_keyboard_flags()
{
    unsigned char far *bios_ptr;

    bios_ptr = (unsigned char far *)LOC_KEYBOARD_FLAGS;
    *bios_ptr = keyboard_flags;
}

int get_yesno (int x, int y, short foreground, short background)
{
    int          option,
                 got_yn;
    unsigned int key;

    got_yn = FALSE;
    option = 0;
    while (! got_yn)
    {
        
        SetColourPair (foreground, background);
        WriteString (x, y, "(Y)es or (N)o");
        if ((initial_config_byte2 & MSK_VIDEO_DETECT) == MSK_VIDEO_MONO)
            SetColourPair (palette.bright_white, palette.black);
        else
            SetColourPair (palette.dark_grey, palette.bright_white);
        if (option == 0)
            WriteString (x, y, "(Y)es");
        else
            WriteString (x+9, y, "(N)o");
        key = toupper(get_key());
        switch ( key )
        {
            case K_CSR_LT    : 
                               option = 0;
                               break;
            case K_CSR_RT    : 
                               option = 1;
                               break;
            case 'Y'         : option = 0;
                               got_yn = TRUE;
                               break;
            case 'N'         : option = 1;
                               got_yn = TRUE;
                               break;
            case K_CR        : 
                               got_yn = TRUE;
                               break;
            default          : break;
        }
    }
    return (( key == 'Y') || ((key == K_CR) && ( option == 0)));
}

int file_exists (char *name)
{
    FILE *fp;
    int ok;
    
    fp = fopen (name, "rb");
    ok = (fp != NULL);

    if (ok)
       fclose (fp);
    return (ok);
}
    
int blank_record (struct telnum_str rec)
{
    return((strnicmp (rec.name,     string (' ', MAX_NAME_SIZE), MAX_NAME_SIZE) == 0) &&
           (strnicmp (rec.number,   string (' ', MAX_NUMBER_SIZE), MAX_NUMBER_SIZE) == 0) &&
           (strnicmp (rec.password, string (' ', MAX_PASSWORD_SIZE), MAX_PASSWORD_SIZE) == 0)
          );
}

void write_blanks (int x, int y, int n)
{
    int idx;

    for (idx = 0; idx < n; idx++)
       WriteChar (x+idx, y, ' ');
}

int difference (int ch1, int ch2 )
{
    int v;

    v = ch1-ch2;
    return ( v > 0 ? v : -v );
}

void hide_cursor ()
{
    regs.h.ah = BIOS_SET_CURSOR_TYPE;
    regs.h.ch = 17;
    regs.h.cl = 16;
    int86 ( BIOS_VIDEO, &regs, &regs );
}

void show_datafile_name ()
{
    SetColourPair (palette.black, palette.cyan);
    write_blanks ( 19, 20, 35);
    WriteString ( 6, 20, "Phone List : ");
    SetColourPair (palette.light_red, palette.cyan);
    if ( strlen (datafile) == 0) /* No File Specified */
        WriteString ( 19, 20, "NONE");
    else
    {
        trim_filename (datafile, 35);
        WriteString ( 19, 20, strupr (trim_name));
    }
}

void show_cursor ()
{
    regs.h.ah = BIOS_SET_CURSOR_TYPE;
    regs.h.ch = 1;
    regs.h.cl = 15;
    int86 ( BIOS_VIDEO, &regs, &regs );
}

char *string ( char ch, int n )
{
    int idx;

    for ( idx = 0; idx < n; idx++)
       n_str[idx] = ch;
    n_str[n] = (char)0;
    return (n_str);
}

void SetColourPair ( short foreground, short background )
{
    current_foreground = foreground;
    current_background = background;

    _settextcolor ( foreground );
    _setbkcolor ( (long)background );

}

void centre_text ( int y, char *str )
{
    int x = ( 80 - strlen ( str ) + 1 )/2;

    WriteString ( x, y, str);
}

void ShowHeader ( int x1, int x2, int y, char *header)
{
    int pos = ( x2 - x1 + 1 - strlen (header))/2;

    SetColourPair ( palette.red, palette.green);
    WriteString ( x1 + pos, y, header);
    SetColourPair (palette.black, palette.green);
}

void WriteChar (int x, int y, char ch )
{
    char str[2];

    if ( config_byte3 & MSK_MEMORY_WRITE)
    {
       *(video_base+CHAR_POS(x,y)) = ch;
       *(video_base+ATTR_POS(x,y)) = ATTR_BYTE(current_foreground, current_background);
    }
    else
    {
        str[0] = ch; str[1] = (char)0;
        _settextposition ( y, x);
        _outtext ( str );
    }
}

void WriteMultiChar (int x, int y, char ch, int n)
{
    int i;

    for (i=0; i < n; i++)
       WriteChar (x+i, y, ch);
}

void WriteString ( int x, int y, char *str )
{
    int i;

    for (i = 0; i < strlen (str); i++)
        WriteChar ( x + i, y, str[i]);
}

void flush_keyboard()
{
    while (kbhit ())
        get_key();
}



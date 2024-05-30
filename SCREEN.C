#include <stdio.h>
#include <malloc.h>
#include <bios.h>
#include "palette.h"
#include "keys.h"
#include "config.h"
#include "windows.h"
#include "dostel2.h"

extern unsigned char config_byte2;
extern unsigned char config_byte3;
extern struct palette_str palette;
extern struct palette_str mono_mode_palette;
extern struct palette_str colour_mode_palette;

extern unsigned char far *video_base;

char *mode_buff;
char *vid_buffer;


int get_current_video_mode ()
{
    union REGS regs;

    regs.h.ah = BIOS_GET_VIDEO_MODE;
    int86(BIOS_VIDEO, &regs, &regs);
    return ((int)regs.h.al);
}

int change_video_mode ( int direction )
{
    union REGS regs;

    regs.h.ah = BIOS_SET_VIDEO_MODE;

    /* Copy contents of old video memory to new block */
    /* Having top bit set prevents screen clearance */

    if ( direction == COLOUR_TO_MONO )
    {
        memcpy ( (void *)MONO_BASE_ADDRESS, (void *)COLOUR_BASE_ADDRESS, 4000);
        regs.h.al = 7 + 128;
        video_base = (unsigned char far *)MONO_BASE_ADDRESS;
        memcpy ( &palette.black, &mono_mode_palette.black, sizeof (struct palette_str));
    }
    else
    {
        memcpy ( (void *)COLOUR_BASE_ADDRESS, (void *)MONO_BASE_ADDRESS, 4000);
        regs.h.al = 3+128;
        video_base = (unsigned char far *)COLOUR_BASE_ADDRESS;
        memcpy (&palette.black, &colour_mode_palette.black, sizeof (struct palette_str));

    }
    int86 (BIOS_VIDEO, &regs, &regs);

    /* Check to see if the mode changed ok */

    return ((( direction == COLOUR_TO_MONO) && ( get_current_video_mode() == 7+128 )) ||
            (( direction == MONO_TO_COLOUR) && ( get_current_video_mode() == 3+128 ))
           );
}

void show_video_options ()
{
    SetColourPair (palette.bright_white, palette.green);
    if ( (config_byte2 & MSK_VIDEO_DETECT) == MSK_VIDEO_DETECT)
        WriteString (50, 15, "Auto Detect");
    else
    {
        if ( config_byte2 & MSK_VIDEO_MONO)
            WriteString ( 50, 15, "Monochrome ");
        else
            WriteString ( 50, 15, "Colour     ");
    }
    if ( config_byte2 & MSK_WINDOW_TYPE)
        WriteString ( 50, 16, "Exploding");
    else
        WriteString ( 50, 16, "Normal   ");

    if ( config_byte2 & MSK_SHADOW_TYPE)
        WriteString ( 50, 17, "Yes");
    else
        WriteString ( 50, 17, "No ");
    if ( config_byte3 & MSK_MEMORY_WRITE)
        WriteString (50, 18, "Memory");
    else
        WriteString (50, 18, "BIOS  ");

    if ( (config_byte3 & MSK_NO_BORDER) == MSK_NO_BORDER)
        WriteString (50, 19, "None       ");
    else
    {
        if ( (config_byte3 & MSK_SINGLE_LINE_BORDER) == MSK_SINGLE_LINE_BORDER)
            WriteString (50, 19, "Single Line");
        else
            WriteString (50, 19, "Double Line");
    }
    if ( config_byte3 & MSK_DISPLAY_MODE)
    {
        WriteString ( 50, 20, "Hexadecimal");
    }
    else
        WriteString ( 50, 20, "Text       ");
}

extern void SelectPalette();

void choose_video_mode()
{
    int ch;

    mode_buff = (char *)malloc (PORTSIZE( 40, 10, 58, 15));
    if (mode_buff == NULL)
        return((void)0);

    save_area ( 40, 10, 58, 15, mode_buff);

    SetColourPair (palette.black, palette.green);
    draw_window ( 40, 10, 57, 14);
    WriteString (43, 11, "Colour");
    WriteString (43, 12, "Monochrome");
    WriteString (43, 13, "Auto Detect");
    SetColourPair (palette.red, palette.green);
    WriteString (43, 11, "C"); WriteString (43, 12, "M"); WriteString (43, 13, "A");

    do
    {
        ch = toupper(get_key());
    }
    while ( (strchr ("CMA", ch) == NULL)  && (ch != K_ESC));

    if ( ch != K_ESC)
    {
        config_byte2 &= ~MSK_VIDEO_DETECT;
        switch ( ch )
        {
                case 'C' : config_byte2 |= MSK_VIDEO_COLOUR;
                           break;
                case 'M' : config_byte2 |= MSK_VIDEO_MONO;
                           break;
                case 'A' : config_byte2 |= MSK_VIDEO_DETECT;
                           break;
        }
    }
    restore_area (40, 10, 58, 15, mode_buff);
    free (( char *)mode_buff);
}

void choose_border_type ()
{
    char *border_buff;
    char ch;

    border_buff = (char *)malloc(PORTSIZE (52, 18, 69, 23));
    if ( border_buff == NULL)
        return;
    save_area (52, 18, 69, 23, border_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (52, 18, 68, 22);
    WriteString ( 55, 19, "No Border");
    WriteString ( 55, 20, "Single Line");
    WriteString (55, 21, "Double Line");
    SetColourPair (palette.red, palette.green);
    WriteChar (55, 19, 'N'); WriteChar (55, 20, 'S'); WriteChar (55, 21, 'D');
    ch = toupper(get_key());
    while ( (strchr ("NSD", ch) == NULL) && (ch != K_ESC))
        ch = toupper(get_key());
    switch ( ch)
    {
        case K_ESC : break;
        case 'N'   : config_byte3 &= ~MSK_NO_BORDER;
                     config_byte3 |= MSK_NO_BORDER;
                     break;
        case 'S'   : config_byte3 &= ~MSK_NO_BORDER;
                     config_byte3 |= MSK_SINGLE_LINE_BORDER;
                     break;
        case 'D'   : config_byte3 &= ~MSK_NO_BORDER;
                     config_byte3 |= MSK_DOUBLE_LINE_BORDER;
                     break;
    }
    restore_area ( 52, 18, 69, 23, border_buff);
    free ((char *)border_buff);
}

void do_video_options()
{
    int done, ch;

    vid_buffer = (char *)malloc (PORTSIZE( 30, 12, 64, 23));
    if ( vid_buffer == NULL)
         return((void)0);

    save_area (30,12,64,23, vid_buffer);
    SetColourPair (palette.black, palette.green);
    draw_window (30, 12, 63, 22);
    ShowHeader ( 30, 63, 13, "Video Options");

    WriteString (34, 15, "Video Mode  ");
    WriteString (34, 16, "Window Type");
    WriteString (34, 17, "Drop Shadow");
    WriteString (34, 18, "Screen Updates");
    WriteString (34, 19, "Border Type");
    WriteString (34, 20, "File Viewer");
    SetColourPair (palette.red, palette.green);
    WriteChar (34, 15, 'V'); WriteChar (34, 16, 'W'); WriteChar (34, 17, 'D');
    WriteChar ( 34, 18, 'S'); WriteChar (34, 19, 'B'); WriteChar (34, 20, 'F');
    done=FALSE;
    while(!done)
    {
        show_video_options();
        ch = toupper(get_key());
        switch (ch)
        {
                case K_ESC : done=TRUE;
                             break;
                case 'F'   : if ( config_byte3 & MSK_DISPLAY_MODE)
                                 config_byte3 &= ~MSK_DISPLAY_MODE;
                             else
                                 config_byte3 |= MSK_DISPLAY_MODE;
                             break;

                case 'B'   : choose_border_type();
                             break;
                case 'S'   : 
                             if ( config_byte3 & MSK_MEMORY_WRITE)
                                 config_byte3 &= ~MSK_MEMORY_WRITE;
                             else
                                 config_byte3 |= MSK_MEMORY_WRITE;
                             break;
                case 'V'   : choose_video_mode();
                             break;
                case 'W'   : 
                             if ( config_byte2 & MSK_WINDOW_TYPE)
                                 config_byte2 &= ~MSK_WINDOW_TYPE;
                             else
                                 config_byte2 |= MSK_WINDOW_TYPE;
                             break;
                case 'D'   : if ( config_byte2 & MSK_SHADOW_TYPE)
                                 config_byte2 &= ~MSK_SHADOW_TYPE;
                             else
                                 config_byte2 |= MSK_SHADOW_TYPE;
                             break;
                default    : break;
        }
    }
    restore_area (30, 12, 64, 22, vid_buffer);
    free ((char *)vid_buffer);
}

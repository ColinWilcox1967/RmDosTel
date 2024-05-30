#include <stdio.h>
#include <malloc.h>

#include "keys.h"
#include "dostel2.h"
#include "palette.h"
#include "windows.h"

extern struct palette_str palette;

#define ASCII_TRANS(x,y)    (( 16 * (y - 5)) + ((x - 42) / 2))
#define DEFAULT_DELIMITER   ','

char HexString[17]={"0123456789ABCDEF\0"};

void draw_ascii_edging()
{
    int i;

    SetColourPair (palette.bright_white, palette.green);

    WriteChar (41, 4, ' '); /* Bit of a fudge to print first space */
    for (i = 0; i < 16; i++)
    {
        WriteChar ( 42 + (i*2), 4, HexString[i]);
        WriteChar ( 42 + (i*2)+1, 4, ' ');
        WriteChar ( 39, 5+i, HexString[i]);
    }
}

void fill_ascii_table()
{
    int i;
    int x, y;
    int cnt;

    hide_cursor();
    SetColourPair (palette.black, palette.green);
    x = 42; y = 5;
    cnt = 0;
    for (i = 0; i < 256;i +=2)
    {
        WriteChar (x, y, (char)i);
        cnt++;
        x += 4;
        if (cnt == 8)
        {
            cnt=0;
            x = 42; y++;
        }
    }
    x = 44; y = 5; cnt = 0;
    for (i = 1; i < 257; i+=2)
    {
        WriteChar (x, y, (char)i);
        cnt++;
        x += 4;
        if (cnt == 8)
        {
            cnt = 0;
            x = 44;
            y++;
        }
    }
}

int invisible ( int c)
{
    return ((c == 0) || (c == 0x0A) || (c == 0x0D));
}

void ShowValue ( int code)
{
    char str[4];

    SetColourPair (palette.bright_white, palette.green);
    if (invisible ( code))
        WriteChar ( 49, 22, ' ');
    else
    {
        sprintf (str, "%c", code); 
        WriteString (49, 22, str); 
    }
    sprintf ( str, "%02X", code); WriteString ( 60, 22, str);
    sprintf ( str, "%03d", code); WriteString ( 70, 22, str);

}

int select_from_ascii_table()
{
    int          xpos,
                 ypos,
                 code,
                 selected,
                 aborted;
    unsigned int k;


    xpos = 42; ypos = 5; selected = aborted = FALSE;
    while (! ( selected || aborted ))
    {
        code = ASCII_TRANS(xpos, ypos);
        ShowValue(code);
        SetColourPair (palette.bright_white, palette.black);

        if ( invisible(code))
            WriteChar (xpos, ypos, ' ');
        else
            WriteChar ( xpos, ypos, (char)code);
        k = get_key();
        SetColourPair (palette.black, palette.green);
        if ( invisible (code))
            WriteChar (xpos, ypos, ' ');
        else
            WriteChar (xpos, ypos, (char)code);
        switch (k)
        {
            case K_ESC    : aborted = TRUE;
                            break;
            case K_CR     : selected = TRUE;
                            break;
            case K_CSR_LT : xpos -= 2;
                            if ( xpos < 42 ) 
                            {
                                xpos = 72;
                                ypos--;
                                if ( ypos < 5)
                                    ypos = 20;
                            }
                            break;
            case K_CSR_RT : xpos += 2;
                            if ( xpos > 72) 
                            {
                                xpos = 42;
                                ypos++;
                                if ( ypos >20)
                                   ypos = 5;
                            }
                            break;
            case K_CSR_UP : ypos--;
                            if ( ypos < 5 )
                                ypos = 20;
                            break;
            case K_CSR_DN : ypos++;
                            if ( ypos > 20)
                                ypos = 5;
                            break;
            default       : break;
        }
    }
    if ( aborted )
        return ( DEFAULT_DELIMITER );
    if ( selected)
       return (code);
}

int ascii_table ()
{
    char *ascii_table_buff;
    int   choice;

    ascii_table_buff = (char *)malloc(PORTSIZE(38, 3, 76, 23));
    if ( ascii_table_buff == NULL)
        return;
    save_area (38, 3, 76, 23, ascii_table_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (38, 3, 75, 22);
    draw_box (39, 4, 74, 21); /* Note draw_box () -> not shadow */
    WriteString ( 40, 22, " ASCII :   ");
    WriteString ( 53, 22, " Hex :    ");
    WriteString ( 63, 22, " Dec :     ");
    SetColourPair (palette.red, palette.green);
    WriteString ( 51, 3, " ASCII Table ");
    SetColourPair (palette.bright_white, palette.green);
    draw_ascii_edging();
    fill_ascii_table();
    choice = select_from_ascii_table();
    restore_area ( 38, 3, 76, 23, ascii_table_buff);
    free ((char *)ascii_table_buff);
    return (choice);
}

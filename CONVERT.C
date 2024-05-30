#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "keys.h"
#include "palette.h"
#include "dostel2.h"
#include "config.h"
#include "windows.h"

extern unsigned char config_byte2;
extern struct palette_str palette;
extern int phone_idx, changed;

extern int get_yesno (int, int, short, short);

void show_conversion_options ()
{
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 35, 16, " " ); WriteString (35, 17, " "); WriteString (35, 18, " ");
    if ( ( config_byte2 & MSK_CONVERT_UPPER) == MSK_CONVERT_UPPER)
        WriteChar ( 35, 17,OPTION_CHECKED );
    else
    {
       if ( ( config_byte2 & MSK_CONVERT_LOWER ) == MSK_CONVERT_LOWER)
           WriteChar ( 35, 16, OPTION_CHECKED );
       else
           WriteChar ( 35, 18, OPTION_CHECKED );
    }
}

void convert_records ()
{
   int i;

    for ( i = 0; i <= phone_idx; i++)
        if ( config_byte2 & MSK_CONVERT_UPPER )
        {
            strcpy ( phone_list[i].name, strupr ( phone_list[i].name));
            strcpy ( phone_list[i].number, strupr (phone_list[i].number));
            strcpy ( phone_list[i].password, strupr ( phone_list[i].password));
        }
        else
        {
            strcpy ( phone_list[i].name, strlwr (phone_list[i].name));
            strcpy ( phone_list[i].number, strlwr (phone_list[i].number));
            strcpy ( phone_list[i].password, strlwr ( phone_list[i].password));
        }
}

int ask_convert_records ()
{
    char         *cnv_buff;
    int          retval;

    retval = FALSE;
    if ((( config_byte2 & MSK_CONVERT_UPPER) == MSK_CONVERT_UPPER) ||
        (( config_byte2 & MSK_CONVERT_LOWER) == MSK_CONVERT_LOWER)
       )
    {
        cnv_buff = (char *)malloc (PORTSIZE (25, 7, 56, 11));
        if (cnv_buff == NULL)
            return;
        save_area (25, 7, 56, 11, cnv_buff);
        SetColourPair (palette.bright_white, palette.red);
        draw_window ( 25, 7, 55, 10);
        SetColourPair (palette.yellow, palette.red);
        WriteString ( 30, 8, "Convert Records Now ?");
        retval = get_yesno (33, 9, palette.yellow, palette.red);
        restore_area ( 25, 7, 56, 11, cnv_buff);
        free ((char *)cnv_buff);
    }
    return (retval);
}

void do_data_conversion ()
{
    int done,ch;
    char *convert_buff;

    convert_buff = (char *)malloc (PORTSIZE (30, 13, 63, 21));
    if (convert_buff == NULL)
        return;

    save_area ( 30, 13, 63, 21, convert_buff);
    SetColourPair (palette.black, palette.green );
    draw_window (30, 13, 62, 20);
    ShowHeader ( 30, 62, 14, "Data Conversion Options");

    WriteString (33, 16, "[   ] Convert To LowerCase");
    WriteString (33, 17, "[   ] Convert To UpperCase");
    WriteString (33, 18, "[   ] No Conversion");
    SetColourPair (palette.red, palette.green);
    WriteString (50, 16, "L"); WriteString (50, 17, "U"); WriteString (39, 18, "N");

    done = FALSE;
    while (! done)
    {
       show_conversion_options();
       ch = toupper (get_key());
       if ( strchr ("LUN", ch) != NULL)
                config_byte2 &= ~MSK_CONVERSIONS;
       switch ( ch )
       {
           case K_ESC : done = TRUE;
                        break;
           case 'L' : config_byte2 |= MSK_CONVERT_LOWER;
                      break;
           case 'U' : config_byte2 |= MSK_CONVERT_UPPER;
                      break;
           case 'N' :
                      /* Do nothing since bits have already been reset */
                      break;
           default  :
                      break;
       }
    }
    changed = ask_convert_records ();
    if ( changed)
        convert_records ();
    restore_area (30, 13, 63, 21, convert_buff);
    free ((char *)convert_buff);
}

#include <stdio.h>
#include <malloc.h>

#include "keys.h"
#include "palette.h"
#include "windows.h"
#include "dostel2.h"
#include "config.h"

extern struct palette_str palette;
extern unsigned char config_byte1;

void show_search_settings ()
{
    if ( ( config_byte1 & MSK_SEARCH_ALL) == MSK_SEARCH_ALL)
    {
        SetColourPair ( palette.black, palette.green);
        WriteString (50, 12, "      NOne");
        SetColourPair ( palette.red, palette.green);
        WriteString (57, 12, "O");
    }
    else
    {
        SetColourPair (palette.black, palette.green);
        WriteString (50,12, "      All ");
        SetColourPair (palette.red, palette.green);
        WriteString (56,12, "A");
    }

    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 36, 12, " ");
    WriteString ( 36, 13, " ");
    WriteString ( 36, 14, " ");
    WriteString ( 36, 15, " ");
    WriteString ( 52, 12, " ");

    if ( config_byte1 & MSK_SEARCH_CASE )
       WriteChar ( 36, 15, OPTION_CHECKED);
    if ( config_byte1 & MSK_SEARCH_NAME)
        WriteChar ( 36, 12, OPTION_CHECKED);
    if ( config_byte1 & MSK_SEARCH_NUMBER)
        WriteChar ( 36, 13, OPTION_CHECKED);
    if ( config_byte1 & MSK_SEARCH_PASSWORD)
        WriteChar (36, 14, OPTION_CHECKED);

}

void do_search_options()
{
     int done, ch;
     char *srch_buffer;

     srch_buffer = (char *)malloc (PORTSIZE (30, 9, 65, 18));
     if (srch_buffer == NULL)
         return;

     save_area ( 30,9, 65, 18, srch_buffer);
     SetColourPair (palette.black, palette.green);
     draw_window (30, 9, 64, 17);
     ShowHeader ( 30, 64, 10, "Search Options");

     WriteString ( 34, 12, "[   ] Name");
     WriteString ( 34, 13, "[   ] NUmber");
     WriteString ( 34, 14, "[   ] Password");
     WriteString ( 34, 15, "[   ] Match Case");
     if ( config_byte1 & MSK_SEARCH_ALL)
        WriteString ( 50, 12, "      NOne");
     else
        WriteString ( 50, 12, "      All ");

     SetColourPair (palette.red, palette.green);
     WriteString (40, 12, "N"); WriteString ( 41,  13, "U"); WriteString (40, 14, "P");
     WriteString (40, 15, "M");
     if ( config_byte1 & MSK_SEARCH_ALL)
         WriteString ( 57, 12, "O");
     else
        WriteString ( 56, 12, "A");

     done = FALSE;
     show_search_settings ();
     while (! done)
     {

         ch = toupper (get_key());

         switch ( ch )
         {
             case 'M'   :
                          if ( config_byte1 & MSK_SEARCH_CASE)
                              config_byte1 &= ~MSK_SEARCH_CASE;
                          else
                              config_byte1 |= MSK_SEARCH_CASE;
                          break;
             case 'N'   :
                          if ( config_byte1 & MSK_SEARCH_NAME)
                                config_byte1 &= ~MSK_SEARCH_NAME;
                          else
                              config_byte1 |= MSK_SEARCH_NAME;
                          break;
             case 'U'   : if ( config_byte1 & MSK_SEARCH_NUMBER)
                                config_byte1 &= ~MSK_SEARCH_NUMBER;
                          else
                              config_byte1 |= MSK_SEARCH_NUMBER;
                          break;
             case 'P'   : if ( config_byte1 & MSK_SEARCH_PASSWORD)
                                config_byte1 &= ~MSK_SEARCH_PASSWORD;
                          else
                                config_byte1 |= MSK_SEARCH_PASSWORD;
                          break;
             case 'A'   :
                          if ((config_byte1 & MSK_SEARCH_ALL) != MSK_SEARCH_ALL)
                                config_byte1 |= MSK_SEARCH_ALL;
                          break;
             case 'O'   :
                         if ( config_byte1 & MSK_SEARCH_ALL)
                             config_byte1 &= ~MSK_SEARCH_ALL;
                         break;
             case K_ESC : done = TRUE;
                          break;
             default    : break;
         }
         show_search_settings ();
    }
    restore_area (30, 9, 65,18,srch_buffer);
    free ((char *)srch_buffer);
}

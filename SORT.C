#include <stdio.h>
#include <malloc.h>

#include "keys.h"
#include "palette.h"
#include "config.h"
#include "windows.h"
#include "dostel2.h"

extern unsigned char config_byte1;
extern struct palette_str palette;

void show_sort_options ()
{
    SetColourPair (palette.bright_white, palette.green);

    if (config_byte1 & MSK_SORT_ORDER)
        WriteString ( 50, 13, "Ascending ");
    else
        WriteString ( 50, 13, "Descending");
    if ( config_byte1 & MSK_SORT_CASE )
        WriteString ( 50, 14, "No ");
    else
        WriteString ( 50, 14, "Yes");

    if ( ( config_byte1 & MSK_SORT_NAME) == MSK_SORT_NAME)
        WriteString (50, 15, "Name     ");
    if (( config_byte1 & MSK_SORT_NUMBER) == MSK_SORT_NUMBER)
        WriteString (50, 15, "Number   ");
    if (( config_byte1 & MSK_SORT_PASSWORD) == MSK_SORT_PASSWORD)
        WriteString (50, 15, "Password ");
}

void get_key_field ()
{
     int done;
     char *key_buff;
     unsigned char k;

     key_buff = (char *)malloc (PORTSIZE (52, 14, 68, 22));
     if (key_buff == NULL)
         return;
     save_area (52,14,68,22,key_buff);
     SetColourPair (palette.black, palette.green);
     draw_window (52, 14, 67, 21);
     ShowHeader (52, 68, 15, "Sort Key");
     SetColourPair (palette.black, palette.green);
     WriteString (55, 17, "Name");
     WriteString (55, 18, "NUmber");
     WriteString (55, 19, "Password");
     SetColourPair (palette.red, palette.green);
     WriteString (55,17,"N"); WriteString (56,18,"U"); WriteString (55,19,"P");
     done = FALSE;
     while (! done)
     {
         k = toupper(get_key());
         if ( strchr ("NUP", k))
         {
             config_byte1 &= ~MSK_SORT_KEY;
             done = TRUE;
         }
         switch (k)
         {
             case  'N'  : config_byte1 |= MSK_SORT_NAME;
                          break;
             case 'U'   : config_byte1 |= MSK_SORT_NUMBER;
                           break;
             case 'P'   : config_byte1 |= MSK_SORT_PASSWORD;
                          break;
             case K_ESC : done = TRUE;
                          break;
             default    : break;
         }
     }
     restore_area (52,14,68,22,key_buff);
     free ((char *)key_buff);
}

void do_sort_options()
{
    char *sort_buffer;
    int done,ch;

    sort_buffer = (char *)malloc (PORTSIZE(30, 10, 66, 18));
    if ( sort_buffer == NULL)
        return;

    save_area ( 30,10,66,18,sort_buffer);
    SetColourPair (palette.black, palette.green);
    draw_window (30,10,65,17);
    ShowHeader ( 30, 65, 11, "Sort Options");

    WriteString (33, 13, "Sort Order     : ");
    WriteString (33, 14, "Case Sensitive : ");
    WriteString (33, 15, "Key Field      : ");
    SetColourPair (palette.red, palette.green);
    WriteString (33,13,"S");
    WriteString (33,14,"C");
    WriteString (33,15,"K");

    done = FALSE;
    while (! done)
    {
        show_sort_options();
        ch = toupper (get_key());
        switch ( ch)
        {
            case K_ESC : done = TRUE;
                         break;
            case 'K'   : get_key_field ();
                         break;
            case 'S'   :
                         if ( config_byte1 & MSK_SORT_ORDER)
                             config_byte1 &= ~MSK_SORT_ORDER;
                         else
                             config_byte1 |= MSK_SORT_ORDER;
                         break;
            case 'C'   :
                         if ( config_byte1 & MSK_SORT_CASE)
                             config_byte1 &= ~MSK_SORT_CASE;
                         else
                             config_byte1 |= MSK_SORT_CASE;
                         break;
            default    :
                          break;
        }
    }
    restore_area ( 30, 10, 66, 18, sort_buffer);
    free ((char *)sort_buffer);
}

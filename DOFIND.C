#include <stdio.h>
#include <malloc.h>
#include "keys.h"
#include "config.h"
#include "dostel2.h"
#include "palette.h"
#include "windows.h"

extern struct palette_str palette;
extern int                current_entry;

int found_search_matches;

int continue_search,
    still_searching;

int offset;
char search_string[80];

void process_find ()
{
    char hotkey[2];
    char *find_buff;
    char *err_buff;
    int done;
    int state;
    static int last_index;

    if (! continue_search)
    {
        find_buff = (char *)malloc (PORTSIZE(27, 9, 58, 15));
        if ( find_buff == NULL)
            return;
        clear_search_tags();
        still_searching = TRUE;
        save_area (27,9,58,15, find_buff);
        SetColourPair (palette.bright_white, palette.blue);
        draw_window(27,9,57,14);
        done = FALSE;
        while (! done)
        {
            show_cursor();
            hotkey[0] = K_ESC;
            hotkey[1]=(char)0;
            SetColourPair (palette.yellow, palette.blue);
            WriteString (34, 11, "Enter Search Text");
            SetColourPair (palette.bright_white, palette.black);
            WriteString ( 32,12,"                    ");
            state = getstr (32,12, search_string, hotkey, 20);
            if ( state != K_ESC)
            {
                    done = TRUE;
                    last_index = find_text ( 0, search_string );
                    continue_search = (last_index != -1);
                    if ( continue_search )
                    {
                         restore_area (27,9,58,15, find_buff);
                         free ((char *)find_buff);
                         found_search_matches = TRUE;
                         phone_list[last_index].search_match = TRUE;
                         offset = (last_index / 11) * 11;
                         current_entry = last_index;
                         show_range ( offset, offset+10);
                    }
                    else
                    {
                         err_buff = (char *)malloc (PORTSIZE(28, 16, 57, 19));
                         if ( err_buff == NULL)
                             return;

                         hide_cursor();
                         save_area (28, 16, 57, 19, err_buff);
                         SetColourPair (palette.bright_white, palette.red);
                         draw_window (28, 16, 56, 18);
                         SetColourPair (palette.yellow, palette.red);
                         WriteString (31, 17, "Search String Not Found");
                         get_key();
                         restore_area (28, 16, 57, 19, err_buff);
                         free ((char*)err_buff);
                         restore_area (27,9,58,15,find_buff);
                         free ((char *)find_buff);
                         still_searching = FALSE;
                     }
             }
             else
             {
                     restore_area (27,9,58,15, find_buff);
                     free ((char *)find_buff);
                     continue_search = FALSE;
                     still_searching = FALSE;
                     done = TRUE;
                     hide_cursor();
              }
        }
    }
    else
    {
         last_index = find_text ( last_index+1, search_string );
         continue_search = (last_index != -1);
         if ( continue_search )
         {
               found_search_matches = TRUE;
               phone_list[last_index].search_match = TRUE;
               offset = ( last_index / 11)*11;
               current_entry = last_index;
               show_range ( offset, offset+10);
         }
         else
         {
               still_searching = FALSE;
               err_buff = (char *)malloc (PORTSIZE (27,16,55,19));
               if (err_buff == NULL)
                   return;
               save_area (27,16,55,19,err_buff);
               SetColourPair (palette.bright_white, palette.red);
               draw_window (27, 16, 54, 18);
               SetColourPair (palette.yellow, palette.red);
               WriteString (30, 17, "No More Matches Found");
               get_key();
               restore_area (27,16,55,19, err_buff);
               free ((char *)err_buff);
               hide_cursor();
          }
     }
}

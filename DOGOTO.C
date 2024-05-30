#include <stdio.h>
#include <malloc.h>
#include "config.h"
#include "palette.h"
#include "dostel2.h"
#include "keys.h"
#include "windows.h"

extern struct palette_str palette;
extern int                offset, current_entry;
extern int phone_idx;

extern int string_is_numeric (char []);

void process_goto ()
{
   char goto_str[9];
   char hot[2];
   int  new_idx;
   char *goto_buff;
   char *err_buff;
   int  done;
   char state;

   done = FALSE;

   hot[0] = (char)K_ESC;
   hot[1] = (char)0;

   goto_buff = (char *)malloc ( PORTSIZE(28, 9, 51, 15));
   if ( goto_buff == NULL)
       return;
   save_area ( 28,9,51,15, goto_buff);
   SetColourPair (palette.bright_white, palette.blue);
   draw_window ( 28,9,50,14);

   while (! done)
   {
       SetColourPair (palette.bright_white, palette.black);
       SetColourPair (palette.yellow, palette.blue);
       WriteString (30, 11, "Goto Which Record ?");
       show_cursor();
       SetColourPair (palette.bright_white, palette.black);
       write_blanks (35, 12, 8);
       SetColourPair (palette.bright_white, palette.black);
       state = getstr ( 35, 12, goto_str, hot, 8);

       if (state != K_ESC)
       {
           if (strnicmp (goto_str, "LAST") == 0)
               new_idx = phone_idx;
           else
           {
               if (strnicmp (goto_str, "FIRST") == 0)
                   new_idx = 0;
               else
                   new_idx = atoi (goto_str) - 1;
           }
           if ((new_idx >= 0) && (new_idx <= phone_idx))
           {
               restore_area (28,9,51,15,goto_buff);
               free ((char*)goto_buff);
               done=TRUE;
               current_entry = offset = new_idx;
               show_range (offset, offset+10);
           }
           else
           {
               err_buff = (char *)malloc (PORTSIZE (25, 16, 57, 19));
               if (err_buff != NULL)
               {
                  hide_cursor();
                  save_area ( 25,16,57,19, err_buff);
                  SetColourPair (palette.bright_white, palette.red);
                  draw_window (25, 16, 56, 18);
                  SetColourPair (palette.yellow, palette.red);
                  if ( string_is_numeric ( goto_str ))
                      WriteString ( 28, 17, "Record Number Out Of Range");
                  else
                      WriteString ( 31, 17, "Entry Must be Numeric");
                  get_key ();
                  restore_area ( 25, 16, 57, 19, err_buff);
                  free ((char *)err_buff);
               }
           }
       }
       else
           done = TRUE;
   }
   restore_area (28, 9, 51, 15, goto_buff);
   free ((char *)goto_buff);
   show_range (offset, offset+10);
}

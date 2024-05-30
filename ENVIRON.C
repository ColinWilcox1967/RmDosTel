#include <stdio.h>
#include <malloc.h>
#include "dostel2.h"
#include "palette.h"
#include "config.h"
#include "windows.h"
#include "keys.h"

extern struct palette_str palette;
extern unsigned char config_byte3;

void show_env_settings()
{
   SetColourPair (palette.bright_white, palette.green);
   if ( config_byte3 & MSK_FILE_READ_TYPE)
       WriteString (56, 16, "New");
   else
       WriteString (56, 16, "Old");
   if ( config_byte3 & MSK_FILE_WRITE_TYPE)
       WriteString (57, 17, "New");
   else
       WriteString (57, 17, "Old");
}

void process_environment ()
{
    char *env_buff;    
    int done;
    unsigned int ch;

    env_buff = (char *)malloc(PORTSIZE(35, 15, 72, 19));
    if (env_buff == NULL)
        return;
    save_area (35, 15, 72, 19, env_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (35, 15, 71, 18);
    WriteString (38, 16, "Read Phonebook As     File Type");
    WriteString (38, 17, "Write Phonebook As     File Type");
    SetColourPair (palette.red, palette.green);
    WriteChar (38, 16, 'R'); WriteChar (38, 17, 'W');
    show_env_settings();

    done = FALSE;
    while (! done)
    {
        show_env_settings();
        ch = toupper(get_key());     
        switch (ch )
        {
              case K_ESC : done = TRUE;
                           break;
              case 'R'   : if (config_byte3 & MSK_FILE_READ_TYPE)
                                config_byte3 &= ~MSK_FILE_READ_TYPE;
                           else
                               config_byte3 |= MSK_FILE_READ_TYPE;
                           break;
              case 'W'   : if ( config_byte3 & MSK_FILE_WRITE_TYPE)
                                config_byte3 &= ~MSK_FILE_WRITE_TYPE;
                           else 
                               config_byte3 |= MSK_FILE_WRITE_TYPE;
                           break;
              default    : break;
        }
    }
    restore_area (35, 15, 72, 19, env_buff);
    free ((char *)env_buff);
}


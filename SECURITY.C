#include <stdio.h>
#include <malloc.h>
#include "keys.h"
#include "config.h"
#include "palette.h"
#include "dostel2.h"
#include "windows.h"

extern unsigned char config_byte2;
extern struct palette_str palette;
extern unsigned char config_byte3;

void show_file_security_options ()
{
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 34, 14, " "); WriteString ( 34, 15, " "); WriteChar (34, 13, ' ');

    if (config_byte2 & MSK_BACKUP_LIST)
       WriteChar ( 34, 14, OPTION_CHECKED);
    if ( config_byte2 & MSK_STORE_DELETED_ENTRIES)
        WriteChar ( 34, 15, OPTION_CHECKED);
    if ( config_byte3 & MSK_SAVE_MERGE_LIST)
        WriteChar ( 34, 13, OPTION_CHECKED);

}

void do_file_security_options()
{
    char *sec_buffer;
    int  done,ch;

    sec_buffer = (char *)malloc ( PORTSIZE( 30, 10, 63, 18));
    if (sec_buffer == NULL)
        return;

    save_area ( 30, 10, 63, 18, sec_buffer);
    SetColourPair (palette.black, palette.green);

    draw_window ( 30, 10, 62, 17);
    ShowHeader ( 30, 62, 11, "File Security Options");

    WriteString ( 32, 13, "[   ] Store Merge Files");
    WriteString ( 32, 14, "[   ] Backup Phone List");
    WriteString ( 32, 15, "[   ] Store Deleted Records");
    SetColourPair (palette.red, palette.green);
    WriteChar (44, 13, 'M');
    WriteChar (45, 14, 'P'); WriteChar (44,15,'D');
    done = FALSE;
    while (! done)
    {
       show_file_security_options();
       ch =toupper (get_key());
       switch ( ch )
       {
           case K_ESC : done=TRUE;
                        break;
           case 'P'   : if (config_byte2 & MSK_BACKUP_LIST)
                            config_byte2 &= ~MSK_BACKUP_LIST;
                        else
                            config_byte2 |= MSK_BACKUP_LIST;
                        break;
           case 'D'   : if ( config_byte2 & MSK_STORE_DELETED_ENTRIES)
                            config_byte2 &= ~MSK_STORE_DELETED_ENTRIES;
                        else
                            config_byte2 |= MSK_STORE_DELETED_ENTRIES;
                        break;
           case 'M'   : if ( config_byte3 & MSK_SAVE_MERGE_LIST)
                            config_byte3 &= ~MSK_SAVE_MERGE_LIST;
                        else
                            config_byte3 |= MSK_SAVE_MERGE_LIST;
                        break;
           default    :
                        break;
       }
    }
    restore_area (30, 10, 63, 18, sec_buffer);
    free ((char *)sec_buffer);
}

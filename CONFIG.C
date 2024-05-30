/* Rewrite 'cos some stupid bastard overwrote the original source */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "keys.h"
#include "config.h"
#include "dostel2.h"
#include "palette.h"
#include "windows.h"

extern struct palette_str palette;
extern unsigned char far *video_base;
extern int file_exists (char []);
extern char datafile[];

unsigned char far *old_base;

unsigned char config_byte1,
              initial_config_byte1,
              config_byte2,
              initial_config_byte2,
              config_byte3,
              initial_config_byte3;
unsigned int  changed;

unsigned int backup_rev;
char         stub[9];
char         backup_name[13];

void get_stub (char name[])
{   
    int i, j;

    memset (stub, 0, 9);

    if ( strchr ( name, '.') == NULL) /* No Extension */
    {
        i = 0;
        while ( i < 8 )
        {
            stub[7 - i] = name[strlen (name) - i - 1];
            i++;
        }
    }
    else
    {
        j = strlen ( name ) - 1;
        while ( name[j] != '.')
            j--;

        i = 0;
        while ( i < 8)
        {
            stub [7 - i] = name [j - i - 1];
            i++;
        }

    }
}

void get_backup_rev (char name[])
{
    char ext[4];
    char tmp[4];
    int done;

    get_stub (name);
    done = FALSE;
    backup_rev = 0;    
    while ( ! done )
    {
        backup_rev++;
        itoa ( backup_rev, ext, 10);
        strcpy ( tmp, strrev ( ext));
        while ( strlen (tmp) < 3)
        strcat ( tmp, "0");
        strcpy ( ext, strrev ( tmp));
        sprintf (backup_name, "%s.%s", stub, ext);
        done = ! file_exists ( backup_name );
    }
}

void do_backup_list()
{
    char *bk_buff;
    char msg[30];
    FILE *srce, *tgt;
    int ch;

    get_backup_rev (datafile);

    if (backup_rev <= MAX_BACKUP_FILES) /* Exceeded 99 file limit */
    {
        bk_buff = (char *)malloc(PORTSIZE(25, 7, 56, 10));
        if ( bk_buff == NULL)
            return;
        save_area ( 25, 7, 56, 10, bk_buff);
        SetColourPair ( palette.bright_white, palette.red);
        draw_window ( 25, 7, 55, 9);
        SetColourPair (palette.yellow, palette.red);

        sprintf (msg, "Backing Up As %s", strupr ( backup_name));
        WriteString ( 25 + ((30 - 14 - strlen ( backup_name)) / 2), 8, msg );

        srce = fopen ( datafile, "rb");
        tgt = fopen ( backup_name, "wb");
        if ( ( srce != NULL) && ( tgt != NULL)) /* Just in case there's a prob */
        {
            while (! feof ( srce ))    
            {
               ch = fgetc (srce);
               if ( ! feof (srce))
                   fputc ( ch, tgt);
            }
            fclose (srce);
            fclose (tgt);
        }
        restore_area ( 25, 7, 56, 10, bk_buff);
    }
    else
    {
        bk_buff = (char *)malloc(PORTSIZE (25, 8, 56, 11));    
        if ( bk_buff == NULL)
            return;
        save_area ( 25, 8, 56, 11, bk_buff);
        SetColourPair (palette.bright_white, palette.red);
        draw_window (25, 8, 55, 10);
        SetColourPair (palette.yellow, palette.red);
        WriteString ( 28, 9, "Backup File Limit Reached");
        get_key();
        restore_area ( 25, 8, 56, 11, bk_buff);
    }
    free ((char *)bk_buff);

}

int save_new_config ()
{
    char *save_config_buff;
    int answer;
    unsigned char config1_saved,
                  config2_saved,
                  config3_saved;


    /* Only give prompt if there are actually any changes */

    if (( config_byte1 == initial_config_byte1) &&
        ( config_byte2 == initial_config_byte2) &&
        ( config_byte3 == initial_config_byte3)
       )
       return (FALSE);

    save_config_buff = (char *)malloc(PORTSIZE(22, 10, 59, 14));
    if ( save_config_buff == NULL)
        return (FALSE);     
    
    /* When displaying the "Save Configuration" Windows,
       use the initial config byte3 for getting the border type
    */

    config1_saved = config_byte1;
    config2_saved = config_byte2;
    config3_saved = config_byte3;
    config_byte1 = initial_config_byte1;
    config_byte2 = initial_config_byte2;
    config_byte3 = initial_config_byte3;

    save_area ( 22, 10, 59, 14, save_config_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window ( 22, 10, 58, 13);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 29, 11, "Save New Configuration ?");
    answer = get_yesno (33, 12, palette.yellow, palette.red);
    restore_area ( 22, 10, 59, 14, save_config_buff);
    free ((char *)save_config_buff);
    
    /* Put it back as it was */

    config_byte1 = config1_saved;
    config_byte2 = config2_saved;
    config_byte3 = config3_saved;
    return ( answer);
}

void configure ()
{
    char *config_buff;
    int   done;
    unsigned int ch;

    config_buff = (char *)malloc(PORTSIZE (20, 7, 61, 20));
    if ( config_buff == NULL)
        return;

    initial_config_byte1 = config_byte1;
    initial_config_byte2 = config_byte2;
    initial_config_byte3 = config_byte3;
    old_base = video_base;

    save_area ( 20, 7, 61, 20, config_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (20, 7, 60, 19);
    ShowHeader (20, 60, 8, "Configuration Options");
    WriteString ( 25, 10, "Search Options");
    WriteString ( 25, 11, "SOrt Options");
    WriteString ( 25, 12, "File Security Options");
    WriteString ( 25, 13, "Video Options");
    WriteString ( 25, 14, "Record Conversion Options");
    WriteString ( 25, 15, "Data File Options");
    WriteString ( 25, 16, "Printer Options");
    WriteString ( 25, 17, "Environment");
    SetColourPair (palette.red, palette.green);
    WriteChar (25, 10, 'S'); WriteChar (26, 11, 'O'); WriteChar (25, 12, 'F');
    WriteChar (25, 13, 'V'); WriteChar (25, 14, 'R'); WriteChar (25, 15, 'D');
    WriteChar (25, 16, 'P'); WriteChar (25, 17, 'E');
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 30, 19, " Make Your Selection ");
    done = FALSE;
    while (! done)
    {
        ch = toupper (get_key());
        while ((strchr ("SOFVRDPE", ch) == NULL) && ( ch != K_ESC))
           ch = toupper(get_key());
        switch ( ch )
        {
            case K_ESC : done = TRUE;
                         break;
            case 'S' : do_search_options ();
                       break;
            case 'O' : do_sort_options ();
                       break;
            case 'F' : do_file_security_options ();
                       break;
            case 'V' : do_video_options();
                       break;
            case 'R' : do_data_conversion();
                       break;
            case 'D' : do_datafile_options();
                       break;
            case 'P' : process_print();
                       break;
            case 'E' : process_environment();
                       break;
        }
    }
    if ( ! save_new_config ())
    {
        config_byte1 = initial_config_byte1;
        config_byte2 = initial_config_byte2;
        config_byte3 = initial_config_byte3;
    }
    else
    {
        initial_config_byte1 = config_byte1;
        initial_config_byte2 = config_byte2;
        initial_config_byte3 = config_byte3;
        if (old_base == (unsigned char far *)MONO_BASE_ADDRESS)
        {
             if ( config_byte2 & 32) /* Either colour mode */
                 change_video_mode ( MONO_TO_COLOUR );
        }    
        else
        {
            
            if ( (config_byte2 & MSK_VIDEO_DETECT) == MSK_VIDEO_DETECT)
                     ;
            else
            {
                if ((config_byte2 & 16) == 16)
                    change_video_mode ( COLOUR_TO_MONO );
            }
        }
    }
    restore_area (20, 7, 61, 20, config_buff);
    free ((char *)config_buff);
}



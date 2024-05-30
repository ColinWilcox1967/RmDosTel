#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "keys.h"
#include "config.h"
#include "telnum.err"
#include "palette.h"
#include "windows.h"
#include "dostel2.h"

#define TRANSIENT_MODE 0
#define INTERNAL_MODE  1
#define EXTERNAL_MODE  2

extern char gen_filename[];
extern unsigned char config_byte2;
extern unsigned char config_byte3;
extern struct palette_str palette;
extern char datafile[];
extern char config_file[];
extern int phone_idx, changed;

int altered_cfg, altered_data;
char trim_name[255];
char external_editor[67];

extern int get_yesno (int, int, short, short);
extern void exec_external (char []);

void trim_filename (char [], int);

void show_datafile_settings()
{
    SetColourPair (palette.bright_white, palette.blue);
    WriteString (35, 18, string (' ', MAX_FILENAME_LENGTH));
    WriteString (35, 20, string (' ', MAX_FILENAME_LENGTH));

    if ( config_file[0] == (char)0)
        WriteString (35, 18, "   ** NONE SPECIFIED **");
    else
    {
        if ( strlen ( config_file) <= MAX_FILENAME_LENGTH)
        {
            WriteString ( 35, 18, config_file);
        }
        else
        {
            WriteString ( 35, 18, "<<");
            trim_filename (config_file, MAX_FILENAME_LENGTH-2);
            WriteString ( 37, 18, trim_name);
        }

    }

    if ( datafile[0] == (char)0)
        WriteString (35, 20, "   ** NONE SPECIFIED **");
    else
    {
        if ( strlen ( datafile) <= MAX_FILENAME_LENGTH)
        {
            WriteString ( 35, 20, datafile);
        }
        else
        {
            WriteString ( 35, 20, "<<");
            trim_filename ( datafile, MAX_FILENAME_LENGTH-2);
            WriteString ( 37, 20, trim_name);
        }
    }
}

int reload_config_file ()
{
   char         *reload_cfg_buff;
   int          retval;

   reload_cfg_buff = (char *)malloc(PORTSIZE (55, 8, 76, 12));
   if ( reload_cfg_buff == NULL)
       return;
   save_area ( 55, 8, 76, 12, reload_cfg_buff);
   SetColourPair (palette.bright_white, palette.red);
   draw_window ( 55, 8, 75, 11);
   SetColourPair (palette.yellow, palette.red);
   WriteString ( 58, 9, "Use This File ?");
   retval = get_yesno (59,10, palette.yellow, palette.red);
   restore_area ( 55, 8, 76, 12, reload_cfg_buff);
   free ((char *)reload_cfg_buff);
   return ( retval );
}

int show_config_file_settings (char name[])
{
    FILE *new_cfg;
    unsigned char byte1, byte2, byte3;
    char str[80], str1[80];                
    
    new_cfg = fopen ( name, "rb");
    fscanf ( new_cfg, "%c%c%c", &byte1, &byte2, &byte3); 
    fclose ( new_cfg);

    SetColourPair (palette.red, palette.green);

    sprintf ( str, " %s ", strupr(name));
    WriteString ( 30+ (47 - strlen (str)) /2,4, str); 
    
    SetColourPair (palette.black, palette.green);

    WriteString (33,  5, "Search On ");   
    WriteString (33,  7, "Sorting On");
    WriteString (33,  9, "Record Conversion");
    WriteString (33, 11, "Data Security");
    WriteString (33, 12, "    Stored Deleted Records ? ");
    WriteString (33, 13, "    Maintain Backup Files  ? ");
    WriteString (33, 15, "Video");
    WriteString (33, 16, "    Mode To Be Used        ? ");
    WriteString (33, 17, "    Exploding Windows      ? ");
    WriteString (33, 18, "    Drop Down Shadows      ? ");
    WriteString (33, 19, "    Screen Writes Using ");

    SetColourPair (palette.bright_white, palette.green);
    
    str[0] = (char)0;
    if ( (byte1 & MSK_SEARCH_ALL) == MSK_SEARCH_ALL)
        strcpy ( str, "Name, Number & Password");
    else
    {
        if ( byte1 & MSK_SEARCH_NAME)    
            strcpy ( str, "Name");
        if ( byte1 & MSK_SEARCH_NUMBER)
        {
            if (strlen ( str) > 0)
                strcat ( str, ",Number");
            else
                strcpy ( str, "Number");
        }
        if ( byte1 & MSK_SEARCH_PASSWORD)
        {
            if ( strlen ( str) > 0)
                strcat ( str, ",Password");
            else
                strcpy ( str, "Password");
        }
    }
    if ( strlen ( str) == 0)
        strcpy ( str, "Nothing");

    if ( byte1 & MSK_SEARCH_CASE)
        strcat ( str, " (Case Sensitive)");
    WriteString ( 35, 6, str);

    if ( (byte1 & MSK_SORT_NAME) == MSK_SORT_NAME)
        strcpy ( str, "Name");    
    if ( (byte1 & MSK_SORT_NUMBER) == MSK_SORT_NUMBER)
        strcpy ( str, "Number");
    if (( byte1 & MSK_SORT_PASSWORD) == MSK_SORT_PASSWORD)
        strcpy ( str, "Password");
    if ( byte1 & MSK_SORT_ORDER)
        strcat ( str, " (Ascending)");
    else
        strcat ( str, " (Descending)");
    WriteString ( 35, 8, str);

    if ( byte2 & MSK_CONVERT_UPPER)
        strcpy ( str, "Upper Case");
    else
    {
        if ( byte2 & MSK_CONVERT_LOWER)
           strcpy ( str, "Lower Case");
        else
           strcpy ( str, "None");
    }
    WriteString (35, 10, str);

    if ( byte2 & MSK_STORE_DELETED_ENTRIES)
        WriteString ( 62, 12, "Yes");
    else
        WriteString ( 62, 12, "No ");

    if ( byte2 & MSK_BACKUP_LIST)
        WriteString ( 62, 13, "Yes");
    else
        WriteString ( 62, 13, "No ");

    if ( (byte2 & MSK_VIDEO_DETECT) == MSK_VIDEO_DETECT)
        strcpy ( str, "Auto Detect");
    else
    {
        if ( byte2 & MSK_VIDEO_COLOUR)
            strcpy ( str, "Colour");
        else
            strcpy ( str, "Monochrome");
    }
    WriteString (62, 16, str);

    if ( byte2 & MSK_WINDOW_TYPE)
        WriteString ( 62, 17, "Exploding");
    else
        WriteString ( 62, 17, "Normal");

    if ( byte2 & MSK_SHADOW_TYPE)
        WriteString ( 62, 18, "Yes");
    else
        WriteString (62, 18, "No");

    if ( byte3 & MSK_MEMORY_WRITE)
        WriteString (57, 19, "Memory");
    else
        WriteString (57, 19, "BIOS");
       
    get_key();
    return (reload_config_file());
}

void trim_filename( char filename[], int max_length)
{
   int  i, cnt;

   if ( strlen (filename) <= max_length)
       strcpy ( trim_name, filename);    
   else
   {
       cnt = 0;
       i = strlen ( filename );
       while ( cnt < max_length)
       {
           trim_name[cnt] = filename[i - max_length];
           i++;
           cnt++;
       }
       trim_name[max_length] = (char)0;
   }
}


void change_config_file ()
{
    char *change_cfg_buff;
    char  new_config_file[255];
    int   state;
    char  hotkeys[3];
     
    hotkeys[0] = (char)K_ESC; hotkeys[1] = (char)K_ALT_L;hotkeys[2]=(char)0;
    SetColourPair ( palette.bright_white, palette.blue);

    write_blanks ( 35, 18, MAX_FILENAME_LENGTH);
    state = getstr (35, 18, new_config_file, hotkeys, MAX_FILENAME_LENGTH);
    if ( state != K_ESC)
    {
        if ( state == K_ALT_L )
        {
            GetFileFromDirectory (TRUE);
            strcpy ( new_config_file, gen_filename);
            trim_filename (gen_filename, MAX_FILENAME_LENGTH);
            strcpy ( gen_filename, trim_name);
            if ( new_config_file[0] != (char)0) /* Display selected file */
            {
                SetColourPair (palette.bright_white, palette.blue);
                WriteString (35, 18, gen_filename);
            }
        }
        if ( file_exists ( new_config_file ))
        {
            change_cfg_buff = (char *)malloc(PORTSIZE(30, 4, 77, 22));    
            if ( change_cfg_buff == NULL)
                return;
            save_area ( 30, 4, 77, 22, change_cfg_buff);
            SetColourPair (palette.black, palette.green);
            draw_window ( 30, 4, 76, 21);
            if (show_config_file_settings( trim_name ))            
            {
                strcpy (config_file, new_config_file);
                altered_cfg = TRUE;
            }
            restore_area ( 30, 4, 77, 22, change_cfg_buff);
            free ((char *)change_cfg_buff);
        }
        else
        {
            if ( new_config_file[0] != (char)0)
                show_error (ERR_NO_FILE);   
        }
    }
    show_datafile_settings();
    hide_cursor();
}

int combine_phonebooks (char name[])
{
    FILE                *telnum_file;
    struct rmdostel_rec *rec_ptr;
    unsigned long       records_in_telnum_file,
                        number_of_records_read;
    struct stat         stat_buffer;
    char                id_buffer[20];

    rec_ptr    = ( struct rmdostel_rec *)malloc ( sizeof ( struct rmdostel_rec ));

    if ( ( rec_ptr == NULL ))
        return ( ERR_NO_MEMORY );

    if ( ! header_ok (name))   /* Summat wrong with data file header */
        return (ERR_BAD_FILE);

    number_of_records_read = 0L;
    stat ( name, &stat_buffer );
    records_in_telnum_file = ( unsigned long )( ( stat_buffer.st_size - strlen (ID_STRING)) / sizeof ( struct rmdostel_rec )
);
    telnum_file = fopen ( name, "rb" );
    fscanf ( telnum_file, "%s\n\r", id_buffer);  /* read V2.x header and trash it */
    do
    {
        fread ( (char *)rec_ptr, sizeof ( struct rmdostel_rec ), 1, telnum_file );
        number_of_records_read++;
        memcpy ( &phone_list[++phone_idx].name[0], &rec_ptr->name[0], sizeof ( struct telnum_str ));
    }
    while ( number_of_records_read < records_in_telnum_file);
    free (( struct rmdostel_rec *)rec_ptr);
    fclose ( telnum_file );
    return (0);
}

void change_phonebook()
{
    char new_phonebook[MAX_FILENAME_LENGTH+1];
    int retval, state;
    char hotkeys[3];
    char *merge_buff;

    hotkeys[0]=(char)K_ESC;hotkeys[1]=(char)K_ALT_L; hotkeys[2] = (char)0;
    SetColourPair (palette.bright_white, palette.blue);
    write_blanks (35, 20, MAX_FILENAME_LENGTH);
    state = getstr (35, 20, new_phonebook, hotkeys, MAX_FILENAME_LENGTH);
    if ( state != K_ESC)
    {
        if ( state == K_ALT_L)
        {
             GetFileFromDirectory(TRUE);
             strcpy (new_phonebook, gen_filename);
             trim_filename (gen_filename, MAX_FILENAME_LENGTH);
             strcpy ( gen_filename, trim_name);
             if ( new_phonebook[0] != (char)0)
             {
                 SetColourPair (palette.bright_white, palette.blue);
                 WriteString (35, 20, gen_filename);
             }
        }
        if (file_exists ( new_phonebook ))
        {
             merge_buff = (char *)malloc(PORTSIZE(22, 8, 59, 12));  
             if (merge_buff == NULL)
                 return;
             save_area (22, 8, 59, 12, merge_buff);
             SetColourPair (palette.bright_white, palette.red);
             draw_window (22, 8, 58, 11);
             SetColourPair (palette.yellow, palette.red);
             WriteString ( 26, 9, "Merge With Current Phonebook ?");
             retval = get_yesno (33,10, palette.yellow, palette.red);
             if ( retval)
             {
                 if (combine_phonebooks( new_phonebook ) == 0)
                     changed = TRUE;
             }
             else
             {
                 clear_phone_list();
                 strcpy ( datafile, new_phonebook);
                 load_numbers ();
                 changed = TRUE;
             }
             restore_area (22, 8, 59, 12, merge_buff);
             free ((char *)merge_buff);
        }
        else
            show_error (ERR_NO_FILE);
    }
    hide_cursor();
}

void shall_we_alter_now ()
{
    char *alter_buff;
    int retval;

    alter_buff = (char *)malloc(PORTSIZE (27, 7, 54, 11));
    if ( alter_buff == NULL)
        return;
    save_area ( 27, 7, 54, 11, alter_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window ( 27, 7, 53, 10);
    SetColourPair (palette.yellow, palette.red);
    WriteString (32, 8, "Reload Data File ?");
    retval = get_yesno(34, 9, palette.yellow, palette.red);
    restore_area (27, 7, 54, 11, alter_buff);
    free ((char *)alter_buff);
    if ( retval )
    {
        if ( altered_cfg )
            load_config_file ();
        if ( altered_data )
            load_numbers ();
    }
}

void show_viewer_settings()
{
   SetColourPair (palette.black, palette.green);
   WriteChar (40, 17, ' ');
   WriteChar (40, 15, ' ');
   write_blanks (55, 15, 5);

   if (config_byte3 & MSK_INTEXT_VIEWER) /* External Viewer */
   {

       WriteChar (44, 18, '[');
       WriteChar (65, 18, ']');
       SetColourPair (palette.black, palette.blue);
       write_blanks (45, 18, 20);
       SetColourPair (palette.bright_white, palette.green);
       WriteChar ( 40, 17, OPTION_CHECKED);
       SetColourPair (palette.red, palette.green);
       WriteString (40, 20, " ALT-L For Directory List ");
   }
   else
   {
       write_blanks (44, 18, 22);
       if ( config_byte3 & MSK_DOUBLE_LINE_BORDER)
           WriteMultiChar (40, 20, H_DOUBLE, 26);
       else
       {
           if ( config_byte3 & MSK_SINGLE_LINE_BORDER)
               WriteMultiChar (40, 20, H_SINGLE, 26);
           else
               write_blanks (40, 20, 26);
       }

       SetColourPair (palette.bright_white, palette.green);
       WriteChar (40, 15, OPTION_CHECKED);
       if (config_byte3 & MSK_DISPLAY_MODE)
           WriteString ( 55, 15, " Hex ");
       else
           WriteString ( 55, 15, "ASCII");
   }
}

void show_ext_path(int state)
{ 
   SetColourPair (palette.black, palette.green);
   if (state)
   {
       WriteChar (44, 18, '[');
       WriteChar (65, 18, ']');
       SetColourPair (palette.bright_white, palette.blue);
       write_blanks (45, 18, 20);
       trim_filename (external_editor, 20);
       WriteString (45, 18, trim_name);
       SetColourPair (palette.black, palette.green);
   }
   else
      write_blanks (44, 18, 22);       
}

void select_file_viewer()
{
   char *viewer_buff;
   int mode,
       status,
       done;
   char ch;
   char hotkeys[3];
   char str[21];

   viewer_buff = (char *)malloc(PORTSIZE(35, 13, 70, 21));
   if (viewer_buff == NULL)
       return;
   save_area (35, 13, 70, 21, viewer_buff);
   SetColourPair (palette.black, palette.green);
   draw_window (35, 13, 69, 20);
   WriteString (38, 15, "[   ] Internal (       )");      
   WriteString (38, 17, "[   ] External");
   SetColourPair (palette.red, palette.green);
   WriteChar ( 44, 15, 'I'); WriteChar (44, 17, 'E');
   done = FALSE;
   hotkeys[0] = (char)K_ESC; hotkeys[1] = (char)K_ALT_L; hotkeys[2] = (char)0;
   mode = TRANSIENT_MODE;
   while (! done)
   {
       show_viewer_settings();
       if (mode != INTERNAL_MODE)
           show_ext_path(TRUE);
       
       switch ( mode )
       {
           case TRANSIENT_MODE : 
                                 ch = toupper(get_key());
                                 switch ( ch)                                   
                                 {
                                     case K_ESC : done = TRUE;
                                                  break;
                                     case 'I'   : mode = INTERNAL_MODE;
                                                  config_byte3 &= ~MSK_INTEXT_VIEWER;
                                                  config_byte3 &= ~MSK_DISPLAY_MODE;
                                                  show_ext_path(FALSE);
                                                  break;
                                     case 'E'   : mode = EXTERNAL_MODE;
                                                  config_byte3 |= MSK_INTEXT_VIEWER;
                                                  show_ext_path(TRUE);
                                                  break;
                                     default    : break;
                                 }
                                 break;
           case INTERNAL_MODE  : ch = toupper(get_key());
                                 switch ( ch )
                                 {
                                     case K_ESC : done = TRUE;
                                                  break;
                                     case 'E'   : mode = TRANSIENT_MODE;
                                                  config_byte3 |= MSK_INTEXT_VIEWER;
                                                  show_ext_path(TRUE);
                                                  break;
                                     case 'I'   : if ( config_byte3 & MSK_DISPLAY_MODE)
                                                      config_byte3 &= ~MSK_DISPLAY_MODE;
                                                  else
                                                      config_byte3 |= MSK_DISPLAY_MODE;
                                                  break;
                                     default    : break;
                                 }
                                 break;
           case EXTERNAL_MODE  : show_ext_path(TRUE);
                                 SetColourPair (palette.bright_white, palette.blue);
                                 status = getstr (45, 18, str, hotkeys, 20);
                                 if ( status == 0)
                                     strcpy (external_editor, str);
                                 else
                                 {
                                     switch ( status )
                                     {
                                         case K_ALT_L  : GetFileFromDirectory(TRUE);
                                                         if ( gen_filename[0] != (char)0)
                                                             strcpy (external_editor, gen_filename);
                                                         break;
                                         case K_ESC    : mode = TRANSIENT_MODE;
                                                         show_ext_path(TRUE);
                                                         break;
                                         default       : break;
                                     }
                                 }
                                 break;
       }
   }
   restore_area (35, 13, 70, 21, viewer_buff);
   free ((char *)viewer_buff);

}

void do_datafile_options ()
{
    char *df_buff;
    unsigned char k;
    int done;

    df_buff = (char *)malloc(PORTSIZE (30, 12, 65, 23));
    if ( df_buff == NULL)
        return;
    save_area (30,12,65,23, df_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (30,12,64,22);
    ShowHeader (30,64,13, "Data File Options");
    SetColourPair (palette.black, palette.green);
    WriteString ( 32, 15, "File Viewer");
    WriteString ( 32, 17, "Configuration File");
    WriteString ( 32, 18, "  [");
    WriteString ( 35, 18, string (' ', MAX_FILENAME_LENGTH));
    WriteString ( 35+MAX_FILENAME_LENGTH, 18, "]");
    WriteString ( 32, 19, "Phonebook File");
    WriteString ( 32, 20, "  [");
    WriteString ( 35, 20, string (' ', MAX_FILENAME_LENGTH));
    WriteString ( 35+MAX_FILENAME_LENGTH, 20, "]");
    SetColourPair (palette.red, palette.green);
    WriteChar (32, 15, 'F'); WriteChar (32,17,'C'); WriteChar (32,19, 'P');
    done = FALSE;
    show_datafile_settings();
    altered_cfg = altered_data = FALSE;
    while (! done)
    {
        k = toupper(get_key());
        if ( (k == 'C') || (k == 'P'))
        {
            SetColourPair (palette.red, palette.green);
            WriteString (35, 22, " ALT-L For Directory List ");
        }
        switch (k)
        {
            case K_ESC : done = TRUE;
                         break;
            case 'F'   :
                         select_file_viewer();
                         break;
            case 'C'   : 
                         change_config_file ();
                         break;
            case 'P'   : 
                         change_phonebook();
                         break;
            default    : break;
        }
    }
    if (altered_cfg || altered_data)
    {
        shall_we_alter_now ();
    }
    restore_area (30,12,65,23,df_buff);
    free ((char *)df_buff);
    show_datafile_name();
}


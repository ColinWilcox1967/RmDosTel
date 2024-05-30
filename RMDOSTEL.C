#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <graph.h>
#include <dos.h>
#include <string.h>
#include <bios.h>
#include <sys\types.h>
#include <time.h>
#include <sys\stat.h>

#include "telnum.err"
#include "palette.h"
#include "dostel2.h"
#include "keys.h"
#include "windows.h"
#include "break.h"
#include "merge.h"
#include "config.h"

extern void process_purge ( void);
extern void clear_phone_list ( void );
extern int add_phone_number ( struct telnum_str *);
extern void configure ( void );
extern void show_datafile_name (void);
extern void load_pr_file ( char *);
extern void set_empty_pr_file ( void);
extern void save_keyboard_flags(void);
extern void restore_keyboard_flags(void);

extern char trim_name[];
extern struct merge_entry merge_list[];

extern int last_undelete;
extern struct telnum_str undeletes[];
extern int changed;
extern unsigned char config_byte1,
                     config_byte2,
                     config_byte3;
extern unsigned char initial_config_byte1,
                     initial_config_byte2,
                     initial_config_byte3;
extern struct palette_str palette;
extern struct palette_str mono_mode_palette;
extern struct palette_str colour_mode_palette;

extern void create_default_printer_file(void);
extern void assign_palette (struct palette_str );

unsigned int records_marked;

short current_foreground,
      current_background;
int   phone_idx=0;
char  gen_filename[255];
struct stat stat_buffer;

int blank_record ( struct telnum_str );
void remove_deleted_record ( int);
void draw_outline (void);
void show_range (int, int);

unsigned char far *video_base;

unsigned long records_in_telnum_file;
unsigned long number_of_records_read;
unsigned int cursor_type;
unsigned int cursor_position;

char config_file[255];
char datafile[255];

FILE *config_file_ptr;
FILE *telnum_file;
int aborted, finished, offset, current_entry;
unsigned int key;
struct telnum_str empty_record;
char gen_str1[255];
char gen_str2[20];
char hotkeys[5];
int ch;

int still_searching,
    continue_search;

int place;
char *winbuffer1;
char *winbuffer2;

unsigned exe_attr;

void lock_essential_files ()
{
    _dos_getfileattr ("RMDOSTEL.EXE", &exe_attr);
    _dos_setfileattr ("RMDOSTEL.EXE", _A_RDONLY);
}

void unlock_essential_files ()
{
    _dos_setfileattr ("RMDOSTEL.EXE", exe_attr);
}

void draw_basic_screen ()
{
    char str[80];

    SetColourPair (palette.bright_white, palette.black);
    sprintf ( str, "RTC RMDOSTEL - Remote DOS Utility, %s [ %s ]", RM_VERSION, __DATE__);
    centre_text ( 1, str);
    centre_text ( 2, "(c) Real Time Control Plc. 1994");

    draw_outline ();
    hide_cursor();
    show_range (offset, offset+10);
}

void show_error ( int err )
{
    char *err_buff;


    err_buff = (char *)malloc (PORTSIZE (25, 7, 56, 11));
    if (err_buff == NULL)
       return;
    hide_cursor();
    save_area (25, 7, 56, 11, err_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (25, 7, 55, 10);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 34, 8, "*** ERROR ***");

    switch ( err )
    {
       case ERR_NO_SPACE     :
                               WriteString (30, 9, "No Environment Space");
                               break;
       case ERR_BAD_INTERPRETER : 
                               WriteString (33, 9, "Bad COMMAND.COM");
                               break;
       case ERR_NO_FILE      :
                               WriteString ( 33, 9, "File Not Found");
                               break;
       case ERR_NO_MEMORY    : 
                               WriteString ( 31, 9, "Insufficient Memory");
                               break;
       case ERR_BAD_FILE     :
                               WriteString ( 29, 9, "Invalid Telephone List");
                               break;
       case ERR_EMPTY_FILE   :
                               WriteString ( 31, 9, "No Records To Export");
                               break;
       case ERR_NO_REMOVE    : 
                               WriteString ( 32, 9, "Cannot Delete Entry");
                               break;
       case ERR_DIR_EXISTS   :
                               WriteString (28, 9, "Directory Already Exists");
                               break;
       default               :
                               WriteString (34, 9, "Unknown Error");
                               break;
    }
    get_key();
    show_cursor();
    restore_area (25, 7,56, 11, err_buff);
    free ((char *)err_buff);
}

void save_numbers ()
{
    struct rmdostel_rec *rec_ptr;
    int                 idx;

    /* If no filename specified, or it cannot be found, write list
       to default RMDOSTEL.TXT
    */

    if ( datafile[0] == (char)0)
        strcpy ( datafile, DEFAULT_TELNUM_FILENAME);

    telnum_file = fopen ( datafile, "wb");

    /* If configured for version 2.x files then weite header */

    if ( config_byte3 & MSK_FILE_WRITE_TYPE)
         fprintf ( telnum_file, "%s\n\r", ID_STRING);
    
    rec_ptr = (struct rmdostel_rec *)malloc ( sizeof ( struct rmdostel_rec ));

    /* if there are no records then drop out immediately */

    if ( phone_idx < 0)
        idx = 1;
    else
        idx = 0;
    while ( idx <= phone_idx )
    {

        memset (&rec_ptr->name[0], 32, sizeof (struct rmdostel_rec)); 
        strcpy ( rec_ptr->name, phone_list[idx].name);
        strcpy ( rec_ptr->number, phone_list[idx].number);
        strcpy ( rec_ptr->password, phone_list[idx++].password);

        fwrite ( (char *)rec_ptr, sizeof ( struct rmdostel_rec ), 1, telnum_file );
    }
    free (( struct rmdostel_rec *)rec_ptr);
    fclose ( telnum_file );
}

int header_ok (char *fn)
{
    FILE *fp;
    char  str[13];
    int compare_ok;

    fp = fopen (fn, "rb");
    fscanf (fp, "%13s\n\r", str);
    fclose (fp);
    compare_ok = (strncmp (str, ID_STRING, strlen (ID_STRING)) == 0);
    return (( compare_ok && (config_byte3 & MSK_FILE_READ_TYPE)) ||
            ( ! compare_ok && ((config_byte3 & MSK_FILE_READ_TYPE) == 0))
           );
}

int load_numbers ()
{
    struct rmdostel_rec *rec_ptr;
    int                 idx;
    char                id_buffer[13];

    if ( ! file_exists (datafile)) /* No File Found */
        return ( ERR_NO_FILE );

    rec_ptr    = ( struct rmdostel_rec *)malloc ( sizeof ( struct rmdostel_rec ));

    if ( ( rec_ptr == NULL ))
        return ( ERR_NO_MEMORY );

    /* If configured for version 2.x files check header */

    if ( ! header_ok (datafile))   /* Summat wrong with data file header ? */
             return (ERR_BAD_FILE);

    /* Only backup phone lists if it's non-empty */

    stat (datafile, &stat_buffer);
    
    if (config_byte3 & MSK_FILE_READ_TYPE)
    {
       if ( ( config_byte2 & MSK_BACKUP_LIST) && (stat_buffer.st_size > strlen (ID_STRING) + 2 ))
            do_backup_list();
       records_in_telnum_file = (unsigned long)((stat_buffer.st_size - strlen (ID_STRING)) / sizeof ( struct rmdostel_rec));
    }
    else
    {
        if ( (config_byte2 & MSK_BACKUP_LIST) && (stat_buffer.st_size > 0))
            do_backup_list();
        records_in_telnum_file = (unsigned long)((stat_buffer.st_size) / sizeof (struct rmdostel_rec));
    }

    number_of_records_read = 0L;

    clear_phone_list ();

    telnum_file = fopen ( datafile, "rb" );

    if ( config_byte3 & MSK_FILE_READ_TYPE)
        fscanf ( telnum_file, "%s\n\r", id_buffer);  /* read V2.x header and trash it */
    else
       id_buffer[0] = (char)0;
    
    if ( stat_buffer.st_size > strlen ( id_buffer) + 2) /* Is there anything after the header */
    {
        do
        {
            fread ( (char *)rec_ptr, sizeof ( struct rmdostel_rec ), 1, telnum_file );
            number_of_records_read++;

            memcpy ( &phone_list[++phone_idx].name[0], &rec_ptr->name[0], sizeof ( struct telnum_str ));
            phone_list[phone_idx].search_match = FALSE;
            phone_list[phone_idx].marked = FALSE;
        }
        while ( number_of_records_read < records_in_telnum_file);
        records_marked = 0;
    }
    free (( struct rmdostel_rec *)rec_ptr);
    fclose ( telnum_file );

    return ( OK );
}

int load_config_file ()
{
    int i;
    int retval;

    hide_cursor();
    retval = OK;
    if ( file_exists ( config_file ) )
    {
        config_file_ptr = fopen ( config_file, "rb");
        fscanf (config_file_ptr, "%c%c%c", &config_byte1, &config_byte2, &config_byte3);
        
        if ( config_byte3 & MSK_SAVE_MERGE_LIST)
            fread ( &merge_list[0].filename[0], sizeof (struct merge_entry), MAX_MERGE_FILES, config_file_ptr);

        if ( config_byte2 & MSK_STORE_DELETED_ENTRIES)
        {
            fscanf ( config_file_ptr, "%d\n", &last_undelete);
            for (i = 0; i <= last_undelete; i++)
                fread ( &undeletes[i], sizeof (struct telnum_str), 1, config_file_ptr);
        }
        fclose (config_file_ptr);
    }
    else
    {
        config_byte1 = DEFAULT_CONFIG_BYTE1;
        config_byte2 = DEFAULT_CONFIG_BYTE2;
        config_byte3 = DEFAULT_CONFIG_BYTE3;
        retval = ERR_NO_FILE;
    }
    initial_config_byte1 = config_byte1;
    initial_config_byte2 = config_byte2;
    initial_config_byte3 = config_byte3;
    return (retval);
}

void report_missing_config()
{
    char *errfile;
    char str[30];

    
    errfile = (char *)malloc (PORTSIZE (22, 9, 59, 13));
    if (errfile == NULL)
        return;
    save_area (22,9,59,13,errfile);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (22,9,58,12);
    SetColourPair (palette.yellow, palette.red);
    trim_filename (config_file, 12);
    sprintf (str, "Cannot Find '%s'", strupr(trim_name));
    WriteString ( 22+((37 - strlen (str))/2), 10, str);
    WriteString ( 27, 11, "Using Default Configuration");
    get_key();
    restore_area (22,9,59,13,errfile);
    free ((char *)errfile);
    strcpy (config_file, DEFAULT_CONFIG_FILENAME);
}

void save_config_file ()
{
    int i;

    config_file_ptr = fopen ( config_file, "wb");
    fprintf ( config_file_ptr, "%c%c%c", config_byte1, config_byte2, config_byte3);

    /* If so configured save merge file list */

    if ( config_byte3 & MSK_SAVE_MERGE_LIST)
        fwrite (&merge_list[0].filename[0], sizeof ( struct merge_entry ), MAX_MERGE_FILES, config_file_ptr);

    if ( ( config_byte2 & MSK_STORE_DELETED_ENTRIES) && (last_undelete != -1))
    {
        fprintf ( config_file_ptr, "%d\n", last_undelete);
        for (i = 0; i <= last_undelete; i++)
            fwrite ( &undeletes[i], sizeof ( struct telnum_str), 1, config_file_ptr );
    }
    fclose (config_file_ptr);
}

void show_general_help ()
{
    hide_cursor();
    SetColourPair (palette.black, palette.green);
    draw_window ( 10, 2, 70, 23);
    ShowHeader ( 10, 70, 3, "General Help");
    WriteString ( 17, 5, "ALT-C      - Configuration.");
    WriteString ( 17, 6, "ALT-E      - Edit Highlighted Record.");
    WriteString ( 17, 7, "ALT-D      - Delete Backup Files.");
    WriteString ( 17, 8, "ALT-M      - Merge Files.");
    WriteString ( 17, 9, "ALT-T      - Convert Foreign Data Files.");
    WriteString ( 17, 10, "ALT-G      - Goto Specified Record Number.");
    WriteString ( 17, 11, "ALT-F      - Find Text String ( Repeat With ALT-F ).");
    WriteString ( 17, 12, "ALT-S      - Sort Records.");
    WriteString ( 17, 13, "ALT-U      - Restore Deleted Records.");
    WriteString ( 17, 14, "ALT-P      - Print Records.");
    WriteString ( 17, 15, "ALT-L      - Select New Phonebook.");
    WriteString ( 17, 16, "HOME/END   - Jump To First/Last Entry.");
    WriteString ( 17, 17, "PGUP/DN    - Display Previous/Next Page.");
    WriteString ( 17, 18, "INS/DEL    - Insert/Delete Record.");
    WriteString ( 17, 19, "CSR UP/DN  - Step Through Records.");
    WriteString ( 17, 20, "ENTER      - Tag/Untag Highlighted Record.");
    WriteString ( 17, 21, "ALT-X      - Quit Program.");
    SetColourPair (palette.bright_white, palette.green);
    centre_text (23, " Press Any Key To Continue ");
    flush_keyboard();
}

void show_help_main ()
{
    int x, y;
    union REGS regs;

    winbuffer1 = (char *)malloc(PORTSIZE (1, 2, 71, 24));
    if (winbuffer1 != NULL)
    {
        save_area ( 1, 2, 71, 24, winbuffer1);
        show_general_help();
        get_key();
        restore_area ( 1, 2, 71,24, winbuffer1);
        free ((char *)winbuffer1);
    }
}


void draw_outline ()
{
    int i;

    SetColourPair (palette.bright_white, palette.cyan);
    _settextposition ( 5, 4);
    _outtext ("É"); _outtext ( string ( 'Í', 25)); _outtext ("Ë");
    _outtext ( string ('Í', 33)); _outtext ("Ë"); _outtext ( string ('Í', 11)); _outtext ("»");
    _settextposition ( 6, 4);
    _outtext ("º"); _outtext (string (' ', 10)); _outtext ("Name"); _outtext (string (' ', 11));
    _outtext ("º"); _outtext (string (' ', 14)); _outtext ("Number"); _outtext (string (' ', 13));
    _outtext ("º"); _outtext (" Password  "); _outtext ("º");
    _settextposition ( 7, 4);
    _outtext ("Ì"); _outtext (string ('Í', 25)); _outtext ("Î"); _outtext ( string ('Í', 33));
    _outtext ("Î"); _outtext (string ('Í', 11)); _outtext ("¹");
    for ( i = 8; i <= 18; i++)
    {
        _settextposition (i, 4);
        _outtext ("º"); _outtext ( string (' ', 25)); _outtext ("º"); _outtext (string (' ', 33));
        _outtext ("º"); _outtext ( string (' ', 11)); _outtext ("º");
    }
    _settextposition (19, 4);
    _outtext ("Ì"); _outtext ( string ('Í', 25)); _outtext ("Ê"); _outtext ( string ('Í', 33));
    _outtext ("Ê"); _outtext (string ('Í', 11)); _outtext ("¹");
    _settextposition ( 20, 4);
    _outtext ("º"); _outtext ( string (' ', 71)); _outtext ("º");
    _settextposition (21, 4);
    _outtext ("È"); _outtext ( string ( 'Í', 71)); _outtext ("¼");
    SetColourPair (palette.red, palette.cyan);
    centre_text ( 21, " Press F1 For Help ");
    SetColourPair (palette.black, palette.cyan);
}

void clear_range ( )
{
    int i;

    for (i = 0; i < 10; i++)
    {
        WriteString ( 6, 8+i, BLANK_NAME);
        WriteString ( 33, 8+i, BLANK_NUMBER);
        WriteString (67, 8+i, BLANK_PASSWORD);
    }
}

void show_tag_count ()
{
    char tmp[5];

    SetColourPair (palette.black, palette.cyan);
    write_blanks (37, 20, 15);
    if (records_marked > 0)
    {
        WriteString (37, 20, "Tagged : ");
        sprintf (tmp, "%-3d", records_marked);
        SetColourPair (palette.red, palette.cyan);
        WriteString ( 46, 20, tmp);
        SetColourPair (palette.black, palette.cyan);
    }
}

void show_markers (int start, int idx)
{
   if ( phone_list[start+idx].marked)
   {
       SetColourPair (palette.red, palette.cyan);
       WriteChar (5, 8 + idx, LEFT_MARKER);
       WriteChar (75, 8 + idx, RIGHT_MARKER);
       SetColourPair (palette.black, palette.cyan);
  }
  else
  {
      SetColourPair (palette.black, palette.cyan);
      WriteChar (5, 8 + idx, ' ');
      WriteChar (75, 8 + idx, ' ');

  }
  show_tag_count();
}

void show_range ( int start, int finish )
{
    int idx;

    hide_cursor();
    SetColourPair (palette.black, palette.cyan);
    for (idx = start; idx <= finish; idx++)
    {
        WriteChar (5, 8 + (idx - start), ' ');
        WriteChar (75, 8 + (idx - start), ' ');

        WriteString ( 6 , 8 + ( idx - start ), BLANK_NAME);
        WriteString ( 33, 8 + ( idx - start ), BLANK_NUMBER);
        WriteString ( 67, 8 + ( idx - start ), BLANK_PASSWORD);
        if ( idx <= phone_idx)
        {
            WriteString ( 6 , 8 + ( idx - start ), phone_list[idx].name );
            WriteString ( 33, 8 + ( idx - start ), phone_list[idx].number );
            WriteString ( 67, 8 + ( idx - start ), phone_list[idx].password );

            show_markers (start, idx - start);
        }
    }
}

void highlight_entry ( int idx, int pos, int state )
{
    if (state)
        SetColourPair ( palette.bright_white, palette.black);
    else
        SetColourPair ( palette.black, palette.cyan);
    WriteString (  6, 8 + pos, "                       ");
    WriteString ( 33, 8 + pos, "                              ");
    WriteString ( 67, 8 + pos, "        ");
    WriteString ( 6 , 8 + pos, phone_list[idx].name );
    WriteString ( 33, 8 + pos, phone_list[idx].number );
    WriteString ( 67, 8 + pos, phone_list[idx].password );
}

void init_video ()
{
    char str[80];
    union REGS regs;

    video_base = (unsigned char far *)COLOUR_BASE_ADDRESS;

    if ( ( config_byte2 & MSK_VIDEO_DETECT) == MSK_VIDEO_DETECT )
    {
        if ( _setvideomode ( _TEXTC80) == 0) /* give priority to CGA then MDA */
        {
            _setvideomode ( _TEXTMONO);
            video_base = (unsigned char far *)MONO_BASE_ADDRESS;
            assign_palette (mono_mode_palette);
        }
        else
            assign_palette (colour_mode_palette);
    }
    else
    {
        /* MONG MODE PROTECTION :

           Ensure that if configured for colour, but CGA mode is not
           present then drop back to mono - well it may happen i suppose

        */

        if ( ( config_byte2 & MSK_VIDEO_COLOUR) == MSK_VIDEO_COLOUR)
        {
            if ( _setvideomode (_TEXTC80) != 0) /* Mode available */
              assign_palette (colour_mode_palette);
            else
            {
                _setvideomode ( _TEXTMONO);
                video_base = (unsigned char far *)MONO_BASE_ADDRESS;
            }
        }
        else
        {
            _setvideomode (_TEXTMONO);
            video_base = (unsigned char far *)MONO_BASE_ADDRESS;
            assign_palette (mono_mode_palette);
        }
    }
    _clearscreen (_GCLEARSCREEN );

    regs.h.ah = 3;
    regs.h.al = 0;
    int86 (0x10, &regs, &regs);
    cursor_type = regs.x.cx;
    cursor_position = regs.x.dx;

    draw_basic_screen();
}

void reset_video ()
{
   union REGS regs;

   _setvideomode (_DEFAULTMODE);

   regs.h.ah = 2;
   regs.h.bh = 0;
   regs.x.dx = cursor_position;
   int86 (0x10, &regs, &regs);

   regs.h.ah=1;
   regs.x.cx = cursor_type;
   int86 (0x10, &regs, &regs );
}

int IsASwitch ( char Param[])
{
    return ((strchr ("-/", Param[0]) != NULL) ||
            ( Param[0] == '?' )
           );
}

void switch_error (int type)
{
    switch (type)
    {
         case HELP_SWITCH     : printf ("\n*** Error : Duplicate Help Switch\n");
                                break;
         case CONFIG_SWITCH   : printf ("\n*** Error : Duplicate /C Switch\n");
                                break;
         case DATAFILE_SWITCH : printf ("\n*** Error : Duplicate /D Switch\n");
                                break;
         case UNKNOWN_SWITCH  : printf ("\n*** Error : Unknown Switch\n");
                                break;
    }
}

void cmd_line_help ()
{
   printf ("\nRTC RMDOSTEL - Remote DOS Utility, %s [ %s ]\n", RM_VERSION, __DATE__); 
   printf ("(c) Real Time Control Plc. 1994\n\n");
   printf ("Usage :-\n\n");
   printf ("        RMDOSTEL [[ /? ] | [[ /D<filename> ] | [ /C<filename> ]]]\n\n");
   printf ("        /?                  - Display This Help Screen.\n");
   printf ("        /D<filename>        - Specify Default Phonebook.\n");
   printf ("        /C<filename>        - Specify Default Configuration File.\n\n");
}

void sort_out_command_line (int ParamCount, char **ParamList)
{
    int i;

    int switch_fault,
        help_switch,
        config_switch,
        datafile_switch;
    
    switch_fault = help_switch = config_switch = datafile_switch = FALSE;

    strcpy (config_file, DEFAULT_CONFIG_FILENAME);
    strcpy (datafile, DEFAULT_TELNUM_FILENAME);

    if (ParamCount == 1) /* No Switches provided */
    {
        /* Do nothing, the default filenames have alreayd been assignd */

    }
    else
    {
        for (i = 1; i < ParamCount; i++)
            if ( IsASwitch (ParamList[i])) 
            {
                if ( strcmp (ParamList[i], "?") == 0)
                {
                    if (! help_switch)
                        help_switch = TRUE;
                    else
                    {
                        switch_error(HELP_SWITCH);
                        switch_fault = TRUE;
                    }
                }
                else
                    switch (toupper(ParamList[i][1]))
                    {
                        case 'H' :
                        case '?' :
                                   if (! help_switch)
                                       help_switch = TRUE;
                                   else
                                   {
                                      switch_error(HELP_SWITCH);
                                      switch_fault = TRUE;
                                   }
                                   break;
                        case 'D' : if (! datafile_switch)
                                      datafile_switch = TRUE;
                                   else
                                   {
                                      switch_error (DATAFILE_SWITCH);
                                      switch_fault = TRUE;
                                   }
                                   break;
                        case 'C' : if (! config_switch)
                                       config_switch = TRUE;
                                   else
                                   {
                                       switch_error (CONFIG_SWITCH);
                                       switch_fault = TRUE;
                                   }
                                   break;
                        default  : switch_error (UNKNOWN_SWITCH);
                                   switch_fault = TRUE;
                                   break;
                   }
            }
        if (! switch_fault)
        {
           for ( i = 1; i < ParamCount; i++)
               if (IsASwitch ( ParamList[i]))
               {
                   if ( ParamList[i][0] == '?')
                   {
                       cmd_line_help();
                       exit(-1);
                   }
                   else
                       switch ( toupper(ParamList[i][1]))
                       {
                           case 'D' : memcpy (&datafile[0], &ParamList[i][2], strlen (ParamList[i])-1);
                                      if (datafile[0] == (char)0)
                                          strcpy ( datafile, DEFAULT_TELNUM_FILENAME);
                                      break;
                           case 'C' : memcpy (&config_file[0], &ParamList[i][2], strlen (ParamList[i])-1);
                                      if (config_file[0] == (char)0)
                                          strcpy ( config_file, DEFAULT_CONFIG_FILENAME);
                                      break;
                           case 'H' :
                           case '?' : cmd_line_help ();
                                      exit(-1);
                                      break;
                       }
               }
        }
        else
            exit(-999);

    }
}

void show_current_record (int current)
{
    char tmp[5];
    
    SetColourPair (palette.black, palette.cyan);
    write_blanks (57, 20, 17);
    WriteString (57, 20, "Record (   /   )");
    SetColourPair (palette.red, palette.cyan);
    if ( phone_idx > 0)
        sprintf (tmp, "%3d", current+1);    
    else
    {
        if ( (phone_idx == 0) && ( current == 0))
            sprintf ( tmp, "%3d", current+1);
        else
            sprintf ( tmp, "%3d", 0);
    }
    WriteString (65, 20, tmp);
    if (phone_idx > 0)
        sprintf (tmp, "%3d", phone_idx+1);
    else
    {
        if ( (phone_idx == 0) && ( current == 0))
            sprintf ( tmp, "%3d", phone_idx+1);
        else
            sprintf ( tmp, "%3d", 0);
    }
    WriteString (69, 20, tmp);
}

int really_save_changes ()
{
    char *save_buff;
    int  save_data;

    save_buff = (char *)malloc (PORTSIZE (26, 9, 55, 13));
    if ( save_buff == NULL)
        return ( TRUE);
    save_area (26,9,55,13,save_buff);
    SetColourPair (palette.yellow, palette.red);
    draw_window (26,9,54, 12);
    SetColourPair (palette.bright_white, palette.red);
    WriteString (34, 10, "Save Changes ?");
    save_data = get_yesno (34, 11, palette.white, palette.red);
    restore_area (26, 9, 55, 13, save_buff);
    free ((char *)save_buff);
    return (save_data);
}

int quit_to_dos()
{
    char *quit_buff;
    int   goto_dos;

    quit_buff = (char *)malloc(PORTSIZE(28, 9, 53, 13));
    if ( quit_buff == NULL)
        return (TRUE);
    save_area (28, 9, 53, 13, quit_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (28, 9, 52, 12);
    SetColourPair (palette.yellow, palette.blue);
    WriteString ( 34, 10, "Exit To DOS ?");
    goto_dos = get_yesno (34, 11, palette.yellow, palette.blue);
    restore_area(28, 9, 53, 13, quit_buff);
    free ((char *)quit_buff);
    return ( goto_dos );
}

void nothing_to_do (char msg[])
{
    char *err_box;

    err_box = (char *)malloc(PORTSIZE(25, 11, 56, 14));
    if ( err_box == NULL)
        return;
    save_area (25, 11, 56, 14, err_box);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (25, 11, 55, 13);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 25 + ((30 - strlen (msg))/2), 12, msg);
    get_key();
    restore_area (25, 11, 56, 14, err_box);
    free ((char *)err_box);
}

void main (int argc, char **argv)
{
    int               status;

    sort_out_command_line (argc, argv );

    save_keyboard_flags();
    lock_essential_files();
    ctrl_c (INTERCEPT);

    /* Ensure DEFAULT.PR always exists in current directory */

    if (! file_exists (DEFAULT_PR_FILE))
         create_default_printer_file();

    aborted = FALSE;
    finished = FALSE;
    current_entry = 0;
    offset = 0;
    continue_search = FALSE;
    still_searching = FALSE;

    strcpy ( empty_record.name, "                       ");
    strcpy ( empty_record.number, "                              ");
    strcpy ( empty_record.password, "        ");

    /* Load an initial printer driver - since DEFAULT.PR now
       always exists we can just load it in */

    load_pr_file ( DEFAULT_PR_FILE );

    status = load_config_file ();
    init_video();
    
    if ( status != OK)
        report_missing_config();

    status = load_numbers ();
    show_current_record (current_entry);
    show_tag_count();

    if ( status != OK )
    {
        show_error ( status );
        datafile[0] = (char)0;
    }
    else
        show_range ( current_entry, current_entry+10);
    show_datafile_name();

    while (! finished )
    {
        SetColourPair ( palette.black, palette.cyan );
        hide_cursor ();

        highlight_entry ( current_entry, current_entry - offset, TRUE);
        
        show_current_record(current_entry);
        show_tag_count();
        key = get_key();
        copy_check (key);

        if ( ( key == K_CSR_UP) || ( key == K_CSR_DN) || ( key == K_PGUP ) ||
             ( key == K_CSR_LT) || ( key == K_CSR_RT) || ( key == K_PGDN ) ||
             ( key == K_HOME) || ( key == K_END)
            )
                highlight_entry ( current_entry, current_entry - offset, FALSE);
         
        /* Extend Test To All Record Tagging Whilst In A TExt Search */

        if ( still_searching && ( key != K_ALT_F) && ( key != K_CR))
        {            
                continue_search = FALSE;
                still_searching = FALSE;
        }
        
        switch ( key )
        {
            case K_F1     : show_help_main();
                            break;
            case K_ALT_C  : changed = FALSE;
                            configure ();
                            if (changed)
                                show_range (offset, offset+10);
                            break;
            case K_ALT_E  : 
                            edit_record ( current_entry );

                            break;
            case K_ALT_G  :  if ( phone_idx > 0) /* only if more than 1 record */
                                 process_goto();
                             else
                                 nothing_to_do ("No Records To Goto");
                             break;
            case K_ALT_D   : process_purge ();
                             break;
            case K_ALT_F  :  if ( phone_idx >= 0)
                                 process_find();
                             else
                                 nothing_to_do ("No Records To Search");
                             break;
            case K_ALT_S  :  if ( phone_idx >= 0)
                             {
                                 process_sort();
                                 offset = 0; current_entry=0;
                                 show_range (offset, offset+10);
                             }
                             else
                                nothing_to_do ("No Records To Sort");
                             break;
            case K_ALT_P  :  if ( phone_idx >= 0)
                                  process_print();
                             else
                                 nothing_to_do ("No Records To Print");
                             break;
            case K_ALT_T  : 
                             file_conversion();
                             clear_range();
                             show_range (offset, offset+10);
                             break;
            case K_ALT_U  :
                             process_undeletes();
                             break;
            case K_CSR_UP :
                             if ( current_entry > 0 )
                             {
                                 if ( current_entry > offset )
                                      current_entry--;
                                 else
                                 {
                                     if ( offset >= 11)
                                     {
                                        offset -= 11;
                                        current_entry--;
                                        show_range ( offset, offset+10);

                                     }
                                 }
                             }
                             break;
            case K_CSR_DN :
                             if ( current_entry < phone_idx )
                             {
                                 if ( current_entry < offset + 10)
                                        current_entry++;
                                 else
                                 {
                                        if ( offset + 11 <= phone_idx)
                                        {
                                            offset += 11;
                                            current_entry++;
                                            show_range (offset, offset+10);
                                        }
                                 }
                             }
                             break;
            case K_PGUP   :  if ( offset >= 11)
                             {
                                 offset -= 11;
                                 current_entry-=11;
                                 show_range (offset, offset+10);
                             }
                             break;
            case K_PGDN   :
                            if ( offset + 11 <= phone_idx )
                            {
                                offset+=11;
                                current_entry+=11;
                                show_range (offset, offset+10);
                            }
                            break;
            case K_END    : 
                             offset = (phone_idx/11) * 11;
                             current_entry = offset;
                             show_range ( offset, offset+10);
                             break;
            case K_HOME   :
                             offset = 0;
                             current_entry = 0;
                             show_range ( current_entry, current_entry+10);
                             break;
            case K_INS    : 
                            
                            status = insert_phone_number ( empty_record, current_entry);
                            if ( status != OK)
                            {
                                show_error ( ERR_NO_MEMORY );
                            }
                            else
                                show_range ( offset, offset+10);
                            break;
            case K_DEL    :
                            update_deleted_records ( phone_list[current_entry]);
                            if ( phone_idx == 0) /* Only one record in file */
                            {
                                strcpy ( phone_list[0].name, BLANK_NAME);
                                strcpy ( phone_list[0].number, BLANK_NUMBER);
                                strcpy ( phone_list[0].password, BLANK_PASSWORD);
                                phone_list[0].marked = FALSE;
                                records_marked = 0;
                                status = OK;
                            }
                            else
                            {
                                if ( phone_list[current_entry].marked)
                                   records_marked--;
                                status = delete_phone_number ( current_entry );
                            }

                            if (status == OK)
                            {
                                phone_idx--;
                                if (phone_idx < 0)
                                {
                                    current_entry = 0;
                                    phone_idx = -1;
                                }
                                else
                                {
                                    if (current_entry > phone_idx)
                                    {    
                                        current_entry--;
                                        if ( current_entry < offset )
                                        {    
                                            if ( offset > 0 )
                                            {
                                                offset -= 11;
                                                current_entry = min (phone_idx, offset + 10);
                                            }
                                            else
                                            {
                                                current_entry = 0;
                                            }
                                        }
                                    }
                                }
                            }
                            show_range ( offset, offset+10);
                            break;
            case K_ALT_M  : do_merge_files();
                            break;
            case K_ALT_X  : finished = quit_to_dos();
                            break;
            case K_CR     : if ( phone_idx > 0)
                            {
                                if ( phone_list[current_entry].marked)
                                {
                                    phone_list[current_entry].marked = FALSE;
                                    records_marked--;
                                }
                                else
                                {
                                    phone_list[current_entry].marked = TRUE;
                                    records_marked++;
                                }
                                show_markers (offset, current_entry - offset);
                            }
                            else
                                nothing_to_do ("Too Few Records To Tag");
                            break;
            case K_ALT_L  :
                            get_new_phonebook();
                            break;

            default       :
                            /* Do Nothing */
                            break;
        }
    }

    if ( really_save_changes ())
    {
        save_config_file ();
        save_numbers();
    }
    ClearDirList();
    reset_video ();
    ctrl_c (RELEASE);
    restore_keyboard_flags();
    unlock_essential_files();
}



#include <stdio.h>
#include <malloc.h>
#include <io.h>
#include <dos.h>
#include <string.h>
#include "windows.h"
#include "config.h"
#include "dostel2.h"
#include "palette.h"
#include "keys.h"
#include "telnum.err"
#include "merge.h"

#define TYPE_UNKNOWN 0
#define TYPE_SDF     1
#define TYPE_ASCII   2
#define TYPE_RMD1    3
#define TYPE_RMD2    4
#define TYPE_CSV     5
#define TYPE_DELIM   6

extern unsigned int phone_idx;
extern imp_name[];
extern char external_editor[];
extern char trim_name[];
extern unsigned char config_byte3;

extern FILE *vwr_file;
extern char gen_filename[];
extern struct palette_str palette;
extern void show_cursor(),
            hide_cursor();
extern int get_yesno (int, int, int, int);
extern void show_error (int);
extern void trim_file_name (char [], int);


struct type_struct types[6] = {{"SDF        ", 0, 'S'},
                               {"ASCII      ", 0, 'A'},
                               {"RMDOSTEL v1", 10, '1'},
                               {"RMDOSTEL v2", 10, '2'},
                               {"CSV        ", 0, 'C'},
                               {"DELIMITED  ", 0, 'D'}
                              };

struct merge_entry merge_list[MAX_MERGE_FILES]={{"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
                                                {"",0,0}, {"",0,0}, {"",0,0}, {"",0,0},
                                                {"",0,0}, {"",0,0}
                                               };


int any_entries_used ()
{
    int i;
    int retval;

    retval = FALSE;
    i = 0;
    while (!retval && ( i < MAX_MERGE_FILES))
        if ( merge_list[i].used)
           retval = TRUE;
        else
           i++;
    return (retval);
}


void show_file_type (int i)
{
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (52, 9+i, 11);
    switch ( merge_list[i].file_type)
    {
        case TYPE_UNKNOWN : WriteString ( 52, 9+i, "???????????");
                            break;
        case TYPE_SDF     : WriteString ( 52, 9+i, "SDF");
                            break;
        case TYPE_ASCII   : WriteString (52, 9+i, "ASCII");
                            break;
        case TYPE_RMD1    : WriteString ( 52, 9+i, "RMDOSTEL v1");
                            break;
        case TYPE_RMD2    : WriteString ( 52, 9+i, "RMDOSTEL v2"); 
                            break;    
        case TYPE_CSV     : WriteString ( 52, 9+i, "CSV");
                            break;
        case TYPE_DELIM   : WriteString ( 52, 9+i, "DELIMITED");
                            break;
    }
}

void show_option (int base, int optnum, int state)
{
    if (state)
        SetColourPair (palette.bright_white, palette.black);
    else
        SetColourPair (palette.black, palette.green);
    WriteString (55, 9+base+optnum, types[optnum].title);
    if ( state)
        SetColourPair (palette.yellow, palette.black);
    else
        SetColourPair (palette.red, palette.green);
    WriteChar (55 + types[optnum].offset, 9+base+optnum, types[optnum].highlight_char);
}

void view_merge_file (char name[])
{
   char *merge_buff;
   struct find_t c_file;

   if (config_byte3 & MSK_INTEXT_VIEWER)
       exec_external (external_editor);
   else
   {
       if ( config_byte3 & MSK_DISPLAY_MODE)
       {
           merge_buff = (char *)malloc(PORTSIZE(1,1,80,23));
           if (merge_buff == NULL)
              return;
           save_area (1,1,80,23, merge_buff);
           DrawFileViewer(name);
           vwr_file = fopen (name,"rb");
   
           _dos_findfirst (name, FILE_ATTR_LIST, &c_file);
           UseViewer (c_file.size);
           fclose(vwr_file);
           restore_area (1,1,80,23, merge_buff);
           free ((char *)merge_buff);
       }
       else
           TextViewer ( name );
   }
}

void show_entry_empty ()
{
   char *empty_buff;

   empty_buff = (char *)malloc(PORTSIZE(27, 8, 54, 11));
   if (empty_buff == NULL)
       return;
   save_area (27, 8, 54, 11, empty_buff);
   SetColourPair (palette.bright_white, palette.red);
   draw_window (27, 8, 53, 10);
   SetColourPair (palette.yellow, palette.red);
   WriteString ( 30, 9, "No Filename In Entry");
   get_key();
   restore_area(27, 8, 54, 11, empty_buff);
   free((char *)empty_buff);
}

int get_file_type ( int i)
{
   char *type_buff;
   int retval, done, number, option;
   unsigned int ch;

   type_buff = (char *)(malloc(PORTSIZE (52, 8+i, 70, 8+i+8)));
   if (type_buff == NULL)
       return;
   save_area (52, 8+i, 70, 8+i+8, type_buff);
   SetColourPair (palette.black, palette.green);
   draw_window (52, 8+i, 69, 8+i+7);
   
   /* Show standard file type list in red on green */

   for (number = 0; number < 6; number++)
      show_option ( i, number, FALSE);
   
   option = 0;
   done = FALSE;
   while (! done)
   {
       show_option (i, option, TRUE);
       ch = toupper(get_key());
       show_option (i, option, FALSE);
       switch (ch )
       {
           case K_CTRL_D   :    
                             if (merge_list[option+i].used)
                             {
                                 merge_list[option+i].filename[0] = (char)0;
                                 merge_list[option+i].file_type = TYPE_UNKNOWN;
                                 merge_list[option+i].used = FALSE;
                                 write_blanks (20, 9+i, 25);
                                 write_blanks (52, 9+i, 11);
                             }
                             break;
           case K_ALT_V  : if ( ! merge_list[option+i].used)
                               show_entry_empty();
                           else
                           {
                               if (access ( merge_list[option+i].filename, 0) != 0)
                                   show_error(ERR_NO_FILE);
                               else
                                   view_merge_file (merge_list[option+i].filename);
                           }
                           break;
           case K_ESC    : done = TRUE;
                           retval = -1;
                           break;
           case K_TAB    : done = TRUE;
                           retval = -999; /* Indicates move to field one in same record */
                           break;
           case K_CSR_UP : option--; 
                           if (option < 0)
                              option = 5;
                           break;
           case K_CSR_DN : option++;
                           if (option > 5)
                              option = 0;
                           break;
           case K_CR     : done = TRUE;
                           retval = option+1;
                           break;
           case 'S'      : retval = TYPE_SDF;
                           done = TRUE;
                           break;
           case 'A'      : retval = TYPE_ASCII;
                           done = TRUE;
                           break;
           case '1'      : retval = TYPE_RMD1;
                           done = TRUE;
                           break;
           case '2'      : retval = TYPE_RMD2;
                           done = TRUE;
                           break;
           case 'C'      : retval = TYPE_CSV;
                           done = TRUE;
                           break;
           case 'D'      : retval = TYPE_DELIM;
                           done = TRUE;
                           break;
           default       : break;
       }
   }
   if ( strlen (merge_list[i].filename) == 0) /* No corresponding filename */
       retval = -999;
   restore_area (52, 8+i, 70, 8+i+8, type_buff);
   free ((char *)type_buff);
   return (retval);
}

void show_merge_entries()
{
    int i;

    SetColourPair (palette.bright_white, palette.black);
    for (i = 0; i < MAX_MERGE_FILES; i++)
       if (merge_list[i].used)
       {
           trim_filename (merge_list[i].filename, 25);
           WriteString (20, 9+i, trim_name);
           show_file_type (i);
       }
}

void draw_merge_skeleton()
{
    int i;
    char str[80];

    SetColourPair (palette.red, palette.blue);
    WriteString ( 32, 5, "Merge File List");
    SetColourPair (palette.light_red, palette.blue);
    WriteString (28, 7, "File Name");
    WriteString ( 53, 7, "File Type");
    for (i = 1; i <= MAX_MERGE_FILES; i++)
    {
        SetColourPair (palette.yellow, palette.blue);
        sprintf ( str, "[%02d]  [                         ]     [           ]", i);        
        WriteString ( 13, 8+i, str);
        SetColourPair (palette.black, palette.black);        
        write_blanks ( 20, 8+i, 25);
        write_blanks ( 52, 8+i, 11);
    }
}

void show_merge_help()
{
    char *merge_help_buff;

    merge_help_buff = (char *)malloc(PORTSIZE(15, 7, 66, 18));
    if (merge_help_buff == NULL)
        return;
    save_area (15, 7, 66, 18, merge_help_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (15, 7, 65, 17);
    ShowHeader (15, 65, 8, "Merge List Help");
    WriteString ( 18, 10, "F1         - Display This Help Page.");
    WriteString ( 18, 11, "ESC        - Abort Merge List Selection.");
    WriteString ( 18, 12, "ALT-V      - View Highlighted File.");
    WriteString ( 18, 13, "CSR-UP/DN  - Move Through Merge List.");
    WriteString ( 18, 14, "TAB        - Move Between Fields.");
    WriteString ( 18, 15, "CTRL-D     - Erase Current Entry.");
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 27, 17, " Press Any Key To Continue ");
    get_key();
    restore_area (15, 7, 66, 18, merge_help_buff);
    free ((char *)merge_help_buff);
}

void merge_file_in (char name[], int file_type)
{
    /* import file name need to be stored in imp_name[] */

    memcpy (&imp_name[0], &name[0], strlen (name)+1);
    switch ( file_type )
    {
        case TYPE_UNKNOWN   : /* do nowt */
                              break;
        case TYPE_SDF       : do_import_sdf (phone_idx);
                              break;
        case TYPE_ASCII     : do_import_ascii (phone_idx);
                              break;
        case TYPE_RMD1      : do_import_rmv1 (phone_idx);
                              break;
        case TYPE_RMD2      : do_import_rmv2 (phone_idx);
                              break;
        case TYPE_CSV       : do_import_csv (phone_idx);
                              break;
        case TYPE_DELIM     : do_import_delimited ( phone_idx);
                              break;
        default             : /* Should never have this case but keep it nice */
                              break;
    }
    hide_cursor();
    SetColourPair (palette.black, palette.green);
}

void merge_file_list ()
{
   char *do_merge_buff;
   char str[10];

   int status;
   int i, line_offset;
   unsigned int records_prior;

   do_merge_buff = (char *)malloc(PORTSIZE (8, 6, 73, 22));
   if (do_merge_buff == NULL)
       return;
   save_area (8, 6, 73, 22, do_merge_buff);
   SetColourPair (palette.black, palette.green);
   draw_window (8, 6, 72, 21);
   SetColourPair (palette.red, palette.green);
   WriteString (17, 8, "Filename");
   WriteString (38, 8, "Type");
   WriteString (50, 8, "Status");
   WriteString (60, 8, "Records");

   line_offset = -1;
   for (i = 0; i < MAX_MERGE_FILES; i++)
       if (merge_list[i].used)
       {
           line_offset++;
           SetColourPair (palette.black, palette.green);
           status = TRUE;
           trim_filename (merge_list[i].filename, 20);
           WriteString ( 11, 10+line_offset, trim_name);
           switch ( merge_list[i].file_type)
           {
               case TYPE_UNKNOWN : WriteString (35, 10+line_offset, "??????");
                                   status = FALSE;
                                   break;
               case TYPE_SDF     : WriteString (35, 10+line_offset, "SDF");
                                   break;
               case TYPE_ASCII   : WriteString (35, 10+line_offset, "ASCII");
                                   break;
               case TYPE_RMD1    : WriteString (35, 10+line_offset, "RMDOSTEL v1");
                                   break;
               case TYPE_RMD2    : WriteString (35, 10+line_offset, "RMDOSTEL v2");
                                   break;
               case TYPE_CSV     : WriteString (35, 10+line_offset, "CSV");
                                   break;
               case TYPE_DELIM   : WriteString (35, 10+line_offset, "DELIMITED");
                                   break;
           }
           if (status)
                status = (access (merge_list[i].filename, 0) == 0);
           if (status)
           {
               WriteString (50, 10+line_offset, "OK");

               records_prior = phone_idx;
               merge_file_in ( merge_list[i].filename, merge_list[i].file_type);
               if ( phone_idx - records_prior < 0)
                   strcpy (str, "000");
               else
                   sprintf ( str, "%03d", phone_idx - records_prior);
               WriteString (60, 10+line_offset, str);
           }
           else
           {
               SetColourPair (palette.yellow, palette.red);
               WriteString (50, 10+line_offset, "ERROR");
               WriteString (60, 10+line_offset, "NONE");
           }
       }
   SetColourPair (palette.bright_white, palette.green);
   WriteString (27, 21, " Press Any Key To Continue ");
   get_key();
   restore_area (8, 6, 73, 22, do_merge_buff);
   free ((char *)do_merge_buff);

}


int shall_we_merge ()
{
   char *merge_buff;
   int retval;

   merge_buff = (char *)malloc(PORTSIZE(25, 8, 56, 12));
   if (merge_buff == NULL)
       return (FALSE);
   save_area (25, 8, 56, 12, merge_buff);
   SetColourPair (palette.bright_white, palette.cyan);
   draw_window (25, 8, 55, 11);
   SetColourPair (palette.black, palette.cyan);
   WriteString (32, 9, "Merge Files Now ?");
   retval = get_yesno(34, 10, palette.black, palette.cyan);
   restore_area (25, 8, 56, 12, merge_buff);
   free ((char *)merge_buff);
   return (retval);
}

void get_merge_list()
{
    int done,
        this_field,
        this_type,
        this_entry;
    unsigned int status;
    char hotkeys[10],
         str[67];
    short x, y;

    hotkeys[0] = (char)K_F1;
    hotkeys[1] = (char)K_TAB;
    hotkeys[2] = (char)K_ESC;
    hotkeys[3] = (char)K_CSR_UP;
    hotkeys[4] = (char)K_CSR_DN;
    hotkeys[5] = (char)K_ALT_V;
    hotkeys[6] = (char)K_ALT_L;
    hotkeys[7] = (char)K_CTRL_D;
    hotkeys[8] = (char)0;

    SetColourPair (palette.bright_white, palette.black);
    done = FALSE;
    this_field = 1; /* start of in Filename field */
    this_entry = 0; /* Start at first entry in list */
    while (! done)
    {
       /* Set marker at start of correct field and record */

       if ( this_field == 1)
          x = 20;
       else
          x = 52;
       y = 9 + this_entry;

       if ( this_field == 1)
       {
           SetColourPair (palette.light_cyan, palette.blue);
           WriteString ( 28, 20, " ALT-L For Directory List ");

           /* A bit of a fudge to get the cursor up on blank entries */

           SetColourPair (palette.bright_white, palette.black);
           if (! merge_list[this_entry].used)
               WriteChar (x, y, ' ');
           show_cursor();
           status = getstr (x, y, str, hotkeys, 25);
           if ( status != 0)
           {
               switch ( status )
               {
                   case K_CTRL_D :  if (merge_list[this_entry].used)   
                                    {
                                         merge_list[this_entry].filename[0] = (char)0;
                                         merge_list[this_entry].used = FALSE;
                                         merge_list[this_entry].file_type = TYPE_UNKNOWN;
                                         write_blanks (20, 9+this_entry, 25);
                                         write_blanks (52, 9+this_entry, 11);
                                    }
                                     break;
                   case K_ALT_L  : GetFileFromDirectory(TRUE);
                                   SetColourPair (palette.bright_white, palette.black);
                                   if ( gen_filename[0] != (char)0)
                                   {
                                       strcpy (merge_list[this_entry].filename, gen_filename);
                                       trim_filename (gen_filename, 25);
                                       WriteString (x, y, trim_name);
                                       this_field = 2;
                                       merge_list[this_entry].used = TRUE;
                                   }
                                   break;
                   case K_ALT_V  : if ( !merge_list[this_entry].used)
                                       show_entry_empty();
                                   else
                                   {
                                        if ( access ( merge_list[this_entry].filename, 0) != 0)
                                            show_error (ERR_NO_FILE);
                                        else
                                            view_merge_file(merge_list[this_entry].filename);
                                   }
                                   break;
                   case K_F1     : show_merge_help();
                                   break;
                   case K_ESC    : done = TRUE;
                                   break;
                   case K_CSR_DN : 
                                   this_entry++;        
                                   if ( this_entry == MAX_MERGE_FILES)
                                        this_entry = 0;
                                   break;
                   case K_CSR_UP : this_entry--;
                                   if (this_entry < 0)
                                        this_entry = MAX_MERGE_FILES - 1;
                                   break;
                   case K_TAB    : if (! merge_list[this_entry].used)
                                       write_blanks (20, 9+this_entry, 25);
                                   this_field = 2;
                                   break;
                   default       : break;

                }
           }
           else
           {
              if ( strlen ( str) > 0)
              {
                  sprintf (str, "%s", strupr (str));
                  strncpy (merge_list[this_entry].filename, str, 66);
                  WriteString (x, y, merge_list[this_entry].filename);
                  this_field = 2;
                  merge_list[this_entry].used = TRUE;
                  write_blanks (x+strlen(merge_list[this_entry].filename), y, 25 - strlen (merge_list[this_entry].filename));
              }
              if ( strlen ( merge_list[this_entry].filename) > 0)
                      this_field = 2;
           }

       }
       else
       {
           SetColourPair (palette.bright_white, palette.blue);
           if ( (config_byte3 & MSK_NO_BORDER) == MSK_NO_BORDER)
                WriteMultiChar (28, 20, ' ', 26);
           else
           {
              if ( ( config_byte3 & MSK_DOUBLE_LINE_BORDER ) == MSK_DOUBLE_LINE_BORDER)
                  WriteMultiChar (28, 20, H_DOUBLE, 26);
              else
                  WriteMultiChar (28, 20, H_SINGLE, 26);
           }

           this_type = get_file_type (this_entry);
           if (( this_type != -1) && (this_type != -999)) /* NOT pressed ESC */
           {
               merge_list[this_entry].file_type = this_type;  
               show_file_type(this_entry);
           }
           this_field = 1;
           if ( this_type != -999)
           {    
                this_entry++;
                if (this_entry == MAX_MERGE_FILES)
                   this_entry = 0;
           }
       }
    }
    if ( any_entries_used ()) /* Something has been entered */
    {
        if (shall_we_merge ()) 
            merge_file_list();
                                 /* let's do it ..... */
    }

    hide_cursor();
}

void do_merge_files()
{
    char *merge_buff;

    merge_buff = (char *)malloc(PORTSIZE(10, 4, 70, 21));
    if ( merge_buff == NULL)
        return;
    save_area (10, 4, 70, 21, merge_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (10, 4, 69, 20);
    draw_merge_skeleton();
    show_merge_entries();
    get_merge_list();    
    restore_area (10, 4, 70, 21, merge_buff);
    free ((char *)merge_buff);
    show_range (0, 10);
}


#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <malloc.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "keys.h"
#include "telnum.err"
#include "config.h"
#include "windows.h"
#include "dostel2.h"
#include "palette.h"
#include "trans.h"

#define QUOTE        34

#define SDF_EXT      ".SDF"
#define ASCII_EXT    ".ASC"
#define RMV1_EXT     ".RD1"
#define RMV2_EXT     ".RD2"
#define CSV_EXT      ".CSV"
#define DEL_EXT      ".DEL"

unsigned int ff_index;

FILE *new_file_ptr;
int  i;
char  exp_name[13];
char imp_name[41];
char export_name[41];
char *export_status_buff;
int  merge_files;
int  export_search_matches;
int  export_marked;

extern void show_datafile_name(void);
extern void clear_phone_list(void);
extern int header_ok (char []);
extern char gen_filename[];
extern struct palette_str palette;
extern char datafile[];
extern char trim_name[];
extern int phone_idx;

extern unsigned int records_marked;

extern int get_yesno (int, int, short, short);
extern void clear_phone_list (void);
extern void show_error(int);
extern void GetFileFromDirectory (int);


int search_matches_exist ()
{
   int i, found;

   i= 0;
   found = FALSE;
   while (( i <= phone_idx) && !found) 
   {
       if ( phone_list[i].search_match)
           found = TRUE;
       else
          i++;
   }
   return ( found );
}

int export_matches ()
{
    char *exp_match_buff;
    int  answer;

    if ( ! search_matches_exist())
        return (FALSE); /* No matches to print */

    exp_match_buff = (char *)malloc(PORTSIZE(20, 8, 61, 12));
    if (exp_match_buff == NULL)
        return (FALSE);
    save_area (20, 8, 61, 12, exp_match_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window(20, 8, 60, 11);
    SetColourPair (palette.yellow, palette.blue);
    WriteString (24, 9, "Export Matches From Last Search ?");
    answer = get_yesno (33, 10, palette.yellow, palette.blue);

    restore_area (20, 8, 61, 12, exp_match_buff);
    free((char *)exp_match_buff);
    return ( answer);
}

int got_extension (char name[])
{
   int finished, result;
   int idx;

   result = finished = FALSE;
   idx = 0;
   while ( ! finished )
   {
       if (name[idx] == '.')
       {    
            finished = result = TRUE;
       }
       else
           idx++;
       if (! finished)
           finished = ( idx == strlen (name));
   }
   return (result);
}

void get_export_name (int ok_no_exists)
{
    char *export_name_buff;
    char hotkey[3];
    int  status,
         done;

    export_name_buff = (char *)malloc (PORTSIZE(15, 9, 66, 15));
    if (export_name_buff == NULL)
        return;
    save_area (15,9,66,15,export_name_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (15,9,65,14);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 28, 14, " ALT-L For Directory List ");
    done = FALSE;
    hotkey[0] = (char)K_ESC; hotkey[1]=(char)K_ALT_L;
    hotkey[2] = (char)0;
    while (! done)
    {
        SetColourPair (palette.yellow, palette.blue);
        WriteString ( 28, 11, "Enter Filename To Export");
        SetColourPair (palette.bright_white, palette.black);
        WriteString (20, 12, string (' ', 40));
        memset (export_name, 0, sizeof (export_name));
        status = getstr (20, 12, export_name, hotkey, 40);
        if ( status == K_ESC)
            done = TRUE;
        else
        {
            if (status == K_ALT_L)
            {
                GetFileFromDirectory (TRUE);
                strcpy ( export_name, gen_filename);
                if ( export_name[0] == (char)0)
                   done = TRUE;
            }
            if (! done)
            {
                if (file_exists (export_name) || ok_no_exists)
                    done = TRUE;
                else
                {
                      show_error (ERR_NO_FILE);
                      hide_cursor();
                }
            }
        }
    }
    hide_cursor();
    restore_area (15, 9, 66, 15, export_name_buff);
    free ((char *)export_name_buff);
}

void get_import_name (int ok_no_exists)
{
    char *import_name_buff;
    char hotkey[3];
    int  status,
         done;

    import_name_buff = (char *)malloc (PORTSIZE(15, 9, 66, 15));
    if (import_name_buff == NULL)
        return;
    save_area (15,9,66,15,import_name_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (15,9,65,14);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 28, 14, " ALT-L For Directory List ");
    done = FALSE;
    hotkey[0] = (char)K_ESC; hotkey[1]=(char)K_ALT_L;
    hotkey[2] = (char)0;
    while (! done)
    {
        SetColourPair (palette.yellow, palette.blue);
        WriteString ( 28, 11, "Enter Filename To Import");
        SetColourPair (palette.bright_white, palette.black);
        WriteString (20, 12, string (' ', 40));
        memset (imp_name, 0, sizeof (imp_name));
        status = getstr (20, 12, imp_name, hotkey, 40);
        if ( status == K_ESC)
            done = TRUE;
        else
        {
            if (status == K_ALT_L)
            {
                GetFileFromDirectory (TRUE);
                strcpy ( imp_name, gen_filename);
                if ( imp_name[0] == (char)0)
                   done = TRUE;
            }
            if (! done)
            {
                if (file_exists (imp_name) || ok_no_exists)
                    done = TRUE;
                else
                {
                      show_error (ERR_NO_FILE);
                      hide_cursor();
                }
            }
        }
    }
    hide_cursor();
    restore_area (15, 9, 66, 15, import_name_buff);
    free ((char *)import_name_buff);
}

void get_name_stub (char *filename)
{
    int idx;

    memset (exp_name, 0, sizeof (exp_name));
    idx = 0;
    while (( filename[idx] != '.') && ( idx < 9 ))
        exp_name[idx++] = filename[idx];
    if ( filename[idx] == '.')
       idx++;
    exp_name[idx] = (char)0;

}

int overwrite_file ( char *name )
{
    char         *exists_buff;
    unsigned int k;
    int          retval;

    exists_buff = (char *)malloc(PORTSIZE (22, 10, 59, 14)); 
    if ( exists_buff == NULL)
        return;
    save_area ( 22, 10, 59, 14, exists_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (22, 10, 58, 13);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 24, 11, "File Already Exists - Overwrite ?");
    retval = get_yesno (33,12, palette.yellow, palette.red);
    restore_area (22, 10, 59, 14, exists_buff);
    free ((char *)exists_buff);
    return ( retval );
}

int show_export_status(char *name)
{
    int write_file;
    char  str[80];
    char name_trim[12];

    write_file = TRUE;
    if ( file_exists (name)) /* Oops! File's already there */   
        write_file = overwrite_file (name);    
    if ( write_file)
    {
        if ( strlen ( name) > 10)
        {    
            trim_filename (name, 10);
            sprintf (str, "Exporting %s To <<%s", datafile, strupr (trim_name));
        }
        else
            sprintf (str, "Exporting %s To %s", datafile, strupr (name));
        export_status_buff  = (char *)malloc (PORTSIZE (20, 10, 64, 15));
        if ( export_status_buff != NULL)
        {
            save_area ( 20, 10, 64, 15, export_status_buff);
            SetColourPair (palette.bright_white, palette.red);
            draw_window ( 20, 10, 63, 14);
            SetColourPair ( palette.yellow, palette.red);
            WriteString ( 21 + (43 - strlen (str))/2, 12, str);
        }
    }
    return ( write_file );
}

int export_marked_records()
{
    char *export_marked_buff;
    int   retval;

    export_marked_buff = (char *)malloc(PORTSIZE(25, 8, 56, 12));
    if (export_marked_buff == NULL)
       return (FALSE);
    save_area (25, 8, 56, 12, export_marked_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window(25, 8, 55, 11);
    SetColourPair (palette.yellow, palette.blue);
    WriteString (29, 9, "Export Marked records ?");
    retval = get_yesno (34, 10, palette.yellow, palette.blue);
    restore_area (25, 8, 56, 12, export_marked_buff);
    free((char *)export_marked_buff);
    return (retval);
}

void export_sdf ()
{
   get_export_name (TRUE);
   if (export_name[0] == (char)0)
       return;

   if (! got_extension (export_name))
   {
       get_name_stub (export_name);   
       strcat (exp_name, SDF_EXT);
   }
   else
       strcpy (exp_name, export_name);

   export_marked = (records_marked > 0) && export_marked_records();
   export_search_matches = export_matches(); 

   if ( show_export_status(exp_name))
   {
       new_file_ptr = fopen ( exp_name, "wb");
       for (i = 0; i <= phone_idx; i++)
       {
           if ( (( export_search_matches ) && (phone_list[i].search_match)) ||
                (( export_marked ) && (phone_list[i].marked)) ||
                 ( ! ( export_search_matches) && ( ! export_marked ))
              )
                  fprintf ( new_file_ptr, "%c%s%c,%c%s%c,%c%s%c\n\r", QUOTE, phone_list[i].name, QUOTE,
                                                                      QUOTE, phone_list[i].number, QUOTE,
                                                                      QUOTE, phone_list[i].password, QUOTE
                          );
       }
       fclose (new_file_ptr);
       if ( export_status_buff != NULL)
       {
           restore_area (20, 10, 64, 15, export_status_buff);
           free ((char *)export_status_buff);
       }
   }
}

void export_ascii ()
{
    get_export_name (TRUE);
    if (export_name[0] == (char)0)
        return;
    
    if (! got_extension (export_name))
    {
        get_name_stub (export_name);
        strcat ( exp_name, ASCII_EXT);
    }
    else
        strcpy (exp_name, export_name);

    export_marked = (records_marked > 0) && export_marked_records();
    export_search_matches = export_matches();

    if (show_export_status(exp_name))
    {
        new_file_ptr = fopen (exp_name, "wb");
        for (i = 0; i <= phone_idx; i++)
        {
            if ((export_search_matches && phone_list[i].search_match) ||
                (export_marked && phone_list[i].marked) ||
                 ( ! (export_search_matches) && (! export_marked))
               )
                   fprintf ( new_file_ptr, "%s\r\n%s\r\n%s\r\n", phone_list[i].name,
                                                                 phone_list[i].number,
                                                                 phone_list[i].password
                           );
        }
        fclose ( new_file_ptr);
        if (export_status_buff != NULL)
        {
            restore_area ( 20, 10, 64, 15, export_status_buff);
            free ((char *)export_status_buff);
        }
    }
}

void export_rmv2 ()
{
    struct rmdostel_rec outrec;
    int    idx;
    int    write_file;

    get_export_name (TRUE);
    if ( export_name[0] == (char)0)
        return;

    if (! got_extension (export_name))
    {
        get_name_stub (export_name);
        strcat (exp_name, RMV2_EXT);
    }
    else
        strcpy (exp_name, export_name);

    export_marked = (records_marked > 0) && export_marked_records();
    export_search_matches = export_matches();

    if (show_export_status(exp_name))
    {
        new_file_ptr = fopen (exp_name, "wb");
        fprintf ( new_file_ptr, "%s\n\r", ID_STRING);
        memset (outrec.unused, ' ', MAX_BLANK_SIZE);
        for (idx = 0; idx <= phone_idx; idx++)
        {
            if ((export_search_matches && phone_list[idx].search_match) ||
                ( export_marked && phone_list[idx].marked) ||
                ((! export_search_matches) && (! export_marked))
               )
            {
                memcpy (&outrec.name[0], &phone_list[idx].name[0], sizeof (struct telnum_str));
                fwrite ((char *)&outrec.name[0], 1, sizeof (struct rmdostel_rec), new_file_ptr); 
            }
        }
        fclose (new_file_ptr);    
        if (export_status_buff != NULL)
        {
            restore_area (20, 10, 64, 15, export_status_buff);
            free ((char *)export_status_buff);
        }
    }
}

void export_rmv1 ()
{
    struct rmdostel_rec outrec;

    get_export_name (TRUE);
    if (export_name[0] == (char)0)
        return;

    if (! got_extension (export_name))
    {
        get_name_stub (export_name);
        strcat (exp_name, RMV1_EXT);
    }
    else
        strcpy (exp_name, export_name);
    
    export_marked = (records_marked > 0) && export_marked_records();
    export_search_matches = export_matches();

    if (show_export_status(exp_name))
    {
        new_file_ptr = fopen (exp_name, "wb");
        memset( outrec.unused, ' ', MAX_BLANK_SIZE);
        for (i = 0; i <= phone_idx; i++)
        {
            if ((export_search_matches &&  phone_list[i].search_match) ||
                 ( export_marked && phone_list[i].marked) ||
                ((! export_search_matches) && (! export_marked))
               )
            {
                memcpy ( &outrec.name[0], &phone_list[i].name[0], sizeof ( outrec ) - MAX_BLANK_SIZE);
                fwrite ( &outrec.name[0], 1, sizeof (struct rmdostel_rec), new_file_ptr);
            }
        }
        
        fclose (new_file_ptr);
        if (export_status_buff != NULL)
        {
            restore_area (20, 10, 64, 15, export_status_buff);
            free ((char *)export_status_buff);
        }
    }
}


/* What a pisser this is - since all the fields are character based, 
   then this is identical to SDFs */

void export_csv()
{
   get_export_name(TRUE);
   if (export_name[0] == (char)0)
       return;
   
   if (! got_extension (export_name))
   {
       get_name_stub (export_name);
       strcat ( exp_name, CSV_EXT);
   }
   else
       strcpy (exp_name, export_name);

   export_marked = (records_marked > 0) && export_marked_records();
   export_search_matches = export_matches();
   if ( show_export_status(exp_name))
   {
       new_file_ptr = fopen ( exp_name, "wb");
       for (i = 0; i <= phone_idx; i++)
       {
           if ((export_search_matches &&  phone_list[i].search_match ) ||
               ( export_marked && phone_list[i].marked) ||
               ((! export_search_matches) && (! export_marked))
              )
           {
               fprintf ( new_file_ptr, "%c%s%c,%c%s%c,%c%s%c\n\r", QUOTE, phone_list[i].name, QUOTE,
                                                                   QUOTE, phone_list[i].number, QUOTE,
                                                                   QUOTE, phone_list[i].password, QUOTE
                       );
           }
       }
       fclose (new_file_ptr);
       if ( export_status_buff != NULL)
       {
           restore_area (20, 10, 64, 15, export_status_buff);
           free ((char *)export_status_buff);
       }
   }
}

char get_delimiter ()
{
    char *delim_buff;
    int chosen;
    int old_delimiter;
    char delim_char;

    delim_buff = (char *)malloc(PORTSIZE(25, 8, 56, 12));
    if (delim_buff == NULL)
        return;
    save_area (25,8,56,12, delim_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window(25,8,55,11);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 28, 11, " Press Ctrl-A For Table ");
    SetColourPair (palette.yellow, palette.blue);
    WriteString ( 33, 9, "Enter Delimiter");
    SetColourPair (palette.bright_white, palette.black);
    WriteChar ( 38, 10, ' ');

    chosen = FALSE;
    old_delimiter = -1; /* No previos Selection */
    while (! chosen)
    {
        delim_char = get_key(); 
        switch ( delim_char)
        {
            case K_ESC    : 
                            chosen = TRUE;
                            break;
            case K_CTRL_A : 
                            delim_char = ascii_table();
                            old_delimiter = delim_char;
                            SetColourPair (palette.bright_white, palette.black);
                            WriteChar (38, 10, delim_char);
                            break;
            case K_CR     : if ( old_delimiter != -1)
                            {
                                chosen = TRUE;
                                delim_char = old_delimiter;
                            }
                            break;
            default       : WriteChar (38, 10, delim_char);
                            old_delimiter = delim_char;
                            break;
        }
    }
    restore_area (25, 8, 56, 12, delim_buff);
    free ((char *)delim_buff);
    return ( delim_char);
}


void export_delimited()
{
    char delimiter;    
    int  i;
    
    delimiter = get_delimiter ();
    if ( delimiter != (char)K_ESC)
    {
        get_export_name (TRUE);
        if (export_name[0] == (char)0)
            return;

        if (! got_extension (export_name))
        {
            get_name_stub(export_name);
            strcat (exp_name, DEL_EXT);
        }
        else
            strcpy (exp_name, export_name);

        export_marked = (records_marked > 0) && export_marked_records();
        export_search_matches = export_matches();

        if (show_export_status(exp_name))
        {
            new_file_ptr = fopen (exp_name, "wb");
            for (i=0; i <= phone_idx; i++)
            {    
                if ((export_search_matches && phone_list[i].search_match) ||
                    ( export_marked && phone_list[i].marked) ||
                    ((! export_search_matches) && (! export_marked))
                   )
                {
                    fprintf ( new_file_ptr, "%s%c%s%c%s\n\r", phone_list[i].name, delimiter,
                                                              phone_list[i].number, delimiter,
                                                              phone_list[i].password
                            );
                }
            }
            fclose (new_file_ptr);
            if (export_status_buff != NULL)
            {   
                restore_area (20, 10, 64, 15, export_status_buff);
                free ((char *)export_status_buff);
            }
        }
    }
}

int merge_file_lists ()
{
   char *merge_buff;
   int   retval;

   merge_buff = (char *)malloc(PORTSIZE (25, 11, 58, 15));
   if ( merge_buff == NULL )
       return ( TRUE );

   save_area ( 25,11, 58, 15, merge_buff);
   SetColourPair ( palette.bright_white, palette.red);
   draw_window ( 25, 11, 57, 14);
   SetColourPair (palette.yellow, palette.red);
   WriteString ( 28, 12, "Merge With Existing Data ?");
   retval = get_yesno(34,13, palette.yellow, palette.red);
   restore_area (25, 11, 58, 15, merge_buff);
   free ((char *)merge_buff);
   return (retval);
}


int start_import_ascii()
{
    int retval;

    retval = -1;
    get_import_name(FALSE);
    if ( imp_name[0] != (char)0)
    {
         if ( file_exists (imp_name))
         {
            if (merge_file_lists())
            {
                if (phone_idx > 0) /* If no current file then treat as append */
                    retval = phone_idx+1;   
                else
                    retval = 0;
            }
            else
                retval = 0;       
         }
         else
         {
             show_error (ERR_NO_FILE);
             retval = -1;
         }
    }
    else
        retval = -1;
    return (retval);
}               

void do_import_ascii (int number)
{
     int max_length_reached,
         field_number,
         field_idx,
         rec_num;
     char field[40];
     char ch;

     new_file_ptr = fopen (imp_name, "rb");

     field_number = 0;
     field_idx = 0;
     rec_num = number;
     while ( ! feof (new_file_ptr) && ( rec_num <= MAX_PHONE_ENTRIES))
     {
           strcpy ( phone_list[rec_num].name, BLANK_NAME);
           strcpy ( phone_list[rec_num].number, BLANK_NUMBER);
           strcpy ( phone_list[rec_num].password, BLANK_PASSWORD);

           while ( field_number < 3) /* Import 3 fields per record */
           {
                max_length_reached = FALSE;
                ch = fgetc (new_file_ptr);
                while (! feof (new_file_ptr) && ( ch != (char)0x0D) && ! max_length_reached)
                {
                    field[field_idx++] = ch;
                    switch ( field_number )
                    {
                        case 0 : max_length_reached = (field_idx == MAX_NAME_SIZE);
                                 break;
                        case 1 : max_length_reached = (field_idx == MAX_NUMBER_SIZE);
                                 break;
                        case 2 : max_length_reached = (field_idx == MAX_PASSWORD_SIZE);
                                 break;
                    }
                    if (! max_length_reached)
                           ch = fgetc (new_file_ptr);
                }
                field[field_idx] = (char)0;
                switch ( field_number )
                {
                   case 0 : strncpy ( phone_list[rec_num].name, field, MAX_NAME_SIZE);
                            break;
                   case 1 : strncpy ( phone_list[rec_num].number, field, MAX_NUMBER_SIZE);
                            break;
                   case 2 : strncpy ( phone_list[rec_num].password, field, MAX_PASSWORD_SIZE);
                            break;
                }
                field_number++;
                field_idx = 0;
                if (max_length_reached)
                {
                    ch = fgetc ( new_file_ptr);
                    while ( !feof (new_file_ptr) && ( ch != (char)0x0A))
                        ch = fgetc (new_file_ptr);
                }
                else
                     ch = fgetc ( new_file_ptr); /* Skip LF */
            }
            phone_list[rec_num].marked = FALSE;
            phone_list[rec_num].search_match = FALSE;
            field_number = 0;
            rec_num++;
    }
    fclose ( new_file_ptr);
    phone_idx = rec_num-2;
}

void import_ascii()
{
   int number;

   number = start_import_ascii();
   if (number >= 0)
       do_import_ascii( number );
}

void sample_sdf (char name[])
{
    char ch;
    int ff_size;
    int place;

    ff_size  = 0;
    ff_index = 0;
    place    = 0; /* 0 = starting quote, 1 = ending quote */

    new_file_ptr = fopen ( name, "rb");
    ch = fgetc ( new_file_ptr);
    while ( ( ch != (char)13) && ( ch != (char)10) && ! feof ( new_file_ptr))
    {
        if ( ch == QUOTE)    
        {
            if ( place == 0 )
            {
                place = 1;
                ff_size = 0;
            }
            else
            {    
                foreign_fields[ff_index].size = ff_size;
                if ( ff_size < MAX_EXAMPLE_LENGTH)
                    foreign_fields[ff_index].example[ff_size]=0;
                else
                    foreign_fields[ff_index].example[MAX_EXAMPLE_LENGTH]=0;

                /* All SDF Fields Are Textual */

                foreign_fields[ff_index].type = TEXT_FIELD;
                ff_size = 0;
                place = 0;
            }
        }
        else
        {
            if ( ch == ',')
            {
                if ( place == 1)
                {
                    ff_size++;
                }
                else
                {
                    ff_index++;
                }
            }
            else
            {
                if ( ff_size < MAX_EXAMPLE_LENGTH)
                    foreign_fields[ff_index].example[ff_size] = (char)ch;
                ff_size++;
            }
        }
        ch = fgetc (new_file_ptr);
    }
    fclose ( new_file_ptr );
}

int start_import_sdf()
{
    int retval;

    retval = -1;
    get_import_name(FALSE);
    if ( imp_name[0] != (char)0)
    {
        if ( file_exists ( imp_name ))
        {
            if (merge_file_lists())
            {
                if (phone_idx > 0)
                    retval = phone_idx+1;
                else
                    retval = 0;
            }
            else
            {
                clear_phone_list();
                retval = 0;
            }
        }
        else
        {
            show_error (ERR_NO_FILE);
            retval = -1;
        }
    }
    return (retval);
}

void do_import_sdf (int number)
{
     int max_length_reached,
         field_number,
         field_idx;
     char ch;
     char field[40];    
     unsigned int rec_num;

     sample_sdf ( imp_name );
     show_sample ( "SDF Field Selection");

     /* read in the file - filtering out crap */

     if ( ! no_mapped_fields ())
     {   
         new_file_ptr = fopen ( imp_name, "rb");
         field_idx=0;
         field_number = 0;
         rec_num = number;
         while (! feof ( new_file_ptr ) && ( rec_num <= MAX_PHONE_ENTRIES))
         {
             ch = fgetc (new_file_ptr);
            
             /* Set un mapped fields to blanks */

             strcpy ( phone_list[rec_num].name, BLANK_NAME);
             strcpy ( phone_list[rec_num].number, BLANK_NUMBER);
             strcpy ( phone_list[rec_num].password, BLANK_PASSWORD);
             while ( ! feof ( new_file_ptr) && (ch != (char)0x0D))
             {
                 /* Read field between quotes */
                    
                 max_length_reached = FALSE;
                 ch = fgetc ( new_file_ptr);
                 while ( ! feof ( new_file_ptr) && (ch != '\"') && (! max_length_reached))
                 {    
                     field[field_idx++]=ch;
                     switch (field_number)
                     {
                         case 0 : max_length_reached = (field_idx > MAX_NAME_SIZE);
                                  break;
                         case 1 : max_length_reached = (field_idx > MAX_NUMBER_SIZE);
                                  break;
                         case 2 : max_length_reached = (field_idx > MAX_PASSWORD_SIZE);
                                  break;
                     }
                     if ( ! max_length_reached)
                        ch = fgetc ( new_file_ptr);
                 }
                 field[field_idx]=0;
    
                 /* Copy field to correct destination - need to be
                    aware of clipping if fields are not copied to
                    original dest. field */
    
                 if ( field_number == mapped_fields.name)
                     strncpy ( phone_list[rec_num].name, field, MAX_NAME_SIZE);
                 if ( field_number == mapped_fields.number)
                     strncpy ( phone_list[rec_num].number, field, MAX_NUMBER_SIZE);
                 if ( field_number == mapped_fields.password)
                     strncpy ( phone_list[rec_num].password, field, MAX_PASSWORD_SIZE);
                 field_number++;
                 field_idx=0;
                 if (max_length_reached)
                 {
                     ch = fgetc (new_file_ptr);
                     while (! feof (new_file_ptr) && ( (ch != (char)QUOTE) && (ch != (char)0x0D)))
                         ch = fgetc (new_file_ptr);
                 }
                 else
                 {
                     ch = fgetc (new_file_ptr); /* read first quote/CR */
                 }
            }
            phone_list[rec_num].marked = FALSE;
            phone_list[rec_num].search_match = FALSE;
            rec_num++;
            field_idx = 0;
            field_number=0;
       }
       fclose ( new_file_ptr);
       phone_idx = rec_num-2;
   }
}

void import_sdf ()
{
    int number;

    number = start_import_sdf();
    if (number >= 0)
       do_import_sdf (number);
}

int start_import_rmv1()
{
    int retval;

    retval = -1;
    get_import_name (FALSE);
    if (imp_name[0] != (char)0)
    {
        if (file_exists (imp_name))
        {
            if (merge_file_lists ())
            {
                if (phone_idx > 0) /* Append ? */
                    retval = phone_idx+1;
                else
                    retval = 0;
            }
            else
            {
                retval = 0;
                clear_phone_list();
            }
        }
        else
        {
            retval = -1;
            show_error (ERR_NO_FILE);
        }
    }
    return (retval);
}

int skip_rest_of_line (char buffer[], int place)
{
    int ch;
    int pos;

    pos = place;
    while ( ( buffer[pos] >= 32) && (pos < sizeof (struct rmdostel_rec)))
       pos++;
    while ( (buffer[pos] < 32)  && (pos < sizeof (struct rmdostel_rec)))
       pos++;
    return (pos);
}

void check_against_rmv1_template (struct rmdostel_rec *ptr)
{
   char buffer[80];
   struct rmdostel_rec new_rec;
   int field_number,
       start,
       offset;

   strcpy (new_rec.name, BLANK_NAME);
   strcpy (new_rec.number, BLANK_NUMBER);
   strcpy (new_rec.password, BLANK_PASSWORD);

   field_number = 0;
   start = offset = 0;
   memcpy (&buffer[0], &ptr->name[0], sizeof (struct rmdostel_rec));

   while ( (field_number < 3) && ( start+offset < sizeof (struct rmdostel_rec)))
   {
       switch ( field_number )    
       {
           case 0 : if ( (offset < MAX_NAME_SIZE) && (buffer[start+offset] >= 32))
                    {
                        new_rec.name[offset] = buffer[start+offset];
                        offset++;
                    }
                    else
                    {
                        new_rec.name[offset] = (char)0;
                        start = skip_rest_of_line(buffer, start+offset);
                        offset = 0;
                        field_number++;
                        
                    }
                    break;
           case 1 : if ( (offset < MAX_NUMBER_SIZE) && ( buffer[start+offset] >=32))
                    {
                        new_rec.number[offset] = buffer[start+offset];
                        offset++;
                    }
                    else
                    {
                       new_rec.number[offset] = (char)0;
                       start = skip_rest_of_line(buffer, start+offset);
                       offset = 0;
                       field_number++;
                    }
                    break;
           case 2 : if ( (offset < MAX_PASSWORD_SIZE) && ( buffer[start+offset] >= 32))
                    {
                        new_rec.password[offset] = buffer[start+offset];
                        offset++;
                    }
                    else
                    {
                        new_rec.password[offset] = (char)0;
                        start = skip_rest_of_line(buffer, start+offset);
                        offset=0;
                        field_number++;
                    }
                    break;
       }

   }
   
   while ( strlen (new_rec.name) < MAX_NAME_SIZE)
       strcat (new_rec.name, " ");
   while ( strlen (new_rec.number) < MAX_NUMBER_SIZE)
      strcat (new_rec.number, " ");
   while ( strlen ( new_rec.password) < MAX_PASSWORD_SIZE)
      strcat ( new_rec.password, " ");

   memcpy (&ptr->name[0], &new_rec.name[0], sizeof (struct rmdostel_rec));

}

void do_import_rmv1 (int number)
{
   unsigned long number_of_records_read,
                 records_in_telnum_file;
   unsigned int rec_num;
   struct stat stat_buffer;         
   struct rmdostel_rec *rec_ptr;

   number_of_records_read = 0L;
   rec_num = (unsigned int)number;
   stat ( imp_name, &stat_buffer );
   records_in_telnum_file = ( unsigned long )( stat_buffer.st_size  / sizeof ( struct rmdostel_rec ));
   rec_ptr = (struct rmdostel_rec *)malloc (sizeof (struct rmdostel_rec));
   new_file_ptr = fopen (imp_name, "rb");
   do
   {
         fread ( (char *)rec_ptr, sizeof ( struct rmdostel_rec ), 1, new_file_ptr );
         number_of_records_read++;

         check_against_rmv1_template (rec_ptr);
         memcpy ( &phone_list[rec_num].name[0], &rec_ptr->name[0], sizeof ( struct telnum_str ));
         phone_list[rec_num].marked = FALSE;
         phone_list[rec_num++].search_match = FALSE;
   }
   while ( (number_of_records_read < records_in_telnum_file) &&
           ( rec_num <= MAX_PHONE_ENTRIES)
         );
   free ((struct rmdostel_rec *)rec_ptr);
   fclose (new_file_ptr);
   phone_idx = rec_num-1;
}

void import_rmv1 ()
{
    int number;

    number = start_import_rmv1();
    if ( number >= 0)
        do_import_rmv1 (number);
}

int start_import_csv()
{
    int retval;

     retval = -1;
     get_import_name(FALSE);
     if (imp_name[0] != (char)0)
     {
         if (file_exists (imp_name))
         {
             if (merge_file_lists())
             {
                 if (phone_idx > 0)
                     retval = phone_idx+1;
                 else
                    retval = 0;
             }
             else
             {
                 clear_phone_list();
                 retval = 0;
             }
         }
         else
         {
             retval = -1;
             show_error (ERR_NO_FILE);
         }
     }
     return (retval);
}

int field_end (int type, char ch)
{
    switch (type)
    {
        case NUMERIC_FIELD : return ( feof (new_file_ptr) ||
                                      (ch == (char)0x0D) ||
                                      (ch == (char)0x0A) ||
                                      (ch == ',')
                                    );
                             break;
        case TEXT_FIELD    : return ( feof (new_file_ptr) ||
                                      (ch == (char)0x0D) ||
                                      (ch == (char)0x0A) ||
                                      (ch == (char)QUOTE)
                                    );
                             break;
        default           : break;
    }
}


void sample_csv (char name[])
{
     int ff_size,
         ff_type,
         field_idx,
         ch;

    new_file_ptr = fopen (name, "rb");
    ff_index = 0;
    field_idx = 0;
    while (! feof (new_file_ptr) && (ch != (char)0x0D) && (ch != (char)0x0A))
    {
        ch = fgetc (new_file_ptr);

        if ( ch == (char)QUOTE)
        {
            ff_type = TEXT_FIELD;
            ch = fgetc (new_file_ptr);
        }
        else
            ff_type = NUMERIC_FIELD;

       ff_size = 0;
       while ( (ff_size < MAX_EXAMPLE_LENGTH) && ! field_end ( ff_type, ch))
       {
           foreign_fields[ff_index].example[ff_size++] = (char)ch;
           ch = fgetc (new_file_ptr);
       }

       if ( ff_size == MAX_EXAMPLE_LENGTH)
       {
           ch = fgetc (new_file_ptr);
           while ( ! field_end (ff_type, ch))
               ch = fgetc (new_file_ptr);
       }

       foreign_fields[ff_index].example[ff_size] = (char)0;
       foreign_fields[ff_index].type = ff_type;
       foreign_fields[ff_index].size = strlen ( foreign_fields[ff_index].example);

       if ( ff_type == TEXT_FIELD)
       {
           ch = fgetc ( new_file_ptr); /* Skip comma */
       }
       ff_index++;
       ff_size = 0;
    }
    fclose (new_file_ptr);
}

void do_import_csv (int number)
{
    sample_csv (imp_name);
    show_sample ("CSV Field Selection");
}

void import_csv ()
{
   int number;
   
   number = start_import_csv ();
   if (number >= 0)
       do_import_csv(number);
}


int start_import_rmv2()
{
    int retval;

    retval = -1;
    get_import_name(FALSE);
    if (imp_name[0] != (char)0)
    {
        if (file_exists (imp_name))
        {
            if (merge_file_lists())
            {
                if (phone_idx > 0)
                    retval = phone_idx+1;
                else
                    retval = 0;
            }
            else
            {
                clear_phone_list();
                retval = 0;
            }
        }
        else
        {
            show_error (ERR_NO_FILE);
            retval = -1;
        }
    }
    return (retval);
}

void do_import_rmv2 (int number)
{
   unsigned int rec_num;
   char id_buffer[13];
   struct rmdostel_rec *rec_ptr;
   unsigned long records_in_telnum_file,
                 number_of_records_read;
   struct stat stat_buffer;

   if (header_ok (imp_name))
   {
        rec_ptr = (struct rmdostel_rec *)malloc (sizeof (struct rmdostel_rec));
        if (rec_ptr == NULL)
             return;

        rec_num = (unsigned int)number;
        number_of_records_read = 0L;
        stat ( imp_name, &stat_buffer );
        records_in_telnum_file = ( unsigned long )( ( stat_buffer.st_size - strlen (ID_STRING)) / sizeof ( struct rmdostel_rec ));
                
        if ( records_in_telnum_file > 0)
        {
             new_file_ptr = fopen ( imp_name, "rb" );
             fscanf ( new_file_ptr, "%s\n\r", id_buffer);  /* read V2.x header and trash it */
             do
             {
                   fread ( (char *)rec_ptr, sizeof ( struct rmdostel_rec ), 1, new_file_ptr );
                   number_of_records_read++;
                   memcpy ( &phone_list[rec_num].name[0], &rec_ptr->name[0], sizeof ( struct telnum_str ));
                   phone_list[rec_num].search_match = FALSE;
                   phone_list[rec_num].marked = FALSE;
                   rec_num++;
             }
             while ( (number_of_records_read < records_in_telnum_file) &&
                     ( rec_num <= MAX_PHONE_ENTRIES)
                   );

             fclose (new_file_ptr);
             phone_idx = rec_num-1;
         }
         free ((struct rmdostel_rec *)rec_ptr);
     }
     else
         show_error (ERR_BAD_FILE);
}

void import_rmv2 ()
{
   int number;

   number = start_import_rmv2();
   if (number >= 0)
       do_import_rmv2 (number);
}

int start_import_delimited()
{
    int retval;

    retval = -1;

/* Rest Of Code Goes Here */

    return (retval);
}

void do_import_delimited (int number)
{
/* Import Code Goes Here */

}

void import_delimited()
{
    int number;

    number = start_import_delimited();
    if (number >= 0)
        do_import_delimited(number);
}

void limit_reached ()
{
    char *limit_buff;

    limit_buff = (char *)malloc(PORTSIZE(25, 8, 56, 12));
    if ( limit_buff == NULL)
       return;
    save_area (25, 8, 56, 12, limit_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (25, 8, 55, 11);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 28, 9, "Maximum Phonebook Size");
    WriteString ( 32, 10, "Has Been Reached");
    get_key();
    restore_area (25, 8, 56, 12, limit_buff);
    free ((char *)limit_buff);
}

int do_import_functions()
{
    char *imp_buff;
    char  k;
    int   retval,done;

    if ( phone_idx == MAX_PHONE_ENTRIES - 1)
    {
        limit_reached();
        return (FALSE);
    }
    imp_buff = (char *)malloc (PORTSIZE(35, 9, 65, 20));
    if (imp_buff == NULL)
        return;
    save_area ( 35, 9, 65, 20, imp_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (35, 9, 64, 19);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 41, 10, "Import File Formats");
    SetColourPair (palette.yellow, palette.blue);
    WriteString ( 38, 12, "SDF");
    WriteString ( 38, 13, "ASCII");
    WriteString ( 38, 14, "RMDOSTEL v1.x");
    WriteString ( 38, 15, "RMDOSTEL v2.x");
    WriteString ( 38, 16, "CSV");
    WriteString ( 38, 17, "Other Delimited");
    SetColourPair (palette.red, palette.blue);
    WriteString (38, 12, "S"); WriteString (38, 13, "A"); WriteString (48, 14, "1");
    WriteString ( 48, 15, "2"); 
    WriteString (38, 16, "C"); WriteString (38, 17, "O");
    done = FALSE;
    while (! done)
    {
        retval = TRUE;
        k = toupper(get_key());
        switch (k)
        {
            case K_ESC : retval = FALSE;
                         done = TRUE;
                         break;
            case 'S'   : import_sdf ();
                         done = TRUE;
                         break;
            case 'A'   : import_ascii ();
                         done = TRUE;
                         break;
            case '1'   : import_rmv1 ();
                         done = TRUE;
                         break;
            case '2'   : import_rmv2();
                         done = TRUE;
                         break;
            case 'O'   : import_delimited();
                         done = TRUE;
                         break;
            case 'C'   : import_csv();
                         done = TRUE;
                         break;
            default    : break;
        }
    }
    restore_area (35, 9, 65, 20, imp_buff);
    free ((char *)imp_buff);

    /* Add check here, that if initially there was no phonebook, but
       we have since imported records, then set the default phonebook now
    */

    if ( ( datafile[0] == (char)0) &&
         (phone_idx >= 0))
    {
           strcpy ( datafile, DEFAULT_TELNUM_FILENAME);
           show_datafile_name();
    }

    return (retval);
}


int do_export_functions()
{
    char *exp_buff;
    int  retval,done;
    char k;

    exp_buff = (char *)malloc (PORTSIZE (35, 10, 65, 21));
    if (exp_buff == NULL)
        return;
    save_area ( 35, 10, 65, 21, exp_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window ( 35, 10, 64, 20);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 41, 11, "Export File Formats");
    SetColourPair (palette.yellow, palette.blue);
    WriteString (38, 13, "SDF");
    WriteString (38, 14, "ASCII");
    WriteString (38, 15, "RMDOSTEL v1.x");
    WriteString (38, 16, "RMDOSTEL v2.x");
    WriteString ( 38, 17, "CSV");
    WriteString (38, 18, "Other Delimited");
    SetColourPair (palette.red, palette.blue);
    WriteString (38, 13, "S"); WriteString (38, 14, "A"); WriteString (48, 15, "1");
    WriteString ( 48, 16, "2");
    WriteString (38, 17, "C"); WriteString (38, 18, "O");
    done = FALSE;
    while (! done )
    {
        retval = TRUE;
        k = toupper(get_key());
        
        switch ( k )
        {
            case K_ESC : retval = FALSE;
                         done = TRUE;
                         break;
            case 'S'   : export_sdf ();
                         break;
            case 'A'   : export_ascii ();
                         break;
            case '1'   : export_rmv1 ();
                         break;
            case '2'   : export_rmv2();
                         break;
            case 'C'   : export_csv();
                         break;
            case 'O'   : export_delimited();
                         break;
            default    : break;
        }
    }
    restore_area ( 35, 10, 65, 21, exp_buff);
    free ((char *)exp_buff);
    return (retval);
}

void file_conversion()
{
    char *impexp_buff;
    int done;
    unsigned char k;

    impexp_buff = (char *)malloc (PORTSIZE(28, 7, 53, 14));
    if (impexp_buff == NULL)
        return;
    save_area (28, 7, 53, 14, impexp_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window ( 28, 7, 52, 13);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 33, 8, "File Conversions");
    SetColourPair (palette.yellow, palette.blue);
    WriteString (32, 10, "Import Functions");
    WriteString (32, 11, "Export Functions");
    SetColourPair (palette.red, palette.blue);
    WriteString ( 32, 10, "I"); WriteString (32, 11, "E");

    done = FALSE;
    while (! done)
    {
        k = toupper(get_key());
        while ( (strchr ("IE", k) == NULL) && ( k != K_ESC))
            k = toupper(get_key());
        if (k != K_ESC)
        {
            switch ( k )
            {
                case 'I' : if ( phone_idx < MAX_PHONE_ENTRIES)
                               do_import_functions ();
                           else 
                               limit_reached();    
                           break;
                case 'E' : if (phone_idx > 0)
                               do_export_functions();
                           else
                           {
                               show_error (ERR_EMPTY_FILE);
                               hide_cursor();
                           }
                           break;
            }
        }
        else 
            done =TRUE;
    }
    restore_area (28, 7, 53, 14, impexp_buff);
    free ((char *)impexp_buff);
}


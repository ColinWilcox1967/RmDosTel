#include <stdio.h>
#include <malloc.h>
#include <bios.h>
#include "keys.h"
#include "palette.h"
#include "dostel2.h"
#include "windows.h"
#include "config.h"

extern struct palette_str palette;
extern int phone_idx;

extern char *winbuffer2;
extern char hotkeys[];
extern int current_entry;
extern int offset;

extern int get_yesno(int, int, short, short);

void show_edit_help ()
{
    int x, y;
    union REGS regs;

    winbuffer2 = ( char *)malloc ( PORTSIZE(20, 11, 65, 20));
    if ( winbuffer2 != NULL)
    {
        save_area ( 20, 11, 65, 20, winbuffer2);
        hide_cursor();
        SetColourPair (palette.black, palette.green);
        draw_window (20, 11, 64, 19);
        ShowHeader ( 20, 64, 12, "Edit Help");
        WriteString ( 24, 14, "F1          - Display This Help Page.");
        WriteString ( 24, 15, "ESC         - Terminate Editing.");
        WriteString ( 24, 16, "TAB         - Move To Next Field.");
        WriteString ( 24, 17, "ENTER       - Confirm Edit Changes.");

        SetColourPair ( palette.bright_white, palette.green);
        WriteString (28, 19, " Press Any Key To Continue ");
        get_key();
        flush_keyboard();
        restore_area ( 20, 11, 65, 20, winbuffer2);
        free (( char *)winbuffer2);
        show_cursor();
        SetColourPair ( palette.bright_white, palette.black);
    }
}

int check_save_record ()
{
    char *save_buff;
    int retval,pos;
    unsigned char k;

    hide_cursor();
    pos = ((current_entry - offset< 5) ? 15 : 8);
    save_buff = (char *)malloc (PORTSIZE (24, pos, 58, pos+4));
    if (save_buff == NULL)
        return;
    save_area (24, pos, 58, pos+4, save_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (24, pos, 57, pos+3);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 26, pos+1, "Save Changes To This Record  ?");
    retval = get_yesno(34, pos+2, palette.yellow, palette.red);
    restore_area (24, pos, 58, pos+4, save_buff);
    free ((char *)save_buff);
    return (retval);
}

int changes_made (struct telnum_str old_record, 
                  struct telnum_str new_record
                 )
{               
    return ( (strcmp (old_record.name, new_record.name) != 0) ||
             (strcmp (old_record.number, new_record.number) != 0) ||
             (strcmp (old_record.password, new_record.password) != 0)
           );
}

void edit_record (int rec_num )
{
    struct telnum_str new_record;
    int field;
    unsigned int ret;
    unsigned int hotkey;
    int stop_editing;

    field=1;
    hotkeys[0] = (char)K_ESC;
    hotkeys[1] = (char)K_F1;
    hotkeys[2] = (char)0;

    strcpy (new_record.name, phone_list[rec_num].name);
    strcpy (new_record.number, phone_list[rec_num].number);
    strcpy (new_record.password, phone_list[rec_num].password);

    show_cursor();
    stop_editing = FALSE;
    while (! stop_editing )
    {
        SetColourPair (palette.bright_white, palette.black);
        switch (field )
        {
            case 1 : write_blanks (6, 8+(rec_num - offset), MAX_NAME_SIZE);
                     hotkey = getstr ( 6, 8 + ( rec_num - offset), new_record.name, hotkeys, MAX_NAME_SIZE);
                     if ( hotkey != (char)0)
                     {
                         switch ( hotkey )
                         {
                             case K_ESC : stop_editing = TRUE;
                                          
                                          break;
                             case K_F1  : show_edit_help();
                                          break;
                         }
                     }
                     else
                     {
                         if ( new_record.name[0] != (char)0)
                         {    
                             write_blanks (6, 8 + (rec_num - offset), MAX_NAME_SIZE - strlen (new_record.name));
                             WriteString ( 6, 8 + (rec_num - offset), new_record.name);
                         }
                         else
                         {
                            if ( strlen (phone_list[rec_num].name) > 0)
                                strcpy ( new_record.name, phone_list[rec_num].name);
                            
                         }
                         field++;
                     }
                     break;
            case 2 : write_blanks (33, 8+(rec_num-offset), MAX_NUMBER_SIZE);
                     hotkey = getstr ( 33, 8 + ( rec_num - offset), new_record.number, hotkeys, MAX_NUMBER_SIZE);
                     if ( hotkey != (char)0)
                     {
                         switch ( hotkey)
                         {
                             case K_ESC : stop_editing = TRUE;
                                          break;
                             case K_F1  : show_edit_help();
                                          break;
                         }
                     }
                     else
                     {
                         if ( new_record.number[0] != (char)0)
                         {    
                              write_blanks (33, 8 + (rec_num - offset), MAX_NUMBER_SIZE - strlen (new_record.number)); 
                              WriteString ( 33, 8 + (rec_num - offset), new_record.number);
                         }
                         else
                         {
                             if (strlen (phone_list[rec_num].number) > 0)
                                  strcpy ( new_record.number, phone_list[rec_num].number);
                         }
                         field++;
                     }
                     break;
            case 3 : write_blanks (67, 8+(rec_num - offset), MAX_PASSWORD_SIZE);
                     hotkey = getstr ( 67, 8 + ( rec_num - offset),  new_record.password, hotkeys, MAX_PASSWORD_SIZE );
                     if (hotkey != (char)0)
                     {
                         switch ( hotkey )
                         {
                             case K_ESC : stop_editing = TRUE;
                                          break;
                             case K_F1  : show_edit_help();
                                          break;
                         }
                     }
                     else
                     {
                          if ( new_record.password[0] != (char)0 )
                          {
                               write_blanks ( 67, 8 + (rec_num - offset), MAX_PASSWORD_SIZE - strlen (new_record.password));
                               WriteString ( 67, 8 + (rec_num - offset), new_record.password);
                          }
                          else
                          {
                              if ( strlen (phone_list[rec_num].password) > 0)
                                  strcpy (new_record.password, phone_list[rec_num].password);
                          }
                          stop_editing = TRUE;
                     }
                     break;
        }
    }
    if ( hotkey != K_ESC) /* Have we aborted out of the edit ? */
    {
        if ( changes_made (phone_list[rec_num], new_record) &&
             check_save_record()
           )
        {
            strcpy ( phone_list[rec_num].name, new_record.name);
            strcpy ( phone_list[rec_num].number, new_record.number);
            strcpy ( phone_list[rec_num].password, new_record.password);

            if (phone_idx < 0) /* No records so effectively add this edit in */
            {
                phone_idx = 0;
            }
               
         }
    }
}

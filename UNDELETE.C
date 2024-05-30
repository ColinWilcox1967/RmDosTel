#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "palette.h"
#include "keys.h"
#include "dostel2.h"
#include "config.h"
#include "windows.h"

extern struct palette_str palette;
extern int                offset, current_entry;
extern int phone_idx;
extern unsigned int records_marked;

struct telnum_str undeletes[MAX_UNDELETES];
int last_undelete=-1;

extern int get_yesno(int, int, short, short);

void remove_deleted_record (int);

void update_deleted_records (struct telnum_str old_rec)
{
    int i;

    if ( blank_record (old_rec))
       return;   /* don't bother storing empty entires */

    if (last_undelete < MAX_UNDELETES - 1)
        last_undelete++;
    else
    {
        for (i = 0; i < MAX_UNDELETES; i++)
            memcpy (&undeletes[i].name[0], &undeletes[i+1].name[0], sizeof (struct telnum_str));
        last_undelete = MAX_UNDELETES - 1;
    }
    memcpy (&undeletes[last_undelete].name[0], &old_rec.name[0], sizeof (undeletes[last_undelete]));
}

void show_undelete_help ()
{
    char *undel_help_buff;

    undel_help_buff = (char*)malloc (PORTSIZE(15,9, 68, 19));
    if (undel_help_buff == NULL)
        return;
    save_area (15, 9,68,19,undel_help_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (15,9,67,18);
    ShowHeader (15, 67, 10, "Undelete Help");
    WriteString (18, 12, "F1              - Display This Help Page.");
    WriteString (18, 13, "CSR UP/DN       - Move Between Deleted Records.");
    WriteString (18, 14, "ESC             - Abort Record Undeletion.");
    WriteString (18, 15, "ENTER           - Select Highlighted Record.");
    WriteString (18, 16, "DEL             - Delete Highlighted Record.");
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 28, 18, " Press Any Key To Continue ");
    SetColourPair (palette.black, palette.green);
    get_key();
    restore_area (15, 9,68,19,undel_help_buff);
    free ((char*)undel_help_buff);
}

void redisplay_undel_list ()
{
    int i;

    SetColourPair (palette.bright_white, palette.blue);
    for (i = 0; i <= last_undelete + 1;i++)
    {
        write_blanks (26, 9+i, 2);
        write_blanks (27, 9+i, MAX_NAME_SIZE+1);
        write_blanks (27+2+MAX_NAME_SIZE, 9+i, 2);
    }
    for (i = last_undelete; i >= 0; i--)
    {
        if ( undeletes[i].marked)
        {
            SetColourPair (palette.red, palette.blue);
            WriteChar ( 27, 9+(last_undelete-i), LEFT_MARKER);
            WriteChar ( 27+2+MAX_NAME_SIZE, 9+(last_undelete-i), RIGHT_MARKER);
        }
        SetColourPair (palette.bright_white, palette.blue);
        WriteString ( 28, 9 + (last_undelete - i), undeletes[i].name);
    }
}


int choose_deleted_record ()
{
    int idx;
    int done;
    int chosen;
    unsigned int key;

    done = FALSE;
    idx=last_undelete;
    chosen=-1;
    while (! done)
    {
        SetColourPair (palette.yellow, palette.black);
        WriteString (28, 9+(last_undelete-idx), undeletes[idx].name);
        key = get_key();
        SetColourPair (palette.bright_white, palette.blue);
        WriteString (28, 9+(last_undelete-idx), undeletes[idx].name);
        switch (key)
        {
            case K_F1     : show_undelete_help();
                            break;
            case K_DEL    : if ( last_undelete >= 0)
                            {
                                remove_deleted_record ( idx );
                                if (idx > last_undelete)
                                    idx = last_undelete;
                                redisplay_undel_list ();
                                if (idx > 0)
                                    idx--;

                                if ( last_undelete < 0)
                                    done = TRUE;
                            }
                            break;
            case K_ESC    : done = TRUE;
                            chosen=-1;
                            break;
            case K_CSR_DN :
                            idx--;
                            if (idx < 0)
                                idx = last_undelete;
                            break;
            case K_CSR_UP :
                            idx++;
                            if ( idx > last_undelete)
                                idx=0;
                            break;
            case K_CR     : done = TRUE;
                            chosen = idx;
                            break;
            default       :
                            break;
        }
    }
    return ( chosen );
}

int recover_record ()
{
    char *yn_buff;
    int retval;

    yn_buff = (char *)malloc (PORTSIZE (25, 12, 56, 16));
    if (yn_buff == NULL)
        return;
    save_area (25,12,56,16,yn_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (25,12,55,15);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 30, 13, "Recover This Record ? ");
    retval = get_yesno(34,14, palette.yellow, palette.red);
    restore_area (25, 12, 56, 16, yn_buff);
    free ((char *)yn_buff);
    return (retval);
}

void insert_deleted_record ( int rec_num )
{
    char *err_buff;
    int  i;

    if ( phone_idx+1 > MAX_PHONE_ENTRIES)
    {
       err_buff  =(char *)malloc (PORTSIZE (25, 6, 56, 9));
       save_area (25,6,56,9,err_buff);
       SetColourPair (palette.bright_white, palette.red);
       draw_window (25,6,55,8);
       SetColourPair (palette.yellow, palette.red);
       WriteString ( 27,7,"The Phone List Is Full");
       get_key();
       restore_area (25,6,56,9,err_buff);
       free ((char *)err_buff);
    }
    else
    {
        for (i = phone_idx; i >= current_entry; i--)
           memcpy (&phone_list[i+1].name[0], &phone_list[i].name[0], sizeof (phone_list[i+1]));
        memcpy (&phone_list[current_entry].name[0], &undeletes[rec_num].name[0], sizeof (phone_list[current_entry]));
        
        if (phone_list[current_entry].marked)
            records_marked++;
        
        phone_idx++;
    }
}


void remove_deleted_record (int chosen_record )
{
    int idx;

    for (idx = chosen_record; idx <= last_undelete; idx++)
        memcpy ( &undeletes[idx].name[0], &undeletes[idx+1].name[0], sizeof (struct telnum_str));
    last_undelete--;
}

int select_undelete (  int chosen_record )
{
    char *view_buff;
    int  done;
    char key;
    int retval;

    retval = FALSE;
    if (chosen_record == -1)
        return;    /* ESC pressed on selection so do nowt */

    view_buff = (char *)malloc(PORTSIZE (15, 8, 66, 13));
    if ( view_buff == NULL)
        return;

    save_area (15,8,66,13,view_buff);
    SetColourPair (palette.black, palette.blue);
    draw_window (15,8,65,12);
    SetColourPair (palette.yellow, palette.blue);
    WriteString ( 17,  9, "Name     : ");
    WriteString ( 17, 10, "Number   : ");
    WriteString ( 17, 11, "Password : ");
    SetColourPair (palette.bright_white, palette.blue);
    WriteString ( 28,  9, undeletes[chosen_record].name);
    WriteString ( 28, 10, undeletes[chosen_record].number);
    WriteString ( 28, 11, undeletes[chosen_record].password);
    if (recover_record ())
    {
        insert_deleted_record ( chosen_record);
        remove_deleted_record ( chosen_record );
        retval = TRUE;
    }
    restore_area (15,8,66,13, view_buff);
    free ((char *)view_buff);
    return (retval);
}

void process_undeletes()
{
    char *undel_buff;
    char *err_buff;
    int  i;

    if ( last_undelete == -1) /* None there to recover !!! */
    {
        err_buff = (char *)malloc (PORTSIZE(25, 9, 57, 12));
        if (err_buff == NULL)
            return;
        save_area (25,9,57,12,err_buff);
        SetColourPair (palette.bright_white, palette.red);
        draw_window (25,9, 56, 11);
        SetColourPair (palette.yellow, palette.red);
        WriteString (27,10,"No Records Have Been Deleted");
        get_key();
        restore_area (25,9,57,12,err_buff);
        free ((char*)err_buff);
    }
    else
    {
        undel_buff = (char *)malloc (PORTSIZE (25, 6, 56, 23));
        if (undel_buff == NULL)
           return;
        save_area ( 25, 6, 56, 23, undel_buff);
        SetColourPair (palette.bright_white, palette.blue);
        draw_window ( 25, 6, 55, 6 + (last_undelete+1)+4);
        SetColourPair (palette.red, palette.blue);
        WriteString ( 33, 7, "Deleted Records");
        SetColourPair (palette.red, palette.blue);
        WriteString (30, 6+(last_undelete+1)+4, " Press F1 For Help ");

        redisplay_undel_list();
        select_undelete (choose_deleted_record());
        restore_area ( 25, 6, 56, 23, undel_buff);
        free ((char *)undel_buff);
        show_range (offset, offset+10);

    }
}

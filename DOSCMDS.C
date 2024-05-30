#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <bios.h>
#include <process.h>
#include <io.h>
#include <errno.h>
#include "palette.h"
#include "dostel2.h"
#include "telnum.err"
#include "windows.h"
#include "keys.h"

extern struct palette_str palette;
extern unsigned char far *video_base;
extern char gen_filename[];

extern void save_env( void );
extern void restore_env (void);
extern void hide_cursor( void );

char *dos_action_buff;
char dos_name[67];

void copy_file (char srce[], char dest[])
{
    FILE *in,
         *out;

    in = fopen (srce, "rb");
    out = fopen (dest, "wb");
    while (! feof (in))
        fputc (fgetc (in), out);
    fclose (in);
    fclose (out);
}

void get_file (int x, int y, int len)
{
    char hotkey[3];
    int state;

    hotkey[0] = (char)K_ESC;
    hotkey[1] = (char)K_ALT_L;
    hotkey[2] = (char)0;
    SetColourPair (palette.bright_white, palette.black);
    state = getstr (x, y, dos_name, hotkey, len);
    if ( state != 0)
    {
        switch ( state )
        {
            case K_ESC   : dos_name[0] = (char)0;
                           break;
            case K_ALT_L : GetFileFromDirectory (FALSE);
                           strcpy (dos_name, gen_filename);
                           break;
            default      : break;
        }
    }
}

int open_dos_buff()
{
    dos_action_buff = (char *)malloc(PORTSIZE(5, 10, 75, 17));
    if (dos_action_buff == NULL)
        return (0);
    save_area (5, 10, 75, 17, dos_action_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window(5, 10, 74, 16);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 28, 16, " ALT-L For Directory List ");
    return(-1);
}

void close_dos_buff()
{
    restore_area (5, 10, 75, 17, dos_action_buff);
    free((char *)dos_action_buff);
}


void dos_copy ()
{
    char srce[51];
    char dest[51];

    if (! open_dos_buff())
        return;
    
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 12, "Source File ");
    WriteString (7, 14, "Target File ");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar ( 19, 12, '['); WriteChar (70, 12, ']');
    WriteChar (19, 14, '['); WriteChar (70, 14, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (20, 12, 50); write_blanks (20, 14, 50);
    get_file (20, 12, 50);
    if ( dos_name[0] != (char)0)
    {
        strcpy ( srce, dos_name);
        get_file (20, 14, 50);
        if (dos_name[0] != (char)0)
        {
            strcpy ( dest, dos_name);

            if ( strcmpi (srce, dest) == 0)
            {    
                close_dos_buff();
                return;
            }

            if ( file_exists (srce) && ! file_exists (dest))
                 copy_file (srce, dest);
            else
            {
                if (! file_exists (srce))
                {    
                    show_error (ERR_NO_FILE);
                    hide_cursor();
                }
                else
                {
                    if (file_exists (dest))
                        if ( overwrite_file (dest))
                            copy_file (srce, dest);
                }
            }
        }
    }
    close_dos_buff();
}

void dos_del ()
{
    if (! open_dos_buff())
       return;
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 13, "FileName ");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar (16, 13, '['); WriteChar (67, 13, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (17, 13, 50);
    get_file (17, 13, 50);
    if ( dos_name[0] != (char)0)
    {
        if (remove ( dos_name ) != 0)
        {
             switch ( errno )
             {
                case ENOENT  : show_error (ERR_NO_FILE);
                               break;
                case EACCES  : show_error (ERR_NO_REMOVE);
                               break;
                default      : show_error (-999);
                               break;
             }
             hide_cursor();
        }
    }
    close_dos_buff();
}

void dos_ren ()
{
    char oldname[51];
    char newname[51];

    if (!open_dos_buff())
       return;
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 12, "Old Name ");
    WriteString (7, 14, "New Name ");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar ( 16, 12, '['); WriteChar (67, 12, ']');
    WriteChar (16, 14, '['); WriteChar (67, 14, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks(17, 12, 50); write_blanks (17, 14, 50);
    get_file (17, 12, 50);
    if (dos_name[0] != (char)0)
    {
        strcpy (oldname, dos_name);
        get_file (17, 14, 50);
        if ( dos_name[0] != (char)0)
        {
            strcpy ( newname, dos_name);
            if ( strcmpi (oldname, newname) == 0)
            {
                close_dos_buff();
                return;
            }

            if ( file_exists (oldname) && ! file_exists (newname))
                rename (oldname, newname);
            else
            {
                if (! file_exists (oldname))
                {
                    show_error (ERR_NO_FILE);
                    hide_cursor();
                }
                else
                {
                    if (file_exists (newname))
                    {
                        if (overwrite_file(newname))
                        {
                            remove (newname);
                            rename (oldname, newname);
                        }
                    }
                }
            }
        }
    }
    close_dos_buff();
}

void dos_md ()
{
    if (! open_dos_buff())
        return;
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 13, "Directory Name");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar ( 22, 13, '['); WriteChar (63, 13, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (23, 13, 40);
    get_file (23, 13, 40);
    if ( dos_name[0] != (char)0)
    {
        if (mkdir (dos_name) != 0)
        {
            switch ( errno)
            {
                case EACCES : show_error (ERR_DIR_EXISTS);
                              break;
                case ENOENT : show_error (ERR_NO_FILE);
                              break;
                default     : show_error (-999);
                              break;
            }
            hide_cursor();
        }
    }
    close_dos_buff();
}

void dos_rd ()
{
    if (! open_dos_buff())
        return;
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 13, "Directory Name");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar (22, 13, '['); WriteChar (63, 13, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (23, 13, 40);
    get_file (23, 13, 40);
    if (dos_name[0] != (char)0)
    {
        if (rmdir (dos_name) != 0)
        {
            switch ( errno)
            {
                case EACCES : show_error (ERR_NO_REMOVE);
                              break;
                case ENOENT : show_error (ERR_NO_FILE);
                              break;
                default     : show_error (-999);
                              break;
            }
            hide_cursor();
        }
    }
    close_dos_buff();

}

void dos_shell()
{
    char str[25];
    union REGS regs;

    save_env ();    
    regs.h.ah = BIOS_SET_VIDEO_MODE;
    if (video_base == (unsigned char far *)MONO_BASE_ADDRESS)
        regs.h.al = (unsigned char)7;
    else
        regs.h.al = 3;
    int86 (BIOS_VIDEO, &regs, &regs);
    
    SetColourPair (palette.bright_white, palette.cyan);
    draw_box (25, 2, 56, 5);
    SetColourPair (palette.black, palette.cyan);
    sprintf ( str, "RMDOSTEL %s", RM_VERSION);
    WriteString ( 28, 3, str);
    WriteString ( 31, 4, "Type 'EXIT' To Return");
    GotoXY(1, 6);
    if (system ("C:\COMMAND.COM") != -1)
        restore_env ();
}

void dos_commands()
{
    char *dos_buff;
    unsigned int ch;
    int done;

    dos_buff = (char *)malloc(PORTSIZE(10, 2, 70, 8));
    if (dos_buff == NULL)
       return;
    save_area (10, 2, 70, 8, dos_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (10, 2, 69, 7);
    ShowHeader (10, 69, 3, "DOS Commands");
    SetColourPair (palette.black, palette.green);
    WriteString (13, 5, "Copy"); WriteString (20, 5, "Delete");
    WriteString (29, 5, "Rename"); WriteString (38, 5, "MakeDir");
    WriteString (48, 5, "REmoveDir"); WriteString (60, 5, "Shell");
    SetColourPair (palette.red, palette.green);
    WriteChar (13, 5, 'C'); WriteChar (20, 5, 'D'); WriteChar (29, 5, 'R');
    WriteChar (38, 5, 'M'); WriteChar (49, 5, 'E'); WriteChar (60, 5, 'S');
    done = FALSE;
    while (! done)
    {
        ch = toupper(get_key());
        switch ( ch )
        {
           case K_ESC    : done = TRUE;
                           break;
           case 'C'      : dos_copy ();
                           break;
           case 'D'      : dos_del ();
                           break;
           case 'R'      : dos_ren ();
                           break;
           case 'M'      : dos_md ();
                           break;
           case 'E'      : dos_rd ();
                           break;
           case 'S'      : dos_shell ();
                           break;
           default       : break;
        }
    }
    restore_area (10, 2, 70, 8, dos_buff);
    free ((char *)dos_buff);
}


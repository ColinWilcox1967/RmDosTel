#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <bios.h>
#include <errno.h>
#include "telnum.err"
#include "windows.h"

extern unsigned char far *video_base;

char dos_command_line[255];

/* Note : strings passed to exec_external() need to have '\' chars
          replaced with double backslahses before being sent to system()
*/

extern void hide_cursor(void);

struct env_str
{
    unsigned char *screen_buffer;
    int curr_video_mode;
    int curr_display_page;
} current_env;

void save_env ()
{
    union REGS regs;    
    
    /* Get the video mode & display page */

    current_env.screen_buffer = (unsigned char *)malloc(4000);
    if (current_env.screen_buffer == NULL)
        show_error (ERR_NO_MEMORY);
    else
    {
        memcpy (&current_env.screen_buffer[0], &video_base[0], 4000);
        regs.h.ah = BIOS_GET_VIDEO_MODE;
        int86(BIOS_VIDEO, &regs, &regs);
        current_env.curr_video_mode = regs.h.al;
        current_env.curr_display_page = regs.h.bh;
    }   
}

void restore_env ()
{
    union REGS regs;

    /* Reset video mode */

    regs.h.ah = BIOS_SET_VIDEO_MODE;
    regs.h.al = (unsigned char)(current_env.curr_video_mode+128);
    int86 (BIOS_VIDEO, &regs, &regs);

    /* Reset display page */
    
    regs.h.ah = BIOS_SET_DISPLAY_PAGE;
    regs.h.al = current_env.curr_display_page;
    int86 (BIOS_VIDEO, &regs, &regs);
    
    if (current_env.screen_buffer != NULL)
    {
        memcpy (&video_base[0], &current_env.screen_buffer[0], 4000);
        free ((unsigned char *)current_env.screen_buffer);
    }
    hide_cursor();
}

void external_error (int type )
{
    switch ( type )    
    {
        case E2BIG   : show_error (ERR_NO_SPACE);
                       break;
        case ENOENT  :
        case ENOEXEC : show_error (ERR_BAD_INTERPRETER);
                       break;
        case ENOMEM  : show_error (ERR_NO_MEMORY);
                       break;
        default      : show_error (-999);
                       break;
    }

}

void exec_external (char str[])
{
   int offset,
       idx;

   save_env();
   idx = offset = 0;
   dos_command_line[0] = (char)0;
   while (idx < strlen (str))
   {
       if ( str[idx] == '\\')    
       {
           strcat (dos_command_line, "\\");
           offset++;
       }
       else
       {    
           dos_command_line[offset++] = str[idx];
           dos_command_line[offset]  =(char)0;
       }
       idx++;
   }
   restore_env();
}


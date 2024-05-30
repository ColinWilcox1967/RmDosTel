#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <malloc.h>

#include "dostel2.h"
#include "keys.h"
#include "windows.h"
#include "palette.h"

extern struct palette_str palette;

extern void WriteString (int, int, char *);
extern void SetColourPair (short, short);
extern unsigned int get_key();

unsigned int copy_count = 0;

void show_copymsg()
{
    char *copy_buff;
    char date_buff[30];
    char str[30];

    copy_buff = (char *)malloc(PORTSIZE(18, 6, 63, 19));
    if ( copy_buff == NULL)
        return;
    save_area (18, 6, 63, 19, copy_buff);
    SetColourPair (palette.bright_white, palette.green);
    draw_window (18, 6, 62, 18);
    ShowHeader ( 18, 62, 7, "Copyright Details");
    SetColourPair (palette.light_cyan, palette.green);
    sprintf ( str, "RMDOSTEL %s", RM_VERSION);
    WriteString ( 18 + ((45 - strlen (str))/2), 9, str);
    SetColourPair (palette.black, palette.green);
    WriteString ( 22, 11, "Author       : ");
    WriteString ( 22, 12, "Company      : ");
    WriteString ( 22, 16, "Build Date   : ");
    SetColourPair (palette.red, palette.green);
    WriteString ( 38, 11, "Colin Wilcox");
    SetColourPair (palette.yellow, palette.green);
    WriteString ( 38, 12, "Real Time Control Plc.");
    WriteString ( 38, 13, "17/18 Chetham Court,");
    WriteString ( 38, 14, "Calver Road,");
    WriteString ( 38, 15, "Warrington WA2 8RF");
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 27, 18, " Press Any Key To Continue ");
    sprintf (date_buff, "%s %s", __DATE__, __TIME__ );
    WriteString ( 38, 16, date_buff );
    get_key();
    restore_area ( 18, 6, 63, 19, copy_buff);
    free ((char *)copy_buff);
}

void copy_check (unsigned int k)
{
   switch ( copy_count )
   {
       case 0 : if (k == 'C' )
                   copy_count++;
                else
                    copy_count=0;
                break;
       case 1 : if ( k == 'O')
                    copy_count++;
                else
                    copy_count=0;
                break;
       case 2 : if (k == 'L')
                    copy_count++;
                else
                    copy_count = 0;
                break;
       case 3 : if ( k == 'I')
                    copy_count++;
                else
                    copy_count = 0;                 
                break;
       case 4 : if (k == 'N')
                   show_copymsg();
                copy_count=0;
                break;
   }
}


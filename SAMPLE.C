#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "palette.h"
#include "windows.h"
#include "dostel2.h"
#include "config.h"
#include "keys.h"
#include "trans.h"

#define MAX_FOREIGN_FIELDS 99

extern char trim_name[];
extern struct palette_str palette;
extern unsigned int ff_index;
extern char imp_name[];
extern void write_blanks ( int, int, int );
extern void trim_filename (char[], int);

int no_mapped_fields ()
{
    return ( (mapped_fields.name == -1) &&
             (mapped_fields.number == -1) &&
             (mapped_fields.password == -1)
           );
}

void highlight_sample_field (int start, int offset, int state)
{
    char str[10];

    if ( state )
        SetColourPair ( palette.bright_white, palette.black);
    else
        SetColourPair ( palette.black, palette.green);
    write_blanks (20, 12+offset, 35+MAX_EXAMPLE_LENGTH);
    itoa ( start+offset+1, str, 10); WriteString ( 20, 12+offset, str);
    itoa ( foreign_fields[start+offset].size, str, 10); WriteString ( 29, 12+offset, str);
    WriteString ( 35, 12+offset, foreign_fields[offset+start].example);
    if ( foreign_fields[start+offset].type == TEXT_FIELD)
        WriteString ( 45, 12+offset, "TEXTUAL");
    else
        WriteString (45, 12+offset, "NUMERIC");

    if ( mapped_fields.name == start+offset)
        WriteString (55, 12+offset, "NAME    ");
    if ( mapped_fields.number == start+offset)
        WriteString (55, 12+offset, "NUMBER  ");
    if ( mapped_fields.password == start+offset)
        WriteString (55, 12+offset, "PASSWORD");

}

void show_sample_range ( int start )
{
    int i;

    i = 0;
    while ((i != 8) && ( start+i <= ff_index))
    {
        highlight_sample_field ( start, i, FALSE);
        i++;
    }
}

void get_field_mappings(int start, int offset)
{
    char *mapping_buff;
    char ch;

    mapping_buff = (char *)malloc(PORTSIZE(45, 12+offset, 57, 12+offset+5));
    if (mapping_buff == NULL)
        return;
    save_area (45, 12+offset, 57, 12+5+offset, mapping_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (45, 12+offset, 56, 16+offset);
    WriteString ( 47, 12+offset+1, "Name");
    WriteString ( 47, 12+offset+2, "NUmber");
    WriteString ( 47, 12+offset+3, "Password");
    SetColourPair (palette.red, palette.green);
    WriteChar (47, 12+offset+1, 'N');
    WriteChar (48, 12+offset+2, 'U');
    WriteChar (47, 12+offset+3, 'P');
    ch = get_key();
    while ( (strchr ("NnUuPp", ch) == NULL) && ( ch != K_ESC))
       ch = get_key();
    switch ( ch )
    {
        case 'N' :
        case 'n' : mapped_fields.name = start+offset;
                   break;
        case 'U' :
        case 'u' : mapped_fields.number = start+offset;
                   break;
        case 'P' :
        case 'p' : mapped_fields.password = start+offset;
                   break;
        default  : break;
    }
    restore_area (45, 12+offset, 57, 12+offset+5, mapping_buff);
    free ((char *)mapping_buff);
    show_sample_range (start);
}

void show_sample_help()
{
    char *sample_help_buff;

    sample_help_buff = (char *)malloc(PORTSIZE(28, 7, 74, 18));
    if (sample_help_buff == NULL)
        return;
    save_area (28, 7, 74, 18, sample_help_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (28, 7, 73, 17);
    ShowHeader (28, 73, 8, "Field Selection Help");
    WriteString ( 31, 10, "F1         - Display This Help Page.");
    WriteString ( 31, 11, "ESC        - Terminate Field Selection.");
    WriteString ( 31, 12, "CSR-UP/DN  - Move To Previous/Next Field.");
    WriteString ( 31, 13, "SPACE      - Mark Highlighted Field.");
    WriteString ( 31, 14, "DEL        - Remove Mapping From The");
    WriteString ( 31, 15, "             Highlighted Field.");
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 39, 17, " Press Any Key To Continue ");
    get_key();
    restore_area (28, 7, 74, 18, sample_help_buff);
    free((char *)sample_help_buff);
}

void show_sample (char header[])
{
    char *sample_buff;
    int   start, offset, done;
    unsigned int k;

    sample_buff = (char *)malloc(PORTSIZE(15, 5, 66, 22));
    if ( sample_buff == NULL)
        return;
    save_area ( 15, 5, 66, 22, sample_buff);
    SetColourPair (palette.black, palette.green);
    draw_window ( 15, 5, 65, 21);
    SetColourPair (palette.red, palette.green );
    WriteString ( 29, 21, " Press F1 For Help ");
    ShowHeader ( 15, 65, 6, strupr(header));
    WriteString ( 18, 8, "Filename : ");
    WriteString ( 18, 10, "Field");
    WriteString ( 27, 10, "Length");
    WriteString ( 35, 10, "Example");
    WriteString ( 45, 10, "Type");
    WriteString ( 55, 10, "Marked");
    SetColourPair (palette.bright_white, palette.green);
    trim_filename (imp_name, 33);
    WriteString ( 29, 8, trim_name);
    mapped_fields.name = -1;    /* Indicates Not Selected */
    mapped_fields.number = -1;
    mapped_fields.password = -1;

    done = FALSE;
    start = offset = 0;
    show_sample_range ( start);
    while (! done)
    {
         highlight_sample_field ( start, offset, TRUE);
         k = get_key();
         if ( k != K_F1)
             highlight_sample_field (start, offset, FALSE);
         switch ( k)
         {
            case K_DEL    : if ( mapped_fields.name == offset+start)
                                mapped_fields.name = -1;
                            if ( mapped_fields.password == offset+start)
                                mapped_fields.password = -1;
                            if ( mapped_fields.number == offset+start)
                                mapped_fields.number = -1;
                            show_sample_range (start);
                            break;
            case K_F1     : show_sample_help();
                            break;
            case K_ESC    : done = TRUE;
                            break;
            case K_SPACE  : get_field_mappings(start, offset);
                            break;
            case K_CSR_DN : if ( offset+start < ff_index)
                            {
                                offset++;
                                if ( offset == 8)
                                {
                                    start++;
                                    offset--;
                                    if ( start+offset > ff_index)
                                        start--;
                                    show_sample_range(start);
                                }
                            }
                            break;
            case K_CSR_UP : if (start+offset > 0)
                            {
                                offset--;
                                if ( offset < 0)
                                {
                                    start--;
                                    offset=0;
                                    if ( ( start < 0))
                                        start++;
                                    show_sample_range(start);
                                }
                            }
                            break;
            default      : break;
         }

    }
    restore_area ( 15, 5, 66, 22, sample_buff);
    free ((char *)sample_buff);
}

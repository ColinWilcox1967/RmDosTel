#include <bios.h>
#include <graph.h>

#include "config.h"

#include "palette.h"
#include "windows.h"

union REGS regs;
char n_str[255];

extern unsigned char *video_base;

extern unsigned char config_byte2;
extern unsigned char config_byte3;

void clear_area (int, int, int, int);

void draw_shadow (int x1, int y1, int x2, int y2)
{
    int i;

    for ( i = x1+1; i <= x2+1; i++)
        video_base[ATTR_POS(i, y2+1)] &= 15;
     for ( i = y1+1; i <= y2+1; i++)
        video_base[ATTR_POS(x2+1, i)] &= 15;
}

void draw_edging ( int x1, int y1, int x2, int y2)
{
    int i;

    clear_area (x1,y1,x2,y2);
    if ( config_byte2 & MSK_WINDOW_TYPE)
        explode_window (x1,y1,x2,y2);
    WriteChar (x1,y1,'É'); WriteChar (x2,y1,'»');
    WriteChar (x1,y2,'È'); WriteChar (x2,y2,'¼');
    for (i = y1+1; i<=y2-1; i++)
    {
        WriteChar (x1,i, 'º'); WriteChar (x2,i,'º');
    }
    for (i = x1+1; i <= x2-1; i++) 
    {                     
        WriteChar (i, y1, 'Í');
        WriteChar (i, y2, 'Í');
    }
}


void draw_window ( int x1, int y1, int x2, int y2)
{
    if ( config_byte2 & MSK_WINDOW_TYPE )
        explode_window (x1, y1, x2, y2);
    draw_box (x1, y1, x2, y2);
    if ( config_byte2 & MSK_SHADOW_TYPE)
        draw_shadow (x1,y1,x2,y2);
}

void clear_area (int x1, int y1, int x2, int y2)
{
    int j;

    for (j = y1; j <= y2; j++)
        write_blanks (x1, j, x2-x1+1);  
}

void draw_box (int x1, int y1, int x2, int y2 )
{
    int i,j;

    if (( config_byte3 & MSK_NO_BORDER) == MSK_NO_BORDER)
    {
        for (i = y1; i <= y2; i++)
           write_blanks ( x1, i, x2-x1+1);
    }
    else
    {
        if ( (config_byte3 & MSK_DOUBLE_LINE_BORDER) == MSK_DOUBLE_LINE_BORDER)
        {
            WriteChar (x1, y1, TL_DOUBLE);
            for (i=1; i <= x2-x1-1; i++)
                WriteChar ( x1 + i, y1, H_DOUBLE);
            WriteChar (x2, y1, TR_DOUBLE);
            for (i = y1 + 1; i <= y2 - 1 ; i++)
            {
                WriteChar (x1, i, V_DOUBLE);
                for (j = 1; j <= x2-x1-1; j++)
                   WriteChar ( x1+j, i, ' ');
                WriteChar (x2, i, V_DOUBLE);
            }
            WriteChar (x1, y2, BL_DOUBLE);
            for (i = 1; i <= x2-x1-1; i++)
               WriteChar (x1+i, y2, H_DOUBLE);
            WriteChar (x2, y2, BR_DOUBLE);
        }
        else
        {
            if ( (config_byte3 & MSK_SINGLE_LINE_BORDER) == MSK_SINGLE_LINE_BORDER)
            {
                WriteChar (x1, y1, TL_SINGLE);
                for (i=1; i <= x2-x1-1; i++)
                   WriteChar ( x1 + i, y1, H_SINGLE);
                WriteChar (x2, y1, TR_SINGLE);
                for (i = y1 + 1; i <= y2 - 1 ; i++)
                {
                    WriteChar (x1, i, V_SINGLE);
                    for (j = 1; j <= x2-x1-1; j++)
                       WriteChar ( x1+j, i, ' ');
                    WriteChar (x2, i, V_SINGLE);
                }
                WriteChar (x1, y2, BL_SINGLE);
                for (i = 1; i <= x2-x1-1; i++)
                   WriteChar (x1+i, y2, H_SINGLE);
                WriteChar (x2, y2, BR_SINGLE);

            }
        }
    }
}

void explode_window (int x1, int y1, int x2, int y2)
{
    int delta;
    int new_x, new_y;
    float aspect_ratio;

    if ( ( difference ( x1, x2 ) < 3) || ( difference (y1, y2) < 3))
        return; /* Window too small to explode - it'll cock up the maths */

    new_x = (x1 + x2)/2;
    new_y = (y1 + y2)/2;
    delta = 1;

    aspect_ratio = (float)(x2-x1+1)/(y2-y1+1);
    if ( (x2-x1+1) < (y2-y1+1))
        aspect_ratio = 1/aspect_ratio;
    
    while ( new_y-delta >= y1)
    {
        /* Maintain screen aspect ratio when exploding */

        if ( x2-x1+1>y2-y1+1)
                draw_box ( new_x-aspect_ratio * delta, new_y-delta, new_x+aspect_ratio * delta, new_y+delta); 
        else
                draw_box ( new_x - delta, new_y - aspect_ratio * delta, new_x + delta, new_y + aspect_ratio * delta );
        delta++;
    }
}

void restore_area (int x1, int y1, int x2, int y2, char *buffer)
{
    int x, y;
    unsigned char far *tmp, *video, *tmp_buff;

    video = video_base;
    tmp = video;
    tmp_buff = buffer;
    for (y = y1; y <= y2; y++)
        for ( x = x1; x <= x2; x++)
        {
            video = tmp+CHAR_POS(x,y);
            *(video) = *tmp_buff++;
            *(video+1) = *tmp_buff++;
        }
}

void save_area (int x1, int y1, int x2, int y2, char *buffer)
{
    int x, y;
    unsigned char far *video, *tmp, *tmp_buff;

    video = video_base;
    tmp_buff = buffer;
    for ( y = y1; y <= y2; y++)
        for (x = x1; x <= x2; x++)
        {
            tmp           = video+CHAR_POS(x,y);
            *tmp_buff++   = *tmp;
            *tmp_buff++   = *(tmp+1);
        }
}


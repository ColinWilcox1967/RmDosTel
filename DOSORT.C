#include <stdio.h>
#include <malloc.h>
#include "palette.h"
#include "keys.h"
#include "dostel2.h"
#include "windows.h"
#include "config.h"

extern struct palette_str palette;
extern int                offset, current_entry, phone_idx;
extern unsigned char      config_byte1;

void update_sort_status (int sort_pos)
{
    WriteChar (33+sort_pos, 12, (char)178);
}

void swap_records ( int idx1, int idx2)
{
    struct telnum_str tmp;

    memcpy (&tmp.name[0], &phone_list[idx1].name[0], sizeof (struct telnum_str));
    memcpy (&phone_list[idx1].name[0], &phone_list[idx2].name[0], sizeof (struct telnum_str));
    memcpy (&phone_list[idx2].name[0], &tmp.name[0], sizeof (struct telnum_str));
}

void process_sort ()
{
    char   *sort_buff;
    int    sort_pos;
    int    srt_idx1, srt_idx2;
    struct telnum_str temp_telnum;
    int    rec_cnt,
           interval;
    int    ok_to_swap;

    /* No point sorting just one record */

    if ( phone_idx == 0)
        return;

    sort_buff = (char *)malloc (PORTSIZE (30, 8, 51, 15));
    if (sort_buff == NULL)
        return;
    save_area (30, 8, 51, 15, sort_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (30, 8, 50, 14);
    SetColourPair (palette.yellow, palette.blue);
    WriteString (33, 9, "Sorting Records");
    WriteString (35, 10, "Please Wait.");
    SetColourPair (palette.black, palette.white);
    WriteString ( 33, 12, "               ");

    sort_pos = 0;
    interval = (phone_idx + 1) / 14;
    rec_cnt  = 0;

    /* Start the sort here - Use Bubble Sort For Now */

    for (srt_idx1 = 0; srt_idx1 <= phone_idx; srt_idx1++)
    {
        if ( rec_cnt == interval)
        {
            rec_cnt = 0;
            update_sort_status (sort_pos++);
        }

        for (srt_idx2 = 0; srt_idx2 < phone_idx; srt_idx2++)
        {

            ok_to_swap = FALSE;

            /* Using NAME As Sort Key */

            if ( ( config_byte1 & MSK_SORT_NAME ) == MSK_SORT_NAME )
            {
                if ( config_byte1 & MSK_SORT_CASE )
                {
                    if ( config_byte1 & MSK_SORT_ORDER )
                    {
                        ok_to_swap = (strcmpi ( phone_list[srt_idx2].name, phone_list[srt_idx2 + 1].name) > 0);
                    }
                    else
                    {
                        ok_to_swap = (strcmpi ( phone_list[srt_idx2].name, phone_list[srt_idx2 + 1].name) < 0);
                    }
                }
                else
                {
                    if ( config_byte1 & MSK_SORT_ORDER )
                    {
                        ok_to_swap = (strcmp ( phone_list[srt_idx2].name, phone_list[srt_idx2 + 1].name) > 0);
                    }
                    else
                    {
                        ok_to_swap = (strcmp ( phone_list[srt_idx2].name, phone_list[srt_idx2 + 1].name) < 0);
                    }
                }
            }

            /* Using NUMBER AS Sort Key */

            if ( ( config_byte1 & MSK_SORT_NUMBER ) == MSK_SORT_NUMBER )
            {
                if ( config_byte1 & MSK_SORT_CASE )
                {
                    if ( config_byte1 & MSK_SORT_ORDER )
                    {
                        ok_to_swap = (strcmpi ( phone_list[srt_idx2].number, phone_list[srt_idx2 + 1].number) > 0);
                    }
                    else
                    {
                        ok_to_swap = (strcmpi ( phone_list[srt_idx2].number, phone_list[srt_idx2 + 1].number) < 0);
                    }

                }
                else
                {
                    if ( config_byte1 & MSK_SORT_ORDER )
                    {
                        ok_to_swap = (strcmp (phone_list[srt_idx2].number, phone_list[srt_idx2+1].number) > 0);
                    }
                    else
                    {
                        ok_to_swap = (strcmp (phone_list[srt_idx2].number, phone_list[srt_idx2+1].number) < 0);
                    }
                }
            }

            /* Using PASSWORD AS Sort Key */

            if ( ( config_byte1 & MSK_SORT_PASSWORD) == MSK_SORT_PASSWORD )
            {
                if ( config_byte1 & MSK_SORT_CASE )
                {
                    if ( config_byte1 & MSK_SORT_ORDER )
                    {
                         ok_to_swap = (strcmpi ( phone_list[srt_idx2].password, phone_list[srt_idx2 + 1].password) > 0);
                    }
                    else
                    {
                         ok_to_swap = (strcmpi ( phone_list[srt_idx2].password, phone_list[srt_idx2 + 1].password) < 0);
                    }
                }
                else
                {
                    if ( config_byte1 & MSK_SORT_ORDER )
                    {
                         ok_to_swap = (strcmp ( phone_list[srt_idx2].password, phone_list[srt_idx2 + 1].password) > 0);
                    }
                    else
                    {
                         ok_to_swap = ( strcmp ( phone_list[srt_idx2].password, phone_list[srt_idx2 + 1].password) < 0);
                    }
                }
            }

            if ( ok_to_swap )
                swap_records ( srt_idx2, srt_idx2 + 1);
        }
        rec_cnt++;
    }
    WriteString ( 33, 12, string ( (char)178, 15));
    restore_area (30, 8, 51, 15, sort_buff);
    free ((char*)sort_buff);
}

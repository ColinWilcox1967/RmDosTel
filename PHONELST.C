#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "telnum.err"
#include "dostel2.h"

extern struct telnum_str phone_list[];
extern int phone_idx;
extern unsigned char config_byte1, config_byte2;
extern int found_search_matches;

void clear_phone_list ( void );
int add_phone_number ( struct telnum_str * );
int jump_to_entry ( int );
int delete_phone_number ( int );

void clear_search_tags ()
{
    int idx;

    for (idx = 0; idx <= phone_idx; idx++)
        phone_list[idx].search_match = FALSE;
    found_search_matches = FALSE;

}

void clear_phone_list ()
{
   phone_idx = -1; /* Reset Index Ptr */
}

int jump_entry_number ( int new_idx )
{
    if ( ( new_idx >= 0 ) && ( new_idx <= phone_idx ))
    {
        phone_idx = new_idx;
        return (OK);
    }
    else
        return (ERR_OUT_OF_RANGE);
}

int delete_phone_number ( int old_idx )
{
    int idx;

    if ( (old_idx >= 0) && ( old_idx <= phone_idx ))
    {
        for ( idx = old_idx; idx < phone_idx; idx++ ) 
        {    
             strcpy ( phone_list[idx].name, phone_list[idx + 1].name);
             strcpy ( phone_list[idx].number, phone_list[idx + 1].number );
             strcpy ( phone_list[idx].password, phone_list[idx + 1].password );
             phone_list[idx].search_match = phone_list[idx + 1].search_match;
             phone_list[idx].marked = phone_list[idx + 1].marked;
        }
        return (OK);
    }
    else
        return (ERR_OUT_OF_RANGE);
}

int insert_phone_number ( struct telnum_str new_number, int pos )
{
    int  idx;

    if ( phone_idx+1 <= MAX_PHONE_ENTRIES)
    {
        phone_idx++;
        for (idx  = phone_idx; idx >= pos+1; idx--)
        { 
            strcpy ( phone_list[idx].name, phone_list[idx-1].name);
            strcpy ( phone_list[idx].number, phone_list[idx-1].number);
            strcpy ( phone_list[idx].password, phone_list[idx-1].password);
            phone_list[idx].marked = phone_list[idx-1].marked;
            phone_list[idx].search_match = phone_list[idx-1].search_match;
        }

        strcpy ( phone_list[pos].name, new_number.name);
        strcpy ( phone_list[pos].number, new_number.number);
        strcpy ( phone_list[pos].password, new_number.password);
        phone_list[pos].search_match = FALSE;
        phone_list[pos].marked = FALSE;

        return (OK);
    }
    else
       return (ERR_NO_MEMORY);
}

int add_phone_number ( struct telnum_str *new_number )
{
    if ( phone_idx == MAX_PHONE_ENTRIES ) 
        return ( ERR_NO_MEMORY );
    else
    {
        phone_idx++;
        strcpy ( phone_list[phone_idx].name, new_number->name );
        strcpy ( phone_list[phone_idx].number, new_number->number);
        strcpy ( phone_list[phone_idx].password, new_number->password);
        phone_list[phone_idx].search_match = FALSE;
    }
}


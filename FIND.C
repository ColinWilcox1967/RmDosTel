#include <stdio.h>
#include <ctype.h>

#include "keys.h"
#include "dostel2.h"
#include "config.h"
#include "windows.h"

extern unsigned char config_byte1;
extern int phone_idx;

int same_char (char ch1, char ch2)
{
    if (isalpha (ch1) && isalpha (ch2))
    {
        return ( (ch1 == ch2) || ( ch1 == ch2 + 32) || (ch1 == ch2 - 32));
    }
    else
        return ( ch1 == ch2);
}

int case_search (char *str1, char *srch)
{
    int carry_on,
        match;
    int idx,
        ptr;

    carry_on = TRUE;
    match    = TRUE;
    ptr      = 0;
    idx      = 0;
    while ( carry_on )
    {
        while ( carry_on &&
                ( ptr < strlen ( srch )) &&
                ( ptr + idx < strlen ( str1 ))
              )
        {
            carry_on = same_char ( str1[idx + ptr], srch[ptr] );
            if ( carry_on )
                ptr++;
        }
        match = carry_on;

        if ( match )
            carry_on = FALSE;
        else
        {
            ptr = 0;
            idx++;
        }

        if ( ! match )
          carry_on = ( idx < strlen ( str1 ));
    }
    return (match);
}

int find_text ( int start, char *search_string )
{
    int idx;
    int found;

    found = FALSE;
    idx = start;
    while (! found && ( idx <= phone_idx ))
    {
       if ( config_byte1 & MSK_SEARCH_NAME)
       {
           if ( config_byte1 & MSK_SEARCH_CASE)
               found |= (strstr (phone_list[idx].name, search_string) != NULL);
           else
               found |= case_search ( phone_list[idx].name, search_string);
       }
       if ( config_byte1 & MSK_SEARCH_NUMBER)
       {
           if ( config_byte1 & MSK_SEARCH_CASE)
               found |= (strstr (phone_list[idx].number, search_string) != NULL);
           else
               found |= case_search ( phone_list[idx].number, search_string);
       }
       if ( config_byte1 & MSK_SEARCH_PASSWORD)
       {
           if ( config_byte1 & MSK_SEARCH_CASE)
               found |= (strstr (phone_list[idx].password, search_string) != NULL);
           else
               found |= case_search ( phone_list[idx].password, search_string);
       }
       if (! found)
          idx++;
    }
    if ( found )
        return ( idx );
    else
        return ( -1 );
}


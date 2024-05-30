#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "dostel2.h"
#include "windows.h"
#include "palette.h"
#include "keys.h"

#define MAX_PBOOKS   50

extern void show_range (int, int);
extern void show_datafile_name(void);
extern struct palette_str palette;
extern char datafile[];
extern int phone_idx;
extern int current_entry;

char orig_dir[100];
char pbook_dir[100];
int pbook_count;
FILE *ptr;

struct pb_str
{
    char name[13];
    int directory;
} pbook_list[MAX_PBOOKS];

struct find_t c_file;
int first_book, last_book;
struct rmdostel_rec rec_details;

void get_rec (char name[], int n) 
{
    struct rmdostel_rec *rec;

    rec = (struct rmdostel_rec *)malloc(sizeof (struct rmdostel_rec));

    ptr = fopen (name,"rb");
    fseek (ptr, (long)(n * sizeof (struct rmdostel_rec)+strlen (ID_STRING)+2), SEEK_SET);
    fread ((char *)rec, sizeof (struct rmdostel_rec), 1, ptr);
    fclose (ptr);
    memcpy (&rec_details.name[0], &rec->name[0], sizeof (struct rmdostel_rec));
    
    free ((char *)rec);
}

void get_new_phonebook ( void );

void clear_booklist()
{
   int i;

   for (i = 0; i < MAX_PBOOKS; i++)
   {
      pbook_list[i].name[0] = (char)0;
      pbook_list[i].directory = FALSE;
   }
   pbook_count=-1;
}

void read_booklist()
{
    char *read_book_buff;
    FILE *fptr;
    char hdr[sizeof (ID_STRING)+1];
    int i;

    getcwd (pbook_dir, 99);
    read_book_buff = (char *)malloc(PORTSIZE(20, 9, 61, 14));
    if (read_book_buff != NULL)
    {
        save_area (20,9,61,14,read_book_buff);
        SetColourPair (palette.bright_white, palette.red);
        draw_window (20,9,60,13);
        SetColourPair (palette.yellow, palette.red);
        WriteString (29,11,"Looking For Phonebooks ....");
    }
    pbook_count=0;

    /* Scan Subdirs In First */

    if ( _dos_findfirst ("*.*", _A_SUBDIR, &c_file) == 0)
    {
        if ((c_file.attrib & 16) == 16)
        {
            if (c_file.name[0] != '.')
            {
                pbook_list[pbook_count].name[0] = '<';
                memcpy (&pbook_list[pbook_count].name[1], &c_file.name[0], strlen (c_file.name)+1);
                strcat (pbook_list[pbook_count].name, ">");
                pbook_list[pbook_count++].directory = TRUE;
            }
            else
            {
                if ( strcmp (c_file.name, "..") == 0)
                {
                    strcpy (pbook_list[pbook_count].name, "<* PARENT *>");
                    pbook_list[pbook_count++].directory = TRUE;
                }
            }
        }
        while (_dos_findnext (&c_file) == 0)
        {
            if ((c_file.attrib & 16) == 16)
            {
                if (c_file.name[0] != '.')
                {
                    pbook_list[pbook_count].name[0] = '<';
                    memcpy (&pbook_list[pbook_count].name[1], &c_file.name[0], strlen (c_file.name)+1);
                    strcat (pbook_list[pbook_count].name, ">\0");
                    pbook_list[pbook_count++].directory = TRUE;
                }
                else
                {
                    if (strcmp (c_file.name, "..") == 0)
                    {
                        strcpy (pbook_list[pbook_count].name, "<* PARENT *>");
                        pbook_list[pbook_count++].directory = TRUE;
                    }
                }
            }
        }
        pbook_count--;
    }

    /* Read phonebook files */

    _dos_findfirst ("*.*", _A_NORMAL, &c_file);
    fptr = fopen (c_file.name, "rb");
    fread (hdr, sizeof (char), sizeof (ID_STRING), fptr);
    fclose (fptr);
    if (strncmp (hdr, ID_STRING, strlen(ID_STRING)) == 0)
    {    
        pbook_count++;
        strcpy (pbook_list[pbook_count].name, c_file.name);
        pbook_list[pbook_count].directory = FALSE;
    }
    while (_dos_findnext (&c_file) == 0)
    {
        fptr = fopen (c_file.name, "rb");
        fread (hdr, sizeof (char), sizeof (ID_STRING), fptr);
        fclose (fptr);
        if (strncmp (hdr, ID_STRING, strlen(ID_STRING)) == 0)
        {
            pbook_count++;
            strcpy (pbook_list[pbook_count].name, c_file.name);
            pbook_list[pbook_count].directory = FALSE;
        }
    }
    if (read_book_buff != NULL)
    {
        restore_area (20,9,61,14, read_book_buff);
        free((char *)read_book_buff);
    }
}

void show_view(int start, int finish)
{
    int i;
    int y;
    char str[255];

    SetColourPair (palette.bright_white, palette.blue);
    draw_window (15, 7, 65, 19);

    SetColourPair (palette.red, palette.blue); 
    sprintf (str, " %s ", pbook_dir);
    if (strlen (str) > 45)
        str[45] = (char)0;
    WriteString (14+((50-strlen (pbook_dir))/2), 7, str);

    SetColourPair (palette.bright_white, palette.blue);
    y = 8; /* First Line Of Listing */
    for (i = start; i <= finish; i++)
    {
        switch ( (i-start) / 11)
        {
            case 0 : WriteString (18, y + (i-start), pbook_list[i].name);
                     break;
            case 1 : WriteString (34, y + (i-start)-11, pbook_list[i].name);
                     break;
            case 2 : WriteString (50, y +(i-start)-22, pbook_list[i].name);
                     break;
        }
    }
}

void highlight_book (int book_num, int state)
{
   if ( state)
       SetColourPair (palette.yellow, palette.black);
   else
       SetColourPair (palette.bright_white, palette.blue);
   switch ( (book_num - first_book) / 11)
   {
       case 0 : WriteString (18, 8 + (book_num - first_book), pbook_list[book_num].name);
                break;
       case 1 : WriteString (34, 8 + (book_num - first_book)-11, pbook_list[book_num].name);
                break;
       case 2 : WriteString (50, 8 + (book_num - first_book)-22, pbook_list[book_num].name);
                break;
   }

}

void newbook_help()
{
    char *new_help;

    new_help = (char *)malloc(PORTSIZE(18, 7, 63, 20));
    if (new_help != NULL)
    {
        save_area (18, 7, 63, 20, new_help);
        SetColourPair (palette.black, palette.green);
        draw_window (18, 7, 62, 19);
        ShowHeader (18, 62, 8, "New Phonebook Help");
        WriteString (20, 10, "F1           - New Phonebook Help.");
        WriteString (20, 11, "Cursor Keys  - Highlight Phonebook.");
        WriteString (20, 12, "PGUP/DN      - Page Through Phonebooks.");
        WriteString (20, 13, "HOME         - Jump To First Entry.");
        WriteString (20, 14, "ESC          - Abort Selection.");
        WriteString (20, 15, "ALT-V        - View Highlighted Phonebook.");
        WriteString (20, 16, "ALT-I        - Display Phonebook Details.");
        WriteString (20, 17, "ENTER        - Select Phonebook.");
        SetColourPair (palette.bright_white, palette.green);
        WriteString (27, 19, " Press Any Key To Continue ");
        get_key();
        restore_area (18, 7, 63, 20, new_help);
        free ((char*)new_help);
    }
}

void PreviewPhonebook (char name[])
{
    char *prev_buff;
    struct find_t c_file;
    char str[50];
    long records;
    unsigned int i;

    prev_buff = (char *)malloc(PORTSIZE(40, 10, 75, 19)); 
    if (prev_buff != NULL)
    {
        save_area (40, 10, 75, 19, prev_buff);
        SetColourPair (palette.black, palette.green);
        draw_window (40, 10, 74, 18);
        SetColourPair (palette.red, palette.green);
        WriteString (42, 12, "Phonebook    : ");
        WriteString (42, 13, "Customer     : ");
        WriteString (42, 14, "File Size    : ");
        WriteString (42, 15, "Records      : ");
        WriteString (42, 16, "Last Updated : ");

        SetColourPair (palette.bright_white, palette.green);
        WriteString ( 57, 12, name);
        _dos_findfirst (name, _A_NORMAL, &c_file);
        if (c_file.size == 1L)
            strcpy (str, "1 byte");
        else
        {
            if (c_file.size <= 999999L)
                sprintf (str, "%ld bytes", c_file.size);
            else
                sprintf (str, "%ld", c_file.size);
        }
        WriteString (57, 14, str);
        records = (long)((c_file.size - sizeof (ID_STRING))/(sizeof (struct rmdostel_rec)));
        sprintf (str, "%ld", records);
        WriteString (57, 15, str);
        
        i = c_file.wr_date;
        i &= 31;
        sprintf (str, "%d/", i); 
        i = c_file.wr_date;
        i = i >> 5; i &= 15;
        sprintf (str, "%s%d/", str, i);
        i = c_file.wr_date;
        i = i >> 9; i &= 127; i += 1980;
        sprintf (str, "%s%d", str, i);
        WriteString ( 57, 16, str);

        i = c_file.wr_time;
        i = i >> 11; i &= 31;
        if ( i < 10)
            sprintf (str, "%02d:", i);
        else
            sprintf (str, "%d:", i);

        i = c_file.wr_time;
        i = i >> 5; i &= 63;
        if (i < 10)
            sprintf (str, "%s%02d", str, i);
        else
            sprintf (str, "%s%d", str, i);
        WriteString (57,17, str);
        
        SetColourPair (palette.bright_white, palette.green);
        WriteString (50, 18, " Press Any Key ");
        get_key();
        restore_area (40, 10, 75, 19, prev_buff);
        free((char *)prev_buff);
    }
}

int definately_replace ()
{
    int result;
    char *replace_buff;

    replace_buff = (char *)malloc(PORTSIZE(22, 10, 59, 14));
    if ( replace_buff != NULL)
    {
        save_area (22, 10, 59, 14, replace_buff);
        SetColourPair (palette.bright_white, palette.red);
        draw_window (22, 10, 58, 13);
        SetColourPair (palette.yellow, palette.red);
        WriteString (27, 11, "Replace Existing Phonebook ?");
        result = get_yesno (33, 12, palette.yellow, palette.red);
        restore_area (22, 10, 59, 14, replace_buff);
        free ((char *)replace_buff);
    }
    else
       result = TRUE;
    return ( result);
}

void show_pair (char name[], int n, int max)
{
    char tmp[10];

    SetColourPair (palette.bright_white, palette.green);
    write_blanks (46, 9, MAX_NAME_SIZE);
    write_blanks (46, 10, MAX_NUMBER_SIZE);
    write_blanks (46, 11, MAX_PASSWORD_SIZE);
    write_blanks (46, 16, MAX_NAME_SIZE);
    write_blanks (46, 17, MAX_NUMBER_SIZE);
    write_blanks (46, 18, MAX_PASSWORD_SIZE);

    sprintf (tmp, "%03d", n+1);
    get_rec (name, n);
    WriteString (44, 7, tmp); 
    WriteString (46, 9, rec_details.name);
    WriteString (46,10, rec_details.number);
    WriteString (46,11, rec_details.password);
    if (n+1 <= max)
    {
        sprintf (tmp, "%03d", n+2);
        WriteString (44, 14, tmp);
        get_rec (name, n+1);
        WriteString (46, 16, rec_details.name);
        WriteString (46, 17, rec_details.number);
        WriteString (46, 18, rec_details.password);
    }
}


void view_book (char name[])
{
    int recnum;
    char *view_buff;
    int get_out;
    unsigned int k;
    unsigned long total_recs;
    struct find_t c_file;

    view_buff = (char *)malloc(PORTSIZE(33, 6, 77, 20)); 
    if (view_buff == NULL)
        return;
    save_area (33,6,77,20,view_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (33,6,76,19);
    SetColourPair (palette.red, palette.green);
    WriteString (35,  7, "Record # ");
    WriteString (35,  9, "Name     : ");
    WriteString (35, 10, "Number   : ");
    WriteString (35, 11, "Password : ");
    WriteString (35, 14, "Record # ");
    WriteString (35, 16, "Name     : ");
    WriteString (35, 17, "Number   : ");
    WriteString (35, 18, "Password : ");
    get_out = FALSE;
    recnum = 0;
    _dos_findfirst (name, _A_NORMAL, &c_file);
    total_recs = (c_file.size - strlen (ID_STRING))/sizeof (struct rmdostel_rec);

    while (! get_out)
    {
        show_pair (name,recnum, total_recs);
        k = get_key();
        switch (k)
        {
            case K_HOME   : recnum = 0;
                            break;
            case K_CSR_UP : if (recnum > 0)
                                 recnum--;
                            break;
            case K_CSR_DN : if (recnum+1 < total_recs-1)
                                recnum++;
                            break;
            case K_END    : recnum = (int)(total_recs-2);
                            break;
            case K_ESC    : get_out = TRUE;
                            break;
            default       : break;
        }
    }
    restore_area (33,6,77,20,view_buff);
    free((char *)view_buff);
}

void get_new_phonebook()
{
    char *book_buff;
    int  get_out,
         book_chosen;
    int curr_book;
    unsigned int key;

    getcwd (orig_dir, 99);
    book_buff = (char *)malloc(PORTSIZE(15, 7, 66, 20));
    if (book_buff != NULL)
    {
        save_area (15,7,66,20, book_buff);
        SetColourPair (palette.bright_white, palette.blue);
        draw_window (15,7, 65, 19); 
        read_booklist();

        /* Set Initial View */
        first_book = 0;
        last_book = min (33, pbook_count);
        curr_book = first_book;
        get_out = FALSE;
        book_chosen = FALSE;
        show_view(first_book,last_book);
        while (! (get_out || book_chosen))
        {
            highlight_book ( curr_book, TRUE);
            key = get_key();
            if ((key != K_ALT_I) && (key != K_ALT_V))
                highlight_book (curr_book, FALSE);
            switch ( key)
            {
                case K_F1     : newbook_help();
                                break;
                case K_ESC    : get_out = TRUE;
                                break;
                case K_HOME   : if (curr_book <= 32)
                                {
                                    /* We're already on the first page */

                                    highlight_book (curr_book, FALSE);
                                    curr_book = 0;
                                    highlight_book (curr_book, TRUE);
                                }
                                else
                                {
                                    curr_book = 0;
                                    first_book = 0;
                                    last_book = min (33, pbook_count);
                                    show_view (first_book, last_book);
                                }
                                break;

                case K_CR     : if (pbook_list[curr_book].directory)
                                {
                                    SetColourPair (palette.bright_white, palette.blue);
                                    draw_window (15, 7, 65, 19);
                                    pbook_count = -1;
                                    if ( strcmp (pbook_list[curr_book].name, "<* PARENT *>") == 0)
                                       chdir ("..");
                                    else
                                    {
                                       memmove (&pbook_list[curr_book].name[0], &pbook_list[curr_book].name[1], strlen (pbook_list[curr_book].name)-2);
                                       pbook_list[curr_book].name[strlen (pbook_list[curr_book].name)-2] = (char)0;
                                       chdir ( pbook_list[curr_book].name);
                                    }
                                    clear_booklist();
                                    read_booklist();
                                    curr_book = 0;
                                    first_book = 0;
                                    last_book = min (pbook_count, 33);
                                    show_view (first_book, last_book);
                                }
                                else 
                                    book_chosen = TRUE;
                                break;
                case K_CSR_DN : 
                                if (curr_book > last_book - 2)
                                {
                                    if ( last_book <= pbook_count)
                                    {
                                        if ( last_book < pbook_count)
                                        {
                                            first_book = last_book; 
                                            last_book = min (first_book+33, pbook_count);
                                            curr_book = first_book;
                                            show_view (first_book, last_book);
                                        }
                                        else
                                            curr_book = pbook_count;
                                    }
                                 }
                                 else
                                    curr_book++;
                                break;
                case K_CSR_UP : if (curr_book > first_book)
                                    curr_book--;
                                else
                                {
                                    if ( first_book > 0)
                                    {
                                        first_book = max (0, first_book-33);
                                        last_book = min (first_book+33, pbook_count);
                                        curr_book--;
                                        show_view (first_book, last_book);
                                    }
                                }
                                break;
                case K_CSR_LT  : if (curr_book - 11 >= first_book)
                                     curr_book-=11;
                                 break;
                case K_CSR_RT  : if (( curr_book + 11 <= last_book))
                                     curr_book+=11;
                                 break;
                case K_PGDN   : if (first_book+33 <= pbook_count)
                                {
                                    first_book +=33;
                                    last_book = min (pbook_count, first_book+33);
                                    curr_book = min (curr_book+33, pbook_count);
                                    show_view (first_book, last_book);
                                }
                                break;
                case K_PGUP   : if (first_book - 33 >= 0)
                                {
                                    first_book -= 33;
                                    last_book = min (first_book+33, pbook_count);
                                    curr_book -=33;
                                    show_view (first_book, last_book);
                                }
                                break;
                case K_ALT_I  : 
                                if (!pbook_list[curr_book].directory)
                                    PreviewPhonebook (pbook_list[curr_book].name);
                                break;
                case K_ALT_V  : if (!pbook_list[curr_book].directory)
                                    view_book(pbook_list[curr_book].name);
                default       : break;
            }
        }
        restore_area (15, 7, 66, 20, book_buff);
        free ((char *)book_buff);
    }
    if ( book_chosen )
    {
        if ( definately_replace ())
        {
            phone_idx=0;
            strcpy (datafile, pbook_list[curr_book].name);
            load_numbers();
            current_entry = 0;
            show_range (current_entry, current_entry + 10);
            show_datafile_name();
        }
    }
    chdir (orig_dir);
}


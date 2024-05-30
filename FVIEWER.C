#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "config.h"
#include "palette.h"
#include "windows.h"
#include "dostel2.h"
#include "keys.h"

#define ASCII_FORMAT         0
#define HEX_FORMAT           1

#define MAX_PATTERN_LENGTH   10
#define MAX_ASCII_LENGTH     MAX_PATTERN_LENGTH
#define MAX_HEX_LENGTH       3 * MAX_PATTERN_LENGTH

char pattern[MAX_PATTERN_LENGTH+1];

extern unsigned char config_byte2;
extern struct palette_str palette;

extern void SetColourPair (short, short);
extern void clear_area (int, int, int,  int);
extern void draw_window (int, int, int, int);
extern void WriteChar (int, int, char);
extern void WriteString (int, int, char *);
extern unsigned int get_key (void);
extern void save_area (int, int, int, int, char *);
extern void restore_area (int,  int, int,  int, char *);
extern void WriteMultiChar (int, int, char, int);


FILE *vwr_file;
int  vwr_char;

void DrawFileViewer (char *);
void Num2Hex (int, char *);
void ShowFileSegment (long, long);
void UseViewer (long);

void ShowPatternFormats()
{
   int i;
   char HexByte[3];

   SetColourPair (palette.bright_white, palette.black);
   for (i = 0; i < strlen (pattern); i++)
   {
       WriteChar (30+i, 9, pattern[i]);
       Num2Hex ((int)pattern[i], HexByte);
       WriteString (30 + (3 * i), 11, HexByte); 
       WriteChar (30 + (3 * i) + 2, 11, ' ');
   }
}

void DrawFileViewer (char *f)
{
    int  i;

    SetColourPair (palette.black, palette.cyan);
    
    draw_edging (6, 1, 73, 22);
    if ( config_byte2 & MSK_SHADOW_TYPE)
       draw_shadow (6,1,73,22);

    WriteChar (6, 3, (char)204);
    WriteChar (73, 3, (char)185);
    for (i=7; i <= 72; i++)
	WriteChar (i, 3, (char)205);
    WriteChar (54, 22, (char)202);
	 for (i = 4; i <= 21; i++)
	WriteChar (54, i, (char)186);
    WriteChar (54, 3, (char)203);
    SetColourPair (palette.red, palette.cyan);
    WriteString ( 8,2 ,f);
}

char hex_digit (int n)
{
    if (n > 9)
	return ((char)('A' + (n - 10)));
    else
	return ((char)('0' + n));
}

void Num2Hex (int n, char *hex)
{
    hex[0] = hex_digit (n / 16);
    hex[1] = hex_digit (n % 16);
    hex[2] = (char)0;
}

void ShowFileSegment (long page_number, long totalpages)
{
    int  count;
    unsigned  int  line,
		   column1,
		   column2;
    char hex[3];
    char page1[4], page2[4];

    count  = 0;
    line   = 4;
    column1 = 8; column2 = 56;

    clear_area (7, 4, 53, 21);
    clear_area (55, 4, 71, 21);

    WriteMultiChar (48, 3, ' ', 5);

    SetColourPair (palette.red, palette.cyan);
    WriteString (41, 3, " Page ");
    ltoa (page_number, page1, 10);

    WriteString (47, 3, page1);
    WriteChar (47+strlen (page1), 3, '/');
    ltoa (totalpages, page2, 10);
    WriteString (48+strlen(page1), 3, page2);
    WriteChar (48+strlen (page1)+strlen (page2), 3, ' ');

    SetColourPair (palette.bright_white, palette.cyan);
    while ((count < 270L) && ! feof (vwr_file))
    {
	if (! feof (vwr_file))
	{     
	    vwr_char = fgetc (vwr_file);
	    count++;
	    Num2Hex (vwr_char, hex);

	    WriteString (column1, line, hex);
	    WriteChar   (column1 + 2, line, ' ');
	    WriteChar   (column2, line, (char)vwr_char);
 
	    column1 += 3;
	    column2++;
 
	    if ((count % 15) == 0L)
	    {
		line++;
		column1 = 8;
		column2 = 56; 
	    }
	}
    }           
}

int HexDigit (char ch)
{
    if ((ch >= 'A') && (ch <= 'F'))
	return ( 10 + ( ch-'A'));
    else
	return ( ch - '0');
}

int HexToDec (char hex[])

{
    return ( 16 * HexDigit (hex[0])+HexDigit(hex[1]));    
}

int get_n_chars (int x, int y, char *str, int n)
{
    int done,
	retval,
	i;
    unsigned int ch;

    memset ( str, 0, n);
    i = 0;
    done = FALSE;
    retval = 0; /* OK, no error */
    while ( ! done )
    {
       ch = get_key();
       str[i++]=ch;
       if (i == n)
       {
	  str[i-1]=(char)0;
	  done = TRUE;
	  retval=1; /* ok n chars read */
       }
       else
       {
	   done = ((ch == K_ESC) || ( ch == K_TAB));
	   if ( done )
	   {
	       if (ch == K_TAB)
		  retval = -2;  /* tab pressed */
	       else
		  retval = -1;   /* esc pressed */
	   }
       }
       if (! done)
       {
	   SetColourPair (palette.bright_white, palette.black);
	   WriteString (x, y, str);
       }
    }
    return (retval);
}


void do_pattern_search ()
{
     char *pattern_buff;
     int  finished;
     int  format_type;
     int  state, idx;
     char ch_str[3];
     char hotkeys[3];

     pattern_buff = (char *)malloc(PORTSIZE(17, 7, 64, 14));
     if ( pattern_buff == NULL)
	 return;
     save_area (17, 7, 64, 14, pattern_buff);
     SetColourPair (palette.bright_white, palette.blue);
     draw_window (17, 7, 63, 13);
     SetColourPair (palette.yellow, palette.blue);
     WriteString ( 20,  9, "ASCII    [");
     WriteString ( 20, 11, "Hex      [");
     WriteChar ( 30+MAX_ASCII_LENGTH, 9, ']');
     WriteChar ( 30+MAX_HEX_LENGTH, 11, ']');
     SetColourPair (palette.bright_white, palette.black);
     write_blanks ( 30, 9, MAX_ASCII_LENGTH);
     write_blanks (30, 11, MAX_HEX_LENGTH);
     finished = FALSE;
     format_type = ASCII_FORMAT;
     hotkeys[0] = (char)K_ESC; hotkeys[1] = (char)K_TAB; hotkeys[2] = (char)0;
     idx = 0;
     memset (pattern, 0, sizeof (pattern)); /* zero fill receipt buffer */
     while (! finished)
     {
	 ShowPatternFormats();
	 SetColourPair (palette.bright_white, palette.black);
	 if ( format_type == ASCII_FORMAT )
	     state = get_n_chars (30, 9, ch_str, 1);
	 else
	     state = get_n_chars (30, 11, ch_str, 2);
	 switch ( state )
	 {
	     case -1    : finished = TRUE;
			  break;
	     case -2    : if ( format_type == ASCII_FORMAT )
			     format_type = HEX_FORMAT;
			  else
			     format_type = ASCII_FORMAT;
			  break;
	     default    : if ( format_type == ASCII_FORMAT )
			       pattern[idx++] = ch_str[0];
			  else
			       pattern[idx++] = HexToDec (ch_str);
			  break;
	 }
     }
     restore_area (17, 7, 64, 14, pattern_buff);
     free ((char *)pattern_buff);
}

void UseViewer (long FileSize)
{
    unsigned int ch;
    int          abort;
    char         *InfoBuffer;
    long         pages;
    long         TotalPages;

    abort = 0;
    pages = 1L;
    TotalPages = (FileSize / 270L)+1;
    ShowFileSegment (pages, TotalPages);

    while (! abort)
    {
	SetColourPair (palette.bright_white, palette.cyan);
	ch = get_key();
	switch (ch)
	{
	       case K_ESC    : abort = TRUE;
			       break;
	       /*

	       case K_ALT_F  : do_pattern_search ();
			       break;
	       */
	       case K_PGUP : if (pages > 1L)
			       {
				    pages--;
				    rewind (vwr_file);
				    fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
				    ShowFileSegment (pages, TotalPages);
			       }
			       break;
		case K_PGDN : if (pages < TotalPages)
				{
				      pages++;
				      fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
				      ShowFileSegment (pages, TotalPages);
				}
				break;

		case K_HOME  : fseek (vwr_file, 0L, SEEK_SET);
				ShowFileSegment (1L, TotalPages);
				pages = 1L;
				break;

		case K_END  : rewind (vwr_file);
			      pages = TotalPages;
			      fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
			      ShowFileSegment (TotalPages, TotalPages);
			      break;

	       case K_F1  : InfoBuffer = (char *)malloc (PORTSIZE(15, 4, 66, 15));
			    if (InfoBuffer != NULL)
			    {
				  save_area (15, 4, 66, 15, InfoBuffer);
				  SetColourPair (palette.black, palette.green);
				  draw_window (15, 4, 65, 14);
				  ShowHeader ( 15, 65, 5, "File Viewer Help");
				  WriteString (18,  11, "PGUP/DN          - Page Through File.");
				  WriteString (18,   8, "ESC              - Abort File Viewer.");
				  WriteString (18,   9, "HOME             - Jump To First Page.");
				  WriteString (18,  10, "END              - Jump To Last Page.");
				  WriteString (18,   7, "F1               - Display This Help Screen.");
				  /*
				  WriteString (18,  12, "ALT-F            - Find Byte Pattern.");
				  */
				  SetColourPair (palette.bright_white, palette.green);
				  WriteString (26,  14, " Press Any Key To Continue ");
				  SetColourPair (palette.black, palette.green);
				  get_key();
				  restore_area (15, 4, 66, 15, InfoBuffer);
				  free ((char *)InfoBuffer);
			   }               
			   break;

	       default   : break;
	}
    }
}

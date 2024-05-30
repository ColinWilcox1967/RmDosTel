#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <direct.h>
#include "keys.h"
#include "palette.h"
#include "dostel2.h"

#include <stdio.h>

#define MAX_ENTRIES       56

struct dir_s
{
    char DirEntry[13];
    struct dir_s *PreviousEntry;
    struct dir_s *NextEntry;
};

typedef struct dir_s *dir_p;

extern struct palette_str palette;

extern void WriteChar (unsigned int, unsigned int, char);
extern void clear_area (unsigned int, unsigned int, unsigned int, unsigned int);
extern void WriteString (unsigned int, unsigned int, char *);
extern void WriteMultiChar (unsigned int, unsigned int, char, unsigned int);

extern void  InitDirList (void);
extern void  AddDirEntry (dir_p);
extern void  ClearDirList (void);
extern int   BuildDirList (void);
extern dir_p FindEntryN (unsigned int);
extern void  draw_window (unsigned int, unsigned int, unsigned int, unsigned int);
extern void  DisplayDriveSet(void);
extern void  ShowDrive (int, unsigned int);
extern char  *SelectFromDir (void);

int   ActiveDrive;

void DrawDirContents (void);
void ShowDirList (unsigned int, unsigned int, unsigned int);
void ShowDir (unsigned int,  unsigned int);
char *DoDOSDirectory (void);

char *PaddedFilename (char *);

extern dir_p DirList;

char dir_spec[67];

char *PaddedFilename (char *fname)

{
    char f[67];
    int i, j;

    
    i = 0;
    while ((fname[i] != '.') && (i < (int)strlen (fname)))
	i++;
    if (fname[i] == '.')
    {
	for (j=0; j < i; j++)
	    f[j] = fname[j];
	while (j < 8)
		f[j++] = ' ';
	f[8] = '.';
	j = i+1;
	i = 9;
	while (j < (int)strlen (fname))
	    f[i++]=fname[j++];
	f[i] = (char)0;
    }
    else
	strcpy (f, fname);
    while ( strlen (f) < 12)
	strcat (f, " ");
    return (f);
}

void DrawContentsOutline ()

{
    unsigned int i;

    SetColourPair (palette.black, palette.bright_white);
    WriteChar (3, 7, 'Ú');
    WriteChar (3, 22, 'À');
    WriteChar (78, 7, '¿');
    WriteChar (78, 22, 'Ù');
    for (i = 8; i <= 21; i++)
    {
	WriteChar (3, i, '³');
	WriteChar (78, i, '³');
    }
}

void ShowDirList (unsigned int start, unsigned int finish, unsigned InitRow)

{
    unsigned int RowCount,
		 count;
    dir_p        CurrPtr, LastPtr, Ptr;
    char         padded[13];

    
    RowCount   = InitRow;
    CurrPtr    = FindEntryN (start);
    LastPtr    = FindEntryN (finish);
    Ptr        = CurrPtr;
    count = start;

    clear_area ( 4, 7, 77, 21);
    if (DirList != NULL)
    {
	while ((Ptr != LastPtr->NextEntry) && ((count- start) < MAX_ENTRIES))
	{
	      strcpy (padded, PaddedFilename (Ptr->DirEntry));
	      if (Ptr->DirEntry[0] == '<')
	      {
		     SetColourPair (palette.black, palette.bright_white);
		     switch ((count - start) % 4)
		     {
			  case 0 : WriteString ( 6, 8 + RowCount, Ptr->DirEntry);
				   WriteMultiChar (6+strlen(Ptr->DirEntry), 8+RowCount, ' ', (14-strlen (Ptr->DirEntry)));
				   break;                                                              
			  case 1 : WriteString (26, 8 + RowCount, Ptr->DirEntry);
				   WriteMultiChar (26+strlen(Ptr->DirEntry), 8+RowCount, ' ', (14-strlen (Ptr->DirEntry)));
				   break;
			  case 2 : WriteString (46, 8 + RowCount, Ptr->DirEntry);
				   WriteMultiChar (46+strlen (Ptr->DirEntry), 8+RowCount, ' ', (14-strlen (Ptr->DirEntry)));
				   break;                                                             
			  case 3 : WriteString (63, 8 + RowCount, Ptr->DirEntry);
				   WriteMultiChar (63+strlen (Ptr->DirEntry), 8+RowCount, ' ', (14-strlen (Ptr->DirEntry)));
				   break;
		     }
		}
		else
		{
			switch ((count - start) % 4)
			{
			     case 0 : WriteString ( 6, 8 + RowCount, padded);
				      WriteMultiChar (6+strlen (padded), 8+RowCount, ' ', 14-strlen (padded));
				      break;                                                              
			     case 1 : WriteString (26, 8 + RowCount, padded);
				      WriteMultiChar (26+strlen(padded), 8+RowCount, ' ', 14-strlen (padded));
				      break;
			     case 2 : WriteString (46, 8 + RowCount, padded);
				      WriteMultiChar (46+strlen (padded), 8+RowCount, ' ', 14 - strlen (padded));
				      break;                                                             
			     case 3 : WriteString (63, 8 + RowCount, padded);
				      WriteMultiChar (63+strlen (padded), 8+RowCount, ' ', 14-strlen (padded));
				      break;
			}
		 }
		 count++;  
		 if ((count % 4) == 0)
		     RowCount++;
		 Ptr = Ptr->NextEntry;
	}
    }
}

void ShowDir (unsigned int start, unsigned int finish)

{
    dir_spec[0] = (char)0;

    SetColourPair (palette.black, palette.bright_white);
    clear_area (3, 7, 78, 23);
    draw_box (1, 1, 80, 23);
    SetColourPair (palette.red, palette.bright_white);
    WriteString ( 30, 23, " Press F1 For Help ");
    DrawContentsOutline ();

    SetColourPair (palette.black, palette.bright_white);
    WriteString (4, 2, "rives");
    WriteString ( 4, 6, "iles In");
    SetColourPair (palette.red, palette.bright_white);
    WriteChar   (3, 2, 'D');
    WriteChar   (3, 6, 'F');
    
    if (dir_spec[0] == '\0')
	getcwd (dir_spec, 66);

    SetColourPair (palette.black, palette.bright_white);
    WriteString (12, 6, dir_spec);

    DisplayDriveSet ();
    ShowDirList (start, finish, 0);
}

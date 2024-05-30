#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "dostel2.h"
#include "palette.h"
#include "windows.h"
#include "keys.h"

#define WRAP_OFF           0
#define WRAP_ON            1

extern void draw_edging (int, int, int, int);

void   CalculateZoomFactors (void);
void   ShowCursorPositionStats (void);
void   ShowStaticFileStats (char *);
void   ShowThumbNails (unsigned int);
void   DrawViewerOutline (void);
void   TextViewer (char *);
void   UpdateThumbNails (void);
long   lmin (long, long);
void   SetInitView (void);
void   ShowView (void);
void   ViewerCursor (unsigned int);

char   *ViewerBuffer;
FILE   *fptr;
long   CurrentRow,
       CurrentColumn,
       LineCount,
       LongestLine;
int    StopViewing;
unsigned int XZoomFactor,
	     YZoomFactor,
	     XThumbNail,
	     YThumbNail;
long  FirstLine,
      LastLine,
      LeftMargin,
      RightMargin;

extern struct palette_str palette;

void TextViewer (char *FileName)
{
    char         *InfoBuffer;
    unsigned int ch;

    ViewerBuffer = (char *)malloc (PORTSIZE (0, 1, 79, 23));

    if (ViewerBuffer != NULL)
    {
	 save_area (1, 1, 80, 23, ViewerBuffer);        
	 DrawViewerOutline ();
	 ShowStaticFileStats (FileName);
		  
	 fptr = fopen (FileName, "r+b");
	  
	 CurrentRow    = 1;
	 CurrentColumn = 1;
	 StopViewing   = TRUE; 
	 XThumbNail    = 5;
	 YThumbNail    = 4;

	 InfoBuffer = (char *)malloc (PORTSIZE (20, 6, 60, 11));
	 if (InfoBuffer != NULL)
	 {
	       save_area (20, 6, 60, 11, InfoBuffer);
	       SetColourPair (palette.bright_white, palette.blue);
	       draw_window (20, 6, 59, 10);
	       SetColourPair (palette.yellow, palette.blue);
	       WriteString (27, 8, "Calibrating - Please Wait ..");
	 }
	 CalculateZoomFactors ();

	 restore_area (20, 6, 60, 11, InfoBuffer);
	 free ((char *)InfoBuffer);

	 StopViewing = FALSE;
	 SetInitView ();
	
	 while (! StopViewing)   
	 {
	     ShowCursorPositionStats ();
	     ShowThumbNails (palette.bright_white);
	     ShowView ();

	     ch = get_key();
	     switch (ch)
	     {
		   case K_ESC       : StopViewing = TRUE;
				      break;
		   case K_CSR_LT    : if (CurrentColumn > LeftMargin)
					   CurrentColumn--;
				      else
				      {
					  if (LeftMargin > 1L)
					  {
					       clear_area (5, 4, 79, 20);
					       LeftMargin--;
					       RightMargin--;
					       CurrentColumn--;
					   }
				      }
				      ShowCursorPositionStats ();
				      break;
		   case K_CSR_RT    : if (CurrentColumn < RightMargin)
					  CurrentColumn++;
				      else
				      {
					   if (RightMargin < LongestLine)
					   {
						  clear_area (5, 4, 79, 20);
						  LeftMargin++;
						  RightMargin++;
						  CurrentColumn++;
					   }
				      }     
				      ShowCursorPositionStats ();
				      break;
		   case K_CSR_UP    : if (CurrentRow >= FirstLine)
					    CurrentRow--;
				      else
				      {
					  clear_area (5, 4, 79, 20);
					  if (FirstLine > 1L)
					  {
					       FirstLine--;
					       LastLine--;
					       ShowView ();
					       CurrentRow = FirstLine;
					  }
				      }     
				      ShowCursorPositionStats ();
				      break;
		    case K_CSR_DN   : if (CurrentRow <= LastLine)
					    CurrentRow++;
				      else
				      {
					  if (FirstLine + 17L < LineCount)
					  {
					      clear_area (5, 4, 79, 20);
					      FirstLine++;
					      LastLine++;
					      ShowView ();
					      CurrentRow = LastLine;
					  }     
				      }    
				      ShowCursorPositionStats ();   
				      break;                                
		    case K_CTRL_CSR_UP : CurrentRow = 1;
					 FirstLine  = 1L;
					 LastLine       = 17L;
					 ShowCursorPositionStats ();
					 clear_area (5, 4, 79, 20);
					 ShowView ();  
					 break;
		    case K_CTRL_CSR_LT : CurrentColumn = 1L;
					 LeftMargin    = 1L;
					 RightMargin   = 75L;
					 ShowCursorPositionStats ();
					 clear_area (5, 4, 79, 20);
					 ShowView ();
					 break;
		    case K_END         : if (LineCount > 17L)
					 {
					       CurrentRow = LineCount - 17L;
					       FirstLine  = LineCount - 17L;
					       LastLine   = LineCount;
					       ShowCursorPositionStats ();
					       clear_area (5, 4, 79, 20);
					       ShowView ();      
					  }
					  break;
		    case K_HOME      : SetInitView ();
				       ShowCursorPositionStats ();   
				       ShowView ();  
				       clear_area(5, 4, 79, 20);
				       break;
		    case K_F1       : InfoBuffer = (char *)malloc (PORTSIZE (9, 4, 70, 19));
				      if (InfoBuffer != NULL)
				      {
					     save_area (9, 4, 70, 19, InfoBuffer);
					     SetColourPair (palette.black, palette.green);
					     draw_window (9, 4, 69, 18);
					     ShowHeader(9, 69, 5, "List Of Active Keys");
					     WriteString (12,  7, "F1                - Display This Help Page.");
					     WriteString (12,  8, "ESC               - Quit File Viewer.");
					     WriteString (12,  9, "CSR-UP/DN         - Move Up/Down Through File.");
					     WriteString (12, 10, "CSR-LT/RT         - Move Left/Right Along Current Line.");
					     WriteString (12, 11, "HOME              - Move To Top Of File.");
					     WriteString (12, 12, "CTRL-CSR-UP       - Move To First Line In File.");
					     WriteString (12, 13, "CTRL-CSR-LFT      - Move To First Column In File.");
					     WriteString (12, 14, "CTRL-CSR-DN       - Move To Last Line In File.");
					     WriteString (12, 15, "ALT-S             - Search File For Matching Text.");
					     WriteString (12, 16, "ALT-I             - Display Info About File.");
					     SetColourPair (palette.bright_white, palette.green);                                
					     WriteString (25, 18, " Press Any Key To Continue ");
					     get_key();                                                     
					     restore_area (9, 4, 70, 19, InfoBuffer);
					     free ((char *)InfoBuffer);
				       }
				       break;
		  case K_ALT_I       : InfoBuffer = (char *)malloc (PORTSIZE (5, 3, 76, 11));
				       if (InfoBuffer != NULL)
				       {
					    save_area (5, 3, 76, 11, InfoBuffer);
					    SetColourPair (palette.black, palette.green);
					    draw_window (5, 3, 75, 10);
					    ShowEntryStats (FileName);
					    restore_area (5, 3, 76, 11, InfoBuffer);
					    free ((char *)InfoBuffer);
				       }
				       break;
		  case K_ALT_S       : TextSearch (FileName);
				       break;
		  default          :
				       break;
	     }
	     UpdateThumbNails ();      
	}

	restore_area (1, 1, 80, 23, ViewerBuffer);
	free ((char *)ViewerBuffer);
    }
    fclose (fptr);
}

long lmin (long a, long b)
{
    if (a > b)
	return (b);
    else
	return (a);
}

void ShowView ()
{
    long i, l, lc;
    int  ch;

    lc = 1L;

    rewind (fptr);
    while (lc < FirstLine)
    {
	ch = fgetc (fptr);
	if (ch == 10)
	   lc++;
    }           
    l = FirstLine;

    while (l <= LastLine)
    {
	 i  = 0L;
	 ch = fgetc (fptr);
       
	 while ((i < LeftMargin - 1) && (ch != 10))
	 {
	      i++;
	      ch = fgetc (fptr);
	 }

	SetColourPair (palette.black, palette.green);
	while ((i < RightMargin) && (ch != 10))
	{
	     if ( ch == 9) /* TAB */
	     {
		write_blanks (6 + i - LeftMargin, 4 + l - FirstLine, 4);
		i+=4;
	     }
	     else
	     {
		 if (ch == 13)
		 {
		    /* Do nowt - move to next line hadnled by LF */
		 }
		 else
		 {
		     WriteChar ((unsigned int)(6 + i - LeftMargin), (unsigned int)(4 + l - FirstLine), (char)ch);
		     i++;
		 }
	     }
	     ch = fgetc (fptr);
	}
	l++;
    }
}


void SetInitView ()
{
    LeftMargin  = 1L;
    RightMargin = 75L;
    FirstLine   = 1L;
    LastLine    = 17L;
    CurrentRow  = FirstLine;
    CurrentColumn = LeftMargin;    
}

void DrawViewerOutline ()
{
    int i;

    SetColourPair (palette.black, palette.green);
    draw_edging (1,1,80,23);
    
    WriteChar (1, 3, (char)204);
    WriteChar (80, 3, (char)185);
    WriteMultiChar (2, 3, (char)205, 78);     
	
    WriteChar (4, 3, (char)203);
    for (i = 4; i <= 20; i++)
	WriteChar (4, i, (char)186);

    WriteChar (1, 21, (char)204);
    WriteChar (80, 21, (char)185);
    WriteMultiChar (3, 21, (char)205, 2);
    WriteMultiChar (2, 21, (char)205, 78);

    WriteChar (52, 1, (char)203);
    WriteChar (52, 2, (char)186);
    WriteChar (52, 3, (char)202);

    WriteChar (4, 21, (char)206);
    WriteChar (4, 22, (char)186);
    WriteChar (4, 23, (char)202);

    WriteString (54, 2, "ROW :       COLUMN :    ");
}

void ShowStaticFileStats (char *FileName)
{
    char ThisDir[50];
    char str[67];

    getcwd (ThisDir, 49);
    if ( ThisDir[strlen (ThisDir)-1] == '\\')
	sprintf (str, "%s\%s", ThisDir, FileName); 
    else
	sprintf (str, "%s\\%s", ThisDir, FileName);

    SetColourPair (palette.red, palette.green);
    WriteString (3, 2, str);
}

void ShowCursorPositionStats ()
{
    char str[10];

    SetColourPair (palette.black, palette.green);
    write_blanks (60, 2, 6);
    write_blanks (75, 2, 4);

    ltoa (CurrentRow, str, 10);
    SetColourPair (palette.red, palette.green);
    WriteString (60, 2, str);
    
    ltoa (CurrentColumn, str, 10);
    WriteString (75, 2, str);
}

void CalculateZoomFactors ()
{
     long CharCount;
     int  ch;

     LineCount   = 0L;
     LongestLine = 0L;
     CharCount   = 0L;

     while (! feof (fptr))
     {
	 ch = fgetc (fptr);
	 if (ch == 10)
	 {
	       LineCount++;
	       CharCount = 0L;
	 }
	 else
	     CharCount++;
	 if (CharCount > LongestLine)
		LongestLine = CharCount;
	   
     }
     if (LongestLine < 78L)
     {      
	 XZoomFactor = 78L; 
	 XThumbNail = 78L;
     }   
     else
	 XZoomFactor = (unsigned int)(LongestLine / 78);
     if (LineCount < 17)
	 YZoomFactor = 17;
     else
	 YZoomFactor = (unsigned int)(LineCount / 17);
}       
		
void UpdateThumbNails ()
{
    ShowThumbNails (palette.green);

    YThumbNail = 4 + (unsigned int)(CurrentRow / YZoomFactor);
    if (YThumbNail > 20)
	YThumbNail = 20;
    XThumbNail = 5 + (unsigned int)(CurrentColumn / XZoomFactor);
    if (XThumbNail > 78L)
	XThumbNail = 78L;        

    if (CurrentRow == 1)
	 YThumbNail = 4;
    if (CurrentRow == LineCount)
	YThumbNail = 20;
    if (CurrentColumn == 1)
	XThumbNail = 5;
    if (CurrentColumn == LongestLine)
       XThumbNail = 78;
 
    ShowThumbNails (palette.bright_white);
}
   
void ShowThumbNails (unsigned int colour)
{
    SetColourPair (palette.black, (long)colour);
    WriteChar (2, YThumbNail, ' ');
    WriteChar (3, YThumbNail, ' ');
    WriteChar (XThumbNail, 22, ' ');
}

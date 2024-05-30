#include <dos.h>
#include <stdio.h>
#include <bios.h>
#include <malloc.h>
#include "dostel2.h"
#include "keys.h"
#include "windows.h"
#include "palette.h"

#define MAX_DRIVES 26    /* Support DOS Drives A: Thru Z: */

#define FIRST_DRIVE_LINE  2
#define SECOND_DRIVE_LINE (FIRST_DRIVE_LINE + 1)
#define THIRD_DRIVE_LINE  (FIRST_DRIVE_LINE + 2)

int          DriveList[MAX_DRIVES];
int          LastDrive;

extern void         clear_area (int, int, int, int);
extern void         SetColourPair (short, short);
extern void         WriteChar (int, int, char);
extern void         WriteString (int, int, char *);
extern int          ActiveDrive;
extern void         InitDirList (void);
extern void         ClearDirList (void);
extern int          BuildDirList (void);
extern void         ShowDir (unsigned int, unsigned int);
extern void         ShowDirList (unsigned int, unsigned int, unsigned int);
extern unsigned int CheckDriveStatus (unsigned int);
extern void         ShowCurrentPath (void);
extern void         draw_window (int, int, int, int);
extern void         save_area (int, int, int, int, char *);
extern void         restore_area (int, int, int, int, char *);

void SetDriveList (void);
void DisplayDriveSet (void);
void ShowDrive (int, unsigned int);
    
extern struct palette_str palette;

void SetDriveList ()

{
    unsigned int i;
    unsigned int CurrentDrive,
		 Drive,
		 TotalDrives;

    _dos_getdrive (&CurrentDrive); /* Store Current Drive For Now */

    LastDrive = 0;
    for (i = 0; i < MAX_DRIVES; i++)
    {
	 _dos_setdrive (i + 1, &TotalDrives);  
	 _dos_getdrive (&Drive);
	 DriveList[i] = (Drive == (i + 1));
	 if (DriveList[i])
	       LastDrive = i;    
    } 
    _dos_setdrive (CurrentDrive, &TotalDrives);
}

void DisplayDriveSet ()

{
    int CurrentDrive,
	DriveCount;

    CurrentDrive = 0;
    DriveCount   = 0;
    SetColourPair (palette.black, palette.bright_white);
    while (CurrentDrive <= LastDrive)
    {
	if (DriveList[CurrentDrive])
	{
	    if (DriveCount < 12)
	    {
		WriteChar (10 + (5 * DriveCount), FIRST_DRIVE_LINE, '[');
		WriteChar (10 + (5 * DriveCount) + 1, FIRST_DRIVE_LINE, (char)('A' + CurrentDrive));
		WriteString (10 + (5 * DriveCount) + 2, FIRST_DRIVE_LINE, ":]");
	    }
	    else
	    {
		  if (DriveCount < 24)
		  {
			WriteChar (10 + (5 * (DriveCount - 12)), SECOND_DRIVE_LINE, '[');
			WriteChar (10 + (5 * (DriveCount - 12)) + 1, SECOND_DRIVE_LINE, (char)('A' + CurrentDrive));
			WriteString (10 + (5 * (DriveCount - 12)) + 2, SECOND_DRIVE_LINE, ":]");
		  }
		  else
		  {
			WriteChar (10 + (5 * (DriveCount - 24)), THIRD_DRIVE_LINE, '[');
			WriteChar (10 + (5 * (DriveCount - 24)), THIRD_DRIVE_LINE, (char)('A' + CurrentDrive));
			WriteString (10 + (5 * (DriveCount - 24)), THIRD_DRIVE_LINE, ":]");
		  }      
	    }
	    DriveCount++;
	}
	CurrentDrive++;
    }
}

void ShowDrive (int DriveNumber, unsigned int state)

{
    int DrivesCounted,
	found,
	NextDrive,
	DriveIdx;

    DrivesCounted = 0;
    DriveIdx = 0;
    NextDrive = 0;
    found = (DriveNumber == DrivesCounted);
    while ( ! found && (DriveIdx < MAX_DRIVES))
    {
	if (DriveList[DriveIdx] == 1)
	{
	    DrivesCounted++;
	    NextDrive++;
	    if ( DrivesCounted == DriveNumber)
		found = TRUE;
	    else
		DriveIdx++;
	}
	else
	{
	    while (! found && (DriveList[DriveIdx] == 0) && ( DriveIdx < MAX_DRIVES))
	    {
		DrivesCounted++;
		found = (DrivesCounted == DriveNumber);
		DriveIdx++;
	    }
	}
    }

    if (state)
	SetColourPair (palette.red, palette.bright_white);
    else
	SetColourPair (palette.black, palette.bright_white);

    if (NextDrive < 12)
    {
	WriteChar (10 + (5 * NextDrive), FIRST_DRIVE_LINE, '[');
	WriteChar (10 + (5 * NextDrive) + 1, FIRST_DRIVE_LINE, (char)('A'+DrivesCounted));
	WriteString (10 + (5 * NextDrive) + 2, FIRST_DRIVE_LINE, ":]");
    }
    else
    {
	if (NextDrive < 24)
	{
	    WriteChar (10 + (5 * (NextDrive - 12)), SECOND_DRIVE_LINE, '[');
	    WriteChar (10 + (5 * (NextDrive - 12)) + 1, SECOND_DRIVE_LINE, (char)('A'+DrivesCounted));
	    WriteString (10 + (5 * (NextDrive - 12)) + 2, SECOND_DRIVE_LINE, ":]");
	}
	else
	{
	     WriteChar (10 + (5 * (NextDrive - 24)), THIRD_DRIVE_LINE, '[');
	     WriteChar (10 + (5 * (NextDrive - 24)) + 1, THIRD_DRIVE_LINE, (char)('A'+DrivesCounted));
	     WriteString (10 + (5 * (NextDrive - 24)) + 2, THIRD_DRIVE_LINE, ":]");
	}     
    }
}


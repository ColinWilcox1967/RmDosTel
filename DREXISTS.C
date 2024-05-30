#include <stdio.h>
#include <bios.h>
#include <malloc.h>
#include "palette.h"
#include "windows.h"


extern unsigned int CheckDriveStatus(unsigned int);
extern void save_area (int, int, int, int, char *);
extern void restore_area (int, int, int, int, char *);
extern void WriteString (int, int, char *);
extern void WriteChar (int, int, char );
extern void SetColourPair (short, short);
extern unsigned int get_key(void);
extern void draw_window (int, int, int, int);

extern struct palette_str palette;

unsigned int dsk_status1, dsk_status2;
unsigned int AttachedDrives;
char *drive_buffer;

unsigned int DiskInDrive ( unsigned int drive)
{
    AttachedDrives = (_bios_equiplist () & 192) + 1;
    dsk_status1 = CheckDriveStatus (drive);
    dsk_status2 = CheckDriveStatus (drive + 128 - AttachedDrives - 1);
    return (((dsk_status1 == 0) && (drive < AttachedDrives)) || 
            ((dsk_status2 == 0) && (drive >= AttachedDrives)) ||
             (dsk_status1 == 255) ||
             (dsk_status2 == 255)
           );
}

void ReportDriveStatus (unsigned int drive)
{
    if (drive < AttachedDrives)
    {
        switch (dsk_status1)
        {
             case 2  : drive_buffer = (char *)malloc (PORTSIZE(25, 7, 56, 13));
                       if (drive_buffer != NULL)
                       {
                           save_area (25, 7, 56, 13, drive_buffer);
                           SetColourPair (palette.black, palette.cyan);
                           draw_window (25, 7, 55, 12);
                           WriteString (29, 9, "Insert A Formatted Disk");
                           WriteString (34, 10, "And Try Again.");
                           WriteString (27, 12, " Press Any Key To Continue ");
                           get_key ();
                           restore_area (25, 7, 56, 13, drive_buffer);
                           free((char *)drive_buffer);
                        }
                        break;
              case 128 : drive_buffer = (char *)malloc (PORTSIZE(24, 7, 57, 12));
                         if (drive_buffer != NULL)
                         {
                               save_area (24, 7, 57, 12, drive_buffer);
                               SetColourPair (palette.black, palette.cyan);
                               draw_window (24, 7, 56, 11);
                               WriteString (27, 9, "Insert A Disk And Try Again");
                               WriteString (27, 11, " Press Any Key To Continue ");
                               get_key();
                               restore_area (24, 7, 57, 12, drive_buffer);
                               free ((char *)drive_buffer);
                          }
                          break;
          }
     }
  }

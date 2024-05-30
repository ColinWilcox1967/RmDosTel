#include <bios.h>

unsigned int CheckDriveStatus (unsigned int);

extern int DriveList[];
extern unsigned int AttachedDrives;

/****************************************************/
/* Function : CheckDriveStatus                      */
/*                                                  */
/* Parameters : DriveNumber (  0 + for FDDs)        */
/*                          (128 + for HDDs)        */
/*                                                  */
/* Returns    :  0 -> Drive Valid & Disk OK         */
/*               1 -> Invalid Drive                 */
/*               2 -> Drive Valid, Disk Unformatted */
/*             128 -> Drive Valid, No Disk Present  */
/*             255 -> Possible Network Drive        */
/****************************************************/

unsigned int CheckDriveStatus (unsigned int DriveNumber)

{
    struct diskinfo_t DiskInfo;
    unsigned int      DiskStatus;

    /* it would appear that network drives don't return the same bios
       status info as local drives, so this test is included such that
       network drives are scanned properly */

    if ((DriveNumber > AttachedDrives) && DriveList[DriveNumber])
        DiskStatus = 255;
    else
    {
        DiskInfo.drive    = DriveNumber;  /* Read any old track on drive */
        DiskInfo.head     = 0;            /* to check disk status        */
        DiskInfo.track    = 0;
        DiskInfo.sector   = 1;
        DiskInfo.nsectors = 1;

        /* _BIOS_DISK returns disk error status in HIGH order byte,
           so shift return value right 8 bits */
    
        while ((DiskStatus = (_bios_disk (_DISK_VERIFY, &DiskInfo) >> 8)) == 6);
    }

    return (DiskStatus);
}



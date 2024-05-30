#include <stdio.h>
#include <dos.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#include "config.h"
#include "palette.h"
#include "keys.h"
#include "windows.h"
#include "dostel2.h"

#define GET_DAY(x)          ( x & 0x001F )
#define GET_MONTH(x)        (( x & 0x01E0 ) >> 5 )
#define GET_YEAR(x)         ((( x & 0x0FE00 ) >> 9 ) + 80 )
#define GET_HOUR(x)         (( x & 0x0F800 ) >> 11 )
#define GET_MINUTE(x)       (( x & 0x07E0 ) >> 5 )
#define GET_SECOND(x)       (( x & 0x001F ) * 2 )

#define MAX_TOTAL_BACKUPS 250
#define BACKUP_SPEC      "*.0*"
#define EMPTY_LINE       "                                             "

char ext[3];
char month_name[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                         };

extern FILE *vwr_file;

extern unsigned char config_byte3;
extern char external_editor[];
extern struct palette_str palette;
extern int header_ok (char *);
extern void DrawFileViewer ( char []);
extern void UseViewer ( long );

/* Don't bother storing rest of file info here  -just wasting space
   use "stat" later if the file is selected */

struct backup_entry
{
    char name[13];
    int tagged;
} backups[MAX_TOTAL_BACKUPS];

struct find_t c_file;
int page_no;
int tag_count;

int backup_idx;


void ShowPageNumber (int page)
{
   char str[20];

   SetColourPair (palette.red, palette.blue);
   WriteString (50, 5, "            ");
   sprintf (str, "Page (%d/%d)", page_no, 1+(backup_idx/30));
   WriteString (50, 5, str);
}

void erase_purge_area ()
{
    int i;   
    
    SetColourPair (palette.blue, palette.blue);
    for (i = 7; i <= 17; i++)
        WriteString ( 18, i, EMPTY_LINE);
    ShowPageNumber (page_no);
}

void trash_tagged_files ()
{
    int i;

    for (i = 0; i <= backup_idx; i++)
        if ( backups[i].tagged)
        {
            _dos_setfileattr (backups[i].name, 0);
            remove (backups[i].name);
            backup_idx--;
        }
}

int purge_tagged ()
{
    char *tagged_buff;
    int   retval;
    unsigned int k;

    tagged_buff = (char *)malloc(PORTSIZE(25, 8, 56, 12));
    if (tagged_buff == NULL)
        return;
    save_area (25, 8, 56, 12, tagged_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (25, 8, 55, 11);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 30, 9, "Delete Tagged Files ?");
    retval = get_yesno(34,10, palette.yellow, palette.red);
    restore_area (25, 8, 56, 12, tagged_buff);
    free ((char *)tagged_buff);
    return (retval);
}

int get_backup_list ()
{
    int retval;
    char *waiting_buff;
    int carry_on;

    waiting_buff = (char *)malloc (PORTSIZE(25, 7, 57, 10));
    if (waiting_buff != NULL)
    {
        save_area (25,7,57,10,waiting_buff);
        SetColourPair (palette.bright_white, palette.red);
        draw_window (25,7,56,9);
        SetColourPair (palette.yellow, palette.red);
        WriteString (29, 8, "Looking For Backup Files");
    }

    retval = 0; /* OK to start with */
    backup_idx = 0;
    if ( _dos_findfirst ( BACKUP_SPEC, FILE_ATTR_LIST, &c_file ) == 0)
    {
         if ( header_ok (c_file.name))
         {
              strcpy ( backups[backup_idx].name, c_file.name);
              backups[backup_idx++].tagged = FALSE;
         }     
         carry_on = TRUE;
         while ( carry_on )
         {
             
             if ( _dos_findnext ( &c_file ) == 0)
             {
                  if ( header_ok ( c_file.name ) && ( backup_idx < MAX_TOTAL_BACKUPS))
                  {
                      strcpy ( backups[backup_idx].name, c_file.name);
                      backups[backup_idx++].tagged = FALSE;
                  }
                  else
                     carry_on = FALSE;
             }
             else
                 carry_on = FALSE;
         }
         if ( backup_idx == MAX_TOTAL_BACKUPS)
            retval = -2; /* Backup overrun */
    }
    else
        retval = -1; /* No backup files */

    if ( waiting_buff != NULL)
    {
        restore_area (25, 7,57,10, waiting_buff);
        free ((char *)waiting_buff);
    }

    return (retval);
}


void ShowBackupName (int offset, int base, int state)
{
    if ( state)
    {
        SetColourPair (palette.dark_grey, palette.blue);
    }
    else
    {
        if (backups[base+offset].tagged)
        {
             SetColourPair (palette.black, palette.blue);
        }
        else
        {
            SetColourPair (palette.bright_white, palette.blue);
        }
    }

    switch ( offset / 10) /* determine correct column */
    {
        case 0  : WriteString (18, 7 + (offset % 10), backups[offset+base].name);
                  break;
        case 1  : WriteString (33, 7 + (offset  % 10), backups[offset+base].name);
                  break;                
        case 2 :  WriteString (48, 7 + (offset  % 10), backups[offset+base].name);
                 break;

    }
}

void show_backup_files (int base)
{
     int idx;

     SetColourPair (palette.bright_white, palette.blue);
     idx = 0;
     while ( (idx < 30) && ( base + idx < backup_idx))
     {
         ShowBackupName ( idx, base, FALSE);
         idx++;
     }
}

void do_backup_help()
{
    char *backup_help_buff;
        
    backup_help_buff = (char *)malloc (PORTSIZE (17, 6, 64, 21));
    if ( backup_help_buff == NULL)
       return;
    save_area ( 17, 6, 64, 21, backup_help_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (17, 6, 63, 20);
    ShowHeader (17, 63, 7, "Delete Backup Help");
    SetColourPair (palette.black, palette.green);
    WriteString ( 20,  9, "F1         - Display This Help Page.");
    WriteString ( 20, 10, "ESC        - Terminate Deletion.");
    WriteString ( 20, 11, "ENTER      - Display File Information.");
    WriteString ( 20, 12, "SPACE      - Tag/Untag Highlighted File.");
    WriteString ( 20, 13, "ALT-T/U    - Tag/Untag All Files.");
    WriteString ( 20, 14, "ALT-V      - View Highlighted Backup File.");
    WriteString ( 20, 15, "CSR-UP/DN  - Move To Next/Previous File.");
    WriteString ( 20, 16, "CSR-LT/RT  - Move To Next/Previous Column.");
    WriteString ( 20, 17, "HOME       - Move To First Page.");
    WriteString ( 20, 18, "PGUP/DN    - Move To Next/Previous Page.");
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 27, 20, " Press Any Key To Continue ");
    SetColourPair (palette.black, palette.green);
    get_key();
    restore_area (17, 6, 64, 21, backup_help_buff);
    free ((char *)backup_help_buff);
}

void day_ext (int d)
{
    switch ( d)
    {
        case  1 :
        case 21 :
        case 31 :
                  strcpy ( ext, "st");
                  break;
        case 2  :
        case 22 : strcpy ( ext, "nd");
                  break;
        case 3  :
        case 23 : strcpy ( ext, "rd");
                  break;
        default : strcpy ( ext, "th");
                  break;
    }
}

void fill_in_file_details (int current)
{
   struct find_t stat;
   char   str[20];

   /* NOte : call below will work OK since file WILL exists at this stage */

   WriteString ( 41, 13, "-----");

   _dos_findfirst (backups[current].name, FILE_ATTR_LIST, &c_file );

   SetColourPair (palette.red, palette.cyan);

   /* Show File Date */

   day_ext (GET_DAY(c_file.wr_date));
   sprintf ( str, "%d%s %s %d", GET_DAY(c_file.wr_date),
                                ext,
                                month_name [GET_MONTH(c_file.wr_date)-1],
                                1900+GET_YEAR(c_file.wr_date)
           );
   WriteString ( 41, 10, str);

   /* Show Time */

   sprintf ( str, "%02d:%02d%s", GET_HOUR(c_file.wr_time), GET_MINUTE(c_file.wr_time),
                                 GET_HOUR(c_file.wr_time) >= 12 ? "pm":"am"
           );
   WriteString ( 41, 11, str);

   /* Show File Size */

   sprintf ( str, "%ld Bytes", c_file.size);
   WriteString (41, 12, str);

   /* Show Attributes */

   if ( (c_file.attrib & _A_NORMAL) == _A_NORMAL)
       WriteChar (41, 13, 'N');
   if ( ( c_file.attrib & _A_RDONLY) == _A_RDONLY)
       WriteChar (42, 13, 'R');
   if (( c_file.attrib & _A_HIDDEN) == _A_HIDDEN)
       WriteChar (43, 13, 'H');
   if ((c_file.attrib & _A_SYSTEM) == _A_SYSTEM)
       WriteChar (44, 13, 'S');
   if ((c_file.attrib & _A_ARCH) == _A_ARCH)
       WriteChar (45, 13, 'A');

}

void show_file_info ( int current)
{
    char *info_buff;
    char header[20];

    info_buff = (char *)malloc (PORTSIZE (25, 9, 56, 15));
    if ( info_buff == NULL)
        return;
    save_area ( 25, 9, 56, 15, info_buff);
    SetColourPair (palette.bright_white, palette.cyan);
    draw_window (25, 9, 55, 14);
    SetColourPair (palette.red, palette.cyan);
    sprintf ( header, "%c%s%c", ' ', backups[current].name, ' ');
    WriteString ( 25 + ((30 - strlen (header))/2)+1, 9, header);

    SetColourPair (palette.black, palette.cyan);
    WriteString ( 27, 10, "Created On  : ");
    WriteString ( 27, 11, "        At  : ");
    WriteString ( 27, 12, "File Size   : ");
    WriteString ( 27, 13, "Attributes  : ");
    fill_in_file_details (current);
    get_key();
    restore_area (25,9,56,15,info_buff);
    free((char *)info_buff);

}

void do_untag_all ( int base )
{
    int i;

    for (i = 0; i <= backup_idx; i++)
        backups[i].tagged = FALSE;
    tag_count = 0;
    show_backup_files (base);
}

void do_tag_all( int base )
{
    int i;

    tag_count = 0;
    for (i = 0; i <= backup_idx; i++)
    {
       tag_count++;
        backups[i].tagged = TRUE;
    }
    show_backup_files (base);
}


void view_backup_file (unsigned int index)
{
    struct find_t c_file;
    char *bk_view_buff;

    bk_view_buff = (char *)malloc(PORTSIZE (1, 1, 80, 23));
    if ( bk_view_buff == NULL)
        return;
    save_area (1, 1, 80, 23, bk_view_buff);

    if (config_byte3 & MSK_INTEXT_VIEWER)
        exec_external (external_editor);
    else
    {
        if (config_byte3 & MSK_DISPLAY_MODE)
        {
             _dos_findfirst (backups[index].name, FILE_ATTR_LIST, &c_file);
             DrawFileViewer (backups[index].name);
             vwr_file = fopen (backups[index].name, "r+b");
             UseViewer (c_file.size);
             fclose ( vwr_file);
             restore_area (1, 1, 80, 23, bk_view_buff);
             free ((char *)bk_view_buff);
        }
        else
            TextViewer (backups[index].name);
    }
}

void process_purge ()
{
    char *purge_buff;
    char *errbuff;
    unsigned int k;
    int status,
        done,
        current,
        base_idx;

    status = get_backup_list ();
    done = FALSE;
    if ( status != 0)
    {
        errbuff = (char *)malloc (PORTSIZE (25, 7, 56, 10));
        if (errbuff != NULL)
        {
            save_area ( 25, 7, 56, 10, errbuff);
            SetColourPair (palette.bright_white, palette.red);
            draw_window (25, 7, 55, 9);
            SetColourPair (palette.yellow, palette.red);
            if ( status == -1)
            {
                WriteString ( 30, 8, "No Backup Files Found");
                done = TRUE;
            }
            else
                WriteString ( 30, 8, "Backup Limit Reached");
            get_key();
            restore_area (25, 7, 56, 10, errbuff);
            free ((char *)errbuff);
        }
    }
    if ( ! done)
    {
        page_no=1;
        purge_buff = (char *)malloc (PORTSIZE (15, 4, 65, 19));
        if ( purge_buff == NULL)
            return;
        save_area (15, 4,65, 19, purge_buff);
        SetColourPair (palette.bright_white, palette.blue);
        draw_window (15,4,64,18);
        SetColourPair (palette.red, palette.blue);
        WriteString (30, 18, " Press F1 For Help ");
        SetColourPair (palette.bright_white, palette.blue);
        WriteString (33, 5, "Backup Files");
        ShowPageNumber (page_no);
        current = base_idx = 0;
        show_backup_files (base_idx);
        tag_count = 0;
        
        while (! done)
        {
            ShowBackupName (current, base_idx, TRUE);
            k = toupper(get_key());
            if ( ( k != K_F1) && ( k != K_CR))
                ShowBackupName (current, base_idx, FALSE);
            switch ( k)
            {
                case K_ESC    : done = TRUE;
                                break;
                case K_HOME   : current = base_idx = 0;
                                show_backup_files (base_idx);
                                break;
                case K_CR     : show_file_info(base_idx+current);
                                break;
                case K_ALT_T  : do_tag_all(base_idx);
                                break;
                case K_ALT_U  : do_untag_all (base_idx);
                                break;
                case K_ALT_V  :
                                view_backup_file (base_idx+current);
                                break;
                case K_F1     : do_backup_help ();
                                break;
                case K_CSR_LT : if ( current - 10 >= 0)
                                    current -= 10;
                                break;
                case K_CSR_RT : if (( base_idx + current + 10 <= backup_idx - 1) &&
                                    ( current + 10 <= 29)
                                   )
                                    current+=10;
                                break;
                case K_CSR_UP : if ( current > 0)
                                    current--;
                                else
                                {
                                    if ( base_idx >= 30)    
                                    {
                                        page_no--;
                                        erase_purge_area();
                                        base_idx -= 30;
                                        current=29;
                                        show_backup_files (base_idx);
                                    }
                                }
                                break;
                case K_CSR_DN : if ( current < 29)
                                {
                                    if ( base_idx + current < backup_idx - 1)
                                        current++;
                                }
                                else
                                {
                                    if ( base_idx + 30 < backup_idx - 1)
                                    {
                                        page_no++;
                                        erase_purge_area();
                                        base_idx += 30;
                                        current=0;
                                        show_backup_files (base_idx);
                                    }
                                }
                                break;
                case K_PGUP   : if ( base_idx >= 30)
                                {
                                    page_no--;
                                    base_idx -=30;
                                    erase_purge_area();
                                    show_backup_files (base_idx);
                                }
                                break;
                case K_PGDN    : if ( base_idx + 30 < backup_idx)
                                 {
                                     page_no++;
                                     base_idx += 30;
                                     erase_purge_area();
                                     if ( base_idx +  current >= backup_idx)
                                        current = 0;
                                     
                                     show_backup_files (base_idx);
                                 }
                                break;
                case K_SPACE  : if ( backups[base_idx+current].tagged)
                                    tag_count--;
                                else
                                    tag_count++;
                                backups[base_idx+current].tagged = ! backups[base_idx+current].tagged;
                                if ( ( base_idx + current < backup_idx - 1))
                                {
                                    if ( current < 29)
                                    {
                                        ShowBackupName (current, base_idx, FALSE);
                                        current++;
                                        ShowBackupName (current, base_idx, TRUE);
                                    }
                                    else
                                    {
                                        if ( base_idx + 30 < backup_idx - 1)
                                        {
                                            current = 0;
                                            base_idx+=30;
                                            page_no++;
                                            erase_purge_area ();
                                            show_backup_files (base_idx);
                                            ShowBackupName (current, base_idx, backups[base_idx+current].tagged);

                                        }
                                    }
                                }
                                else
                                    ShowBackupName ( current, base_idx, TRUE);

                                break;
                default       : break;
            }
        }  
        if ( tag_count > 0) /* No point asking if none are tagged */
        {
            if (purge_tagged ())
            {
                /* This removes all attributes so be warned !!*/

                trash_tagged_files();
            }
        }
        restore_area (15,4,65,19,purge_buff);
        free ((char *)purge_buff);
     }
}

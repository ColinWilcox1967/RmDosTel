#define MAX_BACKUP_FILES        99   /* ?.001 ----> ?.099 */

/* Video Mode Changes */

#define MONO_TO_COLOUR         0
#define COLOUR_TO_MONO         1

#define MONO_MODE              0
#define COLOUR_MODE            1

/* Masks for Config Byte 1 */

#define MSK_SEARCH_NAME         1     /* 00000001 */
#define MSK_SEARCH_NUMBER       2     /* 00000010 */
#define MSK_SEARCH_PASSWORD     4     /* 00000100 */
#define MSK_SEARCH_ALL          7
#define MSK_SEARCH_CASE         8     /* 00001000 */ /* 1 = Match Case */
                                                     /* 0 = Any Case   */
#define MSK_SORT_ORDER         16     /* 00010000 */ /* 1 = Ascending  */
                                                     /* 0 = Descending */
#define MSK_SORT_CASE          32     /* 00100000 */ /* 1 = Match Case */
                                                     /* 0 = Any Case   */
#define MSK_SORT_KEY           192    /* 11000000 */ /* 11 = Password  */
                                                     /* 10 = Number    */
                                                     /* 01 = Name      */
#define MSK_SORT_NAME          64    
#define MSK_SORT_NUMBER        128
#define MSK_SORT_PASSWORD      192

#define DEFAULT_CONFIG_BYTE1   121    /* 01111001 */ 

/* Masks for Config Byte 2 */

#define MSK_CONVERT_NO_CASE        0
#define MSK_CONVERT_UPPER          1
#define MSK_CONVERT_LOWER          2

#define MSK_BACKUP_LIST            4
#define MSK_STORE_DELETED_ENTRIES  8

#define MSK_VIDEO_DETECT           48
#define MSK_VIDEO_MONO             16
#define MSK_VIDEO_COLOUR           32

#define MSK_WINDOW_TYPE            64    /* 1 = exploding_windows */
                                         /* 0  = normal_windows */
#define MSK_SHADOW_TYPE           128    /* 1 = display window shadow */
                                         /* 0 = no shadow - show flat windows */


#define DEFAULT_CONFIG_BYTE2        56     /* 00111000 */

#define MSK_CONVERSIONS             ( MSK_CONVERT_UPPER | MSK_CONVERT_LOWER )

#define MAX_FILENAME_LENGTH    25


/* Config Byte 3 */

#define MSK_MEMORY_WRITE            1   /* 1 = Screen Writes Using Memory */
                                        /* 0 = Screen Writes Using BIOS */

#define MSK_NO_BORDER               6
#define MSK_SINGLE_LINE_BORDER      2
#define MSK_DOUBLE_LINE_BORDER      4
#define MSK_SAVE_MERGE_LIST         8
#define MSK_DISPLAY_MODE            16
#define MSK_INTEXT_VIEWER           32     /* 
                                              0 - Internal Viewer
                                              1 - External Viewer
                                            */

#define MSK_FILE_READ_TYPE          64          /* 1 = Version 2 File
                                                   0 = Version 1 File
                                                */
#define MSK_FILE_WRITE_TYPE        128         /* 1 = Version 2 File
                                                  0 = Version 1 File
                                               */
#define DEFAULT_CONFIG_BYTE3       197



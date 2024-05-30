
#define BLANK_PR_DESC      "                         "
#define BLANK_PR_INIT      "                    "

#define MAX_PR_FILES       200
#define MAX_PR_DESC_SIZE   25
#define MAX_PR_INIT_SIZE   35

#define NUMBER_OF_FIELDS   8

/* Field Printing Order Definitions */

#define ORDER_NA_NU_PW     1
#define ORDER_NA_PW_NU     2
#define ORDER_PW_NA_NU     3
#define ORDER_PW_NU_NA     4
#define ORDER_NU_PW_NA     5
#define ORDER_NU_NA_PW     6

/* EOL Terminator Definitions */

#define EOL_NONE           1
#define EOL_CR             2
#define EOL_LF             3
#define EOL_CRLF           4

/* Page Throwing Definitions */

#define THROW_USING_LF     1
#define THROW_USING_FF     2


/* Return Codes From BIOS Printer routines */

#define PRINTER_TIME_OUT           1
#define PRINTER_NOT_USED1          2
#define PRINTER_NOT_USED2          4
#define PRINTER_IO_ERROR           8
#define PRINTER_PRINTER_SELECTED  16
#define PRINTER_OUT_OF_PAPER      32
#define PRINTER_ACKNOWLEDGE       64
#define PRINTER_NOT_BUSY         128

/* When should we print header on page */

#define PRINTER_NO_HEADER         0
#define PRINTER_FIRST_PAGE        1
#define PRINTER_ALL_PAGES         2



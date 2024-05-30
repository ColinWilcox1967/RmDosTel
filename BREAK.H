#ifndef TRUE
#define TRUE            ( 0 == 0 )
#endif

#ifndef FALSE
#define FALSE           ( ! TRUE )
#endif

#define INTERCEPT       0
#define RELEASE         1
#define GET             0x00
#define SET             0x01
#define CHKON           0x01
#define CHKOFF          0x00


int             ctrl_c(int);
int             get_ctrl_c_chk(void);
int             set_ctrl_c_chk(int);
int             rawio(int, int);
unsigned int    ioctl(int, int, unsigned int);

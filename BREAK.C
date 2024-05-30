#include <stdio.h>
#include <dos.h>
#include "break.h"

#define BREAKINTR       0x1B
#define DOS             0x21
#define CTRLCCHK        0x33
#define GETVECT         0x35
#define SETVECT         0x25
#define IOCTL           0x44
#define DEVFLAG         0x80
#define RAWFLAG         0x20

int     get_ctrl_c_chk()
{
    union REGS      regs;

    regs.h.ah = CTRLCCHK;
    regs.h.al = GET;
    int86(DOS, &regs, &regs);

    if (regs.x.cflag)
        return(FALSE);

    return(regs.h.dl);
}

int     set_ctrl_c_chk(int value)
{
    union REGS      regs;

    regs.h.ah = CTRLCCHK;
    regs.h.al = SET;
    regs.h.dl = value;
    int86(DOS, &regs, &regs);

    return(!regs.x.cflag);
}

unsigned int    ioctl(int handle, int op, unsigned int value)
{
    union REGS  regs;

    regs.h.ah = IOCTL;
    regs.h.al = op;
    regs.x.bx = handle;
    regs.x.dx = value & 0xFF;
    int86(DOS, &regs, &regs);

    if (regs.x.cflag)
        return(FALSE);

    return(regs.x.dx);
}

int     rawio(int handle, int raw)
{
    unsigned int    flags;

    flags = ioctl(handle, GET, 0);
    if (flags & DEVFLAG) {
        if (raw)
            ioctl(handle, SET, flags | RAWFLAG);
        else
            ioctl(handle, SET, flags & ~RAWFLAG);
    }
    return(flags & RAWFLAG);
}

static void interrupt cdecl far do_intcpt()
{
    return;
}

static void do_break(int op)
{
    union REGS          regs;
    struct SREGS        segs;
    static unsigned int oldseg;
    static unsigned int oldofs;

    if (op == INTERCEPT) {
        segread(&segs);
        regs.h.ah = GETVECT;
        regs.h.al = BREAKINTR;
        int86x(DOS, &regs, &regs, &segs);
        oldseg = segs.es;
        oldofs = regs.x.bx;

        segread(&segs);
        regs.x.dx = (unsigned int) do_intcpt;
        segs.ds   = ((unsigned long) do_intcpt) >> 16;
    }
    else {
        segread(&segs);
        regs.x.dx = oldofs;
        segs.ds   = oldseg;
    }
    regs.h.ah = SETVECT;
    regs.h.al = BREAKINTR;
    int86x(DOS, &regs, &regs, &segs);
}

int     ctrl_c(int op)
{
    static unsigned int outflg;
    static unsigned int inflg;
    static unsigned int ctrlcchk;


    if (op == INTERCEPT) {
        do_break(INTERCEPT);
        inflg  = rawio(fileno(stdin), TRUE);
        outflg = rawio(fileno(stdout), TRUE);
        if ((ctrlcchk = get_ctrl_c_chk()) == CHKON)
            set_ctrl_c_chk(CHKOFF);
    }
    else {
        set_ctrl_c_chk(ctrlcchk);
        rawio(fileno(stdout), outflg);
        rawio(fileno(stdin), inflg);
        do_break(RELEASE);
    }

    return(TRUE);
}


#include <conio.h>
#include <bios.h>
#include <string.h>

#define FALSE (0 == 1)
#define TRUE  (1 == 1)

#define INSERT       0
#define OVERWRITE    1

#define CR          13
#define DEL        211
#define TAB          9
#define CSR_LT     203
#define CSR_RT     205
#define CTL_CSR_LT 243
#define CTL_CSR_RT 244
#define CTL_BS     127
#define BS           8
#define INS        210

extern unsigned int get_key ( void );
extern void WriteChar (int, int, char);

void BlockCursor (void);
void LineCursor (void);
void GotoXY (short, short);

void BlockCursor ()
{
    union REGS regs;

    regs.h.ah = 1;
    regs.h.ch = 0;
    regs.h.cl = 15;
    int86 (0x10, &regs, &regs);
}

void LineCursor ()
{
    union REGS regs;

    regs.h.ah=1;
    regs.h.ch=6;
    regs.h.cl=7;
    int86(0x10, &regs, &regs);
}


void GotoXY (short x, short y)
{
   union REGS regs;

   regs.h.ah = 2;
   regs.h.bh = 0;
   regs.h.dh = (unsigned char)(y-1);
   regs.h.dl = (unsigned char)(x-1);
   int86(0x10, &regs, &regs);
}


int          AcceptableCharacter (unsigned int);
unsigned int getstr (unsigned int, unsigned int, char [], char [], int);


int AcceptableCharacter (unsigned int ch)

{
	 return (((ch >= 32) && (ch <= 90)) || ((ch >= 97) && (ch <= 122)) || (ch == 97) || (ch == '\\'));
}

unsigned int getstr (unsigned int x, unsigned int y, char str[], char SpecialKeys [], int MaxLength)

{
    unsigned int ch,
		 WriteMode;
	 int     FinishedEntry,
		 SpecialKeyPressed;
	 int     CharsRead,
		 index,        
		 CurrentPosn;
     char        s[256];


    CharsRead         = 0;
    CurrentPosn       = 0;
    FinishedEntry     = FALSE;
    SpecialKeyPressed = FALSE;
    s[CharsRead]      = (char)0;
    WriteMode         = INSERT;

BlockCursor ();
while (! FinishedEntry && ! SpecialKeyPressed)
    {
		  GotoXY (x+CurrentPosn, y);    
		  if (CurrentPosn == CharsRead)
		  {
		      WriteMode = INSERT;
				BlockCursor ();
		  }             
		  ch = get_key ();
		  if (strchr(SpecialKeys, (char)ch))
			SpecialKeyPressed = TRUE;
		  else
		      switch (ch)
				{
					 case CR         : 
					 case TAB        : FinishedEntry = TRUE;
							   break;
					 case INS        : if (WriteMode == INSERT)
							   {
								  LineCursor ();      
								  WriteMode = OVERWRITE;
							   }
							   else
							   {
								  BlockCursor ();
								  WriteMode = INSERT;
							   }
							   break;
					 case CSR_RT     : if (CurrentPosn < CharsRead)
								  CurrentPosn++;
							   break;
					 case CSR_LT     : if (CurrentPosn > 0)
								  CurrentPosn--;     
							   break;
					 case CTL_CSR_LT : CurrentPosn = 0;
							   break;
					 case CTL_CSR_RT : CurrentPosn = CharsRead;
							   break;
					 case CTL_BS     : for (index = 0; index < CharsRead; index++)
								  WriteChar (x+index, y, ' ');
							   CharsRead = 0; CurrentPosn = 0;
							   break;

					 case DEL        : if (CharsRead > 0)
							   {
								  for (index = CurrentPosn; index < CharsRead; index++)
									 s[index] = s[index+1];
								  CharsRead--;
								  WriteChar (x + CharsRead, y, ' ');
								  if (CurrentPosn > CharsRead)
								      CurrentPosn = CharsRead;
								  for (index = 0; index < CharsRead; index++)
								      WriteChar (x+index, y, s[index]);
							   }
							   break;
					 case BS         : if (CurrentPosn > 0)
							   {
								  for (index = CurrentPosn; index <= CharsRead; index++)
									 s[index - 1] = s[index];
								  CharsRead--;
								  CurrentPosn--;
								  for (index = 0; index < CharsRead; index++)
									 WriteChar (x+index, y, s[index]);
								  WriteChar (x+CharsRead, y, ' ');
							    }
							    break;
				    default              : if (AcceptableCharacter (ch))
							   {
								 if (MaxLength >= 0)
								 {
								       if (WriteMode == INSERT)
								       {
									       if (CharsRead < MaxLength)
									       {
										      for (index = CharsRead + 1; index >= CurrentPosn; index--)
											    s[index+1] = s[index];
										      s[CurrentPosn++] = (char)ch;
										      CharsRead++;
									       }           
								       }
								       else
									    s[CurrentPosn++] = (char)ch;
								       for (index = 0; index < CharsRead; index++)
									      WriteChar (x+index, y, s[index]);
								  }
								  else
								  {
									  if (WriteMode == INSERT)
									  {
										  for (index = CharsRead + 1; index >= CurrentPosn; index--)
											 s[index+1]=s[index];
										  s[CurrentPosn++] = (char)ch;
										  CharsRead++;
									   }
									   else
										 s[CurrentPosn++] = (char)ch;
									   for (index = 0; index < CharsRead; index++)
										  WriteChar (x+index, y, s[index]);
								   }
							    }      
							    break;
				}
    }
    hide_cursor();
    if (SpecialKeyPressed)
	     return (ch);
    else
    {
       strcpy (str, s);      
	return (0);
    }
}
			

    

    



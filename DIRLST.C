#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>
#include "dostel2.h"

struct dir_s
{
    char DirEntry[13];
    struct dir_s *PreviousEntry;
    struct dir_s *NextEntry;
};

typedef struct dir_s *dir_p;

void  InitDirList (void);
void  ClearDirList (void);
void  AddDirEntry (dir_p);
int   BuildDirList (void);
dir_p FindEntryN (unsigned int);

extern void WriteChar (unsigned int, unsigned int, char);

dir_p DirList,
      NewEntry;

void InitDirList ()

{
    DirList = NULL;
}

void AddDirEntry (dir_p Entry)

{
    dir_p ThisEntry,
	  LastEntry;

    ThisEntry = DirList;
    LastEntry = NULL;
    if (ThisEntry == NULL)
	DirList = Entry;
    else
    {
	 while ((strcmp (Entry->DirEntry, ThisEntry->DirEntry) > 0) && (ThisEntry != NULL))
	 {
	      LastEntry = ThisEntry;
	      ThisEntry = ThisEntry->NextEntry;
	 }
	 if (ThisEntry == NULL)
	 {
	       LastEntry->NextEntry = Entry;
	       Entry->PreviousEntry = LastEntry;
	 }    
	 else
	 {
	       if (LastEntry == NULL)
	       {
		       Entry->NextEntry       = DirList;
		       DirList->PreviousEntry = Entry;
	       }
	       else
	       {
		       Entry->NextEntry         = ThisEntry;
		       Entry->PreviousEntry     = ThisEntry->PreviousEntry;
		       ThisEntry->PreviousEntry = Entry;
		       LastEntry->NextEntry     = Entry;
	       }
	  }
    }           
}

void ClearDirList ()
{
    dir_p ThisEntry,
	  LastEntry;

    ThisEntry = DirList;
    LastEntry = NULL;
    while (ThisEntry != NULL)
    {
	LastEntry = ThisEntry;
	ThisEntry = ThisEntry->NextEntry;
	free ((struct dir_s *)LastEntry);
   }
}

int BuildDirList ()
{
    struct find_t c_file;

    if (_dos_findfirst ("*.*", _A_NORMAL | _A_SUBDIR, &c_file) == 0)
    {
	NewEntry = (struct dir_s *)malloc (sizeof (struct dir_s));
	if (NewEntry == NULL)
	    return (-1);
	if (c_file.name[0] == '.')
	{
	   if (c_file.name[1] == '.')
	   {    
	       strcpy (NewEntry->DirEntry, "<**PARENT**>");
	       NewEntry->PreviousEntry = NULL;
	       NewEntry->NextEntry = NULL;
	       AddDirEntry (NewEntry);
	   }
	}
	while (_dos_findnext (&c_file) == 0)
	{
	   NewEntry = (struct dir_s *)malloc (sizeof (struct dir_s));
	   if (NewEntry == NULL)
		   return (-1);
	   if (c_file.name[0] == '.')
	   {
	       if (c_file.name[1] == '.')
	       {    
		   strcpy (NewEntry->DirEntry, "<**PARENT**>");
		   NewEntry->PreviousEntry = NULL;
		   NewEntry->NextEntry = NULL;
		   AddDirEntry (NewEntry);
	       }
	   }
	   else
	   {
	       if (c_file.attrib & 16)
	       {
		   strcpy (NewEntry->DirEntry, "<");
		   strcat (NewEntry->DirEntry, c_file.name);
		   strcat (NewEntry->DirEntry, ">");
	       }
	       else
		   strcpy (NewEntry->DirEntry, c_file.name);
	       NewEntry->NextEntry = NULL;
	       NewEntry->PreviousEntry = NULL;
	       AddDirEntry (NewEntry);
	   }
       }
    }
    return (0);
}

dir_p FindEntryN (unsigned int n)

{
    dir_p        ThisPtr,
		 LastPtr;
    unsigned int counter;

    ThisPtr = DirList;
    LastPtr = NULL;
    counter = 0;
    while ((ThisPtr != NULL) && (counter < n))
    {
	LastPtr = ThisPtr;
	ThisPtr = ThisPtr->NextEntry;
	counter++;
    }
    if (counter == n)
	 return (ThisPtr);
    else
	 return (LastPtr);
}

 





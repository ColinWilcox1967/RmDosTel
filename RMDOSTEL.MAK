phonelst.obj: phonelst.c
                cl /c /AL   phonelst.c 

rmdostel.obj:  rmdostel.c
                cl /c /AL   rmdostel.c

newbook.obj:    newbook.c
                cl /c /AL newbook.c

view.obj:       view.c
                cl /c /AL view.c

doscmds.obj:    doscmds.c
                cl /c /AL doscmds.c

environ.obj:    environ.c
                cl /c /AL environ.c

external.obj:   external.c
                 cl /c /AL external.c

merge.obj:    merge.c
                cl /c /AL merge.c

copy.obj:    copy.c
                cl /c /AL copy.c

trans.obj:    trans.c
               cl /c /AL trans.c

sample.obj:  sample.c
                cl /c /AL sample.c

asciitab.obj:    asciitab.c
                cl /c /AL asciitab.c

edit.obj:     edit.c
                cl /c /AL edit.c

dofind.obj:   dofind.c
                cl /c /AL dofind.c

dogoto.obj:   dogoto.c
                cl /c /AL dogoto.c

undelete.obj: undelete.c
                cl /c /AL undelete.c

dosort.obj:   dosort.c
                cl /c /AL dosort.c

palette.obj:  palette.c
                cl /c /AL palette.c

find.obj:     find.c
              cl /c /AL find.c

sort.obj:     sort.c
              cl /c /AL sort.c

security.obj: security.c
                cl /c /AL security.c

screen.obj:   screen.c
                cl /c /AL screen.c

convert.obj:  convert.c
                cl /c /AL convert.c

datafile.obj:   datafile.c
                cl /c /AL datafile.c

search.obj:   search.c
                cl /c /AL search.c

purge.obj:    purge.c
                 cl /c /AL purge.c

support.obj:  support.c
                cl /c /AL support.c

fviewer.obj:   fviewer.c
                cl /c /AL fviewer.c

fsearch.obj:   fsearch.c
                cl /c /AL fsearch.c

windows.obj:  windows.c
                cl /c /AL  windows.c

printer.obj:   printer.c  
                cl /c /AL printer.c

getkey.obj: getkey.c
                cl /c /AL   getkey.c

getstr.obj: getstr.c
               cl /c /AL   getstr.c 
               
config.obj: config.c
                cl /c /AL   config.c

selfdir.obj:   selfdir.c
                cl /c /AL selfdir.c

shwdir.obj:  shwdir.c
                cl /c /AL shwdir.c

dskstat.obj:  dskstat.c
                cl /c /AL dskstat.c

flpy.obj:     flpy.asm
                masm flpy.asm;

drvlist.obj:   drvlist.c
                cl /c /AL drvlist.c

dirlst.obj:    dirlst.c
                cl /c /AL dirlst.c

drexists.obj:  drexists.c
                cl /c /AL drexists.c

break.obj:     break.c
                 cl /c /AL break.c

rmdostel.exe: rmdostel.obj purge.obj asciitab.obj getstr.obj printer.obj trans.obj support.obj phonelst.obj \
             windows.obj sample.obj getkey.obj config.obj flpy.obj drvlist.obj dirlst.obj \
             search.obj  palette.obj sort.obj security.obj screen.obj convert.obj datafile.obj \
             dskstat.obj break.obj dosort.obj edit.obj copy.obj shwdir.obj selfdir.obj drexists.obj fviewer.obj fsearch.obj \
             view.obj merge.obj undelete.obj external.obj doscmds.obj environ.obj newbook.obj
             link @rmdostel.lnk


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

OUTDIR=.
INTDIR=.\obj

TARGET=cs_demo

ALL : "$(OUTDIR)\$(TARGET).exe"


CLEAN :
	-@erase "$(INTDIR)\cs_demo.obj"  \
	-@erase "$(INTDIR)\udp.obj"     \


"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "_WIN32" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "BMONI_SERVER" /Fp"$(INTDIR)\$(TARGET).pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c /TP
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /machine:I386 /out:"$(OUTDIR)\$(TARGET).exe" 
LINK32_OBJS= \
	"$(INTDIR)\cs_demo.obj" \
	"$(INTDIR)\udp.obj" \

"$(OUTDIR)\$(TARGET).exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.rc{$(INTDIR)}.res::
   $(RSC) @<<
   $(RSC_PROJ) $< 
<<

#!MESSAGE Begin Compile with RSC
#!MESSAGE End Compile with RSC

SOURCE=.\cs_demo.c
"$(INTDIR)\cs_demo.obj" : $(SOURCE) "$(INTDIR)"

SOURCE=.\udp.c
"$(INTDIR)\udp.obj" : $(SOURCE) "$(INTDIR)"

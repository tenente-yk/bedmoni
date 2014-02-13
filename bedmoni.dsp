# Microsoft Developer Studio Project File - Name="bedmoni" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=bedmoni - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bedmoni.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bedmoni.mak" CFG="bedmoni - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bedmoni - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "bedmoni - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bedmoni - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "bedmoni - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /w /W0 /Gm /GX /ZI /Od /I ".\modules" /I ".\tasks" /I ".\." /I ".\menus" /I ".\grph" /I ".\language" /I ".\modules\grph" /I ".\res" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /TP /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/bedmoni1.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bedmoni - Win32 Release"
# Name "bedmoni - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "tasks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tasks\crbpio.c
# End Source File
# Begin Source File

SOURCE=.\tasks\dio.c
# End Source File
# Begin Source File

SOURCE=.\tasks\dproc.c
# End Source File
# Begin Source File

SOURCE=.\tasks\dview.c
# End Source File
# Begin Source File

SOURCE=.\tasks\gview.c
# End Source File
# Begin Source File

SOURCE=.\tasks\sched.c
# End Source File
# End Group
# Begin Group "modules"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\modules\cframe.c
# End Source File
# Begin Source File

SOURCE=.\modules\ecgm.c
# End Source File
# Begin Source File

SOURCE=.\modules\fifo.c
# End Source File
# Begin Source File

SOURCE=.\modules\filter.c
# End Source File
# Begin Source File

SOURCE=.\modules\iframe.c
# End Source File
# Begin Source File

SOURCE=.\modules\inpfoc.c
# End Source File
# Begin Source File

SOURCE=.\modules\inpfoc_wnd.c
# End Source File
# Begin Source File

SOURCE=.\modules\mainmenu.c
# End Source File
# Begin Source File

SOURCE=.\modules\mframe.c
# End Source File
# Begin Source File

SOURCE=.\modules\nibp.c
# End Source File
# Begin Source File

SOURCE=.\modules\port.c
# End Source File
# Begin Source File

SOURCE=.\modules\sframe.c
# End Source File
# Begin Source File

SOURCE=.\modules\shm.c
# End Source File
# Begin Source File

SOURCE=.\modules\t1t2.c
# End Source File
# Begin Source File

SOURCE=.\modules\unit.c
# End Source File
# Begin Source File

SOURCE=.\modules\utils.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\bedmoni.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bedmoni.h
# End Source File
# Begin Source File

SOURCE=.\modules\bpm.h
# End Source File
# Begin Source File

SOURCE=.\modules\cframe.h
# End Source File
# Begin Source File

SOURCE=.\tasks\crbpio.h
# End Source File
# Begin Source File

SOURCE=.\modules\db.h
# End Source File
# Begin Source File

SOURCE=.\modules\defs.h
# End Source File
# Begin Source File

SOURCE=.\tasks\dio.h
# End Source File
# Begin Source File

SOURCE=.\tasks\dproc.h
# End Source File
# Begin Source File

SOURCE=.\tasks\dview.h
# End Source File
# Begin Source File

SOURCE=.\modules\ecgm.h
# End Source File
# Begin Source File

SOURCE=.\modules\fifo.h
# End Source File
# Begin Source File

SOURCE=.\modules\filter.h
# End Source File
# Begin Source File

SOURCE=.\modules\iframe.h
# End Source File
# Begin Source File

SOURCE=.\modules\inpfoc.h
# End Source File
# Begin Source File

SOURCE=.\modules\mframe.h
# End Source File
# Begin Source File

SOURCE=.\modules\port.h
# End Source File
# Begin Source File

SOURCE=.\tasks\sched.h
# End Source File
# Begin Source File

SOURCE=.\modules\sframe.h
# End Source File
# Begin Source File

SOURCE=.\modules\shm.h
# End Source File
# Begin Source File

SOURCE=.\modules\spo2.h
# End Source File
# Begin Source File

SOURCE=.\modules\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

ifeq ($(MAKECMDGOALS),)
    $(error No target selected. Please type "make palm", "make linux_x86_x11", ...)
    exit
endif

## ------------------------------------------ ##
## Target options:                            ##
## ------------------------------------------ ##

CFLAGS2 =
LDFLAGS2 =
DEFAULT_LIBS =

ifeq ($(MAKECMDGOALS),palm)
    PSDK = /opt/palmdev/sdk-5r3/include/
    CFLAGS2 = -DPALMOS -Wno-multichar
    MCC = m68k-palmos-gcc -O2
    CC = arm-elf-gcc
    CXX = arm-elf-gcc
endif
ifeq ($(MAKECMDGOALS),linux)
    DEFAULT_LIBS += -lm -lpthread -lstdc++ -lSDL2
    CFLAGS2 = -DOS_LINUX -pipe
    CC = gcc
    CXX = gcc
    LD = gcc
endif
ifeq ($(MAKECMDGOALS),win32)
    DEFAULT_LIBS += -ldsound -ldxguid -lwinmm -lole32
    LDFLAGS2 = -static-libgcc -static-libstdc++
    CFLAGS2 = \
	-DOS_WIN \
	-ffunction-sections -fdata-sections \
	-malign-double \
	-fno-exceptions \
	-pipe \
	-Iresources/
    CC = i686-w64-mingw32-gcc
    CXX = $(CC)
    LD = i686-w64-mingw32-g++
endif
ifeq ($(MAKECMDGOALS),win64)
    DEFAULT_LIBS += -ldsound -ldxguid -lwinmm -lole32
    LDFLAGS2 = -static-libgcc -static-libstdc++
    CFLAGS2 = \
	-DOS_WIN \
	-ffunction-sections -fdata-sections \
	-fno-exceptions \
	-pipe \
	-Iresources/
    CC = x86_64-w64-mingw32-gcc
    CXX = $(CC)
    LD = x86_64-w64-mingw32-g++
endif

FINAL_CFLAGS = $(CFLAGS) $(CFLAGS2) -I../
FINAL_LDFLAGS = $(LDFLAGS) $(LDFLAGS2)
FINAL_LIBS = $(LIBS) $(DEFAULT_LIBS)

## ------------------------------------------ ##
## Directories:                               ##
## ------------------------------------------ ##

CORE = ../core/
JPG = ../tiny_jpg/
FILE = ../file/
LOG = ../log/
MEM = ../memory/
PSYNTH = ../psynth/
PSYTEXX = ../psytexx/
SOUND = ../sound/
TIME = ../time/
WM = ../wm/
XM = ../xm/
PALMCODE = palmos/

## ------------------------------------------ ##
## Object files and includes:                 ##
## ------------------------------------------ ##

DEPS = \
    $(CORE)*.h \
    $(JPG)*.h \
    $(FILE)*.h \
    $(MEM)*.h \
    $(PSYNTH)*.h \
    $(PSYTEXX)*.h \
    $(SOUND)*.h \
    $(TIME)*.h \
    $(WM)*.h \
    $(XM)*.h
XM_OBJ = \
    xm_play.o \
    xm_song.o \
    xm_pattern.o \
    xm_instrum.o \
    xm_sample.o \
    xm_tables.o \
    xm_channels.o \
    xm_main.o
PSYNTH_OBJ = \
    psynths.o \
    psynth_net.o \
    win_psynet.o
SOUND_OBJ = sound_player.o sound_sndout.o
WM_OBJ = wm.o wm_eventloop.o
PSYTEXX_OBJ = \
    win_main.o \
    win_files.o \
    win_list.o \
    win_button.o \
    win_scrollbar.o \
    win_popup.o \
    win_text.o \
    win_dialog.o \
    win_psymenu.o \
    win_psytable.o \
    win_psysamples.o \
    win_psypattern.o \
    win_psypatterntab.o \
    win_psyconfig.o \
    win_psyplaylist.o \
    win_psychannels.o \
    win_psyinsteditor.o \
    win_psysmpeditor.o \
    win_psysmpview.o
OBJECTS = \
    main.o \
    memory.o \
    log.o \
    file.o \
    time.o \
    jpg.o \
    $(PSYTEXX_OBJ) \
    $(XM_OBJ) \
    $(SOUND_OBJ) \
    $(WM_OBJ)

ifeq ($(MAKECMDGOALS),palm)
    ARM_PALM_INCS = -Ipalmos/ -I$(PSDK) -I$(PSDK)libraries -I$(PSDK)Extensions/ExpansionMgr -I$(PSDK)Dynamic -I$(PSDK)Core -I$(PSDK)Core/Hardware -I$(PSDK)Core/System -I$(PSDK)Core/System/Unix -I$(PSDK)Core/ui -I$(PSDK)Core/UI -I$(PSDK)Libraries
    ARM_PALM_FLAGS = -DNATIVE -mcpu=arm9tdmi -nostartfiles -ffixed-r9 -mpic-register=r10 -msingle-pic-base -fPIC -fpermissive
    FINAL_CFLAGS += $(ARM_PALM_INCS) $(ARM_PALM_FLAGS) $(ARM_PALM_DEFINES)
    FINAL_LDFLAGS += $(ARM_PALM_DEFINES) $(ARM_PALM_FLAGS)
    ARM_PALM_FILES = $(PALMCODE)palm_functions.cpp $(PALMCODE)PalmTypes.h $(PALMCODE)DebugMgr.h $(PALMCODE)palm_functions.h
    ARM_PALM_OBJECTS = palm_functions.o
    PALM_OBJECTS = $(OBJECTS) $(ARM_PALM_OBJECTS)
else
    OBJECTS += $(PSYNTH_OBJ)
endif

## ------------------------------------------ ##
## Rules for different targets:               ##
## ------------------------------------------ ##

linux: $(OBJECTS)
	$(LD) $(FINAL_LDFLAGS) -o psytexx $(OBJECTS) $(FINAL_LIBS)

win32: $(OBJECTS)
	$(LD) $(FINAL_LDFLAGS) -o psytexx $(OBJECTS) $(FINAL_LIBS)

win64: $(OBJECTS)
	$(LD) $(FINAL_LDFLAGS) -o psytexx $(OBJECTS) $(FINAL_LIBS)

palm: $(PALM_OBJECTS)
#Link main code ( with special ARM script - armscript.ld ) :
	$(CC) $(FINAL_LDFLAGS) -o main_palm.o $(PALM_OBJECTS) -Xlinker --script=palmos/armscript.ld
#Create BIN files from the program_palm.o :
	arm-elf-objcopy -j .text -O binary main_palm.o armcode
	split -b 64000 armcode armc
	if test -e armcaa; then mv armcaa armc0000.bin; fi
	if test -e armcab; then mv armcab armc0001.bin; fi
	if test -e armcac; then mv armcac armc0002.bin; fi
	if test -e armcad; then mv armcad armc0003.bin; fi
	if test -e armcae; then mv armcae armc0004.bin; fi
	if test -e armcaf; then mv armcaf armc0005.bin; fi
	if test -e armcag; then mv armcag armc0006.bin; fi
	if test -e armcah; then mv armcah armc0007.bin; fi
	if test -e armcai; then mv armcai armc0008.bin; fi
	if test -e armcaj; then mv armcaj armc0009.bin; fi
	if test -e armcak; then mv armcak armc000A.bin; fi
	if test -e armcal; then mv armcal armc000B.bin; fi
	if test -e armcal; then mv armcal armc000C.bin; fi
	if test -e armcal; then mv armcal armc000D.bin; fi
	if test -e armcal; then mv armcal armc000E.bin; fi
	arm-elf-objcopy -j .got -O binary main_palm.o armg0000.bin
	arm-elf-objcopy -j .alldata -O binary main_palm.o armdata
	split -b 64000 armdata armd
	if test -e armdaa; then mv armdaa armd0000.bin; fi
	if test -e armdab; then mv armdab armd0001.bin; fi
	if test -e armdac; then mv armdac armd0002.bin; fi
	if test -e armdad; then mv armdad armd0003.bin; fi
	if test -e armdae; then mv armdae armd0004.bin; fi
	if test -e armdaf; then mv armdaf armd0005.bin; fi
	if test -e armdag; then mv armdag armd0006.bin; fi
	if test -e armdah; then mv armdah armd0007.bin; fi
	if test -e armdai; then mv armdai armd0008.bin; fi
	if test -e armdaj; then mv armdaj armd0009.bin; fi
	if test -e armdak; then mv armdak armd000A.bin; fi
	if test -e armdal; then mv armdal armd000B.bin; fi
	if test -e armdal; then mv armdal armd000C.bin; fi
	if test -e armdal; then mv armdal armd000D.bin; fi
	if test -e armdal; then mv armdal armd000E.bin; fi
	arm-elf-objcopy -j .rel -O binary main_palm.o armr0000.bin
#Create Palm resources ( *.BIN files ) :
	pilrc resources/palmos_resources.rcp
#Build PRC file :
	$(MCC) palmos/arm_starter.cpp -o arm_starter.o
	m68k-palmos-obj-res arm_starter.o
	build-prc -v 1.0 resources/palmos_info.def -o psytexx.prc *.grc *.bin

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

main.o: $(PSYTEXX)main.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)main.cpp

win_main.o: $(PSYTEXX)win_main.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_main.cpp

win_files.o: $(PSYTEXX)win_files.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_files.cpp

win_list.o: $(PSYTEXX)win_list.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_list.cpp

win_button.o: $(PSYTEXX)win_button.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_button.cpp

win_popup.o: $(PSYTEXX)win_popup.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_popup.cpp

win_scrollbar.o: $(PSYTEXX)win_scrollbar.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_scrollbar.cpp

win_psymenu.o: $(PSYTEXX)win_psymenu.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psymenu.cpp

win_psytable.o: $(PSYTEXX)win_psytable.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psytable.cpp

win_text.o: $(PSYTEXX)win_text.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_text.cpp

win_psysamples.o: $(PSYTEXX)win_psysamples.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psysamples.cpp

win_psypattern.o: $(PSYTEXX)win_psypattern.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psypattern.cpp

win_psypatterntab.o: $(PSYTEXX)win_psypatterntab.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psypatterntab.cpp

win_dialog.o: $(PSYTEXX)win_dialog.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_dialog.cpp

win_psyconfig.o: $(PSYTEXX)win_psyconfig.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psyconfig.cpp

win_psyplaylist.o: $(PSYTEXX)win_psyplaylist.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psyplaylist.cpp

win_psychannels.o: $(PSYTEXX)win_psychannels.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psychannels.cpp

win_psyinsteditor.o: $(PSYTEXX)win_psyinsteditor.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psyinsteditor.cpp

win_psysmpeditor.o: $(PSYTEXX)win_psysmpeditor.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psysmpeditor.cpp

win_psysmpview.o: $(PSYTEXX)win_psysmpview.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psysmpview.cpp

win_psynet.o: $(PSYTEXX)win_psynet.cpp $(PSYTEXX)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYTEXX)win_psynet.cpp

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

xm_play.o: $(XM)xm_play.cpp $(XM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(XM)xm_play.cpp

xm_song.o: $(XM)xm_song.cpp $(XM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(XM)xm_song.cpp

xm_pattern.o: $(XM)xm_pattern.cpp $(XM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(XM)xm_pattern.cpp

xm_instrum.o: $(XM)xm_instrum.cpp $(XM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(XM)xm_instrum.cpp

xm_sample.o: $(XM)xm_sample.cpp $(XM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(XM)xm_sample.cpp

xm_tables.o: $(XM)xm_tables.cpp $(XM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(XM)xm_tables.cpp

xm_channels.o: $(XM)xm_channels.cpp $(XM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(XM)xm_channels.cpp

xm_main.o: $(XM)xm_main.cpp $(XM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(XM)xm_main.cpp

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

psynth_net.o: $(PSYNTH)psynth_net.cpp $(PSYNTH)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYNTH)psynth_net.cpp

psynths.o: $(PSYNTH)psynths.cpp $(PSYNTH)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(PSYNTH)psynths.cpp

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

sound_player.o: $(SOUND)sound_player.cpp $(SOUND)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(SOUND)sound_player.cpp

sound_sndout.o: $(SOUND)sound_sndout.cpp $(SOUND)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(SOUND)sound_sndout.cpp

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

time.o: $(TIME)time.cpp $(TIME)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(TIME)time.cpp

file.o: $(FILE)file.cpp $(FILE)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(FILE)file.cpp

log.o: $(LOG)log.cpp
	$(CXX) $(FINAL_CFLAGS) -c $(LOG)log.cpp

memory.o: $(MEM)memory.cpp $(MEM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(MEM)memory.cpp

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

jpg.o: $(JPG)jpg.cpp $(JPG)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(JPG)jpg.cpp

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

wm.o: $(WM)wm.cpp $(WM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(WM)wm.cpp

wm_eventloop.o: $(WM)wm_eventloop.cpp $(WM)*.h $(DEPS)
	$(CXX) $(FINAL_CFLAGS) -c $(WM)wm_eventloop.cpp

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

palm_functions.o: $(ARM_PALM_FILES) 
	$(CXX) $(FINAL_CFLAGS) -c $(PALMCODE)palm_functions.cpp

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

clean:
	rm -f *.o *.grc *.bin mem_* armcode armdata log.txt

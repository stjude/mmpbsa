bin_PROGRAMS = mmpbsa mmpbsa_graphics mmpbsa_analyzer mdout_to mtraj
mmpbsa_CPPFLAGS = $(XML_CPPFLAGS) -I$(MEAD_PATH)/include/ -I./libmmpbsa $(BOINC_CPPFLAGS) $(GRAPHICS_CPPFLAGS)
mmpbsa_LDFLAGS = $(CUSTOM_LDFLAGS) -L$(MEAD_PATH)/lib  -L./libmmpbsa -L./molsurf $(BOINC_LDFLAGS) $(GRAPHICS_LDFLAGS) 
mmpbsa_LDADD = -lmmpbsa -lmolsurf -lmead $(BOINC_LIBS) $(GRAPHICS_FLAGS) $(XML_LIBS) $(CUSTOM_LIBS)
mmpbsa_DEPENDENCIES = libmmpbsa molsurf
mmpbsa_SOURCES = mmpbsa.cpp

mmpbsa_graphics_CPPFLAGS = -Wall $(BOINC_CPPFLAGS) $(GRAPHICS_CPPFLAGS)
mmpbsa_graphics_LDFLAGS = $(BOINC_LDFLAGS) $(GRAPHICS_LDFLAGS)
mmpbsa_graphics_LDADD = $(BOINC_LIBS) $(GRAPHICS_LIBS) $(CUSTOM_LIBS)
mmpbsa_graphics_SOURCES = mmpbsa_graphics.cpp mmpbsa_gutil.cpp

mmpbsa_analyzer_CPPFLAGS = -Wall -I. $(BOINC_CPPFLAGS)
mmpbsa_analyzer_LDADD = -lmmpbsa $(CUSTOM_LIBS) $(BOINC_LIBS)
mmpbsa_analyzer_LDFLAGS = -L./libmmpbsa $(CUSTOM_LDFLAGS) $(BOINC_LDFLAGS)
mmpbsa_analyzer_DEPENDENCIES = libmmpbsa
mmpbsa_analyzer_SOURCES = mmpbsa_analyzer.cpp

if BUILD_WITH_GZIP
mmpbsa_analyzer_LDADD += -lz
endif

mdout_to_CPPFLAGS = $(mmpbsa_CPPFLAGS)
mdout_to_LDFLAGS = $(mmpbsa_LDFLAGS)
mdout_to_LDADD = $(mmpbsa_LDADD)
mdout_to_DEPENDENCIES = libmmpbsa
mdout_to_SOURCES = mdout_to.cpp

mtraj_SOURCES = mtraj.cpp
mtraj_CPPFLAGS = $(mmpbsa_CPPFLAGS)
mtraj_LDFLAGS = $(mmpbsa_LDFLAGS)
mtraj_LDADD = $(mmpbsa_LDADD)
mtraj_DEPENDENCIES = libmmpbsa

if BUILD_WITH_MPI
mmpbsa_CPPFLAGS += -I $(MPI_PATH)/include/
endif


if BUILD_WITH_GZIP
mmpbsa_LDADD += -lz
endif

if BUILD_WITH_GROMACS
mmpbsa_CPPFLAGS += -I $(GROMACS_PATH)/include/ 
mmpbsa_LDFLAGS += -L$(GROMACS_PATH)/lib
mmpbsa_LDADD += -lxml2 -lgmx_d -lgmxpreprocess_d -lpthread -ldl

mmpbsa_analyzer_CPPFLAGS += -I $(GROMACS_PATH)/include/ 
mmpbsa_analyzer_LDFLAGS += -L$(GROMACS_PATH)/lib
mmpbsa_analyzer_LDADD += -lxml2 -lgmx_d -lgmxpreprocess_d -lpthread -ldl
endif

if BUILD_WITH_MPI
mmpbsa_SOURCES += mmpbsa_mpi.cpp
endif

#cc  -DHAVE_CONFIG_H -I. -I/home/dcoss/libraries/gromacs-4.5.3/include -I/home/dcoss/opt/gromacs/include -I/usr/include/libxml2 -I /home/dcoss/opt/fftw/include -pthread -c -o gmxdump.o gmxdump.c
#cc -I /home/dcoss/opt/fftw/include -pthread -I./include -static -o gmxdump gmxdump.o  -L/home/dcoss/opt/fftw/lib  /home/dcoss/opt/fftw/lib/libfftw3.a -lxml2 -L /home/dcoss/opt/gromacs/lib -lgmx_d -lgmxpreprocess_d -lmd_d -ldl -lnsl -lm
#endif

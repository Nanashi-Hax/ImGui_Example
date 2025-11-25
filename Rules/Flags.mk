#-------------------------------------------------------------------------------
# options for code generation
#-------------------------------------------------------------------------------
CFLAGS	:=	-Wall -O3 -ffunction-sections \
			$(MACHDEP)

CFLAGS	+=	$(INCLUDE) -D__WIIU__ -D__WUT__ -D__WUPS__

CXXFLAGS	:= $(CFLAGS) -std=c++23

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-g $(ARCH) $(RPXSPECS) -Wl,-Map,$(notdir $*.map) -T$(WUMS_ROOT)/share/libmappedmemory.ld -T$(WUMS_ROOT)/share/libkernel.ld $(WUPSSPECS)

ifeq ($(DEBUG),1)
CXXFLAGS += -DDEBUG -g
CFLAGS += -DDEBUG -g
endif

ifeq ($(DEBUG),VERBOSE)
CXXFLAGS += -DDEBUG -DVERBOSE_DEBUG -g
CFLAGS += -DDEBUG -DVERBOSE_DEBUG -g
endif

LIBS	:= -lwups -lwut -lnotifications -lmappedmemory -lkernel

#-------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level
# containing include and lib
#-------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(WUPS_ROOT) $(WUT_ROOT) $(WUT_ROOT)/usr $(WUMS_ROOT)

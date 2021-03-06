#
# Main Makefile, intended for use on Linux/X11 and compatible platforms
# using GNU Make.
#
# It should guess the paths to the game dependencies on its own, except for
# Boost which is assumed to be installed to the default locations. If you have
# installed Boost to a non-standard location, you will need to override CXXFLAGS
# and LDFLAGS with any applicable -I and -L arguments.
#
# The main options are:
#
#   CCACHE           The ccache binary that should be used when USE_CCACHE is
#                     enabled (see below). Defaults to 'ccache'.
#   CXX              C++ compiler comand line.
#   CXXFLAGS         Additional C++ compiler options.
#   OPTIMIZE         If set to 'yes' (default), builds with compiler
#                     optimizations enabled (-O2). You may alternatively use
#                     CXXFLAGS to set your own optimization options.
#   LDFLAGS          Additional linker options.
#   USE_CCACHE       If set to 'yes' (default), builds using the CCACHE binary
#                     to run the compiler. If ccache is not installed (i.e.
#                     found in PATH), this option has no effect.
#

OPTIMIZE=yes
CCACHE?=ccache
USE_CCACHE?=$(shell which $(CCACHE) 2>&1 > /dev/null && echo yes)
ifneq ($(USE_CCACHE),yes)
CCACHE=
endif

ifeq ($(OPTIMIZE),yes)
BASE_CXXFLAGS += -O2
endif

ifeq ($(CXX), g++)
GCC_GTEQ_490 := $(shell expr `$(CXX) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40900)
ifeq "$(GCC_GTEQ_490)" "1"
BASE_CXXFLAGS += -fdiagnostics-color=auto -fsanitize=undefined
endif
endif

SDL2_CONFIG?=sdl2-config
USE_SDL2?=$(shell which $(SDL2_CONFIG) 2>&1 > /dev/null && echo yes)

ifneq ($(USE_SDL2),yes)
$(error SDL2 not found, SDL-1.2 is not supported)
endif

PROTOC ?= protoc
PROTOC_FLAGS = --cpp_out=src --proto_path=src

# Initial compiler options, used before CXXFLAGS and CPPFLAGS.
# -Wno-reorder -Wno-unused-variable added to make noiseutils.cpp build
BASE_CXXFLAGS += -std=c++11 -g -rdynamic -fno-inline-functions \
	-fthreadsafe-statics -Werror -Wall -Wno-reorder -Wno-unused-variable

# Compiler include options, used after CXXFLAGS and CPPFLAGS.
INC := -Isrc -Iinclude -Isrc/lua -ILuaBridge/Source/LuaBridge $(shell pkg-config --cflags x11 sdl2 SDL2_image SDL2_ttf libpng zlib libenet protobuf)

ifdef STEAM_RUNTIME_ROOT
	INC += -I$(STEAM_RUNTIME_ROOT)/include
endif

# Linker library options.
LIBS := $(shell pkg-config --libs x11 gl ) \
	$(shell pkg-config --libs sdl2 SDL2_image libpng zlib protobuf libenet) \
	-lSDL2_ttf -lSDL2_mixer -lboost_system -lboost_regex -lboost_filesystem -lboost_chrono -lboost_thread -lnoise

PBSRCS := $(wildcard src/*.proto)
PBOBJS := $(PBSRCS:.proto=.pb.o)
PBGENS := $(PBSRCS:.proto=.pb.cc)

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)

include Makefile.common

src/%.pb.cc: src/%.proto
	$(PROTOC) $(PROTOC_FLAGS) $<

src/%.pb.o : src/%.pb.cc
	@echo "Building:" $<
	@$(CCACHE) $(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -c -o $@ $<
	@$(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -MM $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|src/$*.o:|' < $*.d.tmp > src/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> src/$*.d
	@rm -f $*.d.tmp

src/%.o : src/%.cpp 
	@echo "Building:" $<
	@$(CCACHE) $(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -c -o $@ $<
	@$(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -MM $< > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|src/$*.o:|' < $*.d.tmp > src/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> src/$*.d
	@rm -f $*.d.tmp

src/lua/%.o : src/lua/%.c
	@echo "Building:" $<
	@$(CCACHE) $(CXX) $(BASE_CXXFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) -c -o $@ $<

HexWarfare: $(PBOBJS) $(OBJS) $(ogl_objects) $(sdl_objects) liblua.a
	@echo "Linking : HexWarfare"
	@$(CCACHE) $(CXX) \
		$(BASE_CXXFLAGS) $(LDFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(INC) \
		$(OBJS) $(PBOBJS) -o HexWarfare \
		$(LIBS) -fthreadsafe-statics

liblua.a: $(lua_objects)
	@echo "Creating local copy of lua library" 
	@$(AR) -rcs $@ $(lua_objects)

# pull in dependency info for *existing* .o files
-include $(OBJS:.o=.d)

all: HexWarfare

clean:
	rm -f src/*.o src/*.d *.o *.d HexWarfare $(PBOBJS) $(PBGENS)

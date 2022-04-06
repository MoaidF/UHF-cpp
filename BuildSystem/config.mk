#OS type (Linux windows32 ...)
OS_TYPE = $(shell uname -s)
#$(warning $(OS_TYPE))

#configure area you can set the following config to you own system
#arch (armv7-a armv7-r ...)
ARCH = i686

ifeq ($(GCCVERSION), aarch64_RedFlag)
#machine (thumb or arm)
MACHINE = i686

#compiler select
CC  	= aarch64-grg-linux-gnu-gcc
CXX 	= aarch64-grg-linux-gnu-g++
AS  	= aarch64-grg-linux-gnu-as
AR  	= aarch64-grg-linux-gnu-ar
LD  	= aarch64-grg-linux-gnu-g++
STRIP  	= aarch64-grg-linux-gnu-strip
READELF	= aarch64-grg-linux-gnu-readelf

else ifeq ($(PLATFORM), aarch64)
#machine (thumb or arm)
MACHINE = i686

#compiler select
CC  	= aarch64-linux-gnu-gcc
CXX 	= aarch64-linux-gnu-g++
AS  	= aarch64-linux-gnu-as
AR  	= aarch64-linux-gnu-ar
LD  	= aarch64-linux-gnu-g++
STRIP  	= aarch64-linux-gnu-strip
READELF	= aarch64-linux-gnu-readelf

else ifeq ($(PLATFORM), mips64)
#machine (thumb or arm)
MACHINE = mips

#compiler select
CC  	= mips-linux-gnu-gcc -mabi=64
CXX 	= mips-linux-gnu-g++ -mabi=64
AS  	= mips-linux-gnu-as
AR  	= mips-linux-gnu-ar
LD  	= mips-linux-gnu-g++ -mabi=64
STRIP  	= mips-linux-gnu-strip
READELF	= mips-linux-gnu-readelf


else
#machine (thumb or arm)
MACHINE = x86

#compiler select
CC  	= gcc
CXX 	= g++
AS  	= as
AR  	= ar
LD  	= g++
STRIP  	= strip
READELF	= readelf
endif

INCDIR 	= 

#debug options (debug or release)
DEBUG_LEVEL = release

#target type (static or shared)
TARGET_TYPE = shared

#application install path(/data/local)
APP_INSTALL_PATH = /data/sp

#compiler preprocess
DSYMBOL     = -D__linux__ -D__USE_GNU

CXX_EXCEPT  = 
CXXFLAGS    = -std=c++0x -g -U_FORTIFY_SOURCE 
DEPENDFLAG  = 
COMMONFLAGS = 
ASFLAGS     =
CFLAGS      = -U_FORTIFY_SOURCE 
ARFLAGS     = 
LDFLAGS     =-Wl,-rpath=/data/sp,-Bsymbolic,--no-as-needed -g
CXXFLAGS += -DLINUX_GRGSP

SRCS 		= ALL_SRCS

ifeq ($(DEBUG_LEVEL), debug)
DLL_DIR = Debug
BIN_DIR = 
DEPEND_DLL_PATH = 
TARGET_SUFFIX = D
DSYMBOL += -D_DEBUG
else
DLL_DIR = Release
BIN_DIR = 
DEPEND_DLL_PATH = 
TARGET_SUFFIX =
DSYMBOL += -DNDEBUG
endif

DEPEND_STD_DLL = \
-lpthread \
-ldl \
-lrt \
-lstdc++ \
-lc \
-lm \
-lgcc



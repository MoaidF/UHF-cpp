
#do not change the following code
#all source
LOCAL_PATH 		 = ./
MY_FILES_PATH   := $(LOCAL_PATH)

MY_FILES_SUFFIX := %.cpp %.c %.cc

rwildcard 	  = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

MY_ALL_FILES := $(foreach src_path,$(MY_FILES_PATH), $(call rwildcard,$(src_path),*.*) ) 
MY_ALL_FILES := $(MY_ALL_FILES:$(MY_CPP_PATH)./%=$(MY_CPP_PATH)%)
MY_SRC_LIST  := $(filter $(MY_FILES_SUFFIX),$(MY_ALL_FILES)) 
MY_SRC_LIST  := $(MY_SRC_LIST:$(LOCAL_PATH)%=%)

define uniq   
$(eval seen :=)\
$(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))\
${seen}
endef

MY_ALL_DIRS := $(dir $(foreach src_path,$(MY_FILES_PATH), $(call rwildcard,$(src_path),*/) ) )
MY_ALL_DIRS := $(call uniq,$(MY_ALL_DIRS))

ifeq ($(SRCS), ALL_SRCS)
SRCS   := $(MY_SRC_LIST)

#include path
INCDIR := $(MY_ALL_DIRS) $(INCDIR)
endif

SRCS   += $(APPEND_SRCS)

#build path
OUTPATH = ./$(DLL_DIR)
OBJPATH = $(OUTPATH)/obj
DEPPATH = $(OUTPATH)/dep

#objects
OBJS    = $(addprefix $(OBJPATH)/, $(addsuffix .o, $(basename $(SRCS))))

#depends
DEPS    = $(addprefix $(DEPPATH)/, $(addsuffix .d, $(basename $(SRCS))))

#compiler optimize
ifeq ($(DEBUG_LEVEL), debug)
OPTIMIZE = -O0 -g3 -gdwarf-2
else
OPTIMIZE = -O0 -g -gdwarf-2
endif

#depends and compiler parameter (cplusplus in kernel MUST NOT use exceptions and rtti)
DEPENDFLAG  += -MM
CXX_EXCEPT  += -w -fexceptions -frtti -fpermissive
CXXFLAGS    += -fpack-struct=1
COMMONFLAGS += -c -fmessage-length=0 $(OPTIMIZE) #-c -fmessage-length=0 -m$(MACHINE) -march=$(ARCH) $(OPTIMIZE)
ASFLAGS     += -x assembler-with-cpp $(DSYMBOL) $(addprefix -I,$(INCDIR)) $(COMMONFLAGS)
CFLAGS      += $(DSYMBOL) $(addprefix -I,$(INCDIR)) $(COMMONFLAGS) -fPIC
CXXFLAGS    +=  $(DSYMBOL) $(addprefix -I,$(INCDIR)) $(CXX_EXCEPT) $(COMMONFLAGS) -fPIC
ARFLAGS     += -r
LDFLAGS     += #-m$(MACHINE) -march=$(ARCH)

#define some useful variable
DEPEND      = $(CC) $(DEPENDFLAG) $(CFLAGS)
DEPEND.d    = $(subst -g ,,$(DEPEND))
COMPILE.c   = $(CC) $(CFLAGS) -c
COMPILE.cxx = $(CXX) $(CXXFLAGS) -c
COMPILE.s   = $(AS) $(ASFLAGS) -c

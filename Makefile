
include ./BuildSystem/config.mk

TARGET_NAME = RL8000RFIDReaderDll

 
INCDIR = "." \
	"./arch"

BIN_DIR = "./Output"
DEPEND_DLL_PATH = "./Output"

include ./BuildSystem/head.mk
include ./BuildSystem/shared_library.mk
include ./BuildSystem/end.mk

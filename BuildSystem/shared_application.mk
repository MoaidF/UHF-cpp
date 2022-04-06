#target
TARGET_FILE  = $(TARGET_NAME)$(TARGET_SUFFIX)
TARGET  	 = $(OUTPATH)/$(TARGET_FILE)

#target 
all: clean_target $(TARGET)
		@echo create "$(TARGET)" ok.
		
.PHONY: clean_target
clean_target:
		-rm -rf $(TARGET)
		
#link object files
$(TARGET): $(OBJS)
		
ifeq ($(OS_TYPE), Linux)
		$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS) $(addprefix -L,$(DEPEND_DLL_PATH)) $(DEPEND_DLL)
else
		@echo '$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS) $(addprefix -L,$(DEPEND_DLL_PATH)) $(DEPEND_DLL)' > ./build.bat
		build.bat
endif
		
		$(READELF) -a $(TARGET) > ./readelf_log.txt
		cp -rf $(TARGET) $(BIN_DIR)/
		$(STRIP) -s $(BIN_DIR)/$(TARGET_FILE)
				
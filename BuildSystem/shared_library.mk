#target
TARGET_FILE  = lib$(TARGET_NAME)$(TARGET_SUFFIX).so
TARGET  	 = $(OUTPATH)/$(TARGET_FILE)

#target 
all: clean_target $(TARGET)
		@echo create "$(TARGET)" ok.
		
.PHONY: clean_target
clean_target:
		-rm -rf $(TARGET)

#link object files
$(TARGET): $(OBJS)
		$(LD) $(LDFLAGS) -shared -fPIC -g -o $(TARGET) $(OBJS) $(addprefix -L,$(DEPEND_DLL_PATH)) $(DEPEND_DLL)
		cp -rf $(TARGET) $(BIN_DIR)
		$(STRIP) -s $(BIN_DIR)/$(TARGET_FILE)
		cp $(TARGET) $(DEPEND_DLL_PATH)
				
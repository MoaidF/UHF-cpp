
#target
TARGET  = $(OUTPATH)/lib$(TARGET_NAME)$(TARGET_SUFFIX).a

#target 
all: $(TARGET)
		@echo create "$(TARGET)" ok.
		
#link object files
$(TARGET): $(OBJS)
		-rm -rf $(TARGET)
		$(AR) $(ARFLAGS) $(TARGET) $(OBJS)
		cp $(TARGET) $(DEPEND_DLL_PATH)
		

DIRS = Server Client
  
all: subdirs  
  
.PHONY: subdirs clean  
  
subdirs: $(DIRS) 
	make -C Client
#	@for dir in $(DIRS); do [ ! -d \$$dir ] || make -C \$$dir || exit 1; done  
			  
clean:  
	@echo $(DIRS)  
	make -C Client clean
#	@for dir in $(DIRS); do [ ! -d \$$dir ] || make -C \$$dir clean || exit 1; done 

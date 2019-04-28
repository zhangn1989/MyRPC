DIRS = additionCode clientCode multiplicationlsCode

all: subdirs  
  
.PHONY: subdirs install uninstall clean  
  
subdirs: $(DIRS) 
	for dir in $(DIRS); do  make -C $$dir; done

install:
	@echo $(DIRS)  
	for dir in $(DIRS); do  make install -C $$dir; done			
  
uninstall:
	@echo $(DIRS)  
	for dir in $(DIRS); do  make uninstall -C $$dir; done			

clean:  
	@echo $(DIRS)  
	for dir in $(DIRS); do  make clean -C $$dir; done

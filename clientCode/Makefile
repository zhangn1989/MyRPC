CC = gcc  
LINK = gcc  
CFLAGS = -g -Wall -rdynamic -I../ 
LIBS =

SRC_DIR = . ..
SFIX = .c

SOURCES := $(foreach x,${SRC_DIR},\
       $(wildcard  \
       $(addprefix  ${x}/*,${SFIX}) ) )
#SOURCES = $(wildcard *.c)  
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))  
TARGET = client

first: all

%.o: %.c  
	$(CC) -c $(CFLAGS) -o $@ $<  
			  
all: $(TARGET)  

$(TARGET): $(OBJECTS)
	@echo $(TARGET)
	$(LINK) $(CFLAGS) $(LIBS) -o $(TARGET) $(OBJECTS)   

.PHONY: install uninstall clean

install:
	cp $(TARGET) ..

uninstall:
	rm -rf ../$(TARGET)

clean:  
	rm -f $(OBJECTS) $(TARGET)  

CC = gcc  
LINK = gcc  
CFLAGS = -g -Wall -rdynamic  
LIBS =

SOURCES = $(wildcard *.c)  
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))  
TARGET = client

first: all

%.o: %.c  
	$(CC) -c $(CFLAGS) -o $@ $<  
			  
all: $(TARGET)  

$(TARGET): $(OBJECTS)
	@echo $(TARGET)
	$(LINK) $(CFLAGS) $(LIBS) -o ../$(TARGET) $(OBJECTS)   

.PHONY: clean

clean:  
	rm -f $(OBJECTS) ../$(TARGET)  

CC= gcc
CC_FLAGS=-c -Wall -Werror -D_REENTRANT -g
CC_LIBS=-lpthread

SOURCES=main.c yourcraft.c database.c listener.c
OBJECTS=$(SOURCES:.c=.o)
OUTPUT=bin/yourcraft

all: $(SOURCES) $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) $(CC_LIBS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) -c $(CC_FLAGS) $< -o $@

clear:
	rm -f $(OUTPUT) $(OBJECTS)

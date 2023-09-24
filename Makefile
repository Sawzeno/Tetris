CC = gcc-13
CFLAGS = -Wall -Wextra

S = tetris.c
T	= tetris
L = -liconv 

.PHONY: all clean

all: $(T)

$(T): $(S)
	$(CC) $(CFLAGS) $(L) $(S) -o $(T) 

clean:
	rm -f $(TARGET)

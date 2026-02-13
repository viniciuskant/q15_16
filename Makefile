CC = gcc
CFLAGS = -Wall -Wextra -Ilib
SRC = src/main.c src/q15_16.c src/q7_8.c
OBJ = $(SRC:.c=.o)
TARGET = fixed_point

all:
	$(CC) $(CFLAGS) src/test_q7_8.c src/q7_8.c -o test_q7_8 -lm
	$(CC) $(CFLAGS) src/test_q15_16.c src/q15_16.c -o test_q15_16 -lm

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean

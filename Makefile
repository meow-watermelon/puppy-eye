CC = gcc
CFLAGS = -g -Wall -Wextra -Wpedantic -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined
INCLUDES = -I.
SRCS = puppy-eye.c os.c memory.c network.c disk.c utils.c ncurses_utils.c
OBJS = $(SRCS:.c=.o)
TARGET = puppy-eye
LDFLAGS = -lncurses

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

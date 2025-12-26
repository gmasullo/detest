CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O2

TARGET = tests
SRCS = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(TARGET)

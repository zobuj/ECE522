TARGET = msmp1

SRC = msmp1.c

CC = gcc
CFLAGS = -O0 -fno-inline -fno-builtin -fno-omit-frame-pointer

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
CC = gcc
CFLAGS = -Wall -Wextra

LIBS = -lz
SRCS = src/omc_decoder.c src/main.c

TARGET = out/omc-decoder

all: $(TARGET)

$(TARGET): $(SRCS)
	mkdir -p $(dir $(TARGET))
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -rf out

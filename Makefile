CC = gcc
CFLAGS = -Wall -Wextra

LIBS = -lz
INCLUDES = include
SRCS = src/OmcTextDecoder.c src/Entrance.c

TARGET = out/omc-decoder

all: $(TARGET)

$(TARGET): $(SRCS)
	mkdir -p $(dir $(TARGET))
	$(CC) $(CFLAGS) -I$(INCLUDES) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -rf out

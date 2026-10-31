CC      ?= gcc
CFLAGS  := -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -g -O2 -pthread
LDFLAGS := -pthread

SRC := src/main.c src/net.c src/http.c src/threadpool.c src/adaptive.c
OBJ := $(SRC:.c=.o)
BIN := wserver

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.c src/*.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) -p 8080 -t 4 -b 16

clean:
	rm -f $(OBJ) $(BIN)

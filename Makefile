CFLAGS   = -Wall -Wextra -O2 `pkg-config --cflags gtk4 webkitgtk-6.0 libadwaita-1`
LIBS     = `pkg-config --libs gtk4 webkitgtk-6.0 libadwaita-1`

SRC = $(shell find src -type f -name "*.c")
OBJ = $(SRC:src/%.c=build/%.o)

all: vord

build:
	mkdir -p build

vord: $(OBJ)
	gcc $(OBJ) -o $@ $(LIBS)

build/%.o: src/%.c | build
	@mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf build vord

run: vord
	./vord

.PHONY: all clean run

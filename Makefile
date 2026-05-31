.PHONY: build
build:
	@echo "Bulding app"
	gcc ./src/main.c -o ./build/vord `pkg-config --cflags --libs gtk4 webkitgtk-6.0`

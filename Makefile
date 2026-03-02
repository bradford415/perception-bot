
.PHONY: build run clean

build:
	colcon build --symlink-install

clean:
	rm -rf build install log
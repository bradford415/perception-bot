
.PHONY: build run clean

python_source = src/  # Location of python files 

build:
	colcon build --symlink-install

clean:
	rm -rf build install log

format:
	black ${python_source}
	isort ${python_source}
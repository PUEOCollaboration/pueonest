

cmake-build: build/Makefile
	cd build && make 

build/Makefile: build
	cd build && cmake ../ 

configure: build
	cd build && ccmake ../ 

build: 
	mkdir -p build

clean: 
	cd build && make clean

dist-clean: 
	rm -rf build

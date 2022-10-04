bin/fnt2bin main.cpp:
	mkdir -p bin

	c++ main.cpp -o bin/fnt2bin

clean:
	rm -rf "bin"

install:
	sudo cp -rf bin/fnt2bin /usr/bin/
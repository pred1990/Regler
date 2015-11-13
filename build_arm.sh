if [ ! -d "bin_arm/" ]; then
	mkdir bin
	echo creating folder: "bin"
fi

if [ -f "start" ]; then
	rm start
fi

arm-linux-gnueabihf-gcc main.c \
	-Wall \
	-o bin/start \
	-std=c99

if [ -f "bin_arm/start" ]; then
	chmod 755 bin/start
else
	echo "File start was not created"
fi

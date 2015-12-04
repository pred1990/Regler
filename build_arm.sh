if [ ! -d "bin_arm/" ]; then
	mkdir bin_arm
	echo creating folder: "bin_arm"
fi

if [ -f "start" ]; then
	rm start
fi

arm-linux-gnueabihf-gcc main.c \
	-Wall \
	-o bin_arm/start \
	-std=gnu99

if [ -f "bin_arm/start" ]; then
	chmod 755 bin/start
	scp bin_arm/start daniel@192.168.0.108:~/start
else
	echo "File start was not created"
fi

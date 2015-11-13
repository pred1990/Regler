if [ ! -d "bin/" ]; then
	mkdir bin
	echo creating folder: "bin"
fi

if [ -f "start" ]; then
	rm start
fi

if [ -f "server" ]; then
	rm server 
fi

gcc main.c \
	-Wall \
	-o bin/start \
	-std=c99

gcc server.c \
	-Wall \
	-o bin/server \
	-std=c99

if [ -f "bin/start" ]; then
	chmod 755 bin/start
else
	echo "File start was not created"
fi

if [ -f "bin/start" ]; then
	chmod 755 bin/server
else
	echo "File start was not created"
fi

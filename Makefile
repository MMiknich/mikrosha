compiler=g++
progname=-o mikrosha

all:
	$(compiler) main.cpp ConveyorElement.cpp $(progname)

debug:
	$(compiler) main.cpp ConveyorElement.cpp -fsanitize=address -g $(progname)

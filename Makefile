.DEFAULT_GOAL := example

objects = example.o mtplayer.o

example.o: mtplayer.h
mtplayer.o: mtplayer.h

example: $(objects)
	cc $(objects) -lportaudio -lrt -lm -lasound -pthread -o $@


.DEFAULT_GOAL := example

objects = example.o mtplayer.o

example: $(objects)
	cc $(objects) -lportaudio -lrt -lm -lasound -pthread -o $@


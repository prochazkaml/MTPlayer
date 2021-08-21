#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <portaudio.h>

#include "mtplayer.h"

#define AUDIOFREQ 44100

#define USEFLOAT

#ifdef USEFLOAT
#define AUDIOFORMAT paFloat32
#define audio_t float
#else
#define AUDIOFORMAT paInt16
#define audio_t int16_t
#endif

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: %s inputfile\n", argv[0]);
		exit(1);
	}

	FILE *mt = fopen(argv[1], "rb");

	fseek(mt, 0, SEEK_END);
	int size = ftell(mt);
	fseek(mt, 0, SEEK_SET);

	uint8_t *data = malloc(size);

	fread(data, 1, size, mt);
	fclose(mt);

	if(!MTPlayer_Init(data)) {
		printf("MTPlayer_Init() error!\n");
		exit(1);
	}

	PaStream *stream;
	audio_t buf[1024];
	songstatus_t status;

	Pa_Initialize();
	Pa_OpenDefaultStream(&stream, 0, 1, AUDIOFORMAT, AUDIOFREQ, 1024, NULL, NULL);
	Pa_StartStream(stream);

	do {
#ifdef USEFLOAT
		MTPlayer_PlayFloat(buf, 1024, AUDIOFREQ);
#else
		MTPlayer_PlayInt16(buf, 1024, AUDIOFREQ);
#endif

		Pa_WriteStream(stream, buf, 1024);

		MTPlayer_GetStatus(&status);

		printf("Row: %02X, Order: %02d/%02d", status.row, status.order, status.orders);

		for(int ch = 0; ch < status.channels; ch++) {
			if(status.channel[ch].active)
				printf(" %d", status.channel[ch].freq);
			else
				printf(" -");
		}

		putchar('\n');
	} while(1);
	
	Pa_StopStream(stream);
	Pa_CloseStream(stream);
	Pa_Terminate();	

	free(data);
}

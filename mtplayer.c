#include "mtplayer.h"
#include <math.h>
#include <string.h>

#define maxchannels 12
#define IBN 8
#define maxnote (3 + (8 * 12) + 1)
#define vibtablesize 32
#define vibtabledepth (IBN * 8)

uint16_t *data, *rowdata;

uint8_t patterns, channels, ordertable[256];
int row = 0x3F, order = -1, orders, tempo, tempotick = 1, audiospeed, audiotick = 1;

typedef struct {
	int16_t interval, freq, portafreq;
	uint8_t enabled, active;

	uint8_t note, effect, parm1, parm2;
	uint8_t lastnote, effectraw, portadelta;
	uint8_t vibspeed, vibdepth, vibindex;

	int ctr;
} channel_t;

channel_t channel[12];
short notesHz[maxnote * IBN];
short VibTable[vibtablesize];

int MTPlayer_Init(uint8_t *filedata) {
	int i, ch;
	
	patterns = filedata[0x5C];

	if((channels = filedata[0x5D]) > maxchannels) {
		return 0;
	}

	for(orders = 0; orders < 256; orders++) {
		if(filedata[0x5F + orders] == 0xFF) break;

		ordertable[orders] = filedata[0x5F + orders];
	}

	double interval = pow(2, 1.0 / (12 * IBN));
	notesHz[0] = 440;
	double temphz = 27.5;
	notesHz[IBN] = round(temphz);

	for(i = IBN - 1; i >= 1; i--) {
		temphz /= interval;
		if(temphz < 19) temphz = 19;
		notesHz[i] = round(temphz);
	}

	temphz = 27.5;

	for(i = IBN + 1; i < maxnote * IBN; i++) {
		temphz *= interval;
		notesHz[i] = round(temphz);
	}

	tempo = (channels > 4) ? channels : 4;

	for(ch = 0; ch < maxchannels; ch++) {
		channel[ch].enabled = 1;
		channel[ch].active = 0;
	}

	for(i = 0; i < vibtablesize; i++) {
		VibTable[i] = round(vibtabledepth * sin(((double)i) * M_PI / vibtablesize * 2));
	}

	data = (uint16_t *)(filedata + 0x15F);

	return 1;
}

void _MTPlayer_ProcessTick() {
	int i, ch;

	if(!--tempotick) {
		// Process a new row

		rowdata += channels;

		if(++row >= 0x40) {
			// Process the next order

			if(++order >= orders) order = 0;

			rowdata = data + ordertable[order] * 64 * channels;

			row = 0;
		}

		for(ch = 0; ch < channels; ch++) {
			channel[ch].note = rowdata[ch] >> 9;
			channel[ch].effectraw = rowdata[ch] & 0x1FF;
			channel[ch].effect = (rowdata[ch] >> 6) & 7;

			if(channel[ch].effect == 0 || channel[ch].effect == 4) {
				channel[ch].parm1 = (rowdata[ch] >> 3) & 7;
				channel[ch].parm2 = rowdata[ch] & 7;
			} else {
				channel[ch].parm1 = rowdata[ch] & 63;
				channel[ch].parm2 = 0;
			}

			if(channel[ch].note == 127) {
				channel[ch].active = 0;
			} else if(channel[ch].note && channel[ch].note <= maxnote && channel[ch].effect != 3) {
				channel[ch].active = 1;
				channel[ch].interval = channel[ch].note * IBN;
				channel[ch].freq = notesHz[channel[ch].interval];
				channel[ch].lastnote = channel[ch].note;
				channel[ch].vibindex = 0;
				channel[ch].ctr = 0;
			}

			if(channel[ch].effect) switch(channel[ch].effect) {
				case 3:
					if(channel[ch].note && channel[ch].note <= maxnote)
						channel[ch].portafreq = notesHz[channel[ch].note * IBN];
					
					if(channel[ch].parm1) channel[ch].portadelta = channel[ch].parm1;
					break;

				case 4:
					if(channel[ch].parm1) channel[ch].vibspeed = channel[ch].parm1;
					if(channel[ch].parm2) channel[ch].vibdepth = channel[ch].parm2;
					channel[ch].vibindex += channel[ch].vibspeed;
					channel[ch].vibindex %= vibtablesize;
					break;

				case 5:
					order = channel[ch].parm1 - 1;
					row = 0x3F;
					break;

				case 6:
					if(channel[ch].parm1 && channel[ch].parm1 < 0x40) {
						row = channel[ch].parm1 - 1;
						order++;

						if(++order >= orders) order = 0;

						rowdata = data + (ordertable[order] * 64 + row) * channels;
					} else {
						row = 0x3F;
					}
					break;

				case 7:
					tempo = channel[ch].parm1;
					break;
			}
		}

		tempotick = tempo;
	} else {
		// Process effects

		for(ch = 0; ch < channels; ch++) {
			if(channel[ch].effectraw) {
				switch(channel[ch].effect) {
					case 0:
						switch((tempo - tempotick) % 3) {
							case 0:
								channel[ch].interval = channel[ch].lastnote * IBN;
								break;

							case 1:
								channel[ch].interval = (channel[ch].lastnote + channel[ch].parm1) * IBN;
								break;

							case 2:
								channel[ch].interval = (channel[ch].lastnote + channel[ch].parm2) * IBN;
								break;
						}

						channel[ch].freq = notesHz[channel[ch].interval];
						break;

					case 1:
						channel[ch].freq += channel[ch].parm1;
						// I mean, it's not _really_ necessary to clamp the upper frequency limit
						break;

					case 2:
						channel[ch].freq -= channel[ch].parm1;
						if(channel[ch].freq < 20) channel[ch].freq = 20;
						break;

					case 3:
						if(channel[ch].freq < channel[ch].portafreq) {
							channel[ch].freq += channel[ch].portadelta;

							if(channel[ch].freq > channel[ch].portafreq) {
								channel[ch].freq = channel[ch].portafreq;
							}
						}
						
						if(channel[ch].freq > channel[ch].portafreq) {
							channel[ch].freq -= channel[ch].portadelta;

							if(channel[ch].freq < channel[ch].portafreq) {
								channel[ch].freq = channel[ch].portafreq;
							}
						}
						break;

					case 4:
						channel[ch].freq = notesHz[channel[ch].interval + (((VibTable[channel[ch].vibindex]) * channel[ch].vibdepth) / vibtabledepth)];
						channel[ch].vibindex += channel[ch].vibspeed;
						channel[ch].vibindex %= vibtablesize;
						break;
				}
			}
		}
	}
}

void MTPlayer_PlayInt16(int16_t *buf, int bufsize, int audiofreq) {
	int i, ch;

	audiospeed = audiofreq / 60;

	memset(buf, 0, bufsize * sizeof(short));

	for(i = 0; i < bufsize; i++) {
		// Process tick if necessary

		if(!--audiotick) {
			audiotick = audiospeed;

			_MTPlayer_ProcessTick();
		}

		// Render the sound

		for(ch = 0; ch < channels; ch++) {
			if(channel[ch].active && channel[ch].enabled) {
				channel[ch].ctr += channel[ch].freq;

				if((channel[ch].ctr / (audiofreq / 2)) & 1) {
					buf[i] += 32767 / channels;
				}
			}
		}
	}
}

void MTPlayer_PlayFloat(float *buf, int bufsize, int audiofreq) {
	int i, ch;

	audiospeed = audiofreq / 60;

	memset(buf, 0, bufsize * sizeof(float));

	for(i = 0; i < bufsize; i++) {
		// Process tick if necessary

		if(!--audiotick) {
			audiotick = audiospeed;

			_MTPlayer_ProcessTick();
		}

		// Render the sound

		for(ch = 0; ch < channels; ch++) {
			if(channel[ch].active && channel[ch].enabled) {
				channel[ch].ctr += channel[ch].freq;

				if((channel[ch].ctr / (audiofreq / 2)) & 1) {
					buf[i] += 1.0 / ((float)channels);
				}
			}
		}
	}
}


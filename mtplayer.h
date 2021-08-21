#include <stdint.h>

typedef struct {
	int16_t interval, freq, portafreq;
	uint8_t enabled, active;

	uint8_t note, effect, parm1, parm2;
	uint8_t lastnote, effectraw, portadelta;
	uint8_t vibspeed, vibdepth, vibindex;

	int ctr;
} channel_t;

typedef struct {
	uint16_t *data, *rowdata;

	uint8_t patterns, channels, ordertable[256];
	int row, order, orders, tempo, tempotick, audiospeed, audiotick;

	channel_t channel[12];
} songstatus_t;

int MTPlayer_Init(uint8_t *filedata);
void MTPlayer_PlayInt16(int16_t *buf, int bufsize, int audiofreq);
void MTPlayer_PlayFloat(float *buf, int bufsize, int audiofreq);

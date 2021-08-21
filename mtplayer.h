#include <stdint.h>

int MTPlayer_Init(uint8_t *filedata);
void MTPlayer_PlayInt16(int16_t *buf, int bufsize, int audiofreq);
void MTPlayer_PlayFloat(float *buf, int bufsize, int audiofreq);

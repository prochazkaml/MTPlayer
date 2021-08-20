#include <stdint.h>

int MTPlayer_Init(uint8_t *filedata);
void MTPlayer_Play(uint16_t *buf, int bufsize, int audiofreq);

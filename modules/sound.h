#ifndef __SOUND_H
#define __SOUND_H

enum
{
  NO_SOUND,
  PULSE_BEEP,
  HEART_BEAT,
  MAX_NUM_SOUNDS,
};

typedef struct
{
  int   snd;
  char  fname[100];
} sound_data_t;

void play_sound(int snd);

#endif // __SOUND_H

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bedmoni.h"
#include "sound.h"

static sound_data_t sound_data[MAX_NUM_SOUNDS] = 
{
  { NO_SOUND,   "" },
  { PULSE_BEEP, "/usr/local/src/wav/ring.wav" },
  { HEART_BEAT, "/usr/local/src/wav/sound111.wav" },
};

void play_sound(int snd)
{
  int pid1, i;

  if (snd <= NO_SOUND || snd >= MAX_NUM_SOUNDS) return;

  for (i=0; i<MAX_NUM_SOUNDS; i++)
  {
    if (sound_data[i].snd == snd)
    {
      snd = i;
      break;
    }
  }

  if (i == MAX_NUM_SOUNDS)
    return;

  pid1 = fork();
  if (pid1 < 0)
  {
    error("play_sound: unable to fork\n");
    return;
  }
  if (pid1 == 0)
  {
    execl("/usr/bin/aplay", "aplay", sound_data[snd].fname, "--quiet", NULL);
    exit(0);
  }
}

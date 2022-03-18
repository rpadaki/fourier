#include <SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_SDL(X)                                                           \
  do {                                                                         \
    if (X) {                                                                   \
      printf("SDL Error: %s\n", SDL_GetError());                               \
      SDL_Quit();                                                              \
      return -1;                                                               \
    }                                                                          \
  } while (0)

typedef struct CallbackData {
  int position;
  SDL_AudioSpec spec;
  float pitch;
} CallbackData;

void audio_callback(void *userdata, uint8_t *stream, int len);

int main(int argc, char *argv[]) {
  CHECK_SDL(SDL_Init(SDL_INIT_AUDIO));

  SDL_AudioSpec want;
  SDL_AudioDeviceID dev;

  CallbackData callback_data = {0};
  callback_data.pitch = 300;

  want.freq = 128000;
  want.format = AUDIO_F32SYS;
  want.channels = 1;
  want.samples = 512;
  want.callback = audio_callback;
  want.userdata = &callback_data;

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &callback_data.spec, 0);
  CHECK_SDL(dev == 0);

  printf("have freq %d format %d channels %d samples %d\n",
         callback_data.spec.freq, callback_data.spec.format,
         callback_data.spec.channels, callback_data.spec.samples);

  SDL_PauseAudioDevice(dev, 0);

  SDL_Event event;
  while (SDL_WaitEvent(&event)) {
    if (event.type == SDL_QUIT) {
      break;
    }
  }

  SDL_Quit();
  return 0;
}

float sine_tone(int i, int freq, float pitch) {
  return sin(i * 2 * M_PI * pitch / freq);
}

float apply_overtone_profile(float (*tone)(int i, int freq, float pitch), const float* profile, const int profile_len, int i, int freq, float pitch) {
  float sum = 0;
  for (int j = 0; j < profile_len; j++) {
    sum += profile[j] * tone(i, freq, pitch * (j + 1));
  }
  return sum;
}

float instrument_tone(int i, int freq, float pitch) {
  const int num_overtones = 5;
  const float overtone_profile[num_overtones] = {
    1.0, 0.0, 0.0, 0.0, 0.0
  };
  return apply_overtone_profile(sine_tone, overtone_profile, num_overtones, i, freq, pitch);
}

void audio_callback(void *userdata, uint8_t *stream, int len) {
  CallbackData *data = (CallbackData *)userdata;

  static bool initialized_waveform = false;
  static float* waveform;
  if (!initialized_waveform) {
    waveform = malloc(sizeof(float) * data->spec.freq);

    for (int i = 0; i < data->spec.freq; ++i) {
      waveform[i] = instrument_tone(i, data->spec.freq, data->pitch * 2);
      waveform[i] += instrument_tone(i, data->spec.freq, data->pitch * 15/6);
      waveform[i] += instrument_tone(i, data->spec.freq, data->pitch * 3);
    }

    initialized_waveform = true;
  }

  float *out = (float*)stream;
  for (int i = 0; i < len / sizeof(float); ++i) {
    out[i] = waveform[data->position];
    data->position = (data->position + 1) % data->spec.freq;
  }
}

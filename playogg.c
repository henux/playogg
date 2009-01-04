/*
playogg.c - Minimal Ogg Vorbis music player.

Copyright (C) 2008 Henri Häkkinen.

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "SDL.h"

/*
** Audio stream callback.
**
** Called by the SDL background thread each time the audio buffer is ready
** to receive more data. The function decodes PCM samples from the Vorbis
** audio stream and copies them to the buffer for playback. When
** end-of-file is reached, closes the audio stream and exits cleanly.
*/
static void
stream_callback (void *userdata, Uint8 *buffer, int len)
{
  OggVorbis_File *vorbis_file = (OggVorbis_File *) userdata;

  /* Decode PCM samples to the audio buffer from the Vorbis stream. This
     needs to be done in a loop since ov_read is not guarenteed to fill the
     whole buffer in a single call. We don't have to worry about the
     logical section switches inside the stream. */

  while (len > 0) {
    int section;
    long bytes_read;

    /* Decode a chunk of PCM samples to the audio buffer. Uses signed
       16-bit stereo little-endian samples. */
    bytes_read = ov_read (vorbis_file, buffer, len, 0, 2, 1, &section);

    if (bytes_read < 0) {
      fprintf (stderr, "playogg: error while streaming\n");
      exit (1);
    } else if (bytes_read == 0) {
      /* End-of-file reached; cleanup and exit. */
      ov_clear (vorbis_file);
      SDL_Quit ();
      exit (0);
    }

    buffer += bytes_read;
    len -= bytes_read;
  }
}

int
main (int argc, char **argv)
{
  SDL_AudioSpec spec = {0};
  OggVorbis_File vorbis_file = {0};

  /* Handle command-line options. */
  if (argc < 2) {
    fprintf (stderr, "Usage: playogg FILE\n");
    return 1;
  }

  /* Initialize the SDL audio subsystem. */
  if (SDL_Init (SDL_INIT_AUDIO) < 0) {
    fprintf (stderr, "playogg: cannot initialize SDL: %s\n", SDL_GetError ());
    return 1;
  }

  /* Open the Ogg Vorbis file. */
  if (ov_fopen (argv[1], &vorbis_file) < 0) {
    fprintf (stderr, "playogg: cannot open the Ogg Vorbis stream %s\n", argv[1]);
    return 1;
  }
  
  /* Open the audio device for playback. */
  spec.freq = 44100;
  spec.format = AUDIO_S16LSB;
  spec.channels = 2;
  spec.samples = 4096;
  spec.callback = stream_callback;
  spec.userdata = (void *) &vorbis_file;

  if (SDL_OpenAudio (&spec, NULL) < 0) {
    fprintf (stderr, "playogg: cannot open the audio device: %s\n", SDL_GetError ());
    return 1;
  }

  /* Play audio and engage a infinite loop. */
  SDL_PauseAudio (0);
  puts ("Streaming audio... Press Ctrl-C to exit.");
  for (;;) SDL_Delay (100000);

  /* NOT REACHED */
  return 0;
}

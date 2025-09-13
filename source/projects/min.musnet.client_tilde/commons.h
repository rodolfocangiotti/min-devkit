#ifndef COMMONS_H
#define COMMONS_H

#define AUDIO_BUFFER_SIZE 32768  // That is a buffer of about one-second memory with sample rate of 44.1kHz...
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_VECTOR_SIZE 64    // 256
#define NUM_CHANNELS 2

//#define MAX_TID_OFFSET 1  // TODO Check it!

#define MAX_TCP_CONNECTIONS 16
//#define MAX_UDP_CONNECTIONS 16  // Check if this directive is necessary...
#define TCP_BUFFER_SIZE 3 // That is sizeof (Header) + sizeof (Token)...
#define UDP_BUFFER_SIZE 265 // 1033  // That is sizeof (Header) + sizeof (Token) + sizeof (TID) + sizeof (Size) + sizeof (AudioSample) * AUDIO_VECTOR_SIZE * NUM_CHANNELS...
// Header, token, TID and size are 9 bytes...

#define MIN_QUEUE_LENGTH 2 // A.k.a. 8 * 64 samples
#define MAX_QUEUE_LENGTH 6

#ifndef NUM_THREADS
#define NUM_THREADS 1
#endif

#if defined(DEBUG) && !(defined(VERBOSENESS))
#define VERBOSENESS 0
#endif

enum headers {
  OKAY,
  ERROR_RESPONSE,
  ENTRY_REQUEST,
  EXIT_REQUEST,
  AUDIO_STREAM_DATA
};

#define MAX_INT16 32768

// TODO
//#define DEBUG
#define VERBOSENESS 0

#define UDP_SEND_WAIT 110   // Microseconds...
                            // 110 is a reference value to use 20Mbit/sec bandwidth with 64 sample DSP block...
#define BUILD_MESSAGE "Built on May 7th, 2021"
#define REDUNDANCY 2

#endif

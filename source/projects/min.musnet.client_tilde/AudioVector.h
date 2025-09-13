#ifndef AUDIO_VECTOR_H
#define AUDIO_VECTOR_H

#include "types.h"

class AudioVector {
public:
  typedef uint Size;
  AudioVector();
  AudioVector(const Size s);
  AudioVector(const Size s, const AudioSample val);
  AudioVector(const AudioVector& othr);
  ~AudioVector();
  AudioSample& operator[](const uint i);
  const AudioSample& operator[](const uint i) const;
  AudioVector& operator=(const AudioVector& othr);
  AudioVector& operator=(const AudioVector&& othr);
  Size size() const;
private:
  Size vsize;
  std::vector<AudioSample> v;
};

#endif

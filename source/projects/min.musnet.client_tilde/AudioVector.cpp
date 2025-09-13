#include <iostream>
#include "AudioVector.h"

AudioVector::AudioVector():
  vsize(0),
  v() {
//  std::cout << "Construct AudioVector..." << '\n';
}

AudioVector::AudioVector(const Size s):
  vsize(s),
  v(s, 0.0) {
//  std::cout << "Construct AudioVector..." << '\n';
}

AudioVector::AudioVector(const Size s, const AudioSample val):
  vsize(s),
  v(s, val) {
//  std::cout << "Construct AudioVector..." << '\n';
}

AudioVector::AudioVector(const AudioVector& othr):
  vsize(othr.vsize),
  v(othr.v) {
//  std::cout << "Copy and construct AudioVector..." << '\n';
}

AudioVector::~AudioVector() {
//  std::cout << "Destruct AudioVector..." << '\n';
}

AudioSample& AudioVector::operator[](const uint i) {
  return v[i];
}

const AudioSample& AudioVector::operator[](const uint i) const {
  return v[i];
}

AudioVector& AudioVector::operator=(const AudioVector& othr) {
  vsize = othr.vsize;
  v= othr.v;
  return *this;
}

AudioVector& AudioVector::operator=(const AudioVector&& othr) {
  vsize = std::move(othr.vsize);
  v= std::move(othr.v);
  return *this;
}

AudioVector::Size AudioVector::size() const {
  return vsize;
}

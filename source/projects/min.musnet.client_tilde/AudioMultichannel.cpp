#include <cassert>
#include "AudioMultichannel.h"

AudioMultichannel::AudioMultichannel():
_matrix(),
_channels(0),
_frames(0),
_size(0) {
    
}

AudioMultichannel::AudioMultichannel(const Channel channels, const Frame frames):
_matrix(channels, AudioVector(frames, 0.0)),
_channels(channels),
_frames(frames),
_size(channels * frames) {
    
}

AudioMultichannel::AudioMultichannel(const Channel channels, const Frame frames, AudioSample default_value):
_matrix(channels, AudioVector(frames, default_value)),
_channels(channels),
_frames(frames),
_size(channels * frames) {
}

AudioMultichannel::AudioMultichannel(const Channel channels, const Frame frames, AudioVector& audio_vector):
_matrix(channels, AudioVector(frames, 0.0)),
_channels(channels),
_frames(frames),
_size(channels * frames) {
    assert(audio_vector.size() == channels * frames);
    for (uint i {0}; i < audio_vector.size(); i++) {
        ChannelIndex m = i / frames;
        FrameIndex n = i % frames;
        _matrix[m][n] = audio_vector[i];
    }
}

AudioMultichannel::AudioMultichannel(const AudioMultichannel& other):
_matrix(other._matrix),
_channels(other._channels),
_frames(other._frames),
_size(other._size) {
    
}

AudioMultichannel::~AudioMultichannel() {
    
}

AudioMultichannel& AudioMultichannel::operator=(const AudioMultichannel& other) {
    _matrix = other._matrix;
    _channels = other._channels;
    _frames = other._frames;
    _size = other._size;
    return *this;
}

AudioMultichannel& AudioMultichannel::operator=(const AudioMultichannel&& other) {
    _matrix = std::move(other._matrix);
    _channels = std::move(other._channels);
    _frames = std::move(other._frames);
    _size = std::move(other._size);
    return *this;
}

const AudioSample& AudioMultichannel::at(const ChannelIndex i, const FrameIndex j) const {
    assert(i <= _channels);
    assert(j <= _frames);
    return _matrix[i][j];
}

AudioSample& AudioMultichannel::at(const ChannelIndex i, const FrameIndex j) {
    assert(i <= _channels);
    assert(j <= _frames);
    return _matrix[i][j];
}

const AudioVector& AudioMultichannel::get_channel(ChannelIndex i) const {
    assert(i <= _channels);
    return _matrix[i];
}

void AudioMultichannel::set_channel(ChannelIndex i, AudioVector& audio_vector) {
    assert(i <= _channels);
    _matrix[i] = audio_vector;
}

void AudioMultichannel::to_audio_vector(AudioVector& audio_vector) const {
    assert(audio_vector.size() == _size);
    for (uint i {0}; i < _frames; i++) {
        for (uint j {0}; j < _channels; j++) {
            audio_vector[i * _channels + j] = _matrix[j][i];
        }
    }
}

void AudioMultichannel::from_audio_vector(AudioVector& audio_vector) {
    assert(audio_vector.size() == _size);
    for (uint i {0}; i < _frames; i++) {
        for (uint j {0}; j < _channels; j++) {
            _matrix[j][i] = audio_vector[i * _channels + j];
        }
    }
}

AudioMultichannel::Channel AudioMultichannel::channels() const {
    return _channels;
}

AudioMultichannel::Frame AudioMultichannel::frames() const {
    return _frames;
}

AudioMultichannel::Size AudioMultichannel::size() const {
    return _size;
}

#ifndef AUDIO_MULTICHANNEL_H
#define AUDIO_MULTICHANNEL_H

#include "AudioVector.h"
#include "types.h"

class AudioMultichannel {
public:
    typedef uint Channel;
    typedef uint ChannelIndex;
    typedef uint Frame;
    typedef uint FrameIndex;
    typedef uint Size;
    AudioMultichannel();
    AudioMultichannel(const Channel channels, const Frame frames);
    AudioMultichannel(const Channel channels, const Frame frames, AudioSample default_value);
    AudioMultichannel(const Channel channels, const Frame frames, AudioVector& audio_vector);
    AudioMultichannel(const AudioMultichannel& other);
    ~AudioMultichannel();
    AudioMultichannel& operator=(const AudioMultichannel& other);
    AudioMultichannel& operator=(const AudioMultichannel&& other);
    const AudioSample& at(const ChannelIndex i, const FrameIndex j) const;
    AudioSample& at(const ChannelIndex i, const FrameIndex j);
    const AudioVector& get_channel(ChannelIndex i) const;
    void set_channel(ChannelIndex i, AudioVector& audio_vector);
    void to_audio_vector(AudioVector& audio_vector) const;
    void from_audio_vector(AudioVector& audio_vector);
    Channel channels() const;
    Frame frames() const;
    Size size() const;
private:
    typedef std::vector<AudioVector> AudioMatrix;
    AudioMatrix _matrix;
    Channel _channels;
    Frame _frames;
    Size _size;
};

#endif

#include "c74_min.h"
#include "AudioMultichannel.h"
#include "Profiler.h"
#include "StreamManager.h"

using namespace c74::min;


class MuSNetClient : public object<MuSNetClient>, public vector_operator<> {
public:
    MIN_DESCRIPTION	{ "A MuSNet client" };
    MIN_TAGS		{ "audio, network, client" };
    MIN_AUTHOR		{ "Rodolfo Cangiotti" };
    MIN_RELATED		{ "udpsend, udpreceive" };
    
    /*
    argument<number> channels_arg {
        this,
        "channels",
        "Object I/O channels.",
        MIN_ARGUMENT_FUNCTION {
            channels = arg;
        }
    };
    
    attribute<number> channels {
        this,
        "channels",
        1,
        description { "Object I/O channels" },
        setter {
            MIN_FUNCTION {
                if (args.size() > 0) {
                    return { args[0] };
                }
                return { 1 };
            }
        }
    };
    */

    argument<symbol> server_ip_arg {
        this,
        "ip",
        "MuSNet server IP address.",
        MIN_ARGUMENT_FUNCTION {
            server_ip = arg;
        }
    };

    attribute<symbol> server_ip {
        this,
        "ip",
        "0.0.0.0",
        description { "MuSNet server IP address." },
        setter {
            MIN_FUNCTION {
                if (args.size() > 0) {
                    return { args[0] };
                }
                return { "0.0.0.0" };
            }
        }
    };

    argument<number> server_port_arg {
        this,
        "port",
        "MuSNet server port number.",
        MIN_ARGUMENT_FUNCTION {
            server_port = arg;
        }
    };

    attribute<number> server_port {
        this,
        "port",
        0,
        description { "MuSNet server port number." },
        setter {
            MIN_FUNCTION {
                if (args.size() > 0) {
                    return { args[0] };
                }
                return { 0 };
            }
        }
    };
    
    argument<symbol> audio_stream_protocol_arg {
        this,
        "audio_stream_protocol",
        "Transport layer protocol under the hood to stream audio.",
        MIN_ARGUMENT_FUNCTION {
            audio_stream_protocol = arg;
        }
    };

    attribute<symbol> audio_stream_protocol {
        this,
        "audio_stream_protocol",
        "udp",
        description { "Transport layer protocol under the hood to stream audio." },
        setter {
            MIN_FUNCTION {
                if (args.size() > 0) {
                    return { args[0] };
                }
                return { "udp" };
            }
        }
    };
    
    argument<number> max_latency_arg {
        this,
        "max_latency",
        "Maximum amount of audio blocks to keep in the queue from server.",
        MIN_ARGUMENT_FUNCTION {
            max_latency = arg;
        }
    };
    
    attribute<number> max_latency {
        this,
        "max_latency",
        2,
        description { "Maximum amount of audio blocks to keep in the queue from server." },
        setter {
            MIN_FUNCTION {
                if (args.size() > 0) {
                    return { args[0] };
                }
                return { 2 };
            }
        }
    };
    
    argument<symbol> audio_stream_mode_arg {
        this,
        "audio_stream_mode",
        "Audio streaming mode.",
        MIN_ARGUMENT_FUNCTION {
            audio_stream_mode = arg;
        }
    };

    attribute<symbol> audio_stream_mode {
        this,
        "audio_stream_mode",
        "standard",
        description { "Audio streaming mode." },
        setter {
            MIN_FUNCTION {
                if (args.size() > 0) {
                    return { args[0] };
                }
                return { "standard" };
            }
        }
    };

    message<> bang {
        this,
        "bang",
        "Dump object state",
        MIN_FUNCTION {
            std::string is_connected_readable = "false";
            if (_is_connected) {
                is_connected_readable = "true";
            }

            cout << endl;
            cout << "min.musnetclient~ state:" << endl;
            cout << "- server IP: " << server_ip.get() << endl;
            cout << "- server port: " << server_port.get() << endl;
            cout << "- stream protocol: " << audio_stream_protocol.get() << endl;
            cout << "- stream mode: " << audio_stream_mode.get() << endl;
            cout << "- maximum latency (in blocks): " << max_latency.get() << endl;
            cout << "- connected: " << is_connected_readable << endl;
            cout << endl;
            return {};
        }
    };
    
    /*
    message<> set_channels_message {
        this,
        "set_channels",
        "Set I/O channels",
        MIN_FUNCTION {
            if (args.size() > 0) {
                channels = args[0];
            }
            return {};
        }
    };
    */

    message<> set_server_ip_message {
        this,
        "set_server_ip",
        "Set MuSNet server IP",
        MIN_FUNCTION {
            if (args.size() > 0) {
                server_ip = args[0];
            }
            return {};
        }
    };

    message<> set_server_port_message {
        this,
        "set_server_port",
        "Set MuSNet server port number.",
        MIN_FUNCTION {
            if (args.size() > 0) {
                server_port = args[0];
            }
            return {};
        }
    };
    
    message<> set_audio_stream_protocol_message {
        this,
        "set_audio_stream_protocol",
        "Set transport layer protocol under the hood to stream audio.",
        MIN_FUNCTION {
            if (args.size() > 0) {
                audio_stream_protocol = args[0];
            }
            return {};
        }
    };
    
    message<> set_max_latency_message {
        this,
        "set_max_latency",
        "Set the maximum amount of audio blocks to keep in the queue from server.",
        MIN_FUNCTION {
            if (args.size() > 0) {
                max_latency = args[0];
            }
            return {};
        }
    };
    
    message<> set_audio_stream_mode_message {
        this,
        "set_audio_stream_mode",
        "Set audio streaming mode (standard, send only or receive only).",
        MIN_FUNCTION {
            if (args.size() > 0) {
                audio_stream_mode = args[0];
            }
            return {};
        }
    };

    message<> connect {
        this,
        "connect",
        "Negotiate connection to MuSNet server.",
        MIN_FUNCTION {
            _set_audio_stream_protocol();
            _set_max_latency();
            _set_audio_stream_mode();
            _is_connected = true;
            if (this->_connect()) {
                // Connection attemp failed...
                cout << "Connection attempt failed." << endl;
                _is_connected = false;
            }
            return {};
        }
    };

    message<> disconnect {
        this,
        "disconnect",
        "Notify disconnection to MuSNet server.",
        MIN_FUNCTION {
            if (_is_connected) {
                if ((this->_disconnect())) {
                    cout << "Disconnection attempt failed." << endl;
                } else {
                    _is_connected = false;
                }
            }
            return {};
        }
    };
    
    message<> statistics {
        this,
        "statistics",
        "Print RTT statistics.",
        MIN_FUNCTION {
            /*
            RTTStatistics stats = this->_get_rtt_statistics();
            cout << "RTT statistics:" << endl;
            cout << " - average: " << stats.average << endl;
            cout << " - minimum: " << stats.minimum << endl;
            cout << " - maximum: " << stats.minimum << endl;
            */
            cout << Profiler::get_statistics() << endl;
            return {};
        }
    };
    
    message<> clear {
        this,
        "clear",
        "Clear RTT statistics.",
        MIN_FUNCTION {
            /*
            RTTStatistics stats = this->_get_rtt_statistics();
            cout << "RTT statistics:" << endl;
            cout << " - average: " << stats.average << endl;
            cout << " - minimum: " << stats.minimum << endl;
            cout << " - maximum: " << stats.minimum << endl;
            */
            Profiler::clear_records();
            return {};
        }
    };

    message<> dspsetup {
        this,
        "dspsetup",
        MIN_FUNCTION {
            return {};
        }
    };
    
    MuSNetClient(const atoms& args = {}):
    _manager() {
        // Main class constructor...
        if (args.size() > 0) {
            uint channels = static_cast<number>(args[0]);
            for (auto i {1}; i <= channels; i++) {
                auto inlet_obj = std::make_unique<inlet<>>(this, "(signal) input channel n. " + std::to_string(i), "signal");
                auto outlet_obj = std::make_unique<outlet<>>(this, "(signal) output channel n. " + std::to_string(i), "signal");
                _inlets.push_back(std::move(inlet_obj));
                _outlets.push_back(std::move(outlet_obj));
            }
        }
    }
    
    ~MuSNetClient() {
        // Main class destructor...
        _manager.disconnect();
    }

    void _set_audio_stream_protocol() {
        std::string proto = audio_stream_protocol.get();
        if (proto == "tcp") {
            _manager.set_audio_stream_protocol(TCP);
        } else if (proto == "udp") {
            _manager.set_audio_stream_protocol(UDP);
        }
    }

    void _set_max_latency() {
        _manager.setMaxLatency(max_latency.get());
    }
    
    void _set_audio_stream_mode() {
        std::string mode = audio_stream_mode.get();
        if (mode == "standard") {
            _manager.set_flag(STANDARD_STREAM);
        } else if (mode == "send_only") {
            _manager.set_flag(SEND_ONLY_STREAM);
        } else if (mode == "receive_only") {
            _manager.set_flag(RECEIVE_ONLY_STREAM);
        }
    }

    int _connect() {
        _manager.setIPAddress(server_ip.get());
        return _manager.connect();
    }

    int _disconnect() {
        return _manager.disconnect();
    }

    RTTStatistics _get_rtt_statistics() {
        return _manager.getRTTStatistics();
    }

    void operator() (audio_bundle input, audio_bundle output) {
        assert(input.channel_count() == output.channel_count());
        // Change file to recompile it...

        if (_is_connected) {
            const uint64_t channel_amount = input.channel_count();
            const uint64_t sample_amount = input.frame_count();
            const uint64_t packet_amount = sample_amount / AUDIO_VECTOR_SIZE;
            const uint64_t packet_size = AUDIO_VECTOR_SIZE;
            // Allocate a multichannel buffer to easily export the buffer for audio stream exchange...
            AudioMultichannel buffer(NUM_CHANNELS, AUDIO_VECTOR_SIZE, 0);
            // Allocate an interleaved buffer to send and receive data...
            AudioVector musnet_buffer(NUM_CHANNELS * AUDIO_VECTOR_SIZE, 0);
            
            // Sending data...
            if (_manager.get_flag() == RECEIVE_ONLY_STREAM) {
                for (uint64_t packet_index {0}; packet_index < packet_amount; packet_index++) {
                    AudioVector empty_buffer(0);
                    _manager.sendStream(empty_buffer);
                }
            } else {
                for (uint64_t packet_index {0}; packet_index < packet_amount; packet_index++) {
                    for (uint64_t packet_sample_index {0}; packet_sample_index < packet_size; packet_sample_index++) {
                        uint64_t sample_index = packet_index * packet_size + packet_sample_index;
                        for (uint64_t channel_index {0}; channel_index < channel_amount; channel_index++) {
                            // Audio stream is with 16 bit precision...
                            double sample_value = input.samples(channel_index)[sample_index];
                            AudioSample to_int_sample = static_cast<AudioSample>(floor(sample_value * MAX_INT16));
                            buffer.at(channel_index, sample_index) = to_int_sample;
                        }
                    }
                    buffer.to_audio_vector(musnet_buffer);
                    // Send buffer to server...
                    _manager.sendStream(musnet_buffer);
                }
            }
            
            // Receiving data...
            if (_manager.get_flag() == SEND_ONLY_STREAM) {
                for (uint64_t packet_index {0}; packet_index < packet_amount; packet_index++) {
                    AudioVector empty_buffer = _manager.retrieveStreamWithLatency();
                    // assert(empty_buffer.size() == 0);
                }
            } else {
                for (uint64_t packet_index {0}; packet_index < packet_amount; packet_index++) {
                    // Request a audio stream from other connected clients...
                    musnet_buffer = _manager.retrieveStreamWithLatency();
                    buffer.from_audio_vector(musnet_buffer);
                    for (uint64_t packet_sample_index {0}; packet_sample_index < packet_size; packet_sample_index++) {
                        uint64_t sample_index = packet_index * packet_size + packet_sample_index;
                        for (uint64_t channel_index {0}; channel_index < channel_amount; channel_index++) {
                            AudioSample sample_value = buffer.at(channel_index, sample_index);
                            output.samples(channel_index)[sample_index] = sample_value / static_cast<double>(MAX_INT16);
                        }
                    }
                }
            }
        } else {
            // IF NOT CONNECTED...
            // Clean output...
            for (uint64_t channel_index {0}; channel_index < output.channel_count(); channel_index++) {
                for (uint64_t sample_index {0}; sample_index < output.frame_count(); sample_index++) {
                    output.samples(channel_index)[sample_index] = 0.0;
                }
            }
        }
    }
private:
    std::vector<std::unique_ptr<inlet<>>> _inlets;
    std::vector<std::unique_ptr<outlet<>>> _outlets;
    StreamManager _manager;
    bool _is_connected;
};

MIN_EXTERNAL(MuSNetClient);

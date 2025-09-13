#include "c74_min.h"
#include "StreamManager.h"

using namespace c74::min;


class MuSNetClient : public object<MuSNetClient>, public vector_operator<> {
public:
    MIN_DESCRIPTION	{ "A MuSNet client" };
    MIN_TAGS		{ "audio, network, client" };
    MIN_AUTHOR		{ "Rodolfo Cangiotti" };
    MIN_RELATED		{ "udpsend, udpreceive" };

    inlet<>  left_in    { this, "(signal) left input channel", "signal" };
    inlet<>  right_in   { this, "(signal) right input channel", "signal" };
    outlet<> left_out   { this, "(signal) left output channel", "signal" };
    outlet<> right_out  { this, "(signal) right output channel", "signal" };

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
        -1,
        description { "MuSNet server IP address." },
        setter {
            MIN_FUNCTION {
                if (args.size() == 1) {
                    return { args[0] };
                }
                return {};
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

    attribute<int> server_port {
        this,
        "port",
        -1,
        description { "MuSNet server port number." },
        setter {
            MIN_FUNCTION {
                if (args.size() == 1) {
                    return { args[0] };
                }
                return { -1 };
            }
        }
    };

    message<> bang {
        this,
        "bang",
        "Dump object state",
        MIN_FUNCTION {
            cout << endl;
            cout << "min.musnetclient~ state:" << endl;
            cout << "- server IP: " << server_ip.get() << endl;
            cout << "- server port: " << server_port.get() << endl;
            cout << "- connected: " << _is_connected << endl;
            cout << endl;
            return {};
        }
    };

    message<> set_server_ip_message {
        this,
        "set_server_ip",
        "Set MuSNet server IP",
        MIN_FUNCTION {
            if (args.size() == 1) {
                server_ip = args[0];
            }
            return {};
        }
    };

    message<> set_server_port_message {
        this,
        "set_server_port",
        "Set MuSNet server port number",
        MIN_FUNCTION {
            if (args.size() == 1) {
                server_port = args[0];
            }
            return {};
        }
    };

    message<> connect {
        this,
        "connect",
        "Negotiate connection to MuSNet server",
        MIN_FUNCTION {
            _is_connected = true;
            if (this->_connect()) {
                // Connection attemp failed...
                _is_connected = false;
            }
            return {};
        }
    };

    message<> disconnect {
        this,
        "disconnect",
        "Notify disconnection to MuSNet server",
        MIN_FUNCTION {
            if (!(this->_disconnect())) {
                _is_connected = false;
            }
            return {};
        }
    };

    message<> dspsetup {
        this,
        "dspsetup",
        MIN_FUNCTION {
            // TODO
            return {};
        }
    };

    MuSNetClient():
        _manager() {
        // Main class constructor...
    }
    
    ~MuSNetClient() {
        // Main class destructor...
        _manager.disconnect();
    }

    int _connect() {
        _manager.setIPAddress(server_ip.get());
        return _manager.connect();
    }

    int _disconnect() {
        return _manager.disconnect();
    }

    void operator() (audio_bundle input, audio_bundle output) {
        auto* left_input = input.samples(0);
        auto* right_input = input.samples(1);
        auto* left_output = output.samples(0);
        auto* right_output = output.samples(1);

        if (_is_connected) {
            _manager.setMaxLatency(64); // TODO
            // Allocate a buffer to send and receive data...
            AudioVector musnet_buffer(AUDIO_VECTOR_SIZE * NUM_CHANNELS, 0);
            uint64_t sample_amount = input.frame_count();
            uint64_t packet_amount = sample_amount / AUDIO_VECTOR_SIZE;
            uint64_t packet_size = AUDIO_VECTOR_SIZE;
            for (uint64_t packet_index {0}; packet_index < packet_amount; packet_index++) {
                for (uint64_t packet_sample_index {0}; packet_sample_index < packet_size; packet_sample_index++) {
                    uint64_t sample_index = packet_index * packet_size + packet_sample_index;
                    // Buffer is interleaved...
                    musnet_buffer[packet_sample_index * 2] = static_cast<AudioSample>(
                        floor(left_input[sample_index] * MAX_INT16)
                    );
                    musnet_buffer[packet_sample_index * 2 + 1] = static_cast<AudioSample>(
                        floor(right_input[sample_index] * MAX_INT16)
                    );
                }
                // Send buffer to server...
                _manager.sendStream(musnet_buffer);
            }
            for (uint64_t packet_index {0}; packet_index < packet_amount; packet_index++) {
                // Request a audio stream from other connected clients...
                musnet_buffer = _manager.retrieveStream();
                for (uint64_t packet_sample_index {0}; packet_sample_index < packet_size; packet_sample_index++) {
                    uint64_t sample_index = packet_index * packet_size + packet_sample_index;
                    left_output[sample_index] = musnet_buffer[packet_sample_index * 2] / static_cast<double>(MAX_INT16);
                    right_output[sample_index] = musnet_buffer[packet_sample_index * 2 + 1] / static_cast<double>(MAX_INT16);
                }
            }
        } else {
            // Clean output...
            for (uint64_t sample_index {0}; sample_index < output.frame_count(); sample_index++) {
                left_output[sample_index] = 0.0;
                right_output[sample_index] = 0.0;
            }
        }
    }
private:
    StreamManager _manager;
    bool _is_connected;
};

MIN_EXTERNAL(MuSNetClient);

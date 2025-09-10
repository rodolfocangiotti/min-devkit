#include "c74_min.h"

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
            this->_connect();
            return {};
        }
    };

    message<> disconnect {
        this,
        "disconnect",
        "Notify disconnection to MuSNet server",
        MIN_FUNCTION {
            this->_disconnect();
            _is_connected = false;
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

    void _connect() {
        // TODO
    }

    void _disconnect() {
        // TODO
    }

    void operator() (audio_bundle input, audio_bundle output) {
        for (uint8_t chan_index {0}; chan_index < input.channel_count(); chan_index++) {
            auto* input_channel = input.samples(chan_index);
            auto* output_channel = output.samples(chan_index);
            for (uint64_t sample_index {0}; sample_index < input.frame_count(); sample_index++) {
                output_channel[sample_index] = input_channel[sample_index] * -1.0;
            }
        }

    }
private:
    bool _is_connected;
};

MIN_EXTERNAL(MuSNetClient);

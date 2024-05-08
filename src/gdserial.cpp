#include "gdserial.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

#define    BOTHER 0010000

void Serial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_baud"), &Serial::get_baud);
	ClassDB::bind_method(D_METHOD("set_baud"), &Serial::set_baud);
	ClassDB::bind_method(D_METHOD("get_device"), &Serial::get_device);
	ClassDB::bind_method(D_METHOD("set_device"), &Serial::set_device);
	ClassDB::bind_method(D_METHOD("get_eol"), &Serial::get_eol);
	ClassDB::bind_method(D_METHOD("set_eol"), &Serial::set_eol);
	ClassDB::bind_method(D_METHOD("get_trigger"), &Serial::get_trigger);
	ClassDB::bind_method(D_METHOD("set_trigger"), &Serial::set_trigger);

	ClassDB::add_property("Serial", PropertyInfo(Variant::INT, "baudrate"), "set_baud", "get_baud");
	ClassDB::add_property("Serial", PropertyInfo(Variant::STRING, "device"), "set_device", "get_device");
	ClassDB::add_property("Serial", PropertyInfo(Variant::INT, "eol", PROPERTY_HINT_RANGE, "0, 255, 1"), "set_eol", "get_eol");
	ClassDB::add_property("Serial", PropertyInfo(Variant::BOOL, "trigger"), "set_trigger", "get_trigger");

	ADD_SIGNAL(MethodInfo("connected", PropertyInfo(Variant::OBJECT, "node")));
	ADD_SIGNAL(MethodInfo("disconnected", PropertyInfo(Variant::OBJECT, "node")));
	ADD_SIGNAL(MethodInfo("connect_error", PropertyInfo(Variant::OBJECT, "node"), PropertyInfo(Variant::STRING, "error")));
	ADD_SIGNAL(MethodInfo("byte_received", PropertyInfo(Variant::OBJECT, "node"), PropertyInfo(Variant::INT, "val")));
	ADD_SIGNAL(MethodInfo("packet_received", PropertyInfo(Variant::OBJECT, "node"), PropertyInfo(Variant::PACKED_BYTE_ARRAY, "packet")));

	ClassDB::bind_method(D_METHOD("open"), &Serial::open);
	ClassDB::bind_method(D_METHOD("open", "baud"), &Serial::open);
	ClassDB::bind_method(D_METHOD("open", "dev"), &Serial::open);
	ClassDB::bind_method(D_METHOD("open", "dev", "baud"), &Serial::open);
}

Serial::Serial() {
	_baudrate = 115200;
	_device = "/dev/ttyACM0";
	_eol = 13;
	_trigger = false;
}

Serial::~Serial() {
}

void Serial::set_device(String dev) {
	_device = dev;
}

String Serial::get_device() {
	return _device;
}

void Serial::set_baud(const int b) {
	_baudrate = b;
}

int Serial::get_baud() {
	return _baudrate;
}

void Serial::set_eol(int c) {
	_eol = c;
}

int Serial::get_eol() {
	return _eol;
}

void Serial::set_trigger(bool t) {
	_trigger = t;
}

bool Serial::get_trigger() {
	return _trigger;
}

int Serial::open() {
	return open(_device, _baudrate);
}

int Serial::open(String dev) {
	return open(dev, _baudrate);
}

int Serial::open(int b) {
	return open(_device, b);
}

int Serial::open(String dev, int baud) {

	char tmp[dev.length() + 1] = {0};
	for (int i = 0; i < dev.length(); i++) {
		tmp[i] = dev.ptr()[i] & 0xFF;
	}

    struct termios options;
    if (_fd >= 0) {
		close();
    }
    _fd = ::open(tmp, O_RDWR|O_NOCTTY);
	if (_fd <= 0) {
		emit_signal("connect_error", this, "Unable to open device");
		return -1;	
	}
    fcntl(_fd, F_SETFL, 0);
    tcgetattr(_fd, &_savedOptions);
    tcgetattr(_fd, &options);
    cfmakeraw(&options);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag &= ~CBAUD;
    if (strcmp(tmp, "/dev/tty") == 0) {
        options.c_lflag |= ISIG;
    }
    options.c_cflag |= BOTHER;
    options.c_ispeed = baud;
    options.c_ospeed = baud;
    if (tcsetattr(_fd, TCSANOW, &options) != 0) {
		emit_signal("connect_error", this, "Unable to set device options");
        return -1;
    }

	emit_signal("connect", this);

	return 0;
}


void Serial::close() {
	if (_fd < 0) {
		return;
	}
    tcsetattr(_fd, TCSANOW, &_savedOptions);
	::close(_fd);
	_fd = -1;
	emit_signal("closed", this);
}

void Serial::_process(double delta) {
    if (_fd < 0) {
        return;
    }

    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(_fd, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    int retval = select(_fd+1, &rfds, NULL, NULL, &tv);
    if (retval) {
		uint8_t c;
		::read(_fd, &c, 1);
		_buffer.push_back(c);
		emit_signal("byte_received", this, c);
		if (_trigger) {
			if (c == _eol) {
				emit_signal("packet_received", this, _buffer);		
				_buffer.clear();
			}
		}
    }
}

int Serial::available() {
	return _buffer.size();
}

int Serial::read() {
	if (_buffer.is_empty()) {
		return -1;
	}

	int v = _buffer[0];
	_buffer.remove_at(0);
	return v;
}

void Serial::write(int v) {
	if (_fd < 0) {
		return;
	}
	char c = v & 0xFF;
	::write(_fd, &c, 1);
}

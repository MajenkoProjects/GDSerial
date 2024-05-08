#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <godot_cpp/classes/node.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>


namespace godot {

class Serial : public Node {
	GDCLASS(Serial, Node)

private:
	int _baudrate;
	String _device;
	int _fd;
	struct termios _savedOptions;
	PackedByteArray _buffer;
	int _eol;
	bool _trigger;

protected:
	static void _bind_methods();


public:
	Serial();
	~Serial();

	int get_baud();
	void set_baud(const int baud);

	String get_device();
	void set_device(String dev);

	int get_eol();
	void set_eol(const int eol);

	bool get_trigger();
	void set_trigger(bool t);

	int open(); // Open with pre-set device and baud rate
	int open(int baud); // Open with pre-set device but specify the baud rate
	int open(String dev); // Open with pre-set baud but specify the baud rate
	int open(String dev, int baud); // Open specifying both device and baud rate

	void close(); // Close the serial port

	int available(); // Number of bytes available in the circular buffer

	int read(); // Read one byte if available, or -1 if not

	void write(int); // Write one byte

	void _process(double delta);
};

}

#endif

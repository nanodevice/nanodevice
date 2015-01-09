#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#include <sys/ioctl.h>
#include <termios.h>

#include "hid.h"


static char get_keystroke(void);

int blocks = 0;

int main()
{
	int i, r, num, sendBufIndex = 0;
	unsigned char c, buf[64], sendBuf[64];
    
	// C-based example is 16C0:0480:FFAB:0200
	r = rawhid_open(1, 0x16C0, 0x0480, 0xFFAB, 0x0200);
	if (r <= 0) {
		// Arduino-based example is 16C0:0486:FFAB:0200
		r = rawhid_open(1, 0x16C0, 0x0486, 0xFFAB, 0x0200);
		if (r <= 0) {
			printf("no rawhid device found\n");
			return -1;
		}
	}
	printf("found rawhid device\n");
    
    unsigned int expectedSequence = 0;
    unsigned int sequence = 0;
	while (1) {
		// check if any Raw HID packet has arrived
		num = rawhid_recv(0, buf, 64, 220);
		if (num < 0) {
			printf("{\"error\":{\"message\":\"Device went offline.\",\"level\":\"protocol\",\"severity\":\"fatal\"}}\n");
			rawhid_close(0);
			return 0;
		}
		if (num > 0) {
            // Should have received 64 bytes.
            if (num != 64) {
                printf("{\"error\":{\"message\":\"Expected 64 bytes, got %d.\",\"level\":\"protocol\"}}\n", num);
            }
            sequence = (unsigned int)((((buf[62]&0xff)<<8) | ((buf[63]&0xff)<<0))&0xffff);
            
            // The two last bytes are the sequence number. We should get them all. &0xffff to make sure
            // we only look at the last 16 bits when the counter rolls over.
            if (sequence != ((++expectedSequence) & 0xffff) ) {
                printf("{\"error\":{\"message\":\"Expected sequence %u, got %u.\",\"level\":\"protocol\"}}\n",
                        expectedSequence, sequence);
                expectedSequence = sequence;
            }
            switch (buf[0]) {
                case 'd': // A block of data arrived
                    printf("{\"data\":[");
                    // Byte with index 1 until byte with index 61 contain data.
                    // Data is in triplets. There are 20 datapoints.
                    for (int datapoint = 0; datapoint <= 19 ; datapoint ++) {
                        int index = 1+datapoint*3;
                        unsigned int value = (int)((buf[index]<<16) | (buf[index+1]<<8) | (buf[index+2]<<0) );
                        if (datapoint != 19) {
                            printf("%u,",value);
                        } else {
                            printf("%u",value);
                        }
                    }
                    printf("],\"sequence\":%u}\n", sequence);
                    break;
                case 'r': // A readback has arrived
                    
                    break;
                case 'e': // An error has arrived
                    printf("{\"error\":{\"message\":\"%s\",\"level\":\"embedded\"}}\n", buf);
                    break;
                default:
                    printf("{\"error\":{\"message\":\"Undefined client message id: 0x%hhx\",\"level\":\"protocol\"}}\n", buf[0]);
            }
            
            //fflush(stdout);
		}
		// check if any input on stdin
		while ((c = get_keystroke()) >= 32) {
            if (sendBufIndex > 61) {
                printf("{\"error\":{\"message\":\"Too many characters to transmit in one block. Discarded.\",\"level\":\"protocol\"}}\n");
                sendBufIndex = 0;
            }
            sendBuf[sendBufIndex++] = c;
            if (c == ';') {
                for (i=sendBufIndex; i<64; i++) {
                    sendBuf[i] = 0;
                }
                rawhid_send(0, sendBuf, 64, 100);
                printf("{\"status\":{\"message\":\"Sent %d characters to client.\",\"sent\":\"%s\"}}\n", sendBufIndex, sendBuf);
                sendBufIndex=0;
            }
		}
	}
}

static int _kbhit() {
	static const int STDIN = 0;
	static int initialized = 0;
	int bytesWaiting;
    
	if (!initialized) {
		// Use termios to turn off line buffering
		struct termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = 1;
	}
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}
static char _getch(void) {
	char c;
	if (fread(&c, 1, 1, stdin) < 1) return 0;
	return c;
}



static char get_keystroke(void)
{
	if (_kbhit()) {
		char c = _getch();
		if (c >= 32) return c;
	}
	return 0;
}





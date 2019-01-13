//Copyright (c) 2017 Pratik M Tambe <enthusiasticgeek@gmail.com>
//Compile on Raspberry Pi using gcc -g uart_pi.c -o uart_pi
//This file is to be run from Raspberry pi 1 or 2 (UART ttyAMA0 connection) - GPIO14 (TXD) and GPIO15 (RXD)on Raspberry Pi are connected to RXD1 and TXD1 pins on AVR uC, respectively.
//For Raspberry Pi 3 please adjust /dev/ttyAMA0 to something else e.g. /dev/serial0
//For Raspberry Pi 3 See https://raspberrypi.stackexchange.com/questions/45570/how-do-i-make-serial-work-on-the-raspberry-pi3
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		        //Used for UART
#include <getopt.h>
#include <stdint.h>
#include <errno.h>

void usage(char* argv) {
    printf("Usage: %s [OPTIONS]\n", argv);
    printf("  -i interface              USART interface /dev/ttyAMA0(e.g.On Raspberry pi), /dev/ttyUSB0, etc\n");
    printf("  -p pan                    <POS1,POS2,POS3,POS4,POS5,POS6,POS7,POS8,POS9> for pan\n");
    printf("  -t tilt                   <POS1,POS2,POS3,POS4,POS5,POS6,POS7,POS8,POS9> for tilt\n");
    printf("  -s status                 Get the position status for pan/tilt\n");
    printf("  -h, --help                print this help and exit\n");
    printf("   Example to navigate to servo pan/tilt position\n %s -i /dev/ttyUSB0 -p POS3 -t POS4", argv);
    printf("\n");
}

unsigned char CheckSum(char *buffer)
{
    uint8_t i;
    unsigned char xor_1;
    unsigned long length = strlen(buffer);
    for (xor_1 = 0, i = 0; i < length; i++) {
        xor_1 ^= (unsigned char)buffer[i];
    }
    return xor_1;
}

unsigned short crc16_ccitt(const unsigned char* data_p, unsigned char length) {
    unsigned char x;
    unsigned short crc = 0xFFFF;
    while (length--) {
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}

int main(int argc, char * argv[]) {

    if(argc < 3) {
        usage(argv[0]);
        exit(0);
    }

    char interface[256]= {};
    char pan[5]= {};
    char tilt[5]= {};
    char status[7]= {};

//Some flags
    int s_option_specified=0;

    int c;
    const char* short_opt = "hi:p:t:s";
    struct option  long_opt[] =
    {
        {"help",           no_argument,       NULL, 'h'},
        {"interface",      required_argument, NULL, 'i'},
        {"pan",            required_argument, NULL, 'p'},
        {"tilt",           required_argument, NULL, 't'},
        {"status",         no_argument, NULL, 's'},
        {NULL,             0,                 NULL,  0 }
    };

    while((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
    {
        switch(c)
        {
        case -1:       /* no more arguments */
        case 0:        /* long options toggles */
            break;

        case 'i':
            printf("INTERFACE: \"%s\"\n", optarg);
            if(strlen(optarg)>sizeof(interface)) {
                printf("Invalid argument for interface\n");
                exit(-1);
            }
            snprintf(interface,sizeof(interface),"%s",optarg);
            break;

        case 'p':
            if(s_option_specified == 1)
            {
                printf("***Error: Cannot use s with p/t options simultaneously.***\n Use one or the other\n");
                usage(argv[0]);
                return(-3);
            }
            printf("PAN: \"%s\"\n", optarg);
            if(strlen(optarg)>sizeof(pan)) {
                printf("Invalid argument for pan\n");
                exit(-1);
            }
            snprintf(pan,sizeof(pan),"%s",optarg);
            break;

        case 't':
            if(s_option_specified == 1)
            {
                printf("***Error: Cannot use s with p/t options simultaneously.***\n Use one or the other\n");
                usage(argv[0]);
                return(-3);
            }
            printf("TILT: \"%s\"\n", optarg);
            if(strlen(optarg)>sizeof(tilt)) {
                printf("Invalid argument for tilt\n");
                exit(-1);
            }
            snprintf(tilt,sizeof(tilt),"%s",optarg);
            break;

        case 's':
            s_option_specified=1;
            snprintf(pan,sizeof(pan),"%s","----");
            snprintf(tilt,sizeof(tilt),"%s","----");
            break;

        case 'h':
            printf("here0\n");
            usage(argv[0]);
            return(0);

        case ':':
        case '?':
            printf("here1\n");
            fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
            return(-2);

        default:
            printf("here2\n");
            fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
            fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
            return(-2);
        };
    };

//-------------------------
//----- SETUP USART 0 -----
//-------------------------
//At bootup, pins 8 and 10 are already set to UART0_TXD, UART0_RXD (ie the alt0 function) respectively
    int uart0_filestream = -1;
//const char* UART_PORT="/dev/ttyAMA0";
//const char* UART_PORT="/dev/ttyUSB0";
    const char* UART_PORT=(const char*)interface;

//OPEN THE UART
//The flags (defined in fcntl.h):
//	Access modes (use 1 of these):
//		O_RDONLY - Open for reading only.
//		O_RDWR - Open for reading and writing.
//		O_WRONLY - Open for writing only.
//
//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
//											immediately with a failure status if the output can't be written immediately.
//
//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
//uart0_filestream = open(UART_PORT, O_RDWR | O_NOCTTY | O_NDELAY);		//Open in non blocking read/write mode
    uart0_filestream = open(UART_PORT, O_RDWR | O_NOCTTY | O_SYNC);		//Open in non blocking read/write mode
    if (uart0_filestream == -1)
    {
        //ERROR - CAN'T OPEN SERIAL PORT
        printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
    }

//CONFIGURE THE UART
//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
//	CSIZE:- CS5, CS6, CS7, CS8
//	CLOCAL - Ignore modem status lines
//	CREAD - Enable receiver
//	IGNPAR = Ignore characters with parity errors
//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
//	PARENB - Parity enable
//	PARODD - Odd parity (else even)
    int rc=0;
    struct termios options;
    // Get the current options for the port
    if((rc = tcgetattr(uart0_filestream, &options)) < 0) {
        fprintf(stderr, "failed to get attr: %d, %s\n", uart0_filestream, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // Set the baud rates to 115200
    cfsetispeed(&options, B115200);
    // Set the baud rates to 115200
    cfsetospeed(&options, B115200);
    // Setting other Port Stuff - uncomment only as necessary
    options.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;         /* 8-bit characters */
    options.c_cflag &= ~PARENB;     /* no parity bit */
    options.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    options.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 1;

    tcflush(uart0_filestream, TCIFLUSH);
    // Set the new attributes
    if((rc = tcsetattr(uart0_filestream, TCSANOW, &options)) < 0) {
        fprintf(stderr, "failed to set attr: %d, %s\n", uart0_filestream, strerror(errno));
        exit(EXIT_FAILURE);
    }

//----- TX BYTES -----
    unsigned char tx_buffer[20]= {};
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];

     int i;
     int len;
     char checksum_sent_string[20]= {};

     if(s_option_specified != 1)
     {
	//PT - Field 1
          printf("sending PT values\n");

           char prefix[3] = {'P','T'};
	   len = strlen(prefix);
	   for (i = 0; i < len; i++) {
		*p_tx_buffer++ = prefix[i];
	    }
	    strncat(checksum_sent_string,prefix,strlen(prefix));
	    *p_tx_buffer++ = ',';

      } else {

          //PTST - Field 1
          printf("sending PT values\n");

           char prefix[5] = {'P','T','S','T'};
	   len = strlen(prefix);
	   for (i = 0; i < len; i++) {
		*p_tx_buffer++ = prefix[i];
	    }
	    strncat(checksum_sent_string,prefix,strlen(prefix));
	    *p_tx_buffer++ = ',';
      }
	//Pan - Field 2
	    len = strlen(pan);
	    for (i = 0; i < len; i++) {
		*p_tx_buffer++ = pan[i];
	    }
	    strncat(checksum_sent_string,pan,strlen(pan));
	    *p_tx_buffer++ = ',';

	//Tilt - Field 3
	    len = strlen(tilt);
	    for (i = 0; i < len; i++) {
		*p_tx_buffer++ = tilt[i];
	    }
	    strncat(checksum_sent_string,tilt,strlen(tilt));
	    *p_tx_buffer++ = ',';

	//checksum - Field 4
	    unsigned short send_checksum = crc16_ccitt((const unsigned char*)checksum_sent_string,strlen(checksum_sent_string));
	    char send_checksum_char[2];
	    send_checksum_char[0] = (char)(send_checksum & 0xFF);
	    send_checksum_char[1] = (char)((send_checksum >> 8) & 0xFF);
	    printf("checksum %s\n",send_checksum_char);

	    *p_tx_buffer++ = send_checksum_char[0];
	    *p_tx_buffer++ = send_checksum_char[1];
	    *p_tx_buffer++='\r';

    tcdrain(uart0_filestream);    /* delay for output */

    
    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, &tx_buffer[0], (p_tx_buffer - &tx_buffer[0]));		//Filestream, bytes to write, number of bytes to write
        if (count < 0)
        {
            printf("UART TX error\n");
        }
    }
//----- CHECK FOR ANY RX BYTES -----
    if (uart0_filestream != -1)
    {
        printf("\nAttempting read\n");
        int32_t n_read = 0;
        int32_t spot = 0;
        char buf = '\0';
        char response_read[1024];
        memset(response_read, '\0', sizeof response_read);
        do {
            //n_read = read( uart0_filestream, &buf, 1 );
            n_read = read( uart0_filestream, &buf, sizeof(buf) - 1);
            printf("size read = %d\n",n_read);
            sprintf( &response_read[spot], "%c", buf );
            spot += n_read;
        } while( buf != '\r' && n_read > 0);
        if (n_read < 0) {
            printf("Error reading: %s\n", strerror(errno));
        }
        else if (n_read == 0) {
            printf("Read nothing!\n");
        }
        else {
            printf("Response: %s\n", response_read);
            //response = std::string(response_read);
        }
        return n_read;

    }

//----- CLOSE THE UART -----
    close(uart0_filestream);
}

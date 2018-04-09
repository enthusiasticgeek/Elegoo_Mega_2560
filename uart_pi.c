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
    printf("  -d direction              <CCW,CW,STP> for Counter-Clockwise, Clockwise, Stop\n");
    printf("  -s speed                  <F,M,S> for Fast, Medium, and Slave\n");
    printf("  -t steps                  -1 or negative numbers -> Don't care. Positive numbers -> 0 - 999999999 valid\n");
    printf("  -a angle                  angle in 0 to 360 degrees\n");
    printf("  -h, --help                print this help and exit\n");
    printf("   Example to rotate Counter-Clockwise Fast\n %s -i /dev/ttyUSB0 -d CCW -s F -t -1", argv);
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

    if(argc < 9) {
        usage(argv[0]);
        exit(0);
    }

    char interface[256]= {};
    char direction[4]= {};
    char speed[2]= {};
    char steps[10]= {};
    char rotation_angle[4]= {};
    double rotate = 0.0;
    int rotate_steps=0;

//Some flags
    int t_option_specified=0;
    int a_option_specified=0;

    int c;
    const char* short_opt = "hi:d:s:t:a:";
    struct option  long_opt[] =
    {
        {"help",           no_argument,       NULL, 'h'},
        {"interface",      required_argument, NULL, 'i'},
        {"direction",      required_argument, NULL, 'd'},
        {"speed",          required_argument, NULL, 's'},
        {"steps",          required_argument, NULL, 't'},
        {"angle",          required_argument, NULL, 'a'},
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

        case 'd':
            printf("DIRECTION: \"%s\"\n", optarg);
            if(strlen(optarg)>sizeof(direction)) {
                printf("Invalid argument for direction\n");
                exit(-1);
            }
            snprintf(direction,sizeof(direction),"%s",optarg);
            break;

        case 's':
            printf("SPEED: \"%s\"\n", optarg);
            if(strlen(optarg)>sizeof(speed)) {
                printf("Invalid argument for speed\n");
                exit(-1);
            }
            snprintf(speed,sizeof(speed),"%s",optarg);
            break;

        case 't':
            t_option_specified=1;
            if(a_option_specified == 1)
            {
                printf("***Error: Cannot use t and a options simultaneously.***\n Use one or the other\n");
                usage(argv[0]);
                return(-3);
            }
            printf("STEPS: \"%s\"\n", optarg);
            if(strlen(optarg)>sizeof(steps)) {
                printf("Invalid argument for steps\n");
                exit(-1);
            }
            snprintf(steps,sizeof(steps),"%s",optarg);
            break;

        case 'a':
            a_option_specified=1;
            if(t_option_specified == 1)
            {
                printf("***Error: Cannot use t and a options simultaneously.***\n Use one or the other\n");
                usage(argv[0]);
                return(-3);
            }
            printf("ROTATION ANGLE: \"%s\"\n", optarg);
            if(strlen(optarg)>sizeof(rotation_angle)) {
                printf("Invalid argument for rotation angle\n");
                exit(-1);
            }
            rotate = (double)atof(optarg);
            if((rotate > 360.0)||(rotate < 1.0))
            {
                printf("Rotation value should be between 1 and 360 only\n");
                usage(argv[0]);
                return(-4);
            }
            //5.625 degrees is 64 steps
            //Hence, 360 degrees = (360/5.625)*64 = 4096 steps
            //
            rotate_steps =(int)(double(double(rotate/360.0)*4096));
            snprintf(steps,sizeof(steps),"%d",rotate_steps);
            break;

        case 'h':
            usage(argv[0]);
            return(0);

        case ':':
        case '?':
            fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
            return(-2);

        default:
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
    uart0_filestream = open(UART_PORT, O_RDWR | O_NOCTTY);		//Open in non blocking read/write mode
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

    cfmakeraw(&options);
    options.c_cflag &= ~PARENB;        // Make 8n1
    options.c_cflag &= ~CSTOPB;            // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |=  CS8;
    options.c_cflag &= ~CRTSCTS;           // Disable hardware flow control
    options.c_lflag = 0;              // no signaling chars, no echo, no canonical processing
    options.c_oflag = 0;              // no remapping, no delays
    options.c_cc[VMIN] = 0;               // 0 - read doesn't block, 1 - read blocks
    options.c_cc[VTIME] = 5;               // 0.5 seconds read timeout

    options.c_cflag     |=  CREAD | CLOCAL;                  // turn on READ & ignore ctrl lines
    options.c_iflag     &=  ~(IXON | IXOFF | IXANY);         // turn off s/w flow ctrl
    options.c_lflag     &=  ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    options.c_oflag     &=  ~OPOST;                          // make raw

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
    char checksum_sent_string[20]= {};

//Direction - Field 1
    int len = strlen(direction);
    for (i = 0; i < len; i++) {
        *p_tx_buffer++ = direction[i];
    }
    strncat(checksum_sent_string,direction,strlen(direction));
    *p_tx_buffer++ = ',';

//Speed - Field 2
    len = strlen(speed);
    for (i = 0; i < len; i++) {
        *p_tx_buffer++ = speed[i];
    }
    strncat(checksum_sent_string,speed,strlen(speed));
    *p_tx_buffer++ = ',';

//Steps - Field 3
    len = strlen(steps);
    for (i = 0; i < len; i++) {
        *p_tx_buffer++ = steps[i];
    }
    strncat(checksum_sent_string,steps,strlen(steps));
    *p_tx_buffer++ = ',';

//Checksum - Field 4

    unsigned short send_checksum = crc16_ccitt((const unsigned char*)checksum_sent_string,strlen(checksum_sent_string));
    char send_checksum_char[2];
    send_checksum_char[0] = (char)(send_checksum & 0xFF);
    send_checksum_char[1] = (char)((send_checksum >> 8) & 0xFF);
    printf("checksum %s\n",send_checksum_char);

    *p_tx_buffer++ = send_checksum_char[0];
    *p_tx_buffer++ = send_checksum_char[1];
    *p_tx_buffer++='\r';

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
            n_read = read( uart0_filestream, &buf, 1 );
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

 /*
    Copyright (C) 2013-2014  Matthieu Cargnelli

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

 /*
    Parts of the code were taken from: 
    arduino-serial-lib -- simple library for reading/writing serial ports
    2006-2013, Tod E. Kurt, http://todbot.com/blog/
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>   // for usleep()
#include <fcntl.h>    // File control definitions 
#include <errno.h>    // Error number definitions 
#include <termios.h>  // POSIX terminal control definitions 
#include <sys/ioctl.h>


// #define DEBUG

#include "arduino-serial-lib.h"

#define FAN_SPEED_LOWEST 0b1100  //3
#define FAN_SPEED_LOW 0b0010	//4
#define FAN_SPEED_MEDIUM 0b1010  //5
#define FAN_SPEED_HIGH 0b0110    //6
#define FAN_SPEED_HIGHEST 0b1110 //7
#define FAN_SPEED_AUTO 0b0101 	//10

#define SWING_LOWEST 0b10000000  //1
#define SWING_LOW 0b01000000	 //2
#define SWING_MEDIUM 0b11000000  //3
#define SWING_HIGH 0b00100000    //4
#define SWING_HIGHEST 0b10100000 //5
#define SWING_AUTO 0b11110000	 //15

#define MODE_COOL 0b1100
#define MODE_HEAT 0b0010
#define MODE_DRY 0b0100
#define MODE_AUTO 0b0000

#define ON 0b10000000
#define OFF 0b00000000

#define OPTION_POWERFUL 0b10000000
#define OPTION_QUIET 0b00000100
#define OPTION_NONE 0b00000000

#define POSITION_ON_OFF_MODE 5
#define POSITION_TEMPERATURE 6
#define POSITION_FLUX 8
#define POSITION_OPTION 13
#define POSITION_CHECKSUM 18

#define RECEIVER 1

#define SEND_STATUS 0
#define SEND_IR_COMMAND 1

#define PAYLOAD_SIZE 19

unsigned char reverse_byte(unsigned char x)
{
    static const unsigned char table[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };
    return table[x];
}

void print_binary(char c){
	int i;
	//printf("%d in binary = ",c);
	for ( i = 7 ; i >= 0 ; i--){
		printf("%d", (c >> i & 1));
	}
	//printf("\n");
}

void print_usage(char* application_name){
  fprintf(stderr, "Usage: %s [-q|-p] [-o ON/OFF] [-m mode] [-f fanspeed] [-s swing] [-u USB port] [-t target] [-r] [temperature]\n", application_name);
  fprintf(stderr, "Mode:\n\tAUTO\tCOOL\n\tHEAT\tDRY\n");
  fprintf(stderr, "-q: Quiet (force the fanspeed to 1), -p: Powerful\n");
  fprintf(stderr, "fanspeed:\n\t0 <=> AUTO (AUTO is valid)\n\t1 - 5 <=> Speed (increasing)\n");
  fprintf(stderr, "swing: \n\t0 <=> AUTO (AUTO is valid)\n\t1 - 5 <=> position (increasing height)\n");
  fprintf(stderr, "temperature: \n\t16 -> 30 (mandatory unless command is \"-o OFF\"\n");
  fprintf(stderr, "-t target: \n\tindicate the message recipient if distant (RF)\n");
  fprintf(stderr, "-r : \n\tindicate to the recipient to send a status report (RF)\n");
  exit(1);
}


int findserialport(char* dest){
    int i;
    FILE * file;
    char* filenames[] = {"/dev/ttyACM0","/dev/ttyACM1","/dev/ttyUSB0","/dev/ttyUSB1"};
    for (i = 0 ; i < 4 ; i++){
      if (file = fopen(filenames[i], "r"))
      {
	  fclose(file);
	  strcpy(dest,filenames[i]);
	  return 0;
      }
    }
    return -1;
}

int main(int argc, char** argv){

    int opt;
    uint8_t temp;
    uint8_t option = OPTION_NONE;
    uint8_t swing = SWING_AUTO; 
    uint8_t fan = FAN_SPEED_AUTO;
    uint8_t on_off = ON;
    uint8_t mode = MODE_AUTO;
    int tmp_value;
       
    const int buf_max = 256;

    int sender = 0x7A; // defaut
       
    int fd = -1;
    char serialport[250];
    char serialport_given = 0;
    int baudrate = 9600;  // default
    char quiet=0;
    char eolchar = '\n';
    int timeout = 5000;
    char buf[buf_max];
    int rc,n;
    int target = 0;
    int command = SEND_IR_COMMAND;
    
    while ((opt = getopt(argc, argv, "rqps:f:m:o:u:t:")) != -1) {
        switch (opt) {
	case 'o':
	  if (!strcmp(optarg,"ON"))
		 on_off = ON;
	    else if (!strcmp(optarg,"OFF"))
		 on_off = OFF;
	    else{
		printf("on/off %s unknown (ON or OFF expected), ON taken by default\n",optarg);
		on_off = ON;
	    }
	    break;
        case 'q':
            option = OPTION_QUIET;
            break;
        case 'p':
            option = OPTION_POWERFUL;
            break;
        case 'm':
	    //tmp_value = atoi(optarg);
	    if (!strcmp(optarg,"AUTO"))
		 mode = MODE_AUTO;
	    else if (!strcmp(optarg,"COOL"))
		 mode = MODE_COOL;
	    else if (!strcmp(optarg,"HEAT"))
		 mode = MODE_HEAT;
	    else if (!strcmp(optarg,"DRY"))
		mode = MODE_DRY;
	    else{
		printf("mode %s unknown, AUTO by default\n",optarg);
		mode = MODE_AUTO;
	    }
            break;
        case 'f':
	    if (!strcmp(optarg,"AUTO") || !strcmp(optarg,"0"))
		 fan = FAN_SPEED_AUTO;
	    else if (!strcmp(optarg,"1"))
		 fan = FAN_SPEED_LOWEST;
	    else if (!strcmp(optarg,"2"))
		 fan = FAN_SPEED_LOW;
	    else if (!strcmp(optarg,"3"))
		 fan = FAN_SPEED_MEDIUM;
	    else if (!strcmp(optarg,"4"))
		 fan = FAN_SPEED_HIGH;
	    else if (!strcmp(optarg,"5"))
		 fan = FAN_SPEED_HIGHEST;
	    else{
		printf("fan %s unknown, AUTO by default\n",optarg);
		fan = FAN_SPEED_AUTO;
	    }
	    break;
        case 's':
 	    if (!strcmp(optarg,"AUTO") || !strcmp(optarg,"0"))
		 swing = SWING_AUTO;
	    else if (!strcmp(optarg,"1"))
		 swing = SWING_LOWEST;
	    else if (!strcmp(optarg,"2"))
		 swing = SWING_LOW;
	    else if (!strcmp(optarg,"3"))
		 swing = SWING_MEDIUM;
	    else if (!strcmp(optarg,"4"))
		 swing = SWING_HIGH;
	    else if (!strcmp(optarg,"5"))
		 swing = SWING_HIGHEST;
	    else{
		printf("swing %s unknown, AUTO by default\n",optarg);
		swing = SWING_AUTO;
	    }
	    break;
 	case 'u':
	  strcpy(serialport, optarg);
	  serialport_given = 1;
	  break;
	case 't':
	  target = atoi(optarg);
	  break;
	case 'r':
	  command = SEND_STATUS; //report
	  break;
      default: /* '?' */
	    printf("argument %c unknown\n",opt);
            print_usage(argv[0]);
        }
    }

   if (optind >= argc && on_off != OFF && command != SEND_STATUS) {
	printf("Argument expected\n");  
        print_usage(argv[0]);
    }else if (optind < argc){
	temp = atoi (argv[optind]);
    }
    
    if (!serialport_given){
      findserialport(serialport);
    }
    

	char payload[PAYLOAD_SIZE] = {0b01000000, 0b00000100, 0b00000111, 0b00100000,
			  0b00000000, 0b00000000, 0b00000000, 0b00000001, 
			  0b00000000, 0b00000000, 0b00000000, 0b01100000, 
			  0b00000110, 0b00000000, 0b00000000, 0b00000001, 
			  0b00000000, 0b01100000, 0b00000000};
  


	int i, j;

	int message_size = 2 * PAYLOAD_SIZE;
	uint8_t message[message_size];
	char on_off_mode = on_off | mode;

	if (option == OPTION_QUIET){
		fan = FAN_SPEED_LOWEST;
	}
	char flux = swing  | fan; // 4 bits for the swing then 4 for the fan
	
  if (temp > 31)
	  temp = 31;
	if (temp < 16)
	  temp = 16;
	temp <<= 1;
	temp = reverse_byte(temp);
	
	
	/*******************************************
	 *  Modification of the payload template  *
	 *******************************************/
	
	payload[POSITION_TEMPERATURE]=temp;
	payload[POSITION_ON_OFF_MODE] = on_off_mode;
	payload[POSITION_FLUX] = flux;
	payload[POSITION_OPTION] = option;

	/* checksum calculation */
	
	int checksum = 0;
	for ( i = 0 ; i < PAYLOAD_SIZE ; i++ ){
	  checksum+=reverse_byte(payload[i]);
	}
	checksum = reverse_byte(checksum);
	checksum &= 0xFF; // force 1 byte
	
	// checksum insertion
	
	payload[POSITION_CHECKSUM] = checksum;

	// payload transmission
#ifdef DEBUG
	
	printf("payload = \n");
	printf(" ");
	for (i = 0 ; i < PAYLOAD_SIZE ; i++){
	  print_binary(payload[i]);
	  printf(" ");
	}
#endif

  message[0] = 'I';
  // turning binary payload into ASCII characters (HEX representation)
  for (i = 1, j = 0 ; i < 39 ; i+=2, j++){
  	message[i] = (payload[j] & 0xF0) >> 4;
  	if (message[i] < 10){
  	  message[i] += '0'; // ASCII
  	} else {
  	  message[i] += 'A' - 10;
  	}
  	message[i+1] = payload[j] & 0x0F;
  	if (message[i+1] < 10){
  	  message[i+1] += '0'; // ASCII
  	} else {
  	  message[i+1] += 'A' - 10;
  	}

//  	printf("0x%c%c ",message[i], message[i+1]);
  }

//	printf("\n");
//	printf("\nPreparing to send...\n");
	fd = serialport_init(serialport, baudrate);
	if( fd==-1 ){ 
    error("couldn't open port");
    return -1;
  }  
//	printf("opened port %s\n",serialport);
	serialport_flush(fd);
//        rc = serialport_writebyte(fd, 'I');
        rc = serialport_write(fd, message);
//	if (write(fd,message,message_size) != message_size){
//	      error("error writing message data");
//	      return 1;
//	}  
	
	serialport_read_until(fd, buf, '\n', buf_max, timeout);
	printf("%s\n",buf); // should be "OK" but could be an error message
	return 0;
}

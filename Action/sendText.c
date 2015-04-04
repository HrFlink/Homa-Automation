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

    Highly modified by Henrik Vestergaard
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

#define DEBUG

#define TEXTSIZE 16
#define MESSAGESIZE 18

#include "arduino-serial-lib.h"

void print_binary(char c){
	int i;
	//printf("%d in binary = ",c);
	for ( i = 7 ; i >= 0 ; i--){
		printf("%d", (c >> i & 1));
	}
	//printf("\n");
}

void print_usage(char* application_name){
  fprintf(stderr, "Usage: %s [-r Row] [-c Column] [-t Text]\n");
  fprintf(stderr, "Unused options will be set to zero\n");
  fprintf(stderr, "If no text, the line will be cleared\n");
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

    int row = 0;
    int col = 0;
    char text[TEXTSIZE] = "";
    int opt;
       
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

    while ((opt = getopt(argc, argv, "r:c:t:")) != -1) {
	int tmp = 0;
        switch (opt) {
        case 'r':
	    sscanf(optarg, "%d", &tmp);
	    if ((tmp >= 0) && (tmp < 2 ))
		 row = tmp;
	    else
		printf("Row out of range - set to zero\n");
	    break;
        case 'c':
            sscanf(optarg, "%d", &tmp);
            if ((tmp >= 0) && (tmp < 16 ))
                 col = tmp;
            else
                printf("Colomn out of range - set to zero\n");
            break;
        case 't':
	    sscanf(optarg, "%16[0-9:a-zA-Z ]", &text);
            break;
      default: /* '?' */
	    printf("argument %c unknown\n",opt);
            print_usage(argv[0]);
        }
    }

   if (optind > argc) {
	printf("Argument expected\n");  
        print_usage(argv[0]);
    }
    
	if (!serialport_given){
		findserialport(serialport);
	}

	int i, j, actual_message_size;
	char payload[MESSAGESIZE] = {0, 0, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32}; 
	actual_message_size = strlen(text);
	for (i = 0; i < TEXTSIZE ; i++) {
		if (i < actual_message_size) 
			payload[i+2] = text[i];
	} 



	uint8_t message[MESSAGESIZE*2];

	
	/*******************************************
	 *  Modification of the payload template  *
	 *******************************************/
	
	payload[0]=row;
	payload[1]=col;

	// payload transmission
#ifdef DEBUG	
	printf("payload = \n");
	for (i = 0 ; i < MESSAGESIZE ; i++){
	  print_binary(payload[i]);
	  printf(" ");
	}
#endif

  // turning binary payload into ASCII characters (HEX representation)
  for (i = 0, j = 0 ; i < MESSAGESIZE*2 ; i+=2, j++){
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
        rc = serialport_writebyte(fd, 'T');
        rc = serialport_write(fd, message);
//	printf("Done\n");
	
  serialport_read_until(fd, buf, '\n', buf_max, timeout);
  printf("%s\n",buf); // should be "OK" but could be an error message
  return 0;
}

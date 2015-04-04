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
  fprintf(stderr, "Usage: %s [-r Red] [-g Green] [-b Blue]\n");
  fprintf(stderr, "Unused options will be set to zero\n");
  exit(1);
}

/*
int serialport_read_until(int fd, char* buf, char until, int buf_max, int timeout)
{
    char b[1];  // read expects an array, so we give it a 1-byte array
    int i=0;
    do { 
        int n = read(fd, b, 1);  // read a char at a time
        if( n==-1) return -1;    // couldn't read
		if( n==0 ) {
            usleep( 1 * 1000 );  // wait 1 msec try again
            timeout--;
            continue;
        }
#ifdef SERIALPORTDEBUG  
        printf("serialport_read_until: i=%d, n=%d b='%c'\n",i,n,b[0]); // debug
#endif
        buf[i] = b[0]; 
        i++;
    } while( b[0] != until && i < buf_max && timeout>0 );

    buf[i] = 0;  // null terminate the string
    return 0;
}
*/

/*
int serialport_flush(int fd)
{
    //sleep(2); //required to make flush work, for some reason
    return tcflush(fd, TCIOFLUSH);
}
*/


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
    int colorR = 0;
    int colorG = 0;
    int colorB = 0;
       
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
    
    while ((opt = getopt(argc, argv, "r:g:b:")) != -1) {
	int tmp = 0;
        switch (opt) {
        case 'r':

	    sscanf(optarg, "%d", &tmp);
	    if ((tmp >= 0) && (tmp < 256 ))
		 colorR = tmp;
	    else
		printf("Red out of range\n");
	    break;
        case 'g':
            sscanf(optarg, "%d", &tmp);
            if ((tmp >= 0) && (tmp < 256 ))
                 colorG = tmp;
            else
                printf("Green out of range\n");
            break;
        case 'b':
            sscanf(optarg, "%d", &tmp);
            if ((tmp >= 0) && (tmp < 256 ))
                 colorB = tmp;
            else
                printf("Blue out of range\n");
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

	char payload[3] = {0, 0, 0}; 


	int i, j;

	uint8_t message[6];
	int message_size = 6;

	
	/*******************************************
	 *  Modification of the payload template  *
	 *******************************************/
	
	payload[0]=colorR;
	payload[1]=colorG;
	payload[2]=colorB;

	// payload transmission
	
	printf("payload = \n");
	for (i = 0 ; i < 3 ; i++){
	  print_binary(payload[i]);
	  printf(" ");
	}

  // turning binary payload into ASCII characters (HEX representation)
  for (i = 0, j = 0 ; i < 6 ; i+=2, j++){
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
  	printf("0x%c%c ",message[i], message[i+1]);
  }
  printf("\n");
	printf("\nPreparing to send...\n");
	fd = serialport_init(serialport, baudrate);
	if( fd==-1 ){ 
    error("couldn't open port");
    return -1;
  }
	printf("opened port %s\n",serialport);
	serialport_flush(fd);
  rc = serialport_writebyte(fd, 'C');
  for ( i = 0 ; i < 6 ; i++ ) {
      int n = serialport_writebyte(fd, message[i]);

  }
  printf("Done\n");
	
  serialport_read_until(fd, buf, '\n', buf_max, timeout);
  printf("response: %s\n",buf); // should be "OK" but could be an error message
  return 0;
}

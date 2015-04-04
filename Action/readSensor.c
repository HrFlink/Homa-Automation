 /*
    Copyright (C) 2015 Henrik Vestergaard

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

// #define DEBUG

void print_usage(char* application_name){
  fprintf(stderr, "Usage: %s [-t|-h]\n");
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
       
    int fd = -1;
    char serialport[250];
    char serialport_given = 0;
    int baudrate = 9600;  // default
    int timeout = 5000;
    int rc,n;
    char command = '0';
    const int buf_max = 256;
    char buf[buf_max];
    
    while ((opt = getopt(argc, argv, "th")) != -1) {
	int tmp = 0;
        switch (opt) {
        case 't':
		command = 't';
            break;
        case 'h':
		command = 'h';
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

//	printf("\nPreparing to send...\n");
	fd = serialport_init(serialport, baudrate);
	if( fd==-1 ){ 
	error("couldn't open port");
	return -1;
  }
//	printf("opened port %s\n",serialport);
	serialport_flush(fd);
  if (command != '0') {
	rc = serialport_writebyte(fd, command);
//	printf("Sending command %c\n", command);
  }
//  printf("Done\n");
	
  serialport_read_until(fd, buf, '\n', buf_max, timeout);
  printf("%s\n",buf);
  return 0;
}

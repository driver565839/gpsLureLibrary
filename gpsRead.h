/*
Copyright 2016 Netgate

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
Special thanks to Lunatic999 on StackOverflow for the serial read template.
*/

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <termios.h> 
#include <math.h>




int initport(int fd){
	//Initialize Serial Port. 
    int portstatus = 0;

    struct termios options;
    // Get the current options for the port
    tcgetattr(fd, &options);
    // Set the baud rates to 9600
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    // Enable the receiver and set local mode
    options.c_cflag |= (CLOCAL | CREAD);

    options.c_cflag &= ~PARENB;//Enable parity generation on output and parity checking for input.
    options.c_cflag &= ~CSTOPB;//Use 2 stop bits
    options.c_cflag &= ~CSIZE;//Set character size to flag CS8
    options.c_cflag |= CS8;
    options.c_cflag &= ~CRTSCTS;                            // disable hardware flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON XOFF (for transmit and receive)

    options.c_cc[VMIN] = 0;     //min carachters to be read
    options.c_cc[VTIME] = 0;    //Time to wait for data (tenths of seconds)

    //Set the new options for the port...
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &options)==-1)
    {
        perror("On tcsetattr:");
        portstatus = -1;
    }
    else
        portstatus = 1;


    return portstatus;
}

int open_port(void){
	//Opens Serial port for reading.
	//change /dev/ttyS4 to the desired port if needed.
    int fd; 
    fd = open("/dev/ttyS4", O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd == -1)
    {
        perror("open_port: Unable to open /dev/ttyS4 --- \n");
    }
    else
        fcntl(fd, F_SETFL, 0);
    return (fd);
}

void reader(char *gpsString){
	//Reads NMEA sentences from the serial port until a GGA sentence pops up with the location info.
	//Pass in a string and this function will overwrite it with the NMEA sentence.
    int serial_fd = open_port();

    if(serial_fd == -1)
        printf("Error opening serial port /dev/ttyS4 \n");
    else
    {
        if(initport(serial_fd) == -1)
        {
            printf("Error Initializing port");
            close(serial_fd);
        }

        sleep(.5);

		int condition = 1;//Set to zero once a GGA statement is read
		while(condition==1){
		char buffer[80];
			int n = read(serial_fd, buffer, sizeof(buffer));
			if (n < 0)
				fputs("read failed!\n", stderr);
			else
			{
				strcpy(gpsString,buffer);
			}
			if(strstr(gpsString,"GGA") != NULL){
				condition = 0;	
			}
			sleep(.5);
		}
        close(serial_fd);
			
    }
}

double getTime(){
	//Gets raw time in hhmmss.sss format. NOTE: if the hour is <10, the time will be hmmss.sss as the leading
	//zero is lopped off.
	char nmea[80];
	reader(nmea);
	char timeS[11];
	memcpy(timeS,&nmea[7],10);
	timeS[10] = '\0';
	//printf("%s \n",timeS);
	double time = atof(timeS);
	return time;
}

int getHour(){
	//Returns the hour as an int.
	int time = (int)getTime();
	int hour = time/10000;
	return hour;
}

int getMin(){
	//Returns the current minute as an int
	int time = (int)getTime();
	int min = time/100;
	min = min % 100;
	return min;
}

int getSec(){
	//Returns the current second as an int.
	int time = (int)getTime();
	int sec = time%100;
	return sec;	
}

int getMillis(){
	//Returns the milliseconds as an int. 
	double time = getTime() * 1000.0;
	int millis = ((int)time)%1000;
	return millis;
}

double getLat(){
	//Gets Latitude in dddmm.mmmm form. May contain trailing zeros.
	char nmea[80];
	reader(nmea);
	char latS[11];
	int i = 18;
	int len = 0;
	while(i<80){
		if(nmea[i] == ','){
			len = i-18;
			i = 81;}
		i++;
	}
	memcpy(latS,&nmea[18],len);
	latS[len] = '\0';
	//printf("%s \n",timeS);
	double lat = atof(latS);
	return lat;
}

double getLong(){
	//Gets Longtitute in dddmm.mmmm form. May contain trailing zeros.
	char nmea[80];
	reader(nmea);
	int i = 18;
	int count = 0;
	int start = 0;
	int len = 0;
	while(i<80){
		if(nmea[i] == ','){
			count++;}
		if(count == 2 && start == 0){
			start = i+1;}
		if(count == 3){
			len = i-start;
			i = 81;}
		i++;
	}
	char longS[11];
	memcpy(longS,&nmea[start],len);
	longS[len] = '\0';
	double lon = atof(longS);
	return lon;
}

char getNS(){
	//Gets the North/South heading. Returns N or S
	char nmea[80];
	reader(nmea);
	int i = 18;
	int start = 0;
	while(i<80){
		if(nmea[i] == ','){
			start = i+1;
			i = 81;
		}
		i++;
	}
	char NS = nmea[start];
	return NS;
}

char getEW(){
	//Gets the East/West heading. Returns E or W
	char nmea[80];
	reader(nmea);
	int i = 18;
	int count = 0;
	int start = 0;
	while(i<80){
		if(nmea[i] == ',')
			count++;
		if(count == 3){
			start = i+1;
			i = 81;}
		i++;
	}
	char EW = nmea[start];
	return EW;
}

int getFix(){
	//Returns the Fix code. 0: no fix. 1: GPS fix. 2: Differential GPS fix.
	char nmea[80];
	reader(nmea);
	int i = 18;
	int count = 0;
	int start = 0;
	while(i<80){
		if(nmea[i] == ',')
			count++;
		if(count == 4){
			start = i+1;
			i = 81;}
		i++;
	}
	int fix = (int)nmea[start]-48;
	return fix;	
}

int getSats(){
	//Returns the number of satellites available.
	char nmea[80];
	reader(nmea);
	int count = 0;
	int start = 0;
	int len = 0;
	int i = 18;
	while(i<80){
		if(nmea[i] == ','){
			count++;}
		if(count == 5 && start == 0){
			start = i+1;}
		if(count == 6){
			len = i-start;
			i = 81;}
		i++;
	}
	char sats[11];
	memcpy(sats,&nmea[start],len);
	sats[len] = '\0';
	int sat = atof(sats);
	return sat;
}

double getHDOP(){
	//Gets horizontal dilution of precision 
	char nmea[80];
	reader(nmea);
	int i = 18;
	int count = 0;
	int start = 0;
	int len = 0;
	while(i<80){
		if(nmea[i] == ','){
			count++;}
		if(count == 6 && start == 0){
			start = i+1;}
		if(count == 7){
			len = i-start;
			i = 81;}
		i++;
	}
	char hDOPS[11];
	memcpy(hDOPS,&nmea[start],len);
	hDOPS[len] = '\0';
	double hDOP = atof(hDOPS);
	return hDOP;
}

double getAlt(){
	//Gets Altitude  (use getAltUnit() to get the unit on it.)
	char nmea[80];
	reader(nmea);
	int i = 18;
	int count = 0;
	int start = 0;
	int len = 0;
	while(i<80){
		if(nmea[i] == ','){
			count++;}
		if(count == 7 && start == 0){
			start = i+1;}
		if(count == 8){
			len = i-start;
			i = 81;}
		i++;
	}
	char altS[11];
	memcpy(altS,&nmea[start],len);
	altS[len] = '\0';
	double alt = atof(altS);
	return alt;
}

char getAltUnit(){
	char nmea[80];
	reader(nmea);
	int i = 18;
	int count = 0;
	int start = 0;
	while(i<80){
		if(nmea[i] == ',')
			count++;
		if(count == 8){
			start = i+1;
			i = 81;}
		i++;
	}
	char altUnit = nmea[start];
	return altUnit;
}

double getGSep(){
	//Gets geoidal separation (use getGSepUnit() to get the unit!)
	char nmea[80];
	reader(nmea);
	int i = 18;
	int count = 0;
	int start = 0;
	int len = 0;
	while(i<80){
		if(nmea[i] == ','){
			count++;}
		if(count == 9 && start == 0){
			start = i+1;}
		if(count == 10){
			len = i-start;
			i = 81;}
		i++;
	}
	char gSepS[11];
	memcpy(gSepS,&nmea[start],len);
	gSepS[len] = '\0';
	double gSep = atof(gSepS);
	return gSep;
}

char getGSepUnit(){
	char nmea[80];
	reader(nmea);
	int i = 18;
	int count = 0;
	int start = 0;
	while(i<80){
		if(nmea[i] == ',')
			count++;
		if(count == 10){
			start = i+1;
			i = 81;}
		i++;
	}
	char gSepU = nmea[start];
	return gSepU;
}

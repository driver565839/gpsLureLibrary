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

#include <stdio.h>
#include "gpsRead.h"

int main(void){
	char rawNMEA[80];
	reader(rawNMEA);
	printf("%s \n",rawNMEA);
	printf("The time is: %.3lf \n",getTime());	
	printf("The hour is: %d \n",getHour());
	printf("The minute is: %d \n",getMin());
	printf("The second is: %d \n",getSec());
	printf("The millisecond is: %d \n",getMillis());
	printf("The latitude is: %.5lf \n",getLat());
	printf("The longitude is: %.5lf \n",getLong());
	printf("The current heading is %c%c \n",getNS(),getEW());
	int fix = getFix();
	if(fix == 0)
		printf("You have no fix. Fix code: %d",fix);
	else if(fix == 1)
		printf("You have a GPS fix. Fix code: %d",fix);
	else if(fix ==2)
		printf("You have a differential fix. Fid code: %d",fix);
	else
		printf("Oh man, looks like something went wrong...");
	printf("\nThere are currently %d satellites.\n",getSats());
	printf("Curent horizontal dilution of precision: %.5lf\n",getHDOP());
	printf("Current altitude is %.5lf%c \n",getAlt(),getAltUnit());
	printf("Geoidal Separation is: %.5lf%c \n",getGSep(),getGSepUnit());
	return 0;
}

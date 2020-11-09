#include "date.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#define BUF_LEN 256

/*
 * date.c
 *
 *  Created on: Nov 8, 2020
 *      Author: rigoangeles
 *
 *
 */


/*
 *  Returns current Date
 */
int getDate(char *date){
	time_t rawtime = time(NULL);
	if (rawtime == -1) {
		puts("The time() function failed");
		return 1;
	}
	struct tm *ptm = localtime(&rawtime);
	if (ptm == NULL) {
		puts("The localtime() function failed");
		return 1;
	}
	strftime(date, BUF_LEN, "%d/%m/%Y", ptm);
	return 0;
}

/*
 * Returns current time
 */

int getTime(char *curr_time){
	time_t rawtime_ = time(NULL);
	if (rawtime_ == -1) {
		puts("The time() function failed");
		return 1;
	}
	struct tm *p_tm = localtime(&rawtime_);
	if (p_tm == NULL) {
		puts("The localtime() function failed");
		return 1;
	}
	snprintf(curr_time, BUF_LEN, "%d:", p_tm->tm_hour);
	snprintf(curr_time + strlen(curr_time), BUF_LEN, "%02d:", p_tm->tm_min);
	snprintf(curr_time + strlen(curr_time), BUF_LEN, "%02d", p_tm->tm_sec);
	return 0;
}

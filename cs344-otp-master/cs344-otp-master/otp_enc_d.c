/***************************************************************
 * Filename: otp_enc_d.c
 * Author: Brent Irwin
 * Date: 7 June 2017
 * Description: Calls otp_d() and tells it to encode
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "otp.h"

int main(int argc, char *argv[]) {

	// Check arguments
	if (argc < 2) {
		fprintf(stderr,"USAGE: %s port\n", argv[0]);
		exit(1);
	}

	// Call otp_d() and tell it to encode
	otp_d(argv[1], "otp_enc");

	return 0;
}

/***************************************************************
 * Filename: otp.h
 * Author: Brent Irwin
 * Date: 1 June 2017
 * Description: Provides code[] array and function prototypes
***************************************************************/

#ifndef OTP_H
#define OTP_H

static const char code[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

void encode(char*, char*, char*, int);
void decode(char*, char*, char*, int);

int otp(char*, char*, char*, char*);
int otp_d(char*, char*);

#endif

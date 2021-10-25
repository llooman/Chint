#ifndef _Zon_H_
#define _Zon_H_

#define JL_VERSION 10200

#include "Arduino.h"   // pro mini optiboot COM9 // !!! DISCONNECT CHINT RS232  !!!
#include <inttypes.h>
#include <avr/wdt.h>
#include <MyTimers.h>
#include "EEUtil.h"

//#define DEBUG

#define NODE_ID 14

//#define SERIAL_CONSOLE 77
//#include <NetwSerial.h>
//NetwSerial console;

//#define SERIAL_PARENT
//#include <NetwSerial.h>
//NetwSerial parentNode;

#define TWI_PARENT
#include <NetwTWI.h>
NetwTWI parentNode ;

//#define SPI_PARENT
//#include <NetwSPI.h>
//NetwSPI parentNode(NODE_ID);

/****************************
 *
 * 
2020-07-28 refactor timers

2020-01-26 Change 3 bootcount

2017-05-05 TWI_FREQ 200000L

2017-02-13 TWI_FREQ 100000L
 	 	   NetwSerial
           replace Netw by NetwBase
           upgrade NetwI2C to NetwBase
		   optimize eeparm.loop


2017-01-15 HomeNetH.h: i2c op 50Khz
           new Parms

2017-01-12 add bootCount
           retry
		   HomeNetH.h 2017-01-10  refactor bootTimer
           new EEProm class replacing EEParms and Parms
		   System::ramFree()
		   set netw.netwTimer   moved to NETW class
		   HomeNetH.h 2017-01-10  netw.sendData retry on send error

2017-01-08 HomeNetH.h 2017-01-08 fix volatile

2017-01-07 add netw.execParms(cmd, id, value) in execParms
           add if(req->data.cmd != 21 || parms.parmsUpdateFlag==0 ) in loop.handle netw request
           move parms.loop(); before loop.handle netw request
           HomeNetH.h 2017-01-06

2017-01-07 TODO eeparms.exeParms  case 3: netw.netwTimer = millis() + (value * 1000); //  netwSleep

 the  chint from the rs232 !! the upload will fail when the chint is still connected
 // 6.0 2016-08-11 new Chint based on Ketel

 */

// #define TRACE_MSEC 3000
#define TRACE_SEC 3

#define CHINT_SAMPLE_SEC 60      	// sense temp every 1 min
//#define CHINT_SOURCE 	 0x0100  	// just an adress for the Arduino
//#define CHINT_DEST 		 0x0001     // just an adress for the Chint

//tell the Chint.h to trace for testing/debugging
#include "Chint.h"

// #include <ArdUtils.h>      // utilties

// ArdUtils util;                  // utilities

void (*resetFunc)(void) = 0;               //declare reset function @ address 0
void localSetVal(int id, long val);
int  upload(int id, long val, unsigned long timeStamp);
int  upload(int id, long val) ;
int  upload(int id);
int  uploadError(int id, long val);
int  handleParentReq( RxItem *rxItem) ;
int localRequest(RxItem *rxItem);
void trace();

Chint myChint(Serial);

#define TIMERS_COUNT 4
MyTimers myTimers(TIMERS_COUNT);
#define TIMER_TRACE 0
#define TIMER_UPLOAD_LED 1
#define TIMER_UPLOAD_ON_BOOT 2

int 	uploadOnBootCount=0;  

unsigned long 	prevMillis = 0;

// unsigned long 	timeStampTrace = TRACE_MSEC;
// bool		  	timeStampTraceOverflow = false;

//unsigned long todayTotalTimer = 0;

boolean logChintErrors = false;

#endif
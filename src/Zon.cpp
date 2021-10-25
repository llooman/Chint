#include <Zon.h>

//#define DEBUG


class EEParms: public EEUtil
{
public:
	long chk1  = 0x01020304;
	virtual ~EEParms(){}
	EEParms(){	}

    void setup(  ) //
    {
		if( readLong(offsetof(EEParms, chk1)) == chk1  )
    	{
    		readAll();
    		changed = false;
			#ifdef DEBUG
				Serial.println(F("EEUtil.readAll"));
			#endif
    	}
		else
		{
			bootCount=0;
		}
    }

	void loop()
	{
		if(changed)
		{
			#ifdef DEBUG
				Serial.println(F("Parms.loop.changed"));
			#endif
			write(offsetof(EEParms, chk1), chk1);
			EEUtil::writeAll();
			changed = false;
		}
	}
};

EEParms eeParms;

#ifdef SPI_CONSOLE
	ISR (SPI_STC_vect){	console.handleInterupt();}
#endif
#ifdef	TWI_PARENT
	ISR(TWI_vect){ parentNode.tw_int(); }
#endif
#ifdef SPI_PARENT
	ISR (SPI_STC_vect){parentNode.handleInterupt();}
#endif


void localSetVal(int id, long val)
{
	switch(id )
	{
	default:
		eeParms.setVal( id,  val);
		break;
	}
}

void nextUpload(int id){
	switch( id ){
 
		case 50: 	myTimers.nextTimer(TIMER_UPLOAD_LED);		break;
	}
}

int upload(int id)
{
	int ret = 0;
	nextUpload(id);

	switch( id )
	{
	case 8:
		upload(id, JL_VERSION );   
		break;

	#ifdef VOLTAGE_PIN
		case 11: parentNode.upload(id, vin.val, immediate);    		break;
	#endif

	case 54: myChint.upload(id);  	break;  // parentNode.upload(id, myChint.eTotal_f, immediate);
	case 55: myChint.upload(id); 	break; //parentNode.upload(id, myChint.eToday_f * 100.0 , immediate);
	case 56: myChint.upload(id);  	break;  //parentNode.upload(id, myChint.temp_f * 10.0 , immediate);
	case 57: myChint.upload(id);	break;  //parentNode.upload(id, myChint.power_l, immediate);

	case 50: // LED: LOW = on, HIGH = off 
		upload(id, !digitalRead(LED_BUILTIN) );
 
		break;

	default:
		if( 1==2
		 ||	parentNode.upload(id)>0
		 ||	eeParms.upload(id)>0
		){}
		break;
	}
	return ret;
}


int upload(int id, long val) { return upload(id, val, millis()); }
int upload(int id, long val, unsigned long timeStamp)
{
	nextUpload(id);
	return parentNode.txUpload(id, val, timeStamp);
}


int handleParentReq( RxItem *rxItem)  // cmd, to, parm1, parm2
{
	if( rxItem->data.msg.node==2 ||  rxItem->data.msg.node == parentNode.nodeId)
	{
		return localRequest(rxItem);
	}

	#ifdef DEBUG
		parentNode.debug("skip", rxItem);
	#endif

	return 0;
}


void setup()  // TODO
{
	wdt_reset();
	wdt_disable();
	wdt_enable(WDTO_8S);

	pinMode(LED_BUILTIN, OUTPUT);

	Serial.begin(9600);

	eeParms.setup();
	eeParms.onUpload(upload);

//	myChint.timerInterval = (long) CHINT_SAMPLE_SEC * 1000L;
	myChint.id = 55;
	myChint.onUpload(upload);

	#ifdef DEBUG
		//myChint.activate(false);
		Serial.println(F("Strt chint DEBUG t="));
		//    myChint.retCode     = 0;
		//    myChint.power_l       = 999;
		//    myChint.temp_f        = 88.0;
		//    myChint.eToday_f      = 7.7;
		//    myChint.eTotal_f         = 66;
	#else
		myChint.begin();
	#endif

	myChint.loop(3);  // pre-read values


	#ifdef SERIAL_CONSOLE
		console.nodeId = 77;
		console.isConsole = true;
		console.onReceive( localRequest );
		//console.setup(0, 115200);
	#endif

	int nodeId = NODE_ID;

	parentNode.onReceive( handleParentReq);
//	parentNode.onError(uploadError);
	parentNode.onUpload(upload);
	parentNode.nodeId = nodeId;
	parentNode.isParent = true;


	#ifdef SERIAL_PARENT
	//	delay(100);
	//	parentNode.setup(0,115200);
	#endif

	#ifdef TWI_PARENT
		parentNode.begin();
	#endif


	#ifdef NETW_PARENT_SPI
		bool isSPIMaster = false;
		parentNode.setup( SPI_PIN, isSPIMaster);
		parentNode.isParent = true;
	#endif

	myTimers.nextTimer(TIMER_TRACE, TRACE_SEC);
	myTimers.nextTimer(TIMER_UPLOAD_ON_BOOT, 0);
	

	wdt_reset();
	wdt_enable(WDTO_8S);
    digitalWrite(13,LOW);
}


int localRequest(RxItem *rxItem)
{
	#ifdef DEBUG
		parentNode.debug("local", rxItem);
	#endif

	int ret=0;

	switch (  rxItem->data.msg.cmd)
	{
	case 't': trace( ); break;
	case 'x': parentNode.tw_restart(); break;

	case 's':
	case 'S':
		localSetVal(rxItem->data.msg.id, rxItem->data.msg.val);
		upload(rxItem->data.msg.id);
		break;
	case 'r':
	case 'R':
		upload(rxItem->data.msg.id);
		break;

	case 'b':
		eeParms.resetBootCount();
 		upload(3);
		break;

	case 'B':
		wdt_enable(WDTO_15MS);
		while(1 == 1)
			delay(500);
		break;
	default:
		eeParms.handleRequest(rxItem);
		break;
	}

	return ret;
}


void loop()  // TODO
{
	wdt_reset();

	if(prevMillis > millis())
	{
		// eeParms.resetTimers();
		// myChint.resetTimers();
	}
	prevMillis = millis();



	#if defined(SERIAL_CONSOLE) || defined(SPI_CONSOLE)
		console.loop();
	#endif

	myChint.loop();

	parentNode.loop();

	eeParms.loop();


	if( parentNode.isReady() 
	 && ! parentNode.isTxBufFull()
	){
		if( myTimers.isTime(TIMER_UPLOAD_ON_BOOT)
		){
			switch( uploadOnBootCount )
			{
			case 1:
			    if(millis()<60000) upload(1);
				break;    // boottimerr

				#ifdef VOLTAGE_PIN
					case 3: upload(11); break;  	// Vin
				#endif		
		
				case 4:  upload(50); break;
				case 5:  upload(3); break; //readCount
				case 6:  upload(8); break;	// version 

			case 15: myTimers.timerOff(TIMER_UPLOAD_ON_BOOT); break;			
			}

			uploadOnBootCount++;
			myTimers.nextTimerMillis(TIMER_UPLOAD_ON_BOOT, TWI_SEND_ERROR_INTERVAL);
		}

		if( myTimers.isTime(TIMER_UPLOAD_LED))	 { upload(50);}
	}


	#ifdef DEBUG		
		if( myTimers.isTime(TIMER_TRACE)){ trace();}
	#endif
}


void trace( )
{
	myTimers.nextTimer(TIMER_TRACE, TRACE_SEC);
	Serial.print("@ ");
	unsigned long minutes = (millis()/60000)%60;
	Serial.print(minutes);Serial.print(".");Serial.print(int (millis()/1000)%60 );
	Serial.print(": ");

	Serial.println();

    parentNode.trace("pNd");

	#ifdef VOLTAGE_PIN
		vin.trace("vin");
	#endif

}

//#include <Arduino.h>
//#include <inttypes.h>

//#define Aref 2860.0    // in mV
//#define Aref 5000.0    // in mV

// WHEN ARef is connected THEN set analogReference to external before analogRead to prevent DAMAGE
//analogReference(INTERNAL );  // INTERNAL EXTERNAL  Mega only INTERNAL1V1    INTERNAL2V56


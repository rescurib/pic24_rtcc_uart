/*
 * File:   rtcc.c
 * Author: Rodolfo
 *
 * Created on 11 de febrero de 2021, 09:41 PM
 */


#include <xc.h>
#include "rtcc.h"

/*********************************************************************
 * Function: RTCCProcessEvents
 *
 * Preconditions: RTCCInit must be called before.
 *
 * Overview: The function grabs the current time from the RTCC and
 * translate it into strings.
 *
 * Input: None.
 *
 * Output: It update time and date strings  _time_str, _date_str,
 * and _time, _time_chk structures.
 *
 ********************************************************************/
void RTCCgrab(TMS *tm)
{
	// Process time object only if time is not being set
        RTCCPTRS _time; 
        RTCCPTRS _time_chk;  
    
		// Grab the time
		RCFGCALbits.RTCPTR = 3;			
		_time.prt11 = RTCVAL;
		_time.prt10 = RTCVAL;
		_time.prt01 = RTCVAL;
		_time.prt00 = RTCVAL;

		// Grab the time again 
		RCFGCALbits.RTCPTR = 3;			
		_time_chk.prt11 = RTCVAL;
		_time_chk.prt10 = RTCVAL;
		_time_chk.prt01 = RTCVAL;
		_time_chk.prt00 = RTCVAL;

		// Verify there is no roll-over
		if ((_time.prt00 == _time_chk.prt00) &&
			(_time.prt01 == _time_chk.prt01) &&
			(_time.prt10 == _time_chk.prt10) &&
			(_time.prt11 == _time_chk.prt11))
		{
			tm->sec = _time_chk.prt00&0x00FF;
	        tm->min = _time_chk.prt00>>8;
	        tm->hr  = _time_chk.prt01&0x00FF;
	        tm->wkd = _time_chk.prt01>>8; 
	        tm->day = _time_chk.prt10&0x00FF;
	        tm->mth = _time_chk.prt10>>8;
	        tm->yr  = _time_chk.prt11&0x00FF;
		}
	
}

/*********************************************************************
 * Function: RTCCInit
 *
 * Preconditions: RTCCInit must be called before.
 *
 * Overview: Enable the oscillator for the RTCC
 *
 * Input: None.
 *
 * Output: None.
 ********************************************************************/
void RTCCInit(void)
{
    // Unlock sequence must take place for RTCEN to be written
	RCFGCAL	= 0x0000;			
    RTCCUnlock();
    RCFGCALbits.RTCEN = 1;	// bit15
    
    //RTCC pin pad conected to RTCC second clock
	PADCFG1bits.RTSECSEL = 1;	
	RCFGCALbits.RTCOE = 1;		//RTCC Output Enable bit

	/* Enable the RTCC interrupt*/
	IFS3bits.RTCIF = 0;		//clear the RTCC interrupt flag
	IEC3bits.RTCIE = 1;		//enable the RTCC interrupt
}

/*********************************************************************
 * Function: RTCCSet
 *
 * Preconditions: None.
 *
 * Overview: 
 * The function upload time and date from _time_chk into clock.
 *
 * Input: _time_chk - structure containing time and date.
 *
 * Output: None.
 *
 ********************************************************************/
void RTCCSet(TMS *tm)
{
     RTCCPTRS _p;
     //-- Conversión BCD y agrupación
     _p.prt11 =  tm->yr;
     _p.prt10 = (tm->mth<<8) + tm->day;
     _p.prt01 = (tm->wkd<<8) + tm->hr;
     _p.prt00 = (tm->min<<8) + tm->sec;
    
	RTCCUnlock();				// Desbloquear RTCC
	
	RCFGCALbits.RTCPTR = 3;		
	RTCVAL = _p.prt11;	// Establecer año
	RTCVAL = _p.prt10;	// Establecer mes:día
	RTCVAL = _p.prt01;	// Establecer semana:hora
	RTCVAL = _p.prt00;	// Establecer min:seg

	RCFGCALbits.RTCWREN = 0;	// Bloquear RTCC
}


/*********************************************************************
 * Function: RTCCUnlock
 *
 * Preconditions: None.
 *
 * Overview: The function allows a writing into the clock registers.
 *
 * Input: None.
 *
 * Output: None.
 *
 ********************************************************************/
void RTCCUnlock()
{
	asm volatile("disi	#5");
	asm volatile("mov #0x55, w7");		// write 0x55 and 0xAA to
	asm volatile("mov w7, _NVMKEY"); 	//  NVMKEY to disable
	asm volatile("mov #0xAA, w8");		// 	write protection
	asm volatile("mov w8, _NVMKEY");
    asm volatile("bset _RCFGCAL, #13");	// set the RTCWREN bit
	asm volatile("nop");
	asm volatile("nop");
}

/*********************************************************************
 * Function: RTCCSetBinSec
 *
 * Preconditions: None.
 *
 * Overview: The function verifies setting seconds range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Seconds binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
/*********************************************************************
 * Function: RTCCALMSet
 *
 * Preconditions: None.
 *
 * Overview: 
 * The function upload time and date from _alarm into RTCC alarm.
 *
 * Input: _alarm - structure containing time and date.
 *
 * Output: None.
 *
 ********************************************************************/
void RTCCALMSet(void)
{
	RTCCUnlock();				// Unlock the RTCC
	while(RCFGCALbits.RTCSYNC==1);		//wait for RTCSYNC bit to become 0
	
	ALCFGRPTbits.ALRMEN		= 0;		//disable alarm to update it
	ALCFGRPTbits.ALRMPTR	= 2;  	 	//Point to Month/Day register		
	ALRMVAL = _alarm.prt10;				//load month & day	
	ALRMVAL = _alarm.prt01;				//load weekday & hour	
	ALRMVAL = _alarm.prt00;				//load minute & seconds

	ALCFGRPTbits.AMASK		= 2;		//alarm every 10 seconds
	ALCFGRPTbits.ARPT		= 0xff;		//alarm 255 times
	ALCFGRPTbits.CHIME		= 1;		//enable chime
    ALCFGRPTbits.ALRMEN		= 1;  	 	//enable the alarm

	RCFGCALbits.RTCWREN = 0;	// Lock the RTCC
}

void RTCCSetBinSec(TMS *tm,unsigned char Sec)
{
    if(Sec >= 60)  Sec = 0;
    tm->sec = mRTCCBin2Dec(Sec);
}

/*********************************************************************
 * Function: RTCCSetBinMin
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting minutes range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Minutes binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinMin(TMS *tm,unsigned char Min)
{
    if(Min >= 60)  Min = 0;
    tm->min = mRTCCBin2Dec(Min);
}

/*********************************************************************
 * Function: RTCCSetBinHour
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting hours range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Hours binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinHour(TMS *tm,unsigned char Hour)
{
    if(Hour >= 24) Hour = 0;
    tm->hr = mRTCCBin2Dec(Hour);
}

/*********************************************************************
 * Function: RTCCCalculateWeekDay
 *
 * Preconditions: 
 * Valid values of day, month and year must be presented in 
 * _time_chk structure.
 *
 * Overview: The function reads day, month and year from _time_chk and 
 * calculates week day. Than It writes result into _time_chk. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: _time_chk with valid values of day, month and year.
 *
 * Output: Zero based week day in _time_chk structure.
 *
 ********************************************************************/
void RTCCCalculateWeekDay(TMS *tm)
{
	const char MonthOffset[] =
	//jan feb mar apr may jun jul aug sep oct nov dec
	{   0,  3,  3,  6,  1,  4,  6,  2,  5,  0,  3,  5 };
	unsigned Year;
	unsigned Month;
	unsigned Day;
	unsigned Offset;
    // calculate week day 
    Year  = mRTCCDec2Bin(tm->yr);
    Month = mRTCCDec2Bin(tm->mth);
    Day   = mRTCCDec2Bin(tm->day);
    
    // 2000s century offset = 6 +
    // every year 365%7 = 1 day shift +
    // every leap year adds 1 day
    Offset = 6 + Year + Year/4;
    // Add month offset from table
    Offset += MonthOffset[Month-1];
    // Add day
    Offset += Day;

    // If it's a leap year and before March there's no additional day yet
    if((Year%4) == 0)
        if(Month < 3)
            Offset -= 1;
    
    // Week day is
    Offset %= 7;

    tm->wkd = Offset;
}

/*********************************************************************
 * Function: RTCCSetBinDay
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting day range, translates it
 * into BCD format and writes into _time_chk structure. To write the
 * structure into clock RTCCSet must be called.
 *
 * Input: Day binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinDay(TMS *tm,unsigned char Day)
{
	const char MonthDaymax[] =
	//jan feb mar apr may jun jul aug sep oct nov dec
	{  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned Daymax;
	unsigned Month;
	unsigned Year;

    Month = mRTCCDec2Bin(tm->mth);
    Year = mRTCCDec2Bin(tm->yr);

    Daymax = MonthDaymax[Month-1];

    // February has one day more for a leap year
    if(Month == 2)
    if( (Year%4) == 0)
        Daymax++;

    if(Day == 0) Day = Daymax;
    if(Day > Daymax) Day = 1;
    tm->day = mRTCCBin2Dec(Day);
}

/*********************************************************************
 * Function: RTCCSetBinMonth
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting month range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Month binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinMonth(TMS *tm,unsigned char Month)
{
    if(Month < 1) Month = 12;
    if(Month > 12) Month = 1;
    tm->mth = mRTCCBin2Dec(Month);
}

/*********************************************************************
 * Function: RTCCSetBinYear
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting year range, translates it
 * into BCD format and writes into _time_chk structure. To write the 
 * structure into clock RTCCSet must be called.
 *
 * Input: Year binary value.
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinYear(TMS *tm,unsigned char Year)
{
   if(Year >= 100)  Year = 0;
    tm->yr = mRTCCBin2Dec(Year);
    // Recheck day. Leap year influences to Feb 28/29.
    RTCCSetBinDay(tm,mRTCCDec2Bin(tm->day));
}

/*********************************************************************
 * Interrupt for RTCC Alarm.
 *
 ********************************************************************/
void __attribute__((interrupt, no_auto_psv)) _RTCCInterrupt(void)
{
	IFS3bits.RTCIF = 0;	
	
	__builtin_btg((unsigned int *)&LATA, 6);	// Toggle RA6
	
	// TO DO:
	// User Implementation. 
}

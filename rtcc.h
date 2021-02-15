/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */


#include <xc.h> // include processor files - each processor file is guarded.  


#define mRTCCDec2Bin(Dec) (10*(Dec>>4)+(Dec&0x0f))
#define mRTCCBin2Dec(Bin) (((Bin/10)<<4)|(Bin%10))

// Union to access rtcc registers
typedef union tagRTCC {
	struct {
		unsigned char sec;
		unsigned char min;
		unsigned char hr;
		unsigned char wkd;
		unsigned char day;
		unsigned char mth;
		unsigned char yr;
	};
	struct {
		unsigned int prt00;
		unsigned int prt01;
		unsigned int prt10;
		unsigned int prt11;
	};
} RTCC;

typedef struct tm      // Estructura de tiempo
{
   unsigned char sec;
   unsigned char min;
   unsigned char hr;
   unsigned char wkd;
   unsigned char day;
   unsigned char mth;
   unsigned char yr; 
} TMS;

typedef struct rtcptrs // Estructura de punteros (para lectura/escritura)
{
   unsigned int prt00;
   unsigned int prt01;
   unsigned int prt10;
   unsigned int prt11; 
} RTCCPTRS;


void RTCCgrab(TMS *tm);
void RTCCInit(void);
void RTCCSet(TMS *tm);
void RTCCUnlock(void);
//void RTCCALMSet(void);
void RTCCSetBinSec(TMS *tm,unsigned char );
void RTCCSetBinMin(TMS *tm,unsigned char );
void RTCCSetBinHour(TMS *tm,unsigned char );
void RTCCCalculateWeekDay(TMS *tm);
void RTCCSetBinDay(TMS *tm,unsigned char );
void RTCCSetBinMonth(TMS *tm,unsigned char );
void RTCCSetBinYear(TMS *tm,unsigned char );

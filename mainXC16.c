/*
 * File:   mainXC16.c
 * Author: Rodolfo
 *
 * Created on 6 de febrero de 2021, 07:47 PM
 */

//--- Bits de configuración ---
#pragma config POSCMOD = XT    // Oscilador primario en modo XT
#pragma config FNOSC = PRIPLL  // Oscilador primaro con PLL
#pragma config PLL96MHZ = ON   // PLL activado
#pragma config PLLDIV = DIV2   // Entrada de 8MHz

#pragma config RTCOSC = SOSC   // RTCC usa Osc secundario (SOSC))
#pragma config SOSCSEL = SOSC  // Pines de Oscilador Secundario en modo SOSC

#pragma config FWDTEN = OFF    // Timmer de Watchdog desactivado
#pragma config JTAGEN = OFF    // Puerto JTAG desactivado 
//-----------------------------

#include <p24FJ32GB002.h>
#include<stdio.h>
#include "xc.h"

#define FCY 16000000UL 

#include "libpic30.h"
#include "uart_pic24f.h"
#include "xprintf.h"
#include "rtcc.h"


//---- Variables globales ---
TMS tt;         // Estructura de tiempo (formato BCD)
char Line[256];	// Buffer de entrada de consola
//--------------------------

void gpio_init(void);
int setDate(char *dateString);
char checkDiv(char *p,char sym,unsigned char *err);

// Interrupción TMR1 (tik-tak)
void __attribute__((interrupt, auto_psv)) _T1Interrupt (void){
  _T1IF = 0;		// Limpiar bandera irq 
  LATBbits.LATB9 ^= 1;  // Toggle del LED 
}

int main(void) {
    
    //******* SETUP *******
    gpio_init();
    //--- Inicializar módulo UART
    uart_init(9600);
    __delay_ms(100);
    //---
    
    //--- Unir UART con funciones de consola
    xdev_in(uart_getc);		
	xdev_out(uart_putc);
    //----
    
    //--- Configuración RTCC
    OSCCONbits.SOSCEN = 1; // SOSC habilitado
    __delay_ms(250);       // Tiempo de estabilización 
    RTCCInit();
    //---
    
    //----- TMR1
    T1CON = 0x00;           // Detiene temporizador
    T1CONbits.TCS   = 1;    // Entrada por SOSC/T1CK (pag. 150) (32.768 kHz)
    T1CONbits.TCKPS = 0b00; // Prescaler 1:1  (pag. 130)
    TMR1 = 0x00;            //Limpia el regitro de temporizador
    PR1 = 32768;            //Carga el periodo de 1 segundo
    IPC0bits.T1IP = 0x01;   //Prioridad de interrupción
    IFS0bits.T1IF = 0;      //Limpia bandera de periodo
    IEC0bits.T1IE = 1;      //Habilita la interrupción por TMR1
    T1CONbits.TON = 1;      //Inicia TMR1 
    //-----------------
    //*******************
    
    //--- Variables locales ---
    int i;           // Iteradores
    char cmd_status; // Bandera de ejecución
    char *input_ptr; // Puntero para barrer la entrada de consola
    //-------------------------
    
    // Escribir fecha y hora en módulo RTCC. 
    setDate("02/14/21 07:31:22");
    
    //-- Inicia consola
    xputs("-- Consola en PIC24 --\n");
    xputs("Comandos\t| Descripcion\n");
    xputs("dt [OPCION]\t| Muestra fecha\n");
    xputs("   -s <fecha>\t| Establece fecha en fmt MM/DD/AA hh:mm:ss\n" );
    
    for (;;) {
        cmd_status = 1;
		xputc('>'); //prompt
		xgets(Line, sizeof Line); // Captura todo antes del enter (permite borrar)
		input_ptr = Line;
		switch (*input_ptr++) 
        {
            case 'd':
                switch (*input_ptr++)
                {
                    case 't': /*dt - Mostrar fecha*/
                        //ignorar espacios hasta encontrar prox. char.
                        while (*input_ptr == ' ') input_ptr++; 
                        switch(*input_ptr++)
                        {
                            case '\0':
                                RTCCgrab(&tt);
                                xprintf("%02x/%02x/%02x %02x:%02x:%02x\n",tt.mth,tt.day,tt.yr,
                                                                          tt.hr,tt.min,tt.sec);
                                cmd_status = 0; // Comando ejecutado correctamente
                                break;
                                
                            case '-': // opción
                                switch(*input_ptr++)
                                {
                                    case 's':
                                        //ignorar espacios hasta encontrar prox. char.
                                        while (*input_ptr == ' ') input_ptr++;
                                        if(( '/' < *(input_ptr) && *(input_ptr) < ':' ))
                                        {
                                           setDate(input_ptr);
                                           cmd_status = 0; // Comando ejecutado correctamente
                                        }
                                        else
                                            cmd_status = 1; // Error
                                        break;
                                }
                                break;
                        }    
                }
                break; // de case d:
        }
        
        if(cmd_status) xprintf("'%s' no es un comando valido\n",Line);
    }
     
    return 0;
}

void gpio_init(void)
{
    TRISBbits.TRISB7 = 0; // Tx
    TRISBbits.TRISB8 = 1; // Rx
    TRISBbits.TRISB9 = 0;  // B9 como salida (LED_STATUS) (18)   
    AD1PCFG = 0x1FFF;      // Canales ADC desactivados
    
    // Mapeo de pines para UART (ver Sec. 10.4, pag. 129)
    RPINR18bits.U1RXR = 8; // Rx -> RP8/RB8 (Tab. 10-2,UART1 Receive, pag. 130)
                           //               (Registro RPINR18, pag. 139)
    RPOR3bits.RP7R    = 3;  // Tx -> RP7/RB7 (Tab. 10-3, pag. 131)
                            // Los registros RPORX tienen asociados pines
                            // RP. Se debe buscar al registro que contenga
                            // al pin remapeable deseado y al cual se debe
                            // colocar el código de función de salida.
                            // (Registro RPOR3, pag. 143 )
}

int setDate(char *dateString)
{
    TMS ttm;
    volatile unsigned char mth=0,day=0,yr=0,hr=0,min=0,sec=0,err=0;
    volatile char *pt = dateString;
    
    for(;;)
    {
      while ( '/' < *pt && *pt < ':' ) // Mientras sea número (Mes) 
      {
         mth = ( mth * 10 ) + *pt - '0';
         pt++;
      }
      
      if(!checkDiv(pt,'/',&err)) break;
      pt++;
      
      while ( '/' < *pt && *pt < ':' ) // Mientras sea número (Día) 
      {
         day = ( day * 10 ) + *pt - '0';
         pt++;
      }
      
      if(!checkDiv(pt,'/',&err)) break;
      pt++;
      
      while ( '/' < *pt && *pt < ':' ) // Mientras sea número (Año) 
      {
         yr = ( yr * 10 ) + *pt - '0';
         pt++;
      }
      
      //ignorar espacios hasta encontrar prox. char.
      while (*pt == ' ') pt++; 
      
      while ( '/' < *pt && *pt < ':' ) // Mientras sea número (Horas) 
      {
         hr = ( hr * 10 ) + *pt - '0';
         pt++;
      }
      
      if (!checkDiv(pt,':',&err)) break;
      pt++;
              
      while ( '/' < *pt && *pt < ':' ) // Mientras sea número (Minutos) 
      {
         min = ( min * 10 ) + *pt - '0';
         pt++;
      }
      
      if (!checkDiv(pt,':',&err)) break;
      pt++;
      while ( '/' < *pt && *pt < ':' ) // Mientras sea número (Segundos) 
      {
         sec = ( sec * 10 ) + *pt - '0';
         pt++;
      }
      
      break;
    }
    
    if(!err) // Si la lectura fue correcta:
    {
      xprintf("Fecha actualizada: %02d/%02d/%02d %02d:%02d:%02d\n",mth,day,yr,
                                            hr,min,sec);  
      // Escribir fecha y hora en RTCC. 
      //  -- Valores
	  RTCCSetBinSec(&ttm,sec);    // Segundos
	  RTCCSetBinMin(&ttm,min);    // Minutos
      RTCCSetBinHour(&ttm,hr);    // Horas
      RTCCSetBinMonth(&ttm,mth);  // Mes (Siempre antes del día)
      RTCCSetBinDay(&ttm,day);    // Día
      RTCCSetBinYear(&ttm,yr);    // Año
	  RTCCCalculateWeekDay(&ttm);     // Calcular día de la semana
      
	 // Establecer cambios
	 RTCCSet(&ttm);
     return 0;
    }
    xputs("Formato incorrecto, la fecha no fue actualizada.\n");
    return 1;
}

char checkDiv(char *p, char sym,unsigned char* err)
{
    if(*p == sym && ( '/' < *(p+1) && *(p+1) < ':' ))
      {
        *err = 0;
        return 1;
      }
    *err = 1;
    return 0;
}

/*------------------------------------------------------------------------/
/  UART control module for PIC24F                          (C)ChaN, 2010
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

#include <p24FJ32GB002.h>
#include "pic24f.h"
#include "uart_pic24f.h"

#define BUFFER_SIZE 128

//#define _DI()	__asm__ volatile("disi #0x3FFF") //Deshabilitar todas las interrupciones
//#define _EI()	__asm__ volatile("disi #0") //Habilitar todas interupciones

//-- Estructuras de datos para UART --
static volatile int TxRun;	// Bndera de recepción activa
static volatile struct {
	int		ri, wi, ct;		// Read index, Write index, Data counter 
	unsigned char	buff[BUFFER_SIZE];	// FIFO buffer (Cola) 
} TxFIFO, RxFIFO;
//------------------------------------

//-- Interrupciones UART --
// Rutina de interrupción Rx
void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void){
	unsigned char d;
	int i;
	d = (unsigned char)U1RXREG;	// Tomar dato del registro Rx			
    IFS0bits.U1RXIF = 0;        // Bajar bandera de interrupción
	i = RxFIFO.ct;				// Número de bytes en el RxFIFO
	if(i < BUFFER_SIZE){		// No hacer nada si esta lleno
		RxFIFO.ct = ++i;
		i = RxFIFO.wi;
		RxFIFO.buff[i++] = d;	// Encolar dato 
		RxFIFO.wi = i % BUFFER_SIZE;	//Actualizar indice
	}
}

// Rutina de interrupción Tx
void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void){
	int i;
    IFS0bits.U1TXIF = 0;
	i = TxFIFO.ct;	//Indice del dato en la cola
	if(i){	// Hace envio si es diferente de cero
		TxFIFO.ct = --i;
		i = TxFIFO.ri;
		U1TXREG = TxFIFO.buff[i++];	// Envia byte 
		TxFIFO.ri = i % BUFFER_SIZE;// Actualiza indice
	} 
    else{			// Cola vacía
		TxRun = 0;	// Terminar transmisión
	}
}
//---------------------------

//-- Initializar modulo UART 
void uart_init (unsigned long BaudRate){
	//-- Desactivar interrupcines Rx/Tx 
    IEC0bits.U1RXIE = 0; // Pag. 82
    IEC0bits.U1TXIE = 0; // Pag 82

	//-- Inicializar UART1 (baja velocidad))
    U1MODEbits.BRGH = 0; // Modo de baja velocidad
	U1BRG = FCY / 16 / BaudRate - 1;
    U1MODEbits.UARTEN =1;
    U1STAbits.UTXEN = 1;

	// Limpiar FIFOs Tx/Rx 
	TxFIFO.ri = 0; TxFIFO.wi = 0; TxFIFO.ct = 0;
	RxFIFO.ri = 0; RxFIFO.wi = 0; RxFIFO.ct = 0;
	TxRun = 0;

	//-- Habilitar interrupciones Rx/Tx
    IEC0bits.U1RXIE = 1; // Pag. 82
    IEC0bits.U1TXIE = 1; // Pag 82
}

//-- Poner byte en el registro Rx
unsigned char uart_getc(void){
	unsigned char d;
	int i;

	while(!RxFIFO.ct);	//Esperar a que haya algo en el FIFO
    //-- Desencolar byte --
	i = RxFIFO.ri;	
	d = RxFIFO.buff[i++];
    //---------------------
	RxFIFO.ri = i % BUFFER_SIZE;
	_DI();
	RxFIFO.ct--;
	_EI();

	return d;
}

//-- Poner byte en el registro Tx
void uart_putc(unsigned char d){
	int i;
	while (TxFIFO.ct >= BUFFER_SIZE) ;// Esperar hasta que TxFIFO tenga espacio 
    
    //-- Poner dato en la cola de escritura -- 
	i = TxFIFO.wi;		
	TxFIFO.buff[i++] = d;
	TxFIFO.wi = i % BUFFER_SIZE; // actualizar indice de escritura
    //----------------------------------------
	_DI();
	TxFIFO.ct++;
	if (!TxRun) {// Si no se está tramitiendo...
		TxRun = 1;
        IFS0bits.U1TXIF = 1; //Forzar interrupción
	}
	_EI();
}





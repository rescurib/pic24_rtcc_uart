
//-- Inicializar modulo UART
void uart_init(unsigned long BaudRate);

//-- Poner byte en el registro Rx
unsigned char uart_getc(void);

//-- Poner byte en el registro Tx
void uart_putc(unsigned char d);




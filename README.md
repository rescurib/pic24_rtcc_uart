# Módulo RTCC y consola serial en PIC24
Este es un ejemplo utilizaremos el microcontrolador PIC24FJ32GB002 y el compilador XC16. Las páginas referenciadas en el código son de la datasheet del GB002.  Usaremos la librería de consola serial de Chan de la que hablado en [esta](http://stg-pepper.blogspot.com/2020/06/consola-serial-en-pic-en-xc16.html) entrada en mi blog. Para escribir la librería para utilizar el módulo RTCC me basé en los códigos de ejempo que se pueden encontrar en http://www.microchip.com/codeexamples . Para utilizar el módulo RTCC es necesario tener un cristal de 32.768 kHz como oscilador secundario. En este programa he agregado una interrupción de un segundo con SOSC para verificar el funcionamiento del oscilador secundario (que he descrito en esta [entrada](http://stg-pepper.blogspot.com/2021/02/interrupcion-por-tmr1-con-reloj.html) y en general las interrupciones por temporizador las trato en el [repo](https://github.com/rescurib/Curso_PIC24/tree/master/Lec_03) en lo que llevo hecho de mi curso de PIC24). Es importante tener los capacitores correctos para el SOSC (entre 5 y 15 pF):

<p align="center">
<img src="https://1.bp.blogspot.com/-MticAYgekk4/YCXLjYXcSbI/AAAAAAAACdk/vomJvE-5QVs1avuM1U3XreOyJItpiQTSQCLcBGAsYHQ/s471/PIC24_SOSC_TMR1.png" alt="alt text" width="300">
</p>

Para la documentación del módulo se hará referencia al documento [DS39696B](http://ww1.microchip.com/downloads/en/devicedoc/39696b.pdf) que la referencia general del módulo para la familia PIC24F. La parte que hace falta estudiar primero es el metodo de lectura escritura del registro RTCVAL en la página 15 ya que no es directo como en la mayoria de los SFR's sino que requiere del campo RTCPTR del registro RCFGCAL (pag. 4) para establecer puntero de inicio hacia los campos de valor (día, mes, año, etc). Este programa no implementa calibración (auto-ajustes contra los errores de pulsos de reloj), se debe consultar página 16.

### Estructura de tiempo
La estructura que almacena la infromación del tiempo tiene la siguiente forma:

```C
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
```
**IMPORTANTE**: Las funciones de rtcc.h llenan la estructura con valores en formato BCD. Para realizar conversiones se tienen los iguientes macros:
```C
#define mRTCCDec2Bin(Dec) (10*(Dec>>4)+(Dec&0x0f))
#define mRTCCBin2Dec(Bin) (((Bin/10)<<4)|(Bin%10))
```
### Aislamiento
Si están utilizando baterías para alimentar al PIC o van a estar realizando depuración es una buena práctica utilizar opto-acopladores. Esto también permite utilizar módulos de conversión USB-Serial que no tengan salida de 3.3v:
<p align="center">
<img src="https://1.bp.blogspot.com/-lQWnWBZZUMo/YCsBjH1lllI/AAAAAAAACdw/NCVMA_amUQsxbr3EgBlwm7riKhogTvU1wCLcBGAsYHQ/s586/Circuito_Optos.png" alt="alt text" width="500">
</p>

### PuTTY en Debian( y derivados)
```
$ sudo apt install putty
```
Después de instalar:
```
$ sudo adduser <usuario> dialout
```

Buscamos puertos seriales disponibles con:
```
$ ls /dev/tty*
```

#ifndef __PLATFORM_UART_H__
#define __PLATFORM_UART_H__



/* uart functions ,according to your platform modify the Macro function below */
/*
*  UART_READ:   read data from uart buffer
*  UART_WRITE:  send out data by uart
*  UART_PURGECOMM:  purge uart rx buffer 
*/


#include "SerialPort.h"
#define UART_READ(buf ,bytesToRead)        WinSerial_Read(buf ,bytesToRead) 
#define UART_WRITE(ppdata,bytesToWrite)		WinSerial_Write(ppdata,bytesToWrite) 
#define UART_PURGECOMM()						WinSerial_PurgeComm() 


















#endif


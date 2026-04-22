#ifndef SPI_H
#define SPI_H

unsigned char SPI_SendByte(unsigned char data);
unsigned char SPI_ReceiveByte(unsigned char *data,unsigned len);
unsigned char SPI_SendBytes(unsigned char *data , int len);

#endif // SPI_H

#include <main.h>
#include <stm32f1xx_hal.h>
#include <stdint.h>

extern SPI_HandleTypeDef hspi1;

/*--------------------------- Spi Driver ------------------------------------------*/

unsigned char SPI_SendByte(unsigned char data)
{
	unsigned char rcvData;


//	HAL_SPI_Transmit(&hspi3,(uint8_t*)&data,1,100);
	HAL_SPI_TransmitReceive(&hspi1,(uint8_t*)&data,(uint8_t*)&rcvData,1,1000);

	return rcvData;

}

unsigned char SPI_ReceiveByte(unsigned char *data,unsigned len)
{
	if(HAL_SPI_Receive_DMA(&hspi1, (uint8_t*)data, len)!=HAL_OK){
		Error_Handler();
	}

	while(HAL_SPI_GetState(&hspi1)!=HAL_SPI_STATE_READY){
	}

	return len;

}

unsigned char SPI_SendBytes(unsigned char *data , int len)
{

		if(HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)data, len)!=HAL_OK){
			Error_Handler();
		}

		while(HAL_SPI_GetState(&hspi1)!=HAL_SPI_STATE_READY){
		}

		return len;
}

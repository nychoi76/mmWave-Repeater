
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fpga_spi.h"


#define _API_SPI                   hspi2 

int fpga_spi_read(uint32_t addr, uint8_t *val_p);
void fpga_spi_write(uint32_t addr, const uint8_t val);

uint8_t	ApiSpiSend(uint8_t	Data) 
{ 
	uint8_t	ret; 
	HAL_SPI_TransmitReceive(&_API_SPI,&Data,&ret,1,100); 
	return ret;	 
} 

void fpga_spi_write(uint32_t addr, const uint8_t val)
{
	u8 i;
	
  HAL_GPIO_WritePin(API_SPI_CS_GPIO_PORT,API_SPI_CS_PIN,GPIO_PIN_RESET); 
  addr|=0x8000;			//Write cmd
  ApiSpiSend((addr & 0xFF00) >> 8); 
  ApiSpiSend(addr & 0xFF);
  ApiSpiSend(val);
 	HAL_GPIO_WritePin(API_SPI_CS_GPIO_PORT,API_SPI_CS_PIN,GPIO_PIN_SET); 	
}

int fpga_spi_read(uint32_t addr, uint8_t *val_p)
{
	u8 i;
	
  HAL_GPIO_WritePin(API_SPI_CS_GPIO_PORT,API_SPI_CS_PIN,GPIO_PIN_RESET); 
  ApiSpiSend((addr & 0xFF00) >> 8); 
  ApiSpiSend(addr & 0xFF);
  ApiSpiSend(0); 					
 	*val_p=ApiSpiSend(0);
 	HAL_GPIO_WritePin(API_SPI_CS_GPIO_PORT,API_SPI_CS_PIN,GPIO_PIN_SET); 		
 	
 	return 0;
}

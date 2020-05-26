/*
 * fpga_spi.h
 */

#ifndef FPGA_SPI_H_
#define FPGA_SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "main.h"

extern SPI_HandleTypeDef hspi2;

#define _API_SPI                   hspi2 

int fpga_spi_read(uint32_t addr, uint8_t *val_p);
void fpga_spi_write(uint32_t addr, const uint8_t val);

#ifdef __cplusplus
}
#endif
#endif /* FPGA_SPI_H_ */

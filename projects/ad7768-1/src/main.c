/***************************************************************************//**
* @file ad77681_evb.c
* @brief Implementation of Main Function.
* @author SPopa (stefan.popa@analog.com)
********************************************************************************
* Copyright 2019(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* - Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
* - Neither the name of Analog Devices, Inc. nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
* - The use of this software may or may not infringe the patent rights
* of one or more patent holders. This license does not release you
* from the requirement that you obtain separate licenses from these
* patent holders to use this software.
* - Use of the software either in source or binary form, must be run
* on or directly connected to an Analog Devices Inc. component.
*
* THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdio.h>
#include <sleep.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <xil_cache.h>
#include <xparameters.h>

#include "xil_printf.h"
#include "ad7768-1.h"
#include "spi_engine.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define AD77681_DMA_BASEADDR		XPAR_AXI_AD77681_DMA_BASEADDR
#define AD77681_SPI_ENGINE_BASEADDR	XPAR_SPI_ADC_AXI_REGMAP_BASEADDR
#define AD77681_SPI_CS			0
#define AD77681_DDR_BASEADDR		XPAR_DDR_MEM_BASEADDR + 0x800000

#define GPIO_DEVICE_ID			XPAR_PS7_GPIO_0_DEVICE_ID
#define GPIO_OFFSET			32

#define GPIO_0_0			GPIO_OFFSET + 6 // 38
#define GPIO_0_1			GPIO_OFFSET + 5 // 37
#define GPIO_0_2			GPIO_OFFSET + 4 // 36
#define GPIO_0_3			GPIO_OFFSET + 3 // 35
#define GPIO_0_SYNC_IN			GPIO_OFFSET + 2 // 34
#define GPIO_0_SYNC_OUT			GPIO_OFFSET + 1 // 33
#define GPIO_0_RESET			GPIO_OFFSET + 0 // 32

#define GPIO_1_0			GPIO_OFFSET + 22 // 54
#define GPIO_1_1			GPIO_OFFSET + 21 // 53
#define GPIO_1_2			GPIO_OFFSET + 20 // 52
#define GPIO_1_3			GPIO_OFFSET + 19 // 51
#define GPIO_1_SYNC_IN			GPIO_OFFSET + 18 // 50
#define GPIO_1_SYNC_OUT			GPIO_OFFSET + 17 // 49
#define GPIO_1_RESET			GPIO_OFFSET + 16 // 48

static struct spi_engine_init_param spi_engine_init_params = {
	.type = SPI_ENGINE,
	.spi_engine_baseaddr = AD77681_SPI_ENGINE_BASEADDR,
	.cs_delay = 0
};

static struct spi_init_param ad77681_spi_init_param = {
	.max_speed_hz = 1000000,
	.chip_select = AD77681_SPI_CS,
	.mode = SPI_MODE_3,
	.extra = &spi_engine_init_params
};

static struct ad77681_init_param init_params = {
	.spi_init_param = &ad77681_spi_init_param,
	.power_mode = AD77681_FAST,
	.mclk_div = AD77681_MCLK_DIV_8,
	.conv_mode = AD77681_CONV_CONTINUOUS,
	.diag_mux_sel = AD77681_POSITIVE_FS,
	.conv_diag_sel = false,
	.conv_len = AD77681_CONV_16BIT,
	.crc_sel = AD77681_NO_CRC,
	.status_bit = 0
};

#define SPI_ENGINE_OFFLOAD_EXAMPLE

#ifdef SPI_ENGINE_OFFLOAD_EXAMPLE

static struct spi_engine_offload_init_param spi_engine_offload_init_params = {
	.rx_dma_baseaddr = AD77681_DMA_BASEADDR,
	.tx_dma_baseaddr = AD77681_DMA_BASEADDR,
	.offload_config = OFFLOAD_TX_RX_EN
};

static uint32_t spi_engine_cmds[] = {
	CS_HIGH,
	CS_LOW,
	READ_WRITE(1),
	CS_HIGH
};
#endif	
//READ(4),

int main()
{
	struct ad77681_dev	*adc_dev;

	Xil_ICacheEnable();
	Xil_DCacheEnable();

	ad77681_setup(&adc_dev, init_params);

#ifndef SPI_ENGINE_OFFLOAD_EXAMPLE
	uint8_t	adc_data[3];
	uint8_t	i;

	while(1) 
	{
		ad77681_spi_read_adc_data(adc_dev, adc_data);

		xil_printf("\r\n[ADC DATA]: 0x");
		for(i = 0; i < sizeof(adc_data) / sizeof(uint8_t); i++)
			xil_printf("%02X", adc_data[i]);

		usleep(1000000);
	}
#else
	uint32_t rx_buf[20];
	uint32_t tx_buf = AD77681_REG_READ(AD77681_REG_ADC_DATA);
	uint32_t bursts = 8;

	struct spi_engine_offload_message msg ={
		.commands = spi_engine_cmds,
		.no_commands = ARRAY_SIZE(spi_engine_cmds),
		.tx_addr = (uint32_t)&tx_buf,
		.rx_addr = (uint32_t)&rx_buf
	};

	spi_engine_offload_init(adc_dev->spi_desc,
				&spi_engine_offload_init_params);
	
	spi_engine_offload_transfer(adc_dev->spi_desc, msg, bursts);


	// uint8_t 		*data;
	// } else { // offload example
	// 	msg = (spi_engine_msg *)malloc(sizeof(*msg));
	// 	if (!msg)
	// 		return -1;

	// 	msg->spi_msg_cmds = malloc(sizeof(spi_msg_cmds));
	// 	msg->spi_msg_cmds = spi_msg_cmds;
	// 	msg->rx_buf_addr = 0x800000;
	// 	msg->tx_buf_addr = 0xA000000;
	// 	msg->msg_cmd_len = sizeof(spi_msg_cmds) / sizeof(uint32_t);
	// 	msg->tx_buf[0] = AD77681_REG_READ(AD77681_REG_ADC_DATA);
	// 	msg->tx_buf[1] = 0x00;

	// 	spi_engine_offload_load_msg(adc_dev->spi_desc, msg);
	// 	spi_engine_transfer_multiple_msgs(adc_dev->spi_desc, 8);

	// 	data = (uint8_t*)adc_dev->spi_desc->rx_dma_startaddr;

	// 	usleep(10000000);

	// 	for(i = 0; i < adc_dev->spi_desc->rx_length; i++) {
	// 		printf("%x\r\n", *data);
	// 		data += sizeof(uint8_t);
	// 	}
	// 	free(msg);
	// }

	

#endif

	Xil_DCacheDisable();
	Xil_ICacheDisable();

	printf("Exiting program\n");
	
	return 0;
}

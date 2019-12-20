/*******************************************************************************
 *   @file   spi_engine_offload.c
 *   @brief  Implementation of SPI Engine offload feature.
 *   @author Sergiu Cuciurean (sergiu.cuciurean@analog.com)
********************************************************************************
 * Copyright 2019(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
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
 ******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdlib.h>
#include <sleep.h>
#include "error.h"
#include "spi_engine.h"
#include "spi_engine_offload.h"

// /******************************************************************************/
// /***************************** Static variables *******************************/
// /******************************************************************************/

static uint8_t _sync_id = 0x01;

/******************************************************************************/
/************************** Functions Implementation **************************/
/******************************************************************************/

/*******************************************************************************
 *
 * @name	spi_eng_offload_init
 *
 * @brief	Initialize the spi engine and offload system
 *
 * @param
 *		desc		- Spi engine descriptor
 *		param		- Structure containing the spi init parameters
 *
 * @return			- SUCCESS if the transfer finished
 *				- FAILURE if the memory allocation failed
 *
 ******************************************************************************/
int32_t spi_engine_offload_init(struct spi_desc **desc,
			      const struct spi_engine_offload_init_param *param)
{
	struct spi_engine_desc	*eng_desc;

	eng_desc = (*desc)->extra;

	eng_desc->rx_dma_baseaddr = param->rx_dma_baseaddr;
	eng_desc->tx_dma_baseaddr = param->tx_dma_baseaddr;
	eng_desc->offload_config = param->offload_config;
	
	return SUCCESS;
}

/*******************************************************************************
 *
 * @name	spi_eng_offload_load_msg
 *
 * @brief	Prepare the command queue of the offload mode
 *
 * @param
 *		desc		- Spi engine descriptor
 *		msg		- Structure used to store the messages
 *
 * @return
 *
 ******************************************************************************/
int32_t spi_engine_offload_write_and_read(struct spi_desc **desc,
					  struct spi_engine_offload_message msg)
{
	uint32_t 		i;
	uint8_t 		word_len;
	uint8_t 		words_number;
	
	struct spi_engine_msg	*transfer;
	struct spi_engine_desc	*eng_desc;

	eng_desc = (*desc)->extra;

	/* Check if offload is disabled */
	if(!(eng_desc->offload_config & (OFFLOAD_TX_EN | OFFLOAD_TX_EN)))
	{
		printf("\nOffload mode not configured.\r");
		
		return FAILURE;
	}

	/* Get the length of transfered word */
	word_len = spi_get_word_lenght(eng_desc);

	transfer = (spi_engine_msg *)malloc(sizeof(*transfer));
	if (!transfer)
		return FAILURE;

	transfer->cmds = (spi_engine_cmd_queue*)malloc(sizeof(*transfer->cmds));

	/* Write the command queue */
	for (i = 0; i < msg.no_commands; i++)
		spi_engine_write(eng_desc, SPI_ENGINE_REG_OFFLOAD_CMD_MEM(0),
			msg.commands[i]);
	
	//words_number = spi_get_words_number(eng_desc, eng_desc->tx_length);
	

	// spi_eng_compile_message(desc, msg, xfer);

	// /* Writhe the offload command queue */
	// for (i = 0; i < xfer->cmd_fifo_len; i++)
	// 	spi_eng_write(desc_extra,
	// 		      SPI_ENGINE_REG_OFFLOAD_CMD_MEM(0), xfer->cmd_fifo[i]);

	// words_number = spi_get_words_number(desc_extra, desc_extra->tx_length);
	// /* Writhe the SDO command queue */
	// for(i = 0; i < words_number; i++)
	// 	spi_eng_write(desc_extra,
	// 		      SPI_ENGINE_REG_OFFLOAD_SDO_MEM(0), msg->tx_buf[i]);

	// free(xfer);

	return SUCCESS;
}

// /*******************************************************************************
//  *
//  * @name	spi_eng_transfer_multiple_msgs
//  *
//  * @brief	Initiate an offload transfer
//  *
//  * @param
//  *		desc		- Spi engine descriptor
//  *		no_of_messages	- Number of messages to send in offload mode
//  *
//  * @return			- SUCCESS if the transfer finished
//  *				- FAILURE the offload mode isn't configured
//  *
//  ******************************************************************************/
// int32_t spi_eng_transfer_multiple_msgs(spi_desc *desc,
// 				       uint32_t no_of_messages)
// {
// 	uint8_t alignment;
// 	spi_desc_extra		*desc_extra;

// 	desc_extra = desc->extra;

// 	if(!desc_extra->offload_configured)
// 		return FAILURE;

// 	if (desc_extra->data_width > 16)
// 		alignment = sizeof(uint32_t);
// 	else
// 		alignment = sizeof(uint16_t);

// 	if(desc_extra->rx_length) {
// 		desc_extra->rx_length = alignment * no_of_messages;

// 		spi_eng_dma_write(desc_extra, DMAC_REG_CTRL, 0x0);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_CTRL, DMAC_CTRL_ENABLE);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_IRQ_MASK, 0x0);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_IRQ_PENDING, 0xff);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_DEST_ADDRESS,
// 				  desc_extra->rx_dma_startaddr);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_DEST_STRIDE, 0x0);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_X_LENGTH,
// 				  desc_extra->rx_length - 1);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_Y_LENGTH, 0x0);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_START_TRANSFER, 0x1);
// 	}

// 	if(desc_extra->tx_length) {
// 		desc_extra->tx_length = alignment * no_of_messages;

// 		spi_eng_dma_write(desc_extra, DMAC_REG_CTRL, 0x0);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_CTRL, DMAC_CTRL_ENABLE);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_IRQ_MASK, 0x0);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_IRQ_PENDING, 0xff);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_SRC_ADDRESS,
// 				  desc_extra->tx_dma_startaddr);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_SRC_STRIDE, 0x0);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_X_LENGTH,
// 				  desc_extra->rx_length - 1);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_Y_LENGTH, 0x0);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_FLAGS, 0x1);
// 		spi_eng_dma_write(desc_extra, DMAC_REG_START_TRANSFER, 0x1);
// 	}

// 	usleep(1000);
// 	/* Enable SPI engine */
// 	spi_eng_write(desc_extra, SPI_ENGINE_REG_OFFLOAD_CTRL(0), 0x0001);


// 	return SUCCESS;
// }
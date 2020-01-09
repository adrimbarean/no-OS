// /*******************************************************************************
//  *   @file   spi_engine.c
//  *   @brief  Core implementation of the SPI Engine Driver.
//  *   @author Sergiu Cuciurean (sergiu.cuciurean@analog.com)
// ********************************************************************************
//  * Copyright 2019(c) Analog Devices, Inc.
//  *
//  * All rights reserved.
//  *
//  * Redistribution and use in source and binary forms, with or without
//  * modification, are permitted provided that the following conditions are met:
//  *  - Redistributions of source code must retain the above copyright
//  *    notice, this list of conditions and the following disclaimer.
//  *  - Redistributions in binary form must reproduce the above copyright
//  *    notice, this list of conditions and the following disclaimer in
//  *    the documentation and/or other materials provided with the
//  *    distribution.
//  *  - Neither the name of Analog Devices, Inc. nor the names of its
//  *    contributors may be used to endorse or promote products derived
//  *    from this software without specific prior written permission.
//  *  - The use of this software may or may not infringe the patent rights
//  *    of one or more patent holders.  This license does not release you
//  *    from the requirement that you obtain separate licenses from these
//  *    patent holders to use this software.
//  *  - Use of the software either in source or binary form, must be run
//  *    on or directly connected to an Analog Devices Inc. component.
//  *
//  * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
//  * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
//  * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
//  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
//  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  ******************************************************************************/

#ifndef _SPI_ENGINE_
#define _SPI_ENGINE_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

/* In debug mode the printf function used in displaying the messages is causing
significant delays */
//#define DEBUG_LEVEL 0

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sleep.h>

#include "axi_io.h"
#include "error.h"
#include "spi_engine.h"

/******************************************************************************/
/***************************** Static variables *******************************/
/******************************************************************************/

static uint8_t _sync_id = 0x55;

/******************************************************************************/
/************************** Functions Implementation **************************/
/******************************************************************************/

 /**
  * @brief Write SPI Engine's axi registers
  *
  * @param desc Decriptor containing SPI Engine's parameters
  * @param reg_addr The address of the SPI Engine's axi register where the data
  * 	will be written
  * @param reg_data Data that will be written
  * @return int32_t This function allways returns SUCCESS
  */
 int32_t spi_engine_write(struct spi_engine_desc *desc,
 			  uint32_t reg_addr,
 			  uint32_t reg_data)
 {
 	dev_dbg(desc, "reg_addr = 0x%X; reg_data = 0x%X", reg_addr,
		reg_data);
 	axi_io_write(desc->spi_engine_baseaddr, reg_addr, reg_data);

 	return SUCCESS;
 }

/**
 * @brief Read SPI Engine's axi registers
 *
 * @param desc Decriptor containing SPI Engine's parameters
 * @param reg_addr The address of the SPI Engine's axi register from where the
 * 	data where the data will be read
 * @param reg_data Pointer where the read that will be stored
 * @return int32_t This function allways returns SUCCESS
 */
int32_t spi_engine_read(struct spi_engine_desc *desc,
			uint32_t reg_addr,
			uint32_t *reg_data)
{
	dev_dbg(desc, "reg_addr = 0x%X; reg_data = 0x%X", reg_addr,		*reg_data);
	axi_io_read(desc->spi_engine_baseaddr, reg_addr, reg_data);
	return SUCCESS;
}

/**
 * @brief Write SPI Engine's DMA axi registers
 * 
 * @param desc Decriptor containing SPI Engine's parameters
 * @param reg_addr The address of the SPI Engine's DMA axi register where the
 * 	data will be written
 * @param reg_data Data that will be written
 * @return int32_t This function allways returns SUCCESS
 */
int32_t spi_engine_dma_write(struct spi_engine_desc *desc,
			     uint32_t reg_addr,
			     uint32_t reg_data)
{
	dev_dbg(desc, "reg_addr = 0x%X; reg_data = 0x%X", reg_addr,
		reg_data);
	axi_io_write(desc->tx_dma_baseaddr, reg_addr, reg_data);

	return SUCCESS;
}

/**
 * @brief Read SPI Engine's DMA axi registers
 * 
 * @param desc Decriptor containing SPI Engine's parameters
 * @param reg_addr The address of the SPI Engine's DMA axi register from where
 * 	the data where the data will be read
 * @param reg_data Pointer where the read that will be stored
 * @return int32_t This function allways returns SUCCESS
 */
int32_t spi_engine_dma_read(struct spi_engine_desc *desc,
			    uint32_t reg_addr,
			    uint32_t *reg_data)
{
	dev_dbg(desc, "reg_addr = 0x%X; reg_data = 0x%X", reg_addr,
		*reg_data);
	axi_io_read(desc->rx_dma_baseaddr, reg_addr, reg_data);

	return SUCCESS;
}

/**
 * @brief Set width of the transfered word over SPI
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param data_wdith The desired data width
 * 	The supported values are:
 * 		- 8
 * 		- 16
 * 		- 24
 * 		- 32
 */
int32_t spi_engine_set_transfer_wdith(struct spi_desc *desc,
				      uint8_t data_wdith)
{
	struct spi_engine_desc	*desc_extra;

	desc_extra = desc->extra;

	if ((data_wdith * 8) > desc_extra->max_data_width)
	{
		desc_extra->data_width = desc_extra->max_data_width;
		dev_info(desc, "Data width (%d) exeeds the maximum of %d bytes",
			data_wdith, desc_extra->max_data_width);
	}
	else
	{
		dev_info(desc, "Data width set to %d bytes",data_wdith);
		desc_extra->data_width = data_wdith;
	}
	
	return SUCCESS;
}

/**
 * @brief Set the number of words transfered in a single transaction
 * 
 * @param desc Decriptor containing SPI Engine's parameters
 * @param bytes_number The number of bytes to be cnoverted
 * @return uint8_t A number of words in which bytes_number can be grouped
 */
uint8_t spi_get_words_number(struct spi_engine_desc *desc,
			     uint8_t bytes_number)
{
	uint8_t xfer_word_len;
	uint8_t words_number;
	
	xfer_word_len = desc->data_width / 8;
	words_number = bytes_number / xfer_word_len;

	if ((bytes_number % xfer_word_len) != 0)
		words_number++;

	return words_number;
}

/**
 * @brief Get the word lenght in bytes
 * 
 * @param desc Decriptor containing SPI Engine's parameters
 * @return uint8_t Number of bytes that fit in one word
 */
uint8_t spi_get_word_lenght(struct spi_engine_desc *desc)
{
	uint8_t word_lenght;

	word_lenght = desc->data_width / 8;

	return word_lenght;
}

/**
 * @brief  Compute the prescaler used to set the sleep period.
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param sleep_time_ns The amount of time where the transfer hangs
 * @param sleep_div Clock prescaler
 * @return int32_t This function allways returns SUCCESS
 */
int32_t spi_get_sleep_div(struct spi_desc *desc,
			  uint32_t sleep_time_ns,
			  uint32_t *sleep_div)
{
	struct spi_engine_desc	*eng_desc;

	eng_desc = desc->extra;

	/*
	 * Engine Wiki:
	 *
	 * The frequency of the SCLK signal is derived from the
	 * module clock frequency using the following formula:
	 * f_sclk = f_clk / ((div + 1) * 2)
	 */

	*sleep_div = (desc->max_speed_hz / 1000000 * sleep_time_ns / 1000) /
		     ((eng_desc->clk_div + 1) * 2) - 1;

	dev_dbg("sleep_div = 0x%X",sleep_div);

	return SUCCESS;
}

int32_t spi_engine_queue_new_cmd(struct spi_engine_cmd_queue **fifo,
				 uint32_t cmd)
{
	struct spi_engine_cmd_queue *local_fifo;

	local_fifo = (spi_engine_cmd_queue*)malloc(sizeof(*local_fifo));

	if(!local_fifo)
		return FAILURE;

	local_fifo->cmd = cmd;
	local_fifo->next = NULL;
	*fifo = local_fifo;

	return SUCCESS;
}

void spi_engine_queue_add_cmd(struct spi_engine_cmd_queue **fifo,
			      uint32_t cmd)
{
	struct spi_engine_cmd_queue *to_add = NULL;
	struct spi_engine_cmd_queue *local_fifo;

	local_fifo = *fifo;
	while (local_fifo->next != NULL)
	{
		// Get the last element
		local_fifo = local_fifo->next;
	}
	
	// Create a new element
	spi_engine_queue_new_cmd(&to_add, cmd);
	// Add as the next element
	local_fifo->next = to_add;
}

void spi_engine_queue_append_cmd(struct spi_engine_cmd_queue **fifo,
				 uint32_t cmd)
{
	struct spi_engine_cmd_queue *to_add = NULL;
	
	// Create a new element
	spi_engine_queue_new_cmd(&to_add, cmd);
	// Interchange the addresses
	to_add->next = *fifo;
	*fifo = to_add;
}

int32_t spi_engine_queue_get_cmd(struct spi_engine_cmd_queue **fifo,
				 uint32_t *cmd)
{
	struct spi_engine_cmd_queue *local_fifo;
	struct spi_engine_cmd_queue *last_fifo = NULL;

	local_fifo = *fifo;
	while (local_fifo->next != NULL)
	{
		// Get the last element
		last_fifo = local_fifo;
		local_fifo = local_fifo->next;
	}
	*cmd = local_fifo->cmd;
	if(last_fifo->next != NULL)
	{
		// Remove the last element
		free(last_fifo->next);
		last_fifo->next = NULL;
	}
	else
	{
		// Delete the fifo
		free(last_fifo);
	}
	
	return SUCCESS;
}

int32_t spi_engine_queue_fifo_get_cmd(struct spi_engine_cmd_queue **fifo,
				      uint32_t *cmd)
{
	if ((*fifo)->next)
	{
		*cmd = (*fifo)->cmd;
		*fifo = (*fifo)->next;
	}
	else
	{
		return FAILURE;
	}
	
	return SUCCESS;
}

int32_t spi_engine_queue_free(struct spi_engine_cmd_queue **fifo)
{
	if ((*fifo)->next)
		spi_engine_queue_free(&(*fifo)->next);
	free(*fifo);
	*fifo = NULL;

	return SUCCESS;
}

int32_t spi_engine_write_cmd_reg(struct spi_engine_desc *desc,
				  uint32_t cmd)
{
	int32_t ret;

	/* Check if offload is enabled */
	if(desc->offload_config & (OFFLOAD_TX_EN | OFFLOAD_TX_EN))
	{
		ret = spi_engine_write(desc, SPI_ENGINE_REG_OFFLOAD_CMD_MEM(0), cmd);

	}
	else
	{
		ret = spi_engine_write(desc, SPI_ENGINE_REG_CMD_FIFO, cmd);
	}

	return ret;
}

/**
 * @brief 
 * 
 * @param desc Decriptor containing SPI Engine's parameters
 * @param read_write Read/Write operation flag
 * @param bytes_number Number of bytes to transfer
 * @return int32_t This function allways returns SUCCESS
 */
int32_t spi_engine_transfer(struct spi_engine_desc *desc,
			    uint8_t read_write,
			    uint8_t bytes_number)
{
	uint8_t words_number;

	words_number = spi_get_words_number(desc, bytes_number);


	if(read_write & SPI_ENGINE_INSTRUCTION_TRANSFER_W)
	{
		desc->offload_tx_len += words_number;
	}

	/*
	 * Engine Wiki:
	 *
	 * https://wiki.analog.com/resources/fpga/peripherals/spi_engine
	 * 
	 * The words number is zero based
	 */
	
	spi_engine_write_cmd_reg(desc,
		SPI_ENGINE_CMD_TRANSFER(read_write, words_number  - 1));

	return SUCCESS;
}

/**
 * @brief Change the state of the chip select port
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param assert Chip select state.
 * 		 The supported values are :
 * 			-true (HIGH)
 * 			-false (LOW)
 */
void spi_engine_set_cs(struct spi_desc *desc,
		       bool assert)
{
	uint8_t			mask;
	struct spi_engine_desc	*eng_desc;

	eng_desc = desc->extra;

	/* Switch the state only of the selected chip select */
	if (assert)
	{
		mask = 0xFF | BIT(desc->chip_select);
	}
	else
	{
		mask = 0xFF & ~BIT(desc->chip_select);
	}

	spi_engine_write_cmd_reg(eng_desc,
		SPI_ENGINE_CMD_ASSERT(eng_desc->cs_delay, mask));
}

/**
 * @brief Add a delay bewtheen the engine commands
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param sleep_time_ns Number of nanoseconds to sleep between commands
 */
void spi_gen_sleep_ns(struct spi_desc *desc,
		      uint32_t sleep_time_ns)
{
	uint32_t 		sleep_div;
	struct spi_engine_desc	*eng_desc;

	eng_desc = desc->extra;

	spi_get_sleep_div(desc, sleep_time_ns, &sleep_div);
	
	dev_dbg(desc, "Sleep for %d ns", sleep_div);

	spi_engine_write_cmd_reg(eng_desc, SPI_ENGINE_CMD_SLEEP(sleep_div));
}

/**
 * @brief Spi engine command interpreter
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param cmd Command to send to the engine
 * @return int32_t - SUCCESS if the command is transfered
 *		   - FAILURE if the command format is invalid
 */
int32_t spi_engine_write_cmd(struct spi_desc *desc,
			     uint32_t cmd)
{
	uint8_t				engine_command;
	uint8_t				parameter;
	uint8_t				modifier;
	struct spi_engine_desc		*desc_extra;

	desc_extra = desc->extra;
	
	engine_command = (cmd >> 12) & 0x0F;
	modifier = (cmd >> 8) & 0x0F;
	parameter = cmd & 0xFF;

	switch(engine_command) {
	case SPI_ENGINE_INST_TRANSFER:
		dev_dbg(desc, "Transfer %d words", parameter);
		spi_engine_transfer(desc_extra, modifier, parameter);
		break;

	case SPI_ENGINE_INST_ASSERT:
		if(parameter == 0xFF)
		{
			dev_dbg(desc, "CS assert");
			/* Set the CS HIGH */
			spi_engine_set_cs(desc, true);
		}
		else if(parameter == 0x00)
		{
			dev_dbg(desc, "CS deassert");
			/* Set the CS LOW */
			spi_engine_set_cs(desc, false);
		}
		break;

	/* The SYNC and SLEEP commands got the same value but different
	modifier */
	case SPI_ENGINE_INST_SYNC_SLEEP:
		/* SYNC instruction */
		if(modifier == 0x00)
		{
			dev_dbg(desc, "Sync ID : 0x%X", parameter);
			spi_engine_write_cmd_reg(desc_extra, cmd);
		}
		else if(modifier == 0x01)
		{
			spi_gen_sleep_ns(desc, parameter);
		}
		break;
	case SPI_ENGINE_INST_CONFIG:
		switch(modifier) {
		case 0x00:
			dev_dbg(desc, "Prescaler set to 0x%X", parameter);
			break;
		case 0x01:
			dev_dbg(desc, "SPI mode set to 0x%X", parameter);
			break;
		case 0x02:
			dev_dbg(desc, "Transfer length set to 0x%X", parameter);
			break;
		}
		spi_engine_write_cmd_reg(desc_extra, cmd);

		break;

	default:
		dev_err(desc, "Invalid command");
		
		return FAILURE;
		break;
	}

	return SUCCESS;
}

/**
 * @brief Prepare the command queue before sending it to the engine
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param msg Structure used to store the transfer messages
 * @return int32_t This function allways returns SUCCESS
 */
int32_t spi_engine_compile_message(struct spi_desc *desc,
				   struct spi_engine_msg *msg)
{
	struct spi_engine_desc	*desc_extra;

	desc_extra = desc->extra;

	/* Set the data transfer length */
	spi_engine_queue_append_cmd(&msg->cmds,
		SPI_ENGINE_CMD_CONFIG(SPI_ENGINE_CMD_DATA_TRANSFER_LEN,
		desc_extra->data_width));
	/*
	 * Configure the spi mode :
	 *	- 3 wire
	 *	- CPOL
	 *	- CPHA
	 */
	spi_engine_queue_append_cmd(&msg->cmds,
		SPI_ENGINE_CMD_CONFIG(SPI_ENGINE_CMD_REG_CONFIG,
		desc->mode));

	/* Configure the prescaler */
	spi_engine_queue_append_cmd(&msg->cmds,
		SPI_ENGINE_CMD_CONFIG(SPI_ENGINE_CMD_REG_CLK_DIV,
		desc_extra->clk_div));

	/* Add a sync command to signal that the transfer has finished */
	spi_engine_queue_add_cmd(&msg->cmds, SPI_ENGINE_CMD_SYNC(_sync_id));

	return SUCCESS;
}

/**
 * @brief Initiate a spi transfer
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param msg Structure used to store the transfer messages
 * @return int32_t - SUCCESS if the transfer finished
 *		   - FAILURE if the memory allocation failed
 */
int32_t spi_engine_transfer_message(struct spi_desc *desc,
				    struct spi_engine_msg *msg)
{
	uint32_t		i;
	uint32_t		data;
	uint32_t		sync_id;
	bool 			offload_en;
	struct spi_engine_desc	*desc_extra;

	desc_extra = desc->extra;

	spi_engine_compile_message(desc, msg);
	offload_en = (desc_extra->offload_config & 
		     (OFFLOAD_TX_EN | OFFLOAD_TX_EN));

	/* Write the command fifo buffer */
	do
	{
		spi_engine_queue_fifo_get_cmd(&msg->cmds, &data);
		spi_engine_write_cmd(desc, data);
	}
	while(msg->cmds->next != NULL);
	/* Write the last command */
	spi_engine_queue_get_cmd(&msg->cmds, &data);
	spi_engine_write_cmd(desc, data);

	/* Write a number of tx_length WORDS on the SDO line */
	
	if(offload_en)
	{
		for(i = 0; i < desc_extra->offload_tx_len; i++)
		{
			spi_engine_write(desc_extra,
				SPI_ENGINE_REG_OFFLOAD_SDO_MEM(0),
				msg->tx_buf[i]);
		}
		
	}	
	else
	{
		for(i = 0; i < msg->length; i++)
		{
			spi_engine_write(desc_extra,
					 SPI_ENGINE_REG_SDO_DATA_FIFO,
					 msg->tx_buf[i]);
		}
		do {
			spi_engine_read(desc_extra,
					SPI_ENGINE_REG_SYNC_ID,
					&sync_id);
		}
		/* Wait for the end sync signal */
		while(sync_id != _sync_id);
		_sync_id = ~_sync_id;

		/* Read a number of rx_length WORDS from the SDI line and store
		them */
		for(i = 0; i < msg->length; i++)
		{
			spi_engine_read(desc_extra,
					SPI_ENGINE_REG_SDI_DATA_FIFO,
					&data);
			msg->rx_buf[i] = data;
		}
	}

	return SUCCESS;
}

/**
 * @brief Initialize the spi engine
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param param Structure containing the spi init parameters
 * @return int32_t - SUCCESS if the transfer finished
 *		   - FAILURE if the memory allocation failed
 */
int32_t spi_engine_init(struct spi_desc **desc,
			const struct spi_init_param *param)
{
	uint32_t			data_width;
	struct spi_desc			*local_desc;
	struct spi_engine_desc		*eng_desc;
	struct spi_engine_init_param	*spi_engine_init;

	local_desc = (struct spi_desc *)malloc(sizeof(*local_desc));
	eng_desc = (struct spi_engine_desc*)malloc(sizeof(*eng_desc));

	if (!desc || !eng_desc)
		return FAILURE;

	spi_engine_init = param->extra;

	local_desc->max_speed_hz = param->max_speed_hz;
	local_desc->chip_select = param->chip_select;
	local_desc->mode = param->mode;
	local_desc->extra = eng_desc;

	eng_desc->offload_config = OFFLOAD_DISABLED;
	eng_desc->spi_engine_baseaddr = spi_engine_init->spi_engine_baseaddr;
	eng_desc->type = spi_engine_init->type;
	eng_desc->cs_delay = spi_engine_init->cs_delay;
	eng_desc->clk_div =  SPI_ENGINE_MAX_SPEED_HZ /
		((2 * param->max_speed_hz) - 1);

	/* Perform a reset */
	spi_engine_write(eng_desc, SPI_ENGINE_REG_RESET, 0x01);
	usleep(1000);
	spi_engine_write(eng_desc, SPI_ENGINE_REG_RESET, 0x00);

	/* Get current data width */
	spi_engine_read(eng_desc, SPI_ENGINE_REG_DATA_WIDTH, &data_width);
	dev_info(desc, "The default transfer data with is 0x%X", data_width);
	eng_desc->max_data_width = data_width;
	eng_desc->data_width = data_width;

	*desc = local_desc;

	return SUCCESS;
}

/**
 * @brief Write/read on the spi interface
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @param data Pointer to data buffer
 * @param bytes_number Number of bytes to transfer
 * @return int32_t - SUCCESS if the transfer finished
 *		   - FAILURE if the memory allocation or transfer failed
 */
int32_t spi_engine_write_and_read(struct spi_desc *desc,
				  uint8_t *data,
				  uint8_t bytes_number)
{
	uint8_t 		i;
	uint8_t 		word_len;
	uint8_t 		words_number;
	int32_t 		ret;
	struct spi_engine_msg	*msg;
	struct spi_engine_desc	*desc_extra;

	desc_extra = desc->extra;

	words_number = spi_get_words_number(desc_extra, bytes_number);

	msg = (spi_engine_msg *)malloc(sizeof(*msg));
	msg->cmds = (spi_engine_cmd_queue*)malloc(sizeof(*msg->cmds));
	if (!msg->cmds || !msg)
		return FAILURE;
		
	msg->tx_buf =(uint32_t*)malloc(words_number * sizeof(msg->tx_buf[0]));
	msg->rx_buf =(uint32_t*)malloc(words_number * sizeof(msg->rx_buf[0]));
	msg->length = words_number;

	/* Init the rx and tx buffers with 0s */
	for (i = 0; i < words_number; i++) {
		msg->tx_buf[i] = 0;
		msg->rx_buf[i] = 0;
	}

	/* Get the length of transfered word */
	word_len = spi_get_word_lenght(desc_extra);

	/* Make sure the CS is HIGH before starting a transaction */
	msg->cmds->cmd = CS_HIGH;
	msg->cmds->next = NULL;
	spi_engine_queue_add_cmd(&msg->cmds, CS_LOW);
	spi_engine_queue_add_cmd(&msg->cmds, READ_WRITE(words_number));
	spi_engine_queue_add_cmd(&msg->cmds, CS_HIGH);

	/* Pack the bytes into engine WORDS */
	for (i = 0; i < bytes_number; i++)
		msg->tx_buf[i / word_len] |= data[i] << (desc_extra->data_width-
			(i % word_len + 1) * 8);

	ret = spi_engine_transfer_message(desc, msg);

	/* Skip the first byte ( dummy read byte ) */
	for (i = 1; i < bytes_number; i++)
		data[i - 1] = msg->rx_buf[(i) / word_len] >>
			(desc_extra->data_width - ((i) % word_len + 1) * 8);

	spi_engine_queue_free(&msg->cmds);
	free(msg->cmds);
	free(msg->tx_buf);
	free(msg->rx_buf);
	free(msg);

	return ret;
}

int32_t spi_engine_offload_init(struct spi_desc *desc,
			      const struct spi_engine_offload_init_param *param)
{
	struct spi_engine_desc	*eng_desc;

	eng_desc = desc->extra;

	eng_desc->rx_dma_baseaddr = param->rx_dma_baseaddr;
	eng_desc->tx_dma_baseaddr = param->tx_dma_baseaddr;
	eng_desc->offload_config = param->offload_config;
	
	return SUCCESS;
}

int32_t spi_engine_offload_transfer(struct spi_desc *desc,
				    struct spi_engine_offload_message msg,
				    uint32_t no_samples)
{
	uint32_t 		i;
	uint8_t 		word_len;
	uint8_t 		words_number;
	
	struct spi_engine_msg	*transfer;
	struct spi_engine_desc	*eng_desc;

	eng_desc = desc->extra;

	/* Check if offload is disabled */
	if(!(eng_desc->offload_config & (OFFLOAD_TX_EN | OFFLOAD_TX_EN)))
	{
		printf("\nOffload mode not configured.\r");
		
		return FAILURE;
	}

	eng_desc->offload_tx_len = 0;

	transfer = (spi_engine_msg *)malloc(sizeof(*transfer));
	transfer->cmds = (spi_engine_cmd_queue*)malloc(sizeof(*transfer->cmds));

	if (!transfer || !transfer->cmds)
		return FAILURE;

	/* Load the commands into the message */
	transfer->cmds->cmd = msg.commands[0];
	transfer->cmds->next = NULL;
	transfer->tx_buf = &msg.tx_addr;

	i = 1;

	while(i < msg.no_commands)
	{
		spi_engine_queue_add_cmd(&transfer->cmds, msg.commands[i++]);

	}

	spi_engine_transfer_message(desc, transfer);

	if(eng_desc->offload_config & OFFLOAD_TX_EN)
	{
		spi_engine_dma_write(eng_desc,
			DMAC_REG_CTRL, 0x0);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_CTRL, DMAC_CTRL_ENABLE);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_IRQ_MASK, 0x0);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_IRQ_PENDING, 0xff);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_SRC_ADDRESS, msg.tx_addr);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_SRC_STRIDE, 0x0);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_X_LENGTH, no_samples - 2);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_Y_LENGTH, 0x0);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_FLAGS, 0x1);

	}

	if(eng_desc->offload_config & OFFLOAD_RX_EN)
	{
		spi_engine_dma_write(eng_desc,
			DMAC_REG_CTRL, 0x0);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_CTRL, DMAC_CTRL_ENABLE);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_IRQ_MASK, 0x0);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_IRQ_PENDING, 0xff);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_DEST_ADDRESS, msg.rx_addr);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_DEST_STRIDE, 0x0);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_X_LENGTH, no_samples - 2);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_Y_LENGTH, 0x0);
		spi_engine_dma_write(eng_desc,
			DMAC_REG_START_TRANSFER, 0x1);
	}

	usleep(1000);

	/* Start transfer */
	spi_engine_write(eng_desc, SPI_ENGINE_REG_OFFLOAD_CTRL(0), 0x0001);

	spi_engine_queue_free(&transfer->cmds);
	free(transfer->cmds);
	free(transfer);


	return SUCCESS;
}

/**
 * @brief Free the resources allocated by spi_init().
 * 
 * @param desc Decriptor containing SPI interface parameters
 * @return int32_t This function allways returns SUCCESS
 */
int32_t spi_engine_remove(struct spi_desc *desc)
{
	free(desc->extra);
	free(desc);

	return SUCCESS;
} 

#endif //_SPI_ENGINE_

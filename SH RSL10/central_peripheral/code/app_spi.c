/*
 * app_spi.c
 *
 *  Created on: Oct 4, 2017
 *      Author: fg7fww
 */

#include "app.h"
void DMA0_IRQHandler(void)
{
	app_env.app_role_control = DIO->DATA & (1 << ((uint32_t) 8));
	switch ((uint32_t) SPI1->RX_DATA)
	{
		case DMA_SPI1R_DUMMY:
		    Sys_GPIO_Set_High(7);
		    Sys_DMA_ClearChannelStatus(DMA_SPI1R_NUM);
			Sys_DMA_ChannelEnable(DMA_SPI1R_NUM);
		    break;
		default:
 			if(app_env.app_role_control>0)
            {
 				cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_rx_value[0]=(uint8_t) spi1_rx[0];
 				cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_rx_value[1]=(uint8_t) spi1_rx[1];
 				cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_rx_value[2]=(uint8_t) spi1_rx[2];
 				cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_rx_value[3]=(uint8_t) spi1_rx[3];
 				cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_rx_value_changed=1;
            }
            else
            {
            	cs_env[DEVICE_NUM_CENTRAL].client.spi1_rx_value[0]=(uint8_t) spi1_rx[0];
            	cs_env[DEVICE_NUM_CENTRAL].client.spi1_rx_value[1]=(uint8_t) spi1_rx[1];
            	cs_env[DEVICE_NUM_CENTRAL].client.spi1_rx_value[2]=(uint8_t) spi1_rx[2];
            	cs_env[DEVICE_NUM_CENTRAL].client.spi1_rx_value[3]=(uint8_t) spi1_rx[3];
            	cs_env[DEVICE_NUM_CENTRAL].client.spi1_rx_value_changed=1;
            }
	    Sys_DMA_ClearChannelStatus(DMA_SPI1R_NUM);
	    Sys_DMA_ChannelEnable(DMA_SPI1R_NUM);
	}
}

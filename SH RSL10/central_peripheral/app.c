/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * This module is derived in part from example code provided by RivieraWaves
 * and as such the underlying code is the property of RivieraWaves [a member
 * of the CEVA, Inc. group of companies], together with additional code which
 * is the property of ON Semiconductor. The code (in whole or any part) may not
 * be redistributed in any form without prior written permission from
 * ON Semiconductor.
 *
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 * ----------------------------------------------------------------------------
 * app.c
 * - Main application file
 * ----------------------------------------------------------------------------
 * $Revision: 1.13 $
 * $Date: 2017/07/07 15:27:02 $
 * ------------------------------------------------------------------------- */

#include "app.h"

int main(void)
{
    App_Initialize();

    /* Main application loop:
     * - Run the kernel scheduler
     * - Update the battery voltage
     * - Update values for custom service server
     * - Refresh the watchdog and wait for an interrupt before continuing
     * - Send battery request and write request for a custom service attribute */
    app_env.app_role_control = DIO->DATA & (1 << ((uint32_t) 8));
    while (1)
    {
        Kernel_Schedule();

        /* Handle stack actions for connections where this device
         * is the peripheral. */
        if (ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_CONNECTED)
        {
            if (app_env.send_batt_ntf && bass_support_env.enable)
            {
                app_env.send_batt_ntf = 0;
                Batt_LevelUpdateSend(0, app_env.batt_lvl, 0);
            }

            /* Update custom service characteristics,
             * send notifications if notification is enabled */
            if ((cs_env[DEVICE_NUM_PERIPHERAL].server.tx_value_changed)||(cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_tx_value_changed))
            {
                cs_env[DEVICE_NUM_PERIPHERAL].server.tx_value_changed = 0;
                cs_env[DEVICE_NUM_PERIPHERAL].server.val_notif++;
                memset(cs_env[DEVICE_NUM_PERIPHERAL].server.tx_value,
                       cs_env[DEVICE_NUM_PERIPHERAL].server.val_notif,
                       CS_TX_VALUE_MAX_LENGTH);

                if (cs_env[DEVICE_NUM_PERIPHERAL].server.tx_cccd_value &
                    ATT_CCC_START_NTF)
                {
                    memset(cs_env[DEVICE_NUM_PERIPHERAL].server.tx_value,
                           cs_env[DEVICE_NUM_PERIPHERAL].server.val_notif,
                           CS_TX_VALUE_MAX_LENGTH);
                    if(cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_tx_value_changed)
                    {
                    	cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_tx_value_changed=false;
                        memset(cs_env[DEVICE_NUM_PERIPHERAL].server.tx_value,
                               cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_tx_value[0],
                               CS_TX_VALUE_MAX_LENGTH);
                    }
                    CustomService_SendNotification(ble_env[DEVICE_NUM_PERIPHERAL].conidx,
                                                    CS_IDX_TX_VALUE_VAL,
                                                    &cs_env[DEVICE_NUM_PERIPHERAL].server.tx_value[0],
                                                    CS_TX_VALUE_MAX_LENGTH);
                }

            }

            if (cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_rx_value_changed)
            {
                cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_rx_value_changed = 0;

                    CustomService_SendNotification(ble_env[DEVICE_NUM_PERIPHERAL].conidx,
                                                    CS_IDX_TX_VALUE_VAL,
                                                    &cs_env[DEVICE_NUM_PERIPHERAL].server.spi1_rx_value[0],
                                                    1);


            }
         }

        /* Handle stack actions for connections where this device is the central. */
        if (ble_env[DEVICE_NUM_CENTRAL].state == APPM_CONNECTED)
        {
            if((basc_support_env.enable == true) && (app_env.send_batt_req >= 25))
            {
                app_env.send_batt_req = 0;
                Batt_SendReadInfoReq(ble_env[DEVICE_NUM_CENTRAL].conidx, 0, BASC_BATT_LVL_VAL);
            }
/*
            if (cs_env[DEVICE_NUM_CENTRAL].client.state == CS_ALL_ATTS_DISCOVERED &&
                (app_env.wrt_cs_serv >= 10))
            {
                app_env.wrt_cs_serv = 0;
                cs_env[DEVICE_NUM_CENTRAL].client.val_notif++;
                memset(cs_env[DEVICE_NUM_CENTRAL].client.rx_value,
                       cs_env[DEVICE_NUM_CENTRAL].client.val_notif,
                       CS_RX_VALUE_MAX_LENGTH);

                CustomService_SendWrite(ble_env[DEVICE_NUM_CENTRAL].conidx,
                                       cs_env[DEVICE_NUM_CENTRAL].client.rx_value,
                                       cs_env[DEVICE_NUM_CENTRAL].client.disc_att[CS_IDX_RX_CHAR].pointer_hdl,
                                       0, CS_RX_VALUE_MAX_LENGTH,
                                       GATTC_WRITE);
            }
*/


            if (cs_env[DEVICE_NUM_CENTRAL].client.spi1_rx_value_changed)
            {
            	cs_env[DEVICE_NUM_CENTRAL].client.spi1_rx_value_changed=0;
                CustomService_SendWrite(ble_env[DEVICE_NUM_CENTRAL].conidx,
                						cs_env[DEVICE_NUM_CENTRAL].client.spi1_rx_value,
										cs_env[DEVICE_NUM_CENTRAL].client.disc_att[CS_IDX_RX_CHAR].pointer_hdl,
										0, CS_RX_VALUE_MAX_LENGTH,
										GATTC_WRITE_NO_RESPONSE);
            }

        }

        /* Refresh the watchdog timer */
        Sys_Watchdog_Refresh();

        /* Wait for an event before executing the scheduler again */
        SYS_WAIT_FOR_EVENT;
    }
}

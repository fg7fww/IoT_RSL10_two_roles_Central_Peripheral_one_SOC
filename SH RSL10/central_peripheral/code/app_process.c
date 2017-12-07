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
 *
 * ----------------------------------------------------------------------------
 * app_process.c
 * - Application task handler definition and support processes
 * ----------------------------------------------------------------------------
 * $Revision: 1.2 $
 * $Date: 2017/06/16 21:06:57 $
 * ------------------------------------------------------------------------- */

#include "app.h"

const struct ke_task_desc TASK_DESC_APP = { NULL, &appm_default_handler,
                                            appm_state, APPM_STATE_MAX,
                                            APP_IDX_MAX };

/* State and event handler definition */
const struct ke_msg_handler appm_default_state[] =
{
    /* Note: Put the default handler on top as this is used for handling any
     *       messages without a defined handler */
    { KE_MSG_DEFAULT_HANDLER, (ke_msg_func_t) Msg_Handler },
     BLE_MESSAGE_HANDLER_LIST,
     BASC_MESSAGE_HANDLER_LIST,
     BASS_MESSAGE_HANDLER_LIST,
     CS_MESSAGE_HANDLER_LIST,
     APP_MESSAGE_HANDLER_LIST
};

/* Use the state and event handler definition for all states. */
const struct ke_state_handler appm_default_handler
                        = KE_STATE_HANDLER(appm_default_state);

/* Defines a place holder for all task instance's state */
ke_state_t appm_state[APP_IDX_MAX];

/* ----------------------------------------------------------------------------
 * Function      : int APP_Timer(ke_msg_idd_t const msg_id,
*                                 void const *param,
 *                                ke_task_id_t const dest_id,
 *                                ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle timer event message
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter (unused)
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int APP_Timer(ke_msg_id_t const msg_id,
              void const *param,
              ke_task_id_t const dest_id,
              ke_task_id_t const src_id)
{
    uint16_t level;
    app_env.app_role_control = DIO->DATA & (1 << ((uint32_t) 8));
    uint8_t device_indx = DeviceIndx(KE_IDX_GET(src_id));

    /* Restart timer */
    ke_timer_set(APP_TEST_TIMER, TASK_APP, TIMER_200MS_SETTING);

    /* Turn on LED of EVB if the link is established */
    if((ble_env[DEVICE_NUM_CENTRAL].state == APPM_CONNECTED) || //&&
            (ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_CONNECTED))
    {
        Sys_GPIO_Set_High(LED_DIO_NUM);
    }
    else if((ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_READY)&&
    		(app_env.app_role_control > 0))
    {
    	Advertising_Start();
    }
    /*
    else if((app_env.app_role_control == 0)&&
    		(ble_env[DEVICE_NUM_CENTRAL].state > APPM_READY)&&
    		//(ble_env[DEVICE_NUM_CENTRAL].state != APPM_CONNECTING)&&
    		(ble_env[DEVICE_NUM_CENTRAL].state != APPM_CONNECTED)
    		)
    {
    	Sys_GPIO_Toggle(LED_DIO_NUM);
    	//Sys_BootROM_Reset();
    	//Connection_SendStartCmd();
    }
    */
    else if((ble_env[DEVICE_NUM_CENTRAL].state == APPM_READY)&&
    		(app_env.app_role_control == 0))
    {
    	//Connection_Disconnect(src_id);
    	Connection_SendStartCmd();
    }
    else if((ble_env[DEVICE_NUM_CENTRAL].state == APPM_CONNECTING) ||
            (ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_ADVERTISING))
    {
        //Sys_GPIO_Toggle(LED_DIO_NUM);
        // Testing
        if((device_indx==DEVICE_NUM_MAX) 	// Not connected
        &&(app_env.app_role_control == 0)	// Only for client
        &&(DIO->DATA & (1 << ((uint32_t) LED_DIO_NUM)))	// LED original functionality
        		)
        {
        	//Connection_SendStartCmd();
        	Sys_BootROM_Reset();	// This is working, devices are immediately self connected.
        							// But it is whole client system reinitialisation
        }
        Sys_GPIO_Toggle(LED_DIO_NUM);
    }
    else
    {
        Sys_GPIO_Set_Low(LED_DIO_NUM);
    }

    /* Every six seconds report that the custom service TX value changed
     * (notification simulation) */
    cs_env[DEVICE_NUM_PERIPHERAL].server.cnt_notifc++;
    if(cs_env[DEVICE_NUM_PERIPHERAL].server.cnt_notifc == 30)
    {
        cs_env[DEVICE_NUM_PERIPHERAL].server.cnt_notifc = 0;
        cs_env[DEVICE_NUM_PERIPHERAL].server.tx_value_changed = 1;
    }

    /* Calculate the battery level as a percentage, scaling the battery
     * voltage between 1.4V (max) and 1.1V (min) */
    level = ((ADC->DATA_TRIM_CH[0] - VBAT_1p1V_MEASURED) * 100
             / (VBAT_1p4V_MEASURED - VBAT_1p1V_MEASURED));
    level = ((level >= 100) ? 100 : level);

    /* Add to the current sum and increment the number of reads,
     * calculating the average over 16 voltage reads */
    app_env.sum_batt_lvl += level;
    app_env.num_batt_read++;
    if(app_env.num_batt_read == 16)
    {
        if((app_env.sum_batt_lvl >> 4) != app_env.batt_lvl)
        {
            app_env.send_batt_ntf = 1;
        }

        if(ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_CONNECTED &&
           bass_support_env.enable)
        {
            app_env.batt_lvl = (app_env.sum_batt_lvl >> 4);
        }

        app_env.num_batt_read = 0;
        app_env.sum_batt_lvl = 0;
    }

    app_env.send_batt_req++;
    app_env.wrt_cs_serv++;

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int Msg_Handler(ke_msg_id_t const msg_id,
 *                                 void const *param,
 *                                 ke_task_id_t const dest_id,
 *                                 ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle any message received from kernel that doesn't have
 *                 a dedicated handler
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameter (unused)
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Msg_Handler(ke_msg_id_t const msg_id, void *param,
                ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    return (KE_MSG_CONSUMED);
}

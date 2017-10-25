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
 * ble_std.c
 * - Bluetooth standard functions and message handlers
 * ----------------------------------------------------------------------------
 * $Revision: 1.5 $
 * $Date: 2017/07/26 20:46:51 $
 * ------------------------------------------------------------------------- */

#include "app.h"

/* Bluetooth Environment Structure */
struct ble_env_tag ble_env[2];

/* List of functions used to create the database */
const appm_add_svc_func_t appm_add_svc_func_list[] =
                          {SERVICE_ADD_FUNCTION_LIST, NULL };

/* List of functions used to enable client services */
const appm_enable_svc_func_t appm_enable_svc_func_list[] =
                             {SERVICE_ENABLE_FUNCTION_LIST, NULL };

/* Bluetooth Device Address */
uint8_t bdaddr[BDADDR_LENGTH];
uint8_t bdaddr_type;

static struct gapm_set_dev_config_cmd *gapmConfigCmd;

/* ----------------------------------------------------------------------------
 * Standard Functions
 * ------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------
 * Function      : void BLE_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the BLE baseband and application manager
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void BLE_Initialize(void)
{
    struct gapm_reset_cmd* cmd;
    uint8_t *ptr = (uint8_t *) DEVICE_INFO_BLUETOOTH_ADDR;
    uint8_t tmp[2][BDADDR_LENGTH] = { {0xff,0xff,0xff,0xff,0xff,0xff},
                                      {0x00,0x00,0x00,0x00,0x00,0x00} };
    uint8_t default_addr[BDADDR_LENGTH] = PRIVATE_BDADDR;
    uint8_t default_addr_central[BDADDR_LENGTH] = PRIVATE_BDADDR_CENTRAL;

    /* Seed the random number generator */
    srand(1);

    /* Initialize the kernel and Bluetooth stack */
    Kernel_Init(0);
    BLE_InitNoTL(0);
    BLE_Reset();

    /* Enable the Bluetooth related interrupts needed */
    NVIC_EnableIRQ(BLE_EVENT_IRQn);
    NVIC_EnableIRQ(BLE_RX_IRQn);
    NVIC_EnableIRQ(BLE_CRYPT_IRQn);
    NVIC_EnableIRQ(BLE_ERROR_IRQn);
    NVIC_EnableIRQ(BLE_SW_IRQn);
    NVIC_EnableIRQ(BLE_GROSSTGTIM_IRQn);
    NVIC_EnableIRQ(BLE_FINETGTIM_IRQn);
    NVIC_EnableIRQ(BLE_CSCNT_IRQn);
    NVIC_EnableIRQ(BLE_SLP_IRQn);

    /* Reset the Bluetooth environment */
    memset(&ble_env, 0, sizeof(ble_env));

    /* Initialize task state */
    ble_env[DEVICE_NUM_PERIPHERAL].state = APPM_INIT;
    ble_env[DEVICE_NUM_CENTRAL].state = APPM_INIT;

    ble_env[DEVICE_NUM_PERIPHERAL].conidx = GAP_INVALID_CONIDX;
    ble_env[DEVICE_NUM_CENTRAL].conidx = GAP_INVALID_CONIDX;

    /* Use the device's public address if an address is available at
     * DEVICE_INFO_BLUETOOTH_ADDR (located in NVR3). If this address is
     * not defined (all ones) use a pre-defined private address for this
     * application */
/*    if(memcmp(ptr, &tmp[0][0] , BDADDR_LENGTH) == 0 ||
       memcmp(ptr, &tmp[1][0] , BDADDR_LENGTH) == 0)
    {
 */   	if(app_env.app_role_control>0)
    	{
    		memcpy(bdaddr, default_addr, sizeof(uint8_t) * BDADDR_LENGTH);
    	}
    	else
    	{
    		memcpy(bdaddr, default_addr_central, sizeof(uint8_t) * BDADDR_LENGTH);
    	}
        bdaddr_type = GAPM_CFG_ADDR_PRIVATE;
 /*   }
    else
    {
        memcpy(bdaddr, ptr, sizeof(uint8_t) * BDADDR_LENGTH);
        bdaddr_type = GAPM_CFG_ADDR_PUBLIC;
    }
*/
    /* Initialize GAPM configuration command to initialize the stack */
    gapmConfigCmd = malloc(sizeof(struct gapm_set_dev_config_cmd));
    gapmConfigCmd->operation = GAPM_SET_DEV_CONFIG;
    gapmConfigCmd->role = GAP_ROLE_ALL;
    memcpy(gapmConfigCmd->addr.addr, bdaddr, BDADDR_LENGTH);
    gapmConfigCmd->addr_type = bdaddr_type;
    gapmConfigCmd->renew_dur = RENEW_DUR;
    memset(&gapmConfigCmd->irk.key[0], 0, KEY_LEN);
    gapmConfigCmd->pairing_mode = GAPM_PAIRING_DISABLE;
    //gapmConfigCmd->pairing_mode = GAPM_PAIRING_LEGACY;
    gapmConfigCmd->gap_start_hdl = 0;
    gapmConfigCmd->gatt_start_hdl = 0;
    gapmConfigCmd->max_mtu = MTU_MAX;
    gapmConfigCmd->max_mps = MPS_MAX;
    gapmConfigCmd->att_cfg = ATT_CFG;
    gapmConfigCmd->sugg_max_tx_octets = TX_OCT_MAX;
    gapmConfigCmd->sugg_max_tx_time = TX_TIME_MAX;
    gapmConfigCmd->tx_pref_rates = GAP_RATE_ANY;
    gapmConfigCmd->rx_pref_rates = GAP_RATE_ANY;
    gapmConfigCmd->max_nb_lecb = 0x0;
    gapmConfigCmd->audio_cfg = 0;

    /* Reset the stack */
    cmd = KE_MSG_ALLOC(GAPM_RESET_CMD, TASK_GAPM, TASK_APP, gapm_reset_cmd);
    cmd->operation = GAPM_RESET;
    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : bool Service_Add(void)
 * ----------------------------------------------------------------------------
 * Description   : Add the next service in the service list,
 *                 calling the appropriate add service function
 * Inputs        : None
 * Outputs       : return value - Indicates if any service has not yet been
 *                                added
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
bool Service_Add(void)
{
    /* Check if another should be added in the database */
    if(appm_add_svc_func_list[ble_env[0].next_svc] != NULL)
    {
        /* Call the function used to add the required service */
        appm_add_svc_func_list[ble_env[0].next_svc]();

        /* Select the next service to add */
        ble_env[0].next_svc++;
        return true;
    }

    return false;
}

/* ----------------------------------------------------------------------------
 * Standard Message Handlers
 * ------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------
 * Function      : int GAPM_ProfileAddedInd(ke_msg_id_t const msg_id,
 *                                          struct gapm_profile_added_ind
 *                                          const *param,
 *                                          ke_task_id_t const dest_id,
 *                                          ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the received result of adding a profile to the
 *                 attribute database
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapm_profile_added_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPM_ProfileAddedInd(ke_msg_id_t const msg_id,
                         struct gapm_profile_added_ind const *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    /* If the application is creating its attribute database, continue to add
     * services; otherwise do nothing. */
    if(ble_env[0].state == APPM_CREATE_DB)
    {
        /* Add the next requested service */
        if(!Service_Add())
        {
            /* If there are no more services to add, go to the ready state */
            ble_env[DEVICE_NUM_PERIPHERAL].state = APPM_READY;
            ble_env[DEVICE_NUM_CENTRAL].state = APPM_READY;

            /* No more service to add, send advertisement command */
            //BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);

            if(app_env.app_role_control>0)
            {
            	BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
            }
            else
            {
            	BLE_Connection_SelectBegin(DEVICE_NUM_CENTRAL);
            }

        }
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPM_CmpEvt(ke_msg_id_t const msg_id,
 *                                 struct gapm_cmp_evt
 *                                 const *param,
 *                                 ke_task_id_t const dest_id,
 *                                 ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the reception of a GAPM complete event
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapm_cmp_evt
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPM_CmpEvt(ke_msg_id_t const msg_id, struct gapm_cmp_evt const *param,
                ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gapm_set_dev_config_cmd* cmd;

    switch(param->operation)
    {
        /* A reset has occurred, configure the device and
         * start a kernel timer for the application */
        case(GAPM_RESET):
        {
            if(param->status == GAP_ERR_NO_ERROR)
            {
                /* Set the device configuration */
                cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD, TASK_GAPM, TASK_APP,
                                   gapm_set_dev_config_cmd);
                memcpy(cmd, gapmConfigCmd, sizeof(struct gapm_set_dev_config_cmd));
                free(gapmConfigCmd);

                /* Send message */
                ke_msg_send(cmd);

                /* Start a timer to be used as a periodic tick timer for
                 * application */
                ke_timer_set(APP_TEST_TIMER, TASK_APP, TIMER_200MS_SETTING);
            }
        }
        break;

        /* Device configuration updated */
        case(GAPM_SET_DEV_CONFIG):
        {
            /* Start creating the GATT database */
            ble_env[DEVICE_NUM_PERIPHERAL].state = APPM_CREATE_DB;
            ble_env[DEVICE_NUM_CENTRAL].state = APPM_CREATE_DB;

            /* Add the first required service in the database */
            if(!Service_Add())
            {
                /* If there are no more services to add, go to the ready state */
                ble_env[DEVICE_NUM_PERIPHERAL].state = APPM_READY;
                ble_env[DEVICE_NUM_CENTRAL].state = APPM_READY;

                /* Start advertising since there are no services to add
                 * to the attribute database */
                //BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);

                if(app_env.app_role_control>0)
                {
                	BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
                }
                else
                {
                	BLE_Connection_SelectBegin(DEVICE_NUM_CENTRAL);
                }
            }
        }
        break;

        case GAPM_CONNECTION_DIRECT:
        {
            /* If connection request operation is cancelled */
            if (param->status == GAP_ERR_CANCELED)
            {
                ble_env[DEVICE_NUM_CENTRAL].state = APPM_READY;

                //BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
                if(app_env.app_role_control>0)
                {
                	BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
                }
                else
                {
                	BLE_Connection_SelectBegin(DEVICE_NUM_CENTRAL);
                }

            }
        }
        break;

        case GAPM_ADV_UNDIRECT:
        {
            /* If advertisement is cancelled */
            if (param->status == GAP_ERR_CANCELED)
            {
            	if(app_env.app_role_control>0)
            	{
            		ble_env[DEVICE_NUM_PERIPHERAL].state = APPM_READY;

            		BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
            	}
            	else
            	{
            		ble_env[DEVICE_NUM_CENTRAL].state = APPM_READY;

            		BLE_Connection_SelectBegin(DEVICE_NUM_CENTRAL);
            	}
            }
        }
        break;

        default:
        {
            /* No action required for other operations */
        }
        break;
    }

    return(KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void Advertising_Start(void)
 * ----------------------------------------------------------------------------
 * Description   : Send a start advertising
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Advertising_Start(void)
{
    uint8_t device_name_length;
    uint8_t device_name_avail_space;
    uint8_t scan_rsp[SCAN_RSP_DATA_LEN] = APP_SCNRSP_DATA;
    uint8_t company_id[APP_COMPANY_ID_DATA_LEN] = APP_COMPANY_ID_DATA;

    /* Prepare the GAPM_START_ADVERTISE_CMD message */
    struct gapm_start_advertise_cmd *cmd;

    /* If the application is ready, start advertising */
    if(ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_READY)
    {
        /* Prepare the start advertisment command message */
        cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD, TASK_GAPM, TASK_APP,
                           gapm_start_advertise_cmd);
        cmd->op.addr_src = GAPM_STATIC_ADDR;
        cmd->channel_map = APP_ADV_CHMAP;

        cmd->intv_min = APP_ADV_INT_MIN;
        cmd->intv_max = APP_ADV_INT_MAX;

        cmd->op.code = GAPM_ADV_UNDIRECT;
        cmd->op.state = 0;
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;
        cmd->info.host.adv_filt_policy = 0;

        /* Set the scan response data */
        cmd->info.host.scan_rsp_data_len = APP_SCNRSP_DATA_LEN;
        memcpy(&cmd->info.host.scan_rsp_data[0],
               scan_rsp, cmd->info.host.scan_rsp_data_len);

        /* Get remaining space in the advertising data -
         * 2 bytes are used for name length/flag */
        cmd->info.host.adv_data_len = 0;
        device_name_avail_space = (ADV_DATA_LEN - 3) - 2;

        /* Check if data can be added to the advertising data */
        if(device_name_avail_space > 0)
        {
            /* Add as much of the device name as possible */
            device_name_length = strlen(APP_DFLT_DEVICE_NAME);
            if(device_name_length > 0)
            {
                /* Check available space */
                device_name_length = co_min(device_name_length,
                                            device_name_avail_space);
                cmd->info.host.adv_data[cmd->info.host.adv_data_len] =
                        device_name_length + 1;

                /* Fill device name flag */
                cmd->info.host.adv_data[cmd->info.host.adv_data_len + 1] =
                                                                 '\x09';

                /* Copy device name */
                memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len + 2],
                       APP_DFLT_DEVICE_NAME, device_name_length);

                /* Update advertising data length */
                cmd->info.host.adv_data_len += (device_name_length + 2);
            }

            /* If there is still space, add the company ID */
            if(((ADV_DATA_LEN - 3) - cmd->info.host.adv_data_len - 2) >=
                 APP_COMPANY_ID_DATA_LEN)
            {
                memcpy(&cmd->info.host.adv_data[cmd->info.host.adv_data_len],
                        company_id, APP_COMPANY_ID_DATA_LEN);
                cmd->info.host.adv_data_len += APP_COMPANY_ID_DATA_LEN;
            }
        }

        /* Send the message */
        ke_msg_send(cmd);

        /* Set the state of the task to APPM_ADVERTISING  */
        ble_env[DEVICE_NUM_PERIPHERAL].state = APPM_ADVERTISING;
    }
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_GetDevInfoReqInd(ke_msg_id_t const msg_id,
 *                                           struct gapc_get_dev_info_req_ind
 *                                           const *param,
 *                                           ke_task_id_t const dest_id,
 *                                           ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle message device info request received from GAP controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_get_dev_info_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_GetDevInfoReqInd(ke_msg_id_t const msg_id,
                          struct gapc_get_dev_info_req_ind const *param,
                          ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t len = strlen(APP_DFLT_DEVICE_NAME);

    /* Allocate message */
    struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
                                                         src_id, dest_id,
                                                         gapc_get_dev_info_cfm,
                                                         len);

    switch(param->req)
    {
        case GAPC_DEV_NAME:
        {
            cfm->req = GAPC_DEV_NAME;
            memcpy(&cfm->info.name.value[0], APP_DFLT_DEVICE_NAME, len);
            cfm->info.name.length = len;
        }
        break;

        case GAPC_DEV_APPEARANCE:
        {
            /* Set the device appearance (no appearance) */
            cfm->info.appearance = 0;
            cfm->req = GAPC_DEV_APPEARANCE;
            cfm->info.appearance = GAPM_WRITE_DISABLE;
        }
        break;

        case GAPC_DEV_SLV_PREF_PARAMS:
        {
            /* Slave preferred connection interval (minimum) */
            cfm->info.slv_params.con_intv_min = PREF_SLV_MIN_CON_INTERVAL;
            /* Slave preferred connection interval (maximum) */
            cfm->info.slv_params.con_intv_max = PREF_SLV_MAX_CON_INTERVAL;
            /* Slave preferred connection latency */
            cfm->info.slv_params.slave_latency = PREF_SLV_LATENCY;
            /* Slave preferred link supervision timeout */
            cfm->info.slv_params.conn_timeout = PREF_SLV_SUP_TIMEOUT;

            cfm->req = GAPC_DEV_SLV_PREF_PARAMS;
        }
        break;

        default:
            /* No action required for other requests */
        break;
    }

    /* Send message */
    ke_msg_send(cfm);

    return(KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_ConnectionReqInd(ke_msg_idd_t const msg_id,
 *                                           struct gapc_connection_req_ind
 *                                           const *param,
 *                                           ke_task_id_t const dest_id,
 *                                           ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle connection indication message received from
 *                 GAP controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_connection_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_ConnectionReqInd(ke_msg_id_t const msg_id,
                          struct gapc_connection_req_ind const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    /* Instantiate connection confirmation message structure */
    struct gapc_connection_cfm *cfm;

    /* Index of app_env representing role of the application */
    int device_indx;

    /* If application is in peripheral role, set device index appropriately */
    if (ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_ADVERTISING)
    {
        device_indx = DEVICE_NUM_PERIPHERAL;
    }
    /* If application is in central role, set device index appropriately */
    else if (ble_env[DEVICE_NUM_CENTRAL].state == APPM_CONNECTING)
    {
        device_indx = DEVICE_NUM_CENTRAL;
    }
    else
    {
        return (KE_MSG_CONSUMED);
    }

    /* Check if the received connection handle was valid */
    if (KE_IDX_GET(src_id) != GAP_INVALID_CONIDX)
    {
        /* Set connection index using source id received */
        ble_env[device_indx].conidx = KE_IDX_GET(src_id);

        /* Set state to connected */
        ble_env[device_indx].state = APPM_CONNECTED;

        /* Send connection confirmation */
        cfm = KE_MSG_ALLOC(GAPC_CONNECTION_CFM,
                           KE_BUILD_ID(TASK_GAPC, ble_env[device_indx].conidx),
                           TASK_APP, gapc_connection_cfm);

        cfm->auth = GAP_AUTH_REQ_NO_MITM_NO_BOND;

        cfm->svc_changed_ind_enable = 0;

        /* Retrieve the connection info from the parameters */
        ble_env[device_indx].conidx = param->conhdl;

        /* Save the connection parameters */
        ble_env[device_indx].con_interval = param->con_interval;
        ble_env[device_indx].con_latency = param->con_latency;
        ble_env[device_indx].time_out = param->sup_to;

        /* Send the message */
        ke_msg_send(cfm);

        /* If the application is central role */
        if (device_indx == DEVICE_NUM_CENTRAL)
        {
            /* After connection, clear the timer */
            ke_timer_clear(APP_SWITCH_ROLE_TIMEOUT, TASK_APP);
        }
        /* If the application is peripheral role */
        else if (device_indx == DEVICE_NUM_PERIPHERAL)
        {
            /* After connection, clear the timer */
            ke_timer_clear(APP_SWITCH_ROLE_TIMEOUT, TASK_APP);
        }

        /* Start enabling services */
        BLE_SetServiceState(true, ble_env[device_indx].conidx);

        //BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
        if(app_env.app_role_control>0)
        {
        	BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
        }
        else
        {
        	BLE_Connection_SelectBegin(DEVICE_NUM_CENTRAL);
        }
    }
    else
    {
        //BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
        /* Application role control */
        app_env.app_role_control = DIO->DATA & (1 << ((uint32_t) 8));
        if(app_env.app_role_control>0)
        {
        	BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
        }
        else
        {
        	BLE_Connection_SelectBegin(DEVICE_NUM_CENTRAL);
        }
    }

    return(KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_CmpEvt(ke_msg_id_t const msg_id,
 *                                 struct gapc_cmp_evt
 *                                 const *param,
 *                                 ke_task_id_t const dest_id,
 *                                 ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received GAPC complete event
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_cmp_evt
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_CmpEvt(ke_msg_id_t const msg_id, struct gapc_cmp_evt const *param,
                ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    /* No operations in this application use this event */
    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_DisconnectInd(ke_msg_id_t const msg_id,
 *                                        struct gapc_disconnect_ind
 *                                        const *param,
 *                                        ke_task_id_t const dest_id,
 *                                        ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle disconnect indication message from GAP controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_disconnect_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_DisconnectInd(ke_msg_id_t const msg_id,
                       struct gapc_disconnect_ind const *param,
                       ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t device_indx = DeviceIndx(KE_IDX_GET(src_id));
    if(device_indx < DEVICE_NUM_MAX)
    {
        /* Go to the ready state */
        ble_env[device_indx].state = APPM_READY;

        BLE_SetServiceState(false, ble_env[device_indx].conidx);

        /* When the link is lost, it sends connection start command or
         * advertising start command */
        if(device_indx == DEVICE_NUM_PERIPHERAL)
        {
            BLE_Connection_SelectBegin(DEVICE_NUM_CENTRAL);
        }
        else
        {
            BLE_Connection_SelectBegin(DEVICE_NUM_PERIPHERAL);
        }
    }
    return(KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_ParamUpdatedInd(ke_msg_id_t const msg_id,
 *                                          struct gapc_param_updated_ind
 *                                          const *param,
 *                                          ke_task_id_t const dest_id,
 *                                          ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle message parameter updated indication received from
 *                 GAP controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gapc_param_updated_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GAPC_ParamUpdatedInd(ke_msg_id_t const msg_id,
                         struct gapc_param_updated_ind const *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id)
{
    uint8_t device_indx = DeviceIndx(KE_IDX_GET(src_id));

    ble_env[device_indx].updated_con_interval = param->con_interval;
    ble_env[device_indx].updated_latency = param->con_latency;
    ble_env[device_indx].updated_suo_to = param->sup_to;

    return(KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GAPC_ParamUpdateReqInd(ke_msg_id_t const msg_id,
                           struct gapc_param_update_req_ind const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   :
 * ------------------------------------------------------------------------- */
int GAPC_ParamUpdateReqInd(ke_msg_id_t const msg_id,
                           struct gapc_param_update_req_ind const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    struct gapc_param_update_cfm *cfm;

    uint8_t device_indx = DeviceIndx(KE_IDX_GET(src_id));

    cfm= KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CFM,
                      KE_BUILD_ID(TASK_GAPC, ble_env[device_indx].conidx),
                      TASK_APP,
                      gapc_param_update_cfm);
    cfm->accept = 1;
    cfm->ce_len_min = ((param->intv_min * 2));;
    cfm->ce_len_max = ((param->intv_max * 2));

    /* Send message */
    ke_msg_send(cfm);
    return(KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void Connection_SendStartCmd(void)
 * ----------------------------------------------------------------------------
 * Description   : Send a command to establish a connection with peer device
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : Application knows the peer Bluetooth device address,
 *                 so it uses direct connection method
 * ------------------------------------------------------------------------- */
void Connection_SendStartCmd(void)
{
    uint8_t peerAddress0[BD_ADDR_LEN] =DIRECT_PEER_BD_ADDRESS;
    uint8_t peerAddress0_central[BD_ADDR_LEN] =DIRECT_PEER_BD_ADDRESS_CENTRAL;
    struct gapm_start_connection_cmd *cmd;

    /* Prepare the GAPM_START_CONNECTION_CMD message  */
    cmd = KE_MSG_ALLOC_DYN(GAPM_START_CONNECTION_CMD, TASK_GAPM, TASK_APP,
                           gapm_start_connection_cmd,
                           (sizeof(struct gap_bdaddr)));

    cmd->op.code = GAPM_CONNECTION_DIRECT;
    cmd->op.addr_src = GAPM_STATIC_ADDR;
    cmd->op.state = 0;

    /* Set scan interval to 62.5ms and scan window to 50% of the interval */
    cmd->scan_interval = SCAN_INERVAL;
    cmd->scan_window = SCAN_WINDOW;

    /* Set the connection interval to 7.5ms */
    cmd->con_intv_min = CON_INTERVAL_MIN;
    cmd->con_intv_max = CON_INTERVAL_MAX;
    cmd->con_latency = CON_SLAVE_LATENCY;

    /* Set supervisory timeout to 3s */
    cmd->superv_to = CON_SUP_TIMEOUT;

    cmd->nb_peers = 1;

    /* Address Type: Private Address */
    cmd->peers[0].addr_type = DIRECT_PEER_BD_ADDRESS_TYPE;
    if(app_env.app_role_control>0)
    {
    	memcpy(&cmd->peers[0].addr.addr[0], &peerAddress0[0], BD_ADDR_LEN);
    }
    else
    {
    	memcpy(&cmd->peers[0].addr.addr[0], &peerAddress0_central[0], BD_ADDR_LEN);
    }

    /* Send the message */
    ke_msg_send(cmd);

    ble_env[DEVICE_NUM_CENTRAL].state = APPM_CONNECTING;
}

/* ----------------------------------------------------------------------------
 * Function      : void BLE_SetServiceState(bool enable, uint8_t conidx)
 * ----------------------------------------------------------------------------
 * Description   : Set Bluetooth application environment state to enabled
 * Inputs        : - enable    - Indicates that enable request should be sent
 *                               for all services/profiles or their status
 *                               should be set to disabled
 *                 - conidx    - Connection index
 *                               enabled or disabled
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void BLE_SetServiceState(bool enable, uint8_t conidx)
{
    /* All standard services should be send enable request to the stack,
     * for custom services, application should decide if it would want
     * to do a specific action
     * all services should be disabled once the link is lost
     */
    uint8_t device_indx = DeviceIndx(conidx);

    if(enable == true)
    {
        if(device_indx == DEVICE_NUM_CENTRAL)
        {
            /* To enable standard Bluetooth services, an enable request should be
             * sent to the stack (for related profile) and at receiving the
             * response the enable flag can be set. For custom service it is
             * application implementation dependent. Here, it starts service
             * discovery, and if the service UUID and characteristics UUID are
             * discovered, then it goes to an state that is equivalent to the
             * enable flag of standard profiles
             */
            ble_env[device_indx].next_svc_enable = 0;
            ServiceEnable(conidx);
        }
        else
        {
            /* All standard services should be send enable request to the stack,
             * for custom services, application should decide if it would want
             * to do a specific action
             * all services should be disabled once the link is lost
             */
            /* Enable battery service server*/
            Batt_ServiceEnable_Server(conidx);
        }
    }
    else
    {
        if(device_indx == DEVICE_NUM_CENTRAL)
        {
            basc_support_env.enable = false;
            cs_env[DEVICE_NUM_CENTRAL].client.state = CS_INIT;
        }
        else
        {
            bass_support_env.enable = false;
            cs_env[DEVICE_NUM_PERIPHERAL].server.state = CS_INIT;
        }
    }
}

/* ----------------------------------------------------------------------------
 * Function      : bool ServiceEnable(uint8_t conidx)
 * ----------------------------------------------------------------------------
 * Description   : Enable the next service in the service list,
 *                 calling the appropriate enable service function
 * Inputs        : - conidx     - Connection index
 * Outputs       : return value - Indicates if any service has not yet been
 *                                added
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
bool ServiceEnable(uint8_t conidx)
{
    /* Check if another should be added in the database */
    if(appm_enable_svc_func_list[ble_env[DEVICE_NUM_CENTRAL].next_svc_enable] != NULL)
    {
        /* Call the function used to enable the required service */
        appm_enable_svc_func_list[ble_env[DEVICE_NUM_CENTRAL].next_svc_enable](conidx);

        /* Select the next service to enable */
        ble_env[DEVICE_NUM_CENTRAL].next_svc_enable++;
        return true;
    }

    return false;
}

/* ----------------------------------------------------------------------------
 * Function      : void BLE_Operation_Cancel(int device_indx)
 * ----------------------------------------------------------------------------
 * Description   : Sends operation cancel
 * Inputs        : - device_indx   -  index of app_env representing
 *                                    role of the application
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void BLE_Operation_Cancel(int device_indx)
{
    /* Instantiate cancel command message structure */
    struct gapm_cancel_cmd *cmd;

    /* Allocate message for cancel command */
    cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD, TASK_GAPM, TASK_APP, gapm_cancel_cmd);

    /* Set parameters */
    /* Set command operation */
    cmd->operation = GAPM_CANCEL;

    /* Send the message */
    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : int APP_SwitchRole_Timeout(ke_msg_id_t const msg_id,
 *                                       void const *param,
 *                                       ke_task_id_t const dest_id,
 *                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handles a timeout received for application's operations
 * Inputs        : - msg_id     - kernel message ID number
 *                 - param      - message parameter (unused)
 *                 - dest_id    - destination task ID number
 *                 - src_id     - source task ID number
 * Outputs       : Return value - indicates if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int APP_SwitchRole_Timeout(ke_msg_id_t const msg_id, void const *param,
                          ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    /* If device is advertising, cancel advertisements and
     * switch to central switching done in GAPM_CMPEVT upon reception
     * of successful cancel of advertising operation */
	/*
    if (ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_ADVERTISING)
    {
        BLE_Operation_Cancel(DEVICE_NUM_PERIPHERAL);
    }
    */
    /* If device is sending connection requests, cancel the operation and
     * switch to peripheral. Switching done in GAPM_CMPEVT upon reception of
     * successful cancel of connection request operation */
	/*
    else if (ble_env[DEVICE_NUM_CENTRAL].state == APPM_CONNECTING)
    {
        BLE_Operation_Cancel(DEVICE_NUM_CENTRAL);
    }
	 */

    uint8_t device_indx = DeviceIndx(KE_IDX_GET(src_id));

    if(app_env.app_role_control>0)
    {
    	if(device_indx!=DEVICE_NUM_PERIPHERAL)
    	{
    		BLE_Operation_Cancel(device_indx);
    	}
    }
    else
    {
    	if(device_indx!=DEVICE_NUM_CENTRAL)
    	{
    		BLE_Operation_Cancel(device_indx);
    	}
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : uint8_t DeviceIndx(uint8_t conidx)
 * ----------------------------------------------------------------------------
 * Description   : Returns device index based on the connection index
 * Inputs        : - conidx         - connection index of the sending task
 *                                    instance
 * Outputs       : - device_indx    - index of environment structure
 *                                    representing role of the application
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
uint8_t DeviceIndx(uint8_t conidx)
{
    uint8_t i;

    for (i = 0; i < DEVICE_NUM_MAX; i++)
    {
        /* Check connection index to set device index*/
        if (ble_env[i].state == APPM_CONNECTED && ble_env[i].conidx == conidx)
        {
            return i;
        }
    }

    return i;
}

/* ----------------------------------------------------------------------------
 * Function      : void BLE_Connection_SelectBegin(uint8_t device_indx)
 * ----------------------------------------------------------------------------
 * Description   : Checks if it should send advertisement start command or
 *                 connection start command starting with the requested
 *                 device index
 * Inputs        : - device_indx    - Device index that should be checked first
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void BLE_Connection_SelectBegin(uint8_t device_indx)
{
    if(device_indx == DEVICE_NUM_PERIPHERAL)
    {
        if (ble_env[DEVICE_NUM_PERIPHERAL].state ==
            APPM_READY && ble_env[DEVICE_NUM_CENTRAL].state != APPM_CONNECTING)
        {
            Advertising_Start();
            ke_timer_set(APP_SWITCH_ROLE_TIMEOUT, TASK_APP,
                         APP_SWITCH_ROLE_TIMER);
        }
        else if (ble_env[DEVICE_NUM_CENTRAL].state == APPM_READY &&
                 ble_env[DEVICE_NUM_PERIPHERAL].state != APPM_ADVERTISING)
        {
            Connection_SendStartCmd();
            ke_timer_set(APP_SWITCH_ROLE_TIMEOUT, TASK_APP, APP_SWITCH_ROLE_TIMER);
        }
    }
    else
    {
        if (ble_env[DEVICE_NUM_CENTRAL].state == APPM_READY &&
            ble_env[DEVICE_NUM_PERIPHERAL].state != APPM_ADVERTISING)
        {
            Connection_SendStartCmd();
            ke_timer_set(APP_SWITCH_ROLE_TIMEOUT, TASK_APP, APP_SWITCH_ROLE_TIMER);
        }
        else if (ble_env[DEVICE_NUM_PERIPHERAL].state == APPM_READY &&
                 ble_env[DEVICE_NUM_CENTRAL].state != APPM_CONNECTING)
        {
            Advertising_Start();
            ke_timer_set(APP_SWITCH_ROLE_TIMEOUT, TASK_APP, APP_SWITCH_ROLE_TIMER);
        }
    }
}

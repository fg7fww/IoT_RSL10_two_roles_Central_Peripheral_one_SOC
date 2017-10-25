/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * ble_custom.c
 * - Bluetooth custom service functions
 * ----------------------------------------------------------------------------
 * $Revision: 1.2 $
 * $Date: 2017/07/11 13:14:09 $
 * ------------------------------------------------------------------------- */

#include "app.h"

/* Global variable definition for client and server roles of custom service */
union cs_env_tag   cs_env[2];

/* ----------------------------------------------------------------------------
 * Function      : void CustomService_Env_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize custom service environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_Env_Initialize(void)
{
    /* Reset the application manager environment */
    memset(&cs_env[0].server, 0, sizeof(struct csc_env_tag));
    memset(&cs_env[1].client, 0, sizeof(struct css_env_tag));
}

/* ----------------------------------------------------------------------------
 * Function      : void CustomService_ServiceEnable(uint8_t conidx)
 * ----------------------------------------------------------------------------
 * Description   : Send a command to use service discovery to look for a
 *                 specific service with a known UUID
 * Inputs        : - conidx       - connection index
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_ServiceEnable(uint8_t conidx)
{
    struct gattc_disc_cmd *cmd;

    uint8_t svc_uuid[ATT_UUID_128_LEN] = CS_SVC_UUID;

    cmd = KE_MSG_ALLOC_DYN(GATTC_DISC_CMD, KE_BUILD_ID(TASK_GATTC, conidx),
                           TASK_APP, gattc_disc_cmd,
                           ATT_UUID_128_LEN * sizeof(uint8_t));
    cmd->operation = GATTC_DISC_BY_UUID_SVC;
    cmd->uuid_len = ATT_UUID_128_LEN;
    cmd->seq_num = 0x0000;
    cmd->start_hdl = 0x0001;
    cmd->end_hdl = 0xffff;
    memcpy(cmd->uuid, svc_uuid, ATT_UUID_128_LEN);

    /* Send the message */
    ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_DiscCharInd(ke_msg_id_t const msg_id,
 *                                       struct gattc_disc_char_ind
 *                                       const *param,
 *                                       ke_task_id_t const dest_id,
 *                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle discovered characteristic indication message received
 *                 from GATT controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_disc_char_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_DiscCharInd(ke_msg_id_t const msg_id,
                      struct gattc_disc_char_ind const *param,
                      ke_task_id_t const dest_id,
                      ke_task_id_t const src_id)
{
    uint8_t uuid[2][16] = CS_CHARACTERISTICS_LIST;
    uint8_t i;

    /* Attr_hdl is for characteristic handle and pointer_hdl for value  */
    if (param->attr_hdl != 0 &&
        cs_env[DEVICE_NUM_CENTRAL].client.disc_attnum < CS_IDX_NB_CLIENT)
    {
        for(i=0; i < CS_IDX_NB_CLIENT; i++)
        {
            if (param->uuid_len == ATT_UUID_128_LEN &&
               !memcmp(param->uuid, &uuid[i][0], ATT_UUID_128_LEN))
            {
                memcpy(&cs_env[DEVICE_NUM_CENTRAL].client.disc_att[cs_env[DEVICE_NUM_CENTRAL].client.disc_attnum]
                       ,param, sizeof(struct discovered_char_att));

                cs_env[DEVICE_NUM_CENTRAL].client.disc_attnum++;
                break;
            }
        }

        if (cs_env[DEVICE_NUM_CENTRAL].client.disc_attnum == CS_IDX_NB_CLIENT)
        {
            cs_env[DEVICE_NUM_CENTRAL].client.state = CS_ALL_ATTS_DISCOVERED;

            /* Enable pending client services to be enable */
            ServiceEnable(ble_env[DEVICE_NUM_CENTRAL].conidx);
        }
    }
    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_CmpEvt(ke_msg_id_t const msg_id,
 *                                  struct gattc_cmp_evt
 *                                  const *param,
 *                                  ke_task_id_t const dest_id,
 *                                  ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received GATT controller complete event
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_cmp_evt
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_CmpEvt(ke_msg_id_t const msg_id, struct gattc_cmp_evt const *param,
                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{

    uint8_t device_indx = DeviceIndx(KE_IDX_GET(src_id));
    if (device_indx != DEVICE_NUM_CENTRAL || param->status == GAP_ERR_DISCONNECTED )
    {
        return (KE_MSG_CONSUMED);
    }

    /* Check application state and status of service and characteristic
     * discovery for custom service and if it is unsuccessful we can disconnect
     * the link although it is possible to go to enable state and let the
     * battery service works */
    if (param->status != GAP_ERR_NO_ERROR)
    {
        if (param->operation == GATTC_DISC_BY_UUID_SVC &&
                param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND &&
                cs_env[DEVICE_NUM_CENTRAL].client.state != CS_SERVICE_DISCOVERD
                && ble_env[DEVICE_NUM_CENTRAL].state != APPM_CONNECTED)
        {
            /* Enable pending client services to be enable */
            ServiceEnable(ble_env[DEVICE_NUM_CENTRAL].conidx);
        }
        else if (param->operation == GATTC_DISC_ALL_CHAR &&
                param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND &&
                cs_env[DEVICE_NUM_CENTRAL].client.state == CS_SERVICE_DISCOVERD)
        {
            /* Enable pending client services to be enable */
            ServiceEnable(ble_env[DEVICE_NUM_CENTRAL].conidx);
        }
    }

    if(param->operation == GATTC_WRITE && param->status == GAP_ERR_NO_ERROR)
    {
        Sys_GPIO_Toggle(GPIO_TEST_2);
    }

    if(param->operation == GATTC_WRITE_NO_RESPONSE && param->status == GAP_ERR_NO_ERROR)
    {
        Sys_GPIO_Toggle(GPIO_TEST_2);
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_DiscSvcInd(ke_msg_id_t const msg_id,
 *                                      struct gattc_disc_svc_ind
 *                                      const *param,
 *                                      ke_task_id_t const dest_id,
 *                                      ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive the result of a service discovery
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_disc_svc_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_DiscSvcInd(ke_msg_id_t const msg_id,
            struct gattc_disc_svc_ind const *param, ke_task_id_t const dest_id,
            ke_task_id_t const src_id)
{
    struct gattc_disc_cmd *cmd;

    /* We accepts only discovered attributes with 128-bit UUID
     * according to the defined characteristics in this custom service */
    if (param->uuid_len == ATT_UUID_128_LEN)
    {
        cs_env[DEVICE_NUM_CENTRAL].client.state = CS_SERVICE_DISCOVERD;

        cs_env[DEVICE_NUM_CENTRAL].client.start_hdl = param->start_hdl;
        cs_env[DEVICE_NUM_CENTRAL].client.end_hdl = param->end_hdl;

        cs_env[DEVICE_NUM_CENTRAL].client.disc_attnum = 0;

        /* Allocate and send GATTC discovery command to discover
         * characteristic declarations */
        cmd = KE_MSG_ALLOC_DYN(GATTC_DISC_CMD,
                               KE_BUILD_ID(TASK_GATTC,
                               ble_env[DEVICE_NUM_CENTRAL].conidx),
                               TASK_APP, gattc_disc_cmd,
                               2 * sizeof(uint8_t));

        cmd->operation = GATTC_DISC_ALL_CHAR;
        cmd->uuid_len = 2;
        cmd->seq_num = 0x0000;
        cmd->start_hdl = cs_env[DEVICE_NUM_CENTRAL].client.start_hdl;
        cmd->end_hdl = cs_env[DEVICE_NUM_CENTRAL].client.end_hdl;
        cmd->uuid[0] = 0;
        cmd->uuid[1] = 0;

       /* Send the message */
       ke_msg_send(cmd);

    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_ReadInd(ke_msg_id_t const msg_id,
 *                                   struct gattc_read_ind
 *                                   const *param, ke_task_id_t const dest_id,
 *                                   ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive transmitted value from peripheral, assign to
 *                 tx_value
 *
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_read_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_ReadInd(ke_msg_id_t const msg_id, struct gattc_read_ind *param,
                  ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if(cs_env[DEVICE_NUM_CENTRAL].client.disc_att[CS_IDX_TX_CHAR].pointer_hdl
       == param->handle)
    {
        memcpy(cs_env[DEVICE_NUM_CENTRAL].client.tx_value,
               param->value, param->length);
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_EvtInd(ke_msg_id_t const msg_id,
 *                                  struct gattc_read_ind
 *                                  const *param, ke_task_id_t const dest_id,
 *                                  ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive transmitted value from peripheral, assign to
 *                 tx_value - contains new value of peer attribute handle
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_read_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------ */
int GATTC_EvtInd(ke_msg_id_t const msg_id, struct gattc_event_ind *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if (param->length > 0)
    {
        if(cs_env[DEVICE_NUM_CENTRAL].client.disc_att[CS_IDX_TX_CHAR].pointer_hdl
           == param->handle)
        {
            memcpy(cs_env[DEVICE_NUM_CENTRAL].client.tx_value,
                   param->value, param->length);
            cs_env[DEVICE_NUM_CENTRAL].server.spi1_tx_value[0]=(uint8_t)param->value[0];
            cs_env[DEVICE_NUM_CENTRAL].server.spi1_tx_value_changed=true;
            SPI1->TX_DATA=(uint32_t)param->value[0];
            Sys_GPIO_Set_Low(7);
        }
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void CustomSrvice_SendWrite(uint8_t conidx, uint8_t *value,
 *                                             uint16_t handle, uint8_t offset,
 *                                             uint16_t length, uint8_t type)
 * ----------------------------------------------------------------------------
 * Description   : Send a write command or request to the client device
 * Inputs        : - conidx       - Connection index
 *                 - value        - Pointer to value
 *                 - hanlde       - Attribute handle
 *                 - length       - Length of value
 *                 - type         - Type of write message
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_SendWrite(uint8_t conidx, uint8_t *value, uint16_t handle,
                    uint8_t offset, uint16_t length, uint8_t type)
{
    struct gattc_write_cmd *cmd = KE_MSG_ALLOC_DYN(GATTC_WRITE_CMD,
             KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP, gattc_write_cmd,
             length * sizeof(uint8_t));

     if(type == GATTC_WRITE)
     {
         /* Write request that needs a response from peer device */
         cmd->operation = GATTC_WRITE;
         cmd->auto_execute = 1;

     }
     else if(type == GATTC_WRITE_NO_RESPONSE)
     {
         /* Write command that doesn't need a response from peer device */
         cmd->operation = GATTC_WRITE_NO_RESPONSE;
         cmd->auto_execute = 0;

     }

     cmd->handle = handle;
     cmd->seq_num = 0x0000;
     cmd->offset = offset;
     cmd->cursor =0;
     cmd->length = length;
     memcpy(cmd->value, (uint8_t *)value, length);

     /* Send the message  */
     ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Function      : void CustomService_ServiceAdd(void)
 * ----------------------------------------------------------------------------
 * Description   : Send request to add custom profile into the attribute database.
 *                 Defines the different access functions (setter/getter commands
 *                 to access the different characteristic attributes).
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_ServiceAdd(void)
{
    struct gattm_add_svc_req *req = KE_MSG_ALLOC_DYN(GATTM_ADD_SVC_REQ,
            TASK_GATTM, TASK_APP, gattm_add_svc_req,
            CS_IDX_NB_SERVER * sizeof(struct gattm_att_desc));

    uint8_t i;
    const uint8_t svc_uuid[ATT_UUID_128_LEN] = CS_SVC_UUID;

    const struct gattm_att_desc att[CS_IDX_NB_SERVER] =
    {
        /* Attribute Index  = Attribute properties: UUID,
         *                                          Permissions,
         *                                          Max size,
         *                                          Extra permissions */

        /* TX Characteristic */
                /* TX Characteristic */
                [CS_IDX_TX_VALUE_CHAR]     = ATT_DECL_CHAR(),
                [CS_IDX_TX_VALUE_VAL]      = ATT_DECL_CHAR_UUID_128(CS_CHARACTERISTIC_TX_UUID,
                                                 PERM(RD,ENABLE) | PERM(NTF,ENABLE),
                                                 CS_TX_VALUE_MAX_LENGTH),
                [CS_IDX_TX_VALUE_CCC]      = ATT_DECL_CHAR_CCC(),
                [CS_IDX_TX_VALUE_USR_DSCP] = ATT_DECL_CHAR_USER_DESC( CS_USER_DESCRIPTION_MAX_LENGTH ),

                /* RX Characteristic */
                [CS_IDX_RX_VALUE_CHAR]     = ATT_DECL_CHAR(),
                [CS_IDX_RX_VALUE_VAL]      = ATT_DECL_CHAR_UUID_128(CS_CHARACTERISTIC_RX_UUID,
                                                 PERM(RD, ENABLE) | PERM(WRITE_REQ,ENABLE) | PERM(WRITE_COMMAND,ENABLE),
                                                 CS_RX_VALUE_MAX_LENGTH),
                [CS_IDX_RX_VALUE_CCC]      = ATT_DECL_CHAR_CCC(),
                [CS_IDX_RX_VALUE_USR_DSCP] = ATT_DECL_CHAR_USER_DESC( CS_USER_DESCRIPTION_MAX_LENGTH ),
    };

    /* Fill the add custom service message */
    req->svc_desc.start_hdl = 0;
    req->svc_desc.task_id = TASK_APP;
    req->svc_desc.perm = PERM(SVC_UUID_LEN, UUID_128);
    req->svc_desc.nb_att = CS_IDX_NB_SERVER;

    memcpy(&req->svc_desc.uuid[0], &svc_uuid[0], ATT_UUID_128_LEN);

    for(i = 0; i < CS_IDX_NB_SERVER; i++)
    {
        memcpy(&req->svc_desc.atts[i], &att[i],
               sizeof(struct gattm_att_desc));
    }

    /* Send the message */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTM_AddSvcRsp(ke_msg_id_t const msg_id,
 *                                     struct gattm_add_svc_rsp
 *                                     const *param,
 *                                     ke_task_id_t const dest_id,
 *                                     ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the response from adding a service in the attribute
 *                 database from the GATT manager
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattm_add_svc_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTM_AddSvcRsp(ke_msg_id_t const msg_id,
                    struct gattm_add_svc_rsp const *param,
                    ke_task_id_t const dest_id,
                    ke_task_id_t const src_id)
{
    cs_env[DEVICE_NUM_PERIPHERAL].server.start_hdl = param->start_hdl;

    /* Add the next requested service  */
    if(!Service_Add())
    {
        /* All services have been added, go to the ready state
         * and start advertising */
        ble_env[DEVICE_NUM_PERIPHERAL].state = APPM_READY;
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

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
 *                                      struct gattc_read_req_ind
 *                                      const *param,
 *                                      ke_task_id_t const dest_id,
 *                                      ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received read request indication
 *                 from a GATT Controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_read_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_ReadReqInd(ke_msg_id_t const msg_id,
                     struct gattc_read_req_ind const *param,
                     ke_task_id_t const dest_id,
                     ke_task_id_t const src_id)
{
    uint8_t length = 0;
    uint8_t status = GAP_ERR_NO_ERROR;
    uint16_t attnum;
    uint8_t *valptr = NULL;

    struct gattc_read_cfm *cfm;

    /* Set the attribute handle using the attribute index
     * in the custom service */
    if(param->handle > cs_env[0].server.start_hdl)
    {
        attnum = (param->handle - cs_env[0].server.start_hdl - 1);
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    /* If there is no error, send back the requested attribute value */
    if(status == GAP_ERR_NO_ERROR)
    {
        switch(attnum)
        {
            case CS_IDX_RX_VALUE_VAL:
                length = CS_RX_VALUE_MAX_LENGTH;
                valptr = (uint8_t *)&cs_env[0].server.rx_value;
                break;
            case CS_IDX_RX_VALUE_CCC:
                length = 2;
                valptr = (uint8_t *)&cs_env[0].server.rx_cccd_value;
                break;
            case CS_IDX_RX_VALUE_USR_DSCP:
                length = strlen(CS_RX_CHARACTERISTIC_NAME);
                valptr = (uint8_t *)CS_RX_CHARACTERISTIC_NAME;
                break;
            case CS_IDX_TX_VALUE_VAL:
                length = CS_TX_VALUE_MAX_LENGTH;
                valptr = (uint8_t *)&cs_env[0].server.tx_value;
                break;
            case CS_IDX_TX_VALUE_CCC:
                length = 2;
                valptr = (uint8_t *)&cs_env[0].server.tx_cccd_value;
                break;
            case CS_IDX_TX_VALUE_USR_DSCP:
                length = strlen(CS_TX_CHARACTERISTIC_NAME);
                valptr = (uint8_t *)CS_TX_CHARACTERISTIC_NAME;
                break;
            default:
                status = ATT_ERR_READ_NOT_PERMITTED;
                break;
        }
        if(valptr != NULL)
        {
            memcpy(cfm->value, valptr, length);
            SPI1->TX_DATA=(uint32_t)cfm->value[0];
            Sys_GPIO_Set_Low(7);
        }

    }

    /* Allocate and build message */
    cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                KE_BUILD_ID(TASK_GATTC, ble_env[DEVICE_NUM_PERIPHERAL].conidx),
                TASK_APP, gattc_read_cfm,
                length);

    if(valptr != NULL)
    {
        memcpy(cfm->value, valptr, length);
    }

    cfm->handle = param->handle;
    cfm->length = length;
    cfm->status = status;

    /* Send the message */
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
 *                                       struct gattc_write_req_ind
 *                                       const *param,
 *                                       ke_task_id_t const dest_id,
 *                                       ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle received write request indication
 *                 from a GATT Controller
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct gattc_write_req_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int GATTC_WriteReqInd(ke_msg_id_t const msg_id,
                      struct gattc_write_req_ind const *param,
                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm *cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM,
            KE_BUILD_ID(TASK_GATTC, ble_env[DEVICE_NUM_PERIPHERAL].conidx),
            TASK_APP, gattc_write_cfm);

    uint8_t status = GAP_ERR_NO_ERROR;
    uint16_t attnum;
    uint8_t *valptr = NULL;

    /* Check that offset is not zero */
    if(param->offset)
    {
        status = ATT_ERR_INVALID_OFFSET;
    }

    /* Set the attribute handle using the attribute index
     * in the custom service */
    if(param->handle > cs_env[0].server.start_hdl)
    {
        attnum = (param->handle - cs_env[0].server.start_hdl - 1);
    }
    else
    {
        status = ATT_ERR_INVALID_HANDLE;
    }

    /* If there is no error, save the requested attribute value */
    if(status == GAP_ERR_NO_ERROR)
    {
        switch(attnum)
        {
            case CS_IDX_RX_VALUE_VAL:
                valptr = (uint8_t *)&cs_env[0].server.rx_value;
                cs_env[0].server.rx_value_changed = 1;
                break;
            case CS_IDX_RX_VALUE_CCC:
                valptr = (uint8_t *)&cs_env[0].server.rx_cccd_value;
                break;
            case CS_IDX_TX_VALUE_CCC:
                valptr = (uint8_t *)&cs_env[0].server.tx_cccd_value;
                break;
            default:
                status = ATT_ERR_WRITE_NOT_PERMITTED;
                break;
        }
    }

    if(valptr != NULL)
    {
        memcpy(valptr, param->value, param->length);
        SPI1->TX_DATA=(uint32_t)param->value[0];
        Sys_GPIO_Set_Low(7);
    }

    cfm->handle = param->handle;
    cfm->status = status;

    /* Send the message */
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void CustomService_SendNotification(uint8_t conidx,
 *                               uint8_t attidx, uint8_t *value, uint8_t length)
 * ----------------------------------------------------------------------------
 * Description   : Send a notification to the client device
 * Inputs        : - conidx       - connection index
 *                 - attidx       - index to attributes in the service
 *                 - value        - pointer to value
 *                 - length       - length of value
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void CustomService_SendNotification(uint8_t conidx, uint8_t attidx,
                                    uint8_t *value, uint8_t length)
{
    struct gattc_send_evt_cmd *cmd;
    uint16_t handle = (attidx + cs_env[0].server.start_hdl + 1);

    /* Prepare a notification message for the specified attribute */
    cmd = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                           KE_BUILD_ID(TASK_GATTC, conidx), TASK_APP,
                           gattc_send_evt_cmd,
                           length * sizeof(uint8_t));
    cmd->handle = handle;
    cmd->length = length;
    cmd->operation = GATTC_NOTIFY;
    cmd->seq_num = 0;
    memcpy(cmd->value, value, length);

    /* Send the message */
     ke_msg_send(cmd);
}

/* ----------------------------------------------------------------------------
 * Copyright (c) 2017 Semiconductor Components Industries, LLC (d/b/a
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
 *  This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * ble_basc.c
 * - Bluetooth battery service client functions
 * ----------------------------------------------------------------------------
 * $Revision: 1.1 $
 * $Date: 2017/07/07 13:33:13 $
 * ------------------------------------------------------------------------- */

#include "app.h"

/* Global variable definition */
struct basc_support_env_tag basc_support_env;

/* ----------------------------------------------------------------------------
 * Function      : void Bass_Env_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize battery service server service environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Basc_Env_Initialize(void)
{
    memset(&basc_support_env, 0, sizeof(struct basc_support_env_tag));

    basc_support_env.req_ntf_cfg[0] = ATT_CCC_START_NTF;
    basc_support_env.req_ntf_cfg[1] = ATT_CCC_START_NTF;
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_ServiceAdd_Client(void)
 * ----------------------------------------------------------------------------
 * Description   : Send request to add battery profile in kernel and database
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Batt_ServiceAdd_Client(void)
{
    void* params;

    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                             TASK_GAPM, TASK_APP,
                                                             gapm_profile_task_add_cmd,
                                                             sizeof(params));
    /* Fill message  */
    req->operation = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl = PERM(SVC_AUTH, DISABLE);
    req->prf_task_id = TASK_ID_BASC;
    req->app_task = TASK_APP;
    req->start_hdl = 0;

    /* Send the message  */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_ServiceEnable_Client(uint8_t conidx)
 * ----------------------------------------------------------------------------
 * Description   : Send request for enabling battery service to battery client
 * Inputs        : conidx - Connection index
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Batt_ServiceEnable_Client(uint8_t conidx)
{
    struct basc_enable_req *req;
    struct basc_env_tag* basc_env;

    basc_env = PRF_ENV_GET(BASC, basc);

    /* Allocate the GATT notification message  */
    req = KE_MSG_ALLOC(BASC_ENABLE_REQ,
                       prf_src_task_get(&(basc_env->prf_env), conidx), TASK_APP,
                                        basc_enable_req);

    /* Fill in the parameter structure if the connection type is PRF_CON_NORMAL */
    /* Set con_type to profile discovery */
    req->con_type = PRF_CON_DISCOVERY;

    /*Should be filled according to the last discovery  */
    memcpy(&req->bas, &basc_support_env.bas, sizeof(struct bas_content));
    req->bas_nb = basc_support_env.bas_nb;

    /* Send the message  */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : int Batt_ReadInfoRsp(ke_msg_id_t const msg_id,
 *                                      struct basc_read_info_rsp
 *                                      const *param,
 *                                      ke_task_id_t const dest_id,
 *                                      ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Handle the response to a battery information read request
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct basc_read_info_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Batt_ReadInfoRsp(ke_msg_id_t const msg_id, struct basc_read_info_rsp *param,
                     ke_task_id_t const dest_id, ke_task_id_t const src_id)
{

    if (param->status == GAP_ERR_NO_ERROR)
    {
        if (param->info == BASC_BATT_LVL_VAL)
        {
            basc_support_env.batt_lvl[param->bas_nb] = param->data.batt_level;

            Sys_GPIO_Toggle(GPIO_TEST_1);
        }
        else if (param->info == BASC_NTF_CFG)
        {
            basc_support_env.ntf_cfg[param->bas_nb] = param->data.ntf_cfg;
        }
        else if (param->info == BASC_BATT_LVL_PRES_FORMAT)
        {
            basc_support_env.char_pres_format[param->bas_nb] = param->data.char_pres_format;
        }
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int Batt_LevelInd(ke_msg_id_t const msg_id,
 *                                   struct basc_batt_level_ind
 *                                   const *param, ke_task_id_t const dest_id,
 *                                   ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive battery level characteristic value upon reception of
 *                 a notification or Indication
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct basc_batt_level_ind
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Batt_LevelInd(ke_msg_id_t const msg_id, struct basc_batt_level_ind *param,
                  ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    basc_support_env.batt_lvl[param->bas_nb] = param->batt_level;

    return (KE_MSG_CONSUMED);
}

 /* ----------------------------------------------------------------------------
 * Function      : int Batt_LevelNtfCfgRsp(ke_msg_id_t const msg_id,
 *                                         struct basc_batt_level_ntf_cfg_rsp
 *                                         const *param,
 *                                         ke_task_id_t const dest_id,
 *                                         ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive battery level notification config response
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct basc_batt_level_ntf_cfg_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Batt_LevelNtfCfgRsp(ke_msg_id_t const msg_id,
                        struct basc_batt_level_ntf_cfg_rsp *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id)
{
    if (param->status == GAP_ERR_NO_ERROR)
    {
        basc_support_env.ntf_cfg[param->bas_nb] =
                basc_support_env.req_ntf_cfg[param->bas_nb];
    }

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : int Batt_EnableRsp_Client(ke_msg_id_t const msg_id,
 *                                    struct basc_enable_rsp
 *                                    const *param,
 *                                    ke_task_id_t const dest_id,
 *                                    ke_task_id_t const src_id)
 * ----------------------------------------------------------------------------
 * Description   : Receive a response for client battery service enable request
 * Inputs        : - msg_id     - Kernel message ID number
 *                 - param      - Message parameters in format of
 *                                struct basc_enable_rsp
 *                 - dest_id    - Destination task ID number
 *                 - src_id     - Source task ID number
 * Outputs       : return value - Indicate if the message was consumed;
 *                                compare with KE_MSG_CONSUMED
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
int Batt_EnableRsp_Client(ke_msg_id_t const msg_id,
                   struct basc_enable_rsp const *param,
                   ke_task_id_t const dest_id,
                   ke_task_id_t const src_id)
{

    if(param->status == GAP_ERR_NO_ERROR)
    {
        memcpy(&basc_support_env.bas, param->bas, sizeof(struct bas_content));

        basc_support_env.enable = true;

        Batt_SendLevelNtfCfgReq(ble_env[DEVICE_NUM_CENTRAL].conidx, 0, basc_support_env.req_ntf_cfg[0]);

        if(basc_support_env.bas_nb == 2)
        {
            Batt_SendLevelNtfCfgReq(ble_env[DEVICE_NUM_CENTRAL].conidx, 1, basc_support_env.req_ntf_cfg[1]);
        }

    }

    /* Enable pending client services to be enable */
    ServiceEnable(ble_env[DEVICE_NUM_CENTRAL].conidx);

    return (KE_MSG_CONSUMED);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_SendReadInfoReq(uint8_t conidx,
 *                                           uint8_t bas_nb,
 *                                           uint8_t info)
 * ----------------------------------------------------------------------------
 * Description   : Send a request to read a battery service info from peer
 * Inputs        : - conidx     - Connection index,
 *                 - bas_nb     - Battery instance number
 *                 - info       - Info type
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Batt_SendReadInfoReq(uint8_t conidx, uint8_t bas_nb, uint8_t info)
{
    struct basc_read_info_req *req;
    struct basc_env_tag* basc_env;

    basc_env = PRF_ENV_GET(BASC, basc);

    req = KE_MSG_ALLOC(BASC_READ_INFO_REQ,
                       prf_src_task_get(&(basc_env->prf_env), conidx),
                       TASK_APP, basc_read_info_req);

    /* Fill in the parameter structure  */
    req->bas_nb = bas_nb;

    /* BASC_BATT_LVL_VAL, BASC_NTF_CFG, BASC_BATT_LVL_PRES_FORMAT */
    req->info = info;

    /* Send the message */
    ke_msg_send(req);
}

/* ----------------------------------------------------------------------------
 * Function      : void Batt_SendLevelNtfCfgReq(uint8_t conidx,
 *                                              uint8_t bas_nb,
 *                                              uint8_t ntf_cfg)
 * ----------------------------------------------------------------------------
 * Description   : Send a request to change battery level notification
 *                 configuration in peer device
 * Inputs        : - conidx     - Connection index,
 *                 - bas_nb     - Battery instance number
 *                 - ntf_cfg    - The value of notification config

 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Batt_SendLevelNtfCfgReq(uint8_t conidx, uint8_t bas_nb, uint8_t ntf_cfg)
{

    struct basc_batt_level_ntf_cfg_req *req;
    struct basc_env_tag* basc_env;

    basc_env = PRF_ENV_GET(BASC, basc);
    req = KE_MSG_ALLOC(BASC_BATT_LEVEL_NTF_CFG_REQ,
                       prf_src_task_get(&(basc_env->prf_env), conidx),
                       TASK_APP, basc_batt_level_ntf_cfg_req);

    /* Save the value to be confirmed when notification response is received */
    basc_support_env.req_ntf_cfg[bas_nb] = ntf_cfg;

    /* Fill in the parameter structure */
    req->bas_nb = bas_nb;
    req->ntf_cfg = ntf_cfg;

    /* Send the message  */
    ke_msg_send(req);
}

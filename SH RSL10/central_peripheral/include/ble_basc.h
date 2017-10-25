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
 * ble_basc.h
 * - Bluetooth battery client header
 * ----------------------------------------------------------------------------
 * $Revision: 1.2 $
 * $Date: 2017/06/16 21:09:57 $
 * ------------------------------------------------------------------------- */

#ifndef BLE_BASC_H
#define BLE_BASC_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------------------------------------------------------------
 * Include files
 * --------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/
/* List of message handlers that are used by the battery service server
 * application manager */
#define BASC_MESSAGE_HANDLER_LIST \
        DEFINE_MESSAGE_HANDLER(BASC_ENABLE_RSP, Batt_EnableRsp_Client), \
        DEFINE_MESSAGE_HANDLER(BASC_READ_INFO_RSP, Batt_ReadInfoRsp), \
        DEFINE_MESSAGE_HANDLER(BASC_BATT_LEVEL_IND, Batt_LevelInd), \
        DEFINE_MESSAGE_HANDLER(BASC_BATT_LEVEL_NTF_CFG_RSP, Batt_LevelNtfCfgRsp)

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/
struct basc_support_env_tag
{
    uint8_t bas_nb;
    uint8_t batt_lvl[BASC_NB_BAS_INSTANCES_MAX];
    uint8_t ntf_cfg[BASC_NB_BAS_INSTANCES_MAX];
    uint8_t req_ntf_cfg[BASC_NB_BAS_INSTANCES_MAX];
    struct prf_char_pres_fmt char_pres_format[BASC_NB_BAS_INSTANCES_MAX];
    bool enable;

    struct bas_content bas[BASC_NB_BAS_INSTANCES_MAX];
};

/* Support for the application manager and the application environment */
extern struct basc_support_env_tag basc_support_env;

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
extern void Basc_Env_Initialize(void);
extern void Batt_ServiceAdd_Client(void);
extern void Batt_ServiceEnable_Client(uint8_t conidx);
extern int Batt_LevelNtfCfgRsp(ke_msg_id_t const msgid,
                               struct basc_batt_level_ntf_cfg_rsp *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);
extern void Batt_SendReadInfoReq(uint8_t conidx, uint8_t bas_nb,
            uint8_t info);
extern void Batt_SendLevelNtfCfgReq(uint8_t conidx, uint8_t bas_nb,
                                    uint8_t ntf_cfg);
extern int Batt_EnableRsp_Client(ke_msg_id_t const msgid,
                          struct basc_enable_rsp const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id);
extern int Batt_ReadInfoRsp(ke_msg_id_t const msgid,
                            struct basc_read_info_rsp *param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id);
extern int Batt_LevelInd(ke_msg_id_t const msgid,
                         struct basc_batt_level_ind *param,
                         ke_task_id_t const dest_id,
                         ke_task_id_t const src_id);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* BLE_BASC_H */

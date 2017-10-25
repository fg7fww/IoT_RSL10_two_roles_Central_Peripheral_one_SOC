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
 * ble_std.h
 * - Bluetooth standard header
 * ----------------------------------------------------------------------------
 * $Revision: 1.3 $
 * $Date: 2017/07/24 22:19:37 $
 * ------------------------------------------------------------------------- */

#ifndef BLE_STD_H
#define BLE_STD_H

/* ----------------------------------------------------------------------------
 * If building with a C++ compiler, make all of the definitions in this header
 * have a C binding.
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* Number of APP Task Instances */
#define APP_IDX_MAX                     1

/* Length of Bluetooth Address (in octets) */
#define BDADDR_LENGTH                   6

/* Advertising channel map - 37, 38, 39 */
#define APP_ADV_CHMAP                   0x07

/* Advertising minimum interval - 40ms (64*0.625ms) */
#define APP_ADV_INT_MIN                 64

/* Advertising maximum interval - 40ms (64*0.625ms) */
#define APP_ADV_INT_MAX                 64

/* Length of Bluetooth Address (in octets) */
#define BDADDR_LENGTH                   6

/* Non-resolvable private Bluetooth device address.
 * If public address does not exist, the two MSBs must be zero. */
#define PRIVATE_BDADDR                  {0x94,0x11,0x11,0xff,0xff,0xC8}		// Peripheral
#define PRIVATE_BDADDR_CENTRAL          {0x94,0x11,0x11,0xff,0xff,0xC7}		// Central

#define BD_TYPE_PUBLIC                  0
#define BD_TYPE_PRIVATE                 1

/* Peer Bluetooth device address (1)*/
#define DIRECT_PEER_BD_ADDRESS          	{0x94,0x11,0x11,0xff,0xff,0xC7}	// Peripheral
#define DIRECT_PEER_BD_ADDRESS_CENTRAL      {0x94,0x11,0x11,0xff,0xff,0xC8}	// Central
#define DIRECT_PEER_BD_ADDRESS_TYPE     BD_TYPE_PRIVATE

/* Slave preferred connection parameters */
#define PREF_SLV_MIN_CON_INTERVAL       8
#define PREF_SLV_MAX_CON_INTERVAL       10
#define PREF_SLV_LATENCY                0
#define PREF_SLV_SUP_TIMEOUT            200

/* Set the device name */
#define APP_DEVICE_NAME_LENGTH_MAX      20
#define APP_DFLT_DEVICE_NAME            "Central_Peripheral"

/* ON SEMICONDUCTOR Company ID */
#define APP_COMPANY_ID_DATA             {0x4, 0xff, 0x62, 0x3, 0x3}
#define APP_COMPANY_ID_DATA_LEN         (0x4 + 1)

/* Vendor specific scan response data (ON SEMICONDUCTOR Company ID) */
#define APP_SCNRSP_DATA                 APP_COMPANY_ID_DATA
#define APP_SCNRSP_DATA_LEN             APP_COMPANY_ID_DATA_LEN


/* Set scan interval to 62.5ms and scan window to 50% of the interval */
#define SCAN_INERVAL                    100
#define SCAN_WINDOW                     50

/* Set the connection interval to 7.5ms and slave latency to zero */
#define CON_INTERVAL_MIN                6
#define CON_INTERVAL_MAX                6
#define CON_SLAVE_LATENCY               0

/* Set supervisory timeout to 3s */
#define CON_SUP_TIMEOUT                 300

/* GAPM configuration definitions */
#define RENEW_DUR                       15000
#define MTU_MAX                         0x200
#define MPS_MAX                         0x200
#define ATT_CFG                         0x80
#define TX_OCT_MAX                      0x1b
#define TX_TIME_MAX                     (14 * 8 + TX_OCT_MAX * 8)

/* Define the available application states */
enum appm_state
{
    /* Initialization state */
    APPM_INIT,
    /* Database create state */
    APPM_CREATE_DB,
    /* Ready State */
    APPM_READY,
    /* Advertising state */
    APPM_ADVERTISING,
    /* Connecting state */
    APPM_CONNECTING,
    /* Connected state */
    APPM_CONNECTED,
    /* Number of defined states */
    APPM_STATE_MAX
};

/* List of message handlers that are used by the Bluetooth application manager */
#define BLE_MESSAGE_HANDLER_LIST \
        DEFINE_MESSAGE_HANDLER(GAPM_CMP_EVT, GAPM_CmpEvt),\
        DEFINE_MESSAGE_HANDLER(GAPM_PROFILE_ADDED_IND, GAPM_ProfileAddedInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_CONNECTION_REQ_IND, GAPC_ConnectionReqInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_CMP_EVT, GAPC_CmpEvt),\
        DEFINE_MESSAGE_HANDLER(GAPC_DISCONNECT_IND, GAPC_DisconnectInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_GET_DEV_INFO_REQ_IND, GAPC_GetDevInfoReqInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_PARAM_UPDATED_IND, GAPC_ParamUpdatedInd),\
        DEFINE_MESSAGE_HANDLER(GAPC_PARAM_UPDATE_REQ_IND, GAPC_ParamUpdateReqInd),\
        DEFINE_MESSAGE_HANDLER(APP_SWITCH_ROLE_TIMEOUT, APP_SwitchRole_Timeout)

/* Standard declaration/description UUIDs in 16-byte format */
#define ATT_DECL_CHARACTERISTIC_128     {0x03,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define ATT_DESC_CLIENT_CHAR_CFG_128    {0x02,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define ATT_DESC_CHAR_USER_DESC_128     {0x01,0x29,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

/* Support for the application manager and the application environment */
extern const struct ke_state_handler    appm_default_handler;
extern ke_state_t                       appm_state[APP_IDX_MAX];

struct ble_env_tag {
    /* Connection handle */
    uint16_t conhdl;

    /* Connection index */
    uint8_t conidx;

    /* Next service to initialize */
    uint8_t next_svc;

    /* Next service to enable */
    uint8_t next_svc_enable;

    /* Bond status */
    bool bonded;

    /* Application state */
    uint8_t state;

    /* Connection parameters */
    uint16_t con_interval;
    uint16_t time_out;
    uint16_t con_latency;

    uint16_t updated_con_interval;
    uint16_t updated_latency;
    uint16_t updated_suo_to;
};

/* Support for the application manager and the application environment */
extern struct ble_env_tag ble_env[];

/* Bluetooth Device Address */
extern uint8_t                          bdaddr[BDADDR_LENGTH];

/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/

/* Bluetooth baseband application support functions */
extern void BLE_Initialize(void);
extern bool Service_Add(void);
extern void Connection_SendStartCmd(void);
extern void BLE_SetServiceState(bool enable, uint8_t conidx);
extern bool ServiceEnable(uint8_t conidx);
extern void Advertising_Start(void);
extern uint8_t DeviceIndx(uint8_t conidx);
extern void BLE_Operation_Cancel(int device_indx);
extern void BLE_Connection_SelectBegin(uint8_t device_indx);

/* Bluetooth event and message handlers */
extern int GAPM_ProfileAddedInd(ke_msg_id_t const msgid,
                               struct gapm_profile_added_ind const *param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id);
extern int GAPM_CmpEvt(ke_msg_id_t const msgid,
                       struct gapm_cmp_evt const *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id);
extern int GAPC_CmpEvt(ke_msg_id_t const msgid,
                       struct gapc_cmp_evt const *param,
                       ke_task_id_t const dest_id,
                       ke_task_id_t const src_id);
extern int Gapc_SetDevInfoReqInd(ke_msg_id_t const msgid,
                                 struct gapc_set_dev_info_req_ind const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);
extern int GAPC_DisconnectInd(ke_msg_id_t const msgid,
                              struct gapc_disconnect_ind const *param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id);
extern int GAPC_ParamUpdatedInd(ke_msg_id_t const msgid,
                                struct gapc_param_updated_ind const *param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id);
extern int GAPC_ParamUpdateReqInd(ke_msg_id_t const msg_id,
                           struct gapc_param_update_req_ind const *param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id);
extern  int GAPC_ConnectionReqInd(ke_msg_id_t const msgid,
                                  struct gapc_connection_req_ind const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id);
extern int GAPC_GetDevInfoReqInd(ke_msg_id_t const msg_id,
                                 struct gapc_get_dev_info_req_ind const *param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* BLE_STD_H */

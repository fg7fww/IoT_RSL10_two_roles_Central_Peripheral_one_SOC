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
 * app.h
 * - Main application header
 * ----------------------------------------------------------------------------
 * $Revision: 1.23 $
 * $Date: 2017/07/07 13:33:13 $
 * ------------------------------------------------------------------------- */

#ifndef APP_H
#define APP_H

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
#include <rsl10.h>
#include <rsl10_ke.h>
#include <rsl10_ble.h>
#include <rsl10_profiles.h>
#include <rsl10_map_nvr.h>
#include <stdbool.h>

#include "app_spi.h"

#include "ble_std.h"
#include "ble_custom.h"
#include "ble_bass.h"
#include "ble_basc.h"

/* ----------------------------------------------------------------------------
 * Defines
 * --------------------------------------------------------------------------*/

/* DIO  numbers definition */
#define GPIO_TEST_1                     0
#define GPIO_TEST_2                     1

/* DIO number that is connected to LED of EVB */
#define LED_DIO_NUM                     6

/* Minimum and maximum VBAT measurements */
#define VBAT_1p1V_MEASURED              0x1200
#define VBAT_1p4V_MEASURED              0x16cc

/* Set timer to 200 ms (20 times the 10 ms kernel timer resolution) */
#define TIMER_200MS_SETTING             20

/* Set timer to 10 s (1000 times the 10 ms kernel timer resolution) */
#define APP_SWITCH_ROLE_TIMER           1000

extern const struct ke_task_desc TASK_DESC_APP;

/* Device index */
enum device_number_index {
    DEVICE_NUM_PERIPHERAL = 0,
    DEVICE_NUM_CENTRAL,
    DEVICE_NUM_MAX
};

/* APP Task messages */
enum appm_msg {

    APPM_DUMMY_MSG = TASK_FIRST_MSG(TASK_ID_APP),

    /* Timer used to have a tick periodically for application */
    APP_TEST_TIMER,

    APP_SWITCH_ROLE_TIMEOUT
};

typedef void (*appm_add_svc_func_t)(void);
#define DEFINE_SERVICE_ADD_FUNCTION(func) (appm_add_svc_func_t) func
#define DEFINE_MESSAGE_HANDLER(message, handler) {message, (ke_msg_func_t) handler}

/* List of message handlers that are used by the different profiles/services */
#define APP_MESSAGE_HANDLER_LIST \
        DEFINE_MESSAGE_HANDLER(APP_TEST_TIMER, APP_Timer),\

/* List of functions used to create the database */
#define SERVICE_ADD_FUNCTION_LIST \
        DEFINE_SERVICE_ADD_FUNCTION(Batt_ServiceAdd_Server),\
        DEFINE_SERVICE_ADD_FUNCTION(Batt_ServiceAdd_Client),\
        DEFINE_SERVICE_ADD_FUNCTION(CustomService_ServiceAdd)

typedef void (*appm_enable_svc_func_t)(uint8_t);
#define DEFINE_SERVICE_ENABLE_FUNCTION(func) (appm_enable_svc_func_t) func

/* List of functions used to enable client services */
#define SERVICE_ENABLE_FUNCTION_LIST \
        DEFINE_SERVICE_ENABLE_FUNCTION(Batt_ServiceEnable_Client), \
        DEFINE_SERVICE_ENABLE_FUNCTION(CustomService_ServiceEnable)


/* The number of services that are not custom and are added */
#define BLE_NUM_SVC                     1

/* ----------------------------------------------------------------------------
 * Global variables and types
 * --------------------------------------------------------------------------*/

struct app_env_tag
{
    /* Battery service Server*/
    uint8_t batt_lvl;
    uint32_t sum_batt_lvl;
    uint16_t num_batt_read;
    uint8_t send_batt_ntf;

    /* Battery service Client*/
    uint8_t send_batt_req;
    uint8_t wrt_cs_serv;

    /* Application role control */
    uint32_t app_role_control;
};

/* Support for the application manager and the application environment */
extern struct app_env_tag app_env;

/* List of functions used to create the database */
extern const appm_add_svc_func_t appm_add_svc_func_list[];

/* List of functions used to enable client services */
extern const appm_enable_svc_func_t appm_enable_svc_func_list[];

/* ---------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
extern void App_Initialize(void);
extern void App_Env_Initialize(void);

extern int APP_Timer(ke_msg_id_t const msg_id, void const *param,
                     ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int Msg_Handler(ke_msg_id_t const msgid, void *param,
                       ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int APP_SwitchRole_Timeout(ke_msg_id_t const msg_id, void const *param,
        ke_task_id_t const dest_id, ke_task_id_t const src_id);
/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* APP_H */

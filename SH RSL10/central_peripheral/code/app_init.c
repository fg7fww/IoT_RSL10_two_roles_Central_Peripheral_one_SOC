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
 * app_init.c
 * - Application initialization
 * ----------------------------------------------------------------------------
 * $Revision: 1.30 $
 * $Date: 2017/06/16 17:55:11 $
 * ------------------------------------------------------------------------- */

#include "app.h"

/* Application Environment Structure */
struct app_env_tag app_env;

/* SPI1 Declarations */
uint8_t spi1_rx[4];

/* ----------------------------------------------------------------------------
 * Function      : void App_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize the system for proper application execution
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void App_Initialize(void)
{
    /* Mask all interrupts */
    __set_PRIMASK(PRIMASK_DISABLE_INTERRUPTS);

    /* Disable all interrupts and clear any pending interrupts */
    Sys_NVIC_DisableAllInt();
    Sys_NVIC_ClearAllPendingInt();

    /* Configure the current trim settings for VCC, VDDA */
    ACS_VCC_CTRL->ICH_TRIM_BYTE = VCC_ICHTRIM_16MA_BYTE;
    ACS_VDDA_CP_CTRL->PTRIM_BYTE = VDDA_PTRIM_16MA_BYTE;

    /* Start 48 MHz XTAL oscillator */
    ACS_VDDRF_CTRL->ENABLE_ALIAS = VDDRF_ENABLE_BITBAND;
    ACS_VDDRF_CTRL->CLAMP_ALIAS = VDDRF_DISABLE_HIZ_BITBAND;

    /* Wait until VDDRF supply has powered up */
    while (ACS_VDDRF_CTRL->READY_ALIAS != VDDRF_READY_BITBAND);

    ACS_VDDPA_CTRL->ENABLE_ALIAS = VDDPA_DISABLE_BITBAND;
    ACS_VDDPA_CTRL->VDDPA_SW_CTRL_ALIAS = VDDPA_SW_VDDRF_BITBAND;

    /* Enable RF power switches */
    SYSCTRL_RF_POWER_CFG->RF_POWER_ALIAS = RF_POWER_ENABLE_BITBAND;

    /* Remove RF isolation */
    SYSCTRL_RF_ACCESS_CFG->RF_ACCESS_ALIAS = RF_ACCESS_ENABLE_BITBAND;

    /* Start the 48 MHz oscillator without changing the other register bits */
    RF->XTAL_CTRL = ((RF->XTAL_CTRL & ~XTAL_CTRL_DISABLE_OSCILLATOR) |
                     XTAL_CTRL_REG_VALUE_SEL_INTERNAL);

    /* Enable the 48 MHz oscillator divider using the desired prescale value */
    RF_REG2F->CK_DIV_1_6_CK_DIV_1_6_BYTE = CK_DIV_1_6_PRESCALE_6_BYTE;

    /* Wait until 48 MHz oscillator is started */
    while (RF_REG39->ANALOG_INFO_CLK_DIG_READY_ALIAS != 
           ANALOG_INFO_CLK_DIG_READY_BITBAND);

    /* Switch to (divided 48 MHz) oscillator clock */
    Sys_Clocks_SystemClkConfig(JTCK_PRESCALE_1   |
                               EXTCLK_PRESCALE_1 |
                               SYSCLK_CLKSRC_RFCLK);

    /* Configure clock dividers */
    CLK->DIV_CFG0 = (SLOWCLK_PRESCALE_8 | BBCLK_PRESCALE_1 | 
                     USRCLK_PRESCALE_1);
    CLK->DIV_CFG2 = (CPCLK_PRESCALE_8 | DCCLK_PRESCALE_2);

    BBIF->CTRL = (BB_CLK_ENABLE | BBCLK_DIVIDER_8 | BB_WAKEUP);

    /* Configure ADC channel 0 to measure VBAT/2 */
    Sys_ADC_Set_Config(ADC_VBAT_DIV2_NORMAL | ADC_NORMAL |
                       ADC_PRESCALE_6400);
    Sys_ADC_InputSelectConfig(0,
                              (ADC_NEG_INPUT_GND |
                               ADC_POS_INPUT_VBAT_DIV2));

    /* Configure DIOs */
    Sys_DIO_Config(LED_DIO_NUM, DIO_MODE_GPIO_OUT_0);
    Sys_DIO_Config(GPIO_TEST_1, DIO_MODE_GPIO_OUT_0);
    Sys_DIO_Config(GPIO_TEST_2, DIO_MODE_GPIO_OUT_0);

    /* ------------------------------------------------------------------------
     * SPI Configurations
     * --------------------------------------------------------------------- */
	/*
	 * clk		- 12
	 * cs		- 9
	 * seri		- 10
	 * sero		- 11
	 */
	Sys_SPI_DIOConfig(1, SPI1_SELECT_SLAVE,
			        DIO_LPF_DISABLE | DIO_WEAK_PULL_UP, 12, 9, 10, 11);

    Sys_SPI_Config(1, SPI1_SELECT_SLAVE | SPI1_ENABLE |
        SPI1_CLK_POLARITY_NORMAL |SPI1_UNDERRUN_INT_ENABLE| SPI1_CONTROLLER_DMA |
        SPI1_MODE_SELECT_AUTO | SPI1_PRESCALE_32);

    Sys_SPI_TransferConfig(1, SPI1_IDLE | SPI1_RW_DATA | SPI1_CS_1 |
            SPI1_WORD_SIZE_32);

    /*SPI1 request slave for master service*/
    Sys_DIO_Config(7, DIO_WEAK_PULL_UP | DIO_MODE_GPIO_OUT_1);
    Sys_DIO_Config(8, DIO_NO_PULL | DIO_MODE_GPIO_IN_0);

    /* Application role control */
    //uint32_t delay = (uint32_t) 2048; //Delay for data input pin scan
    //while (delay>0){delay--;}
    app_env.app_role_control = DIO->DATA & (1 << ((uint32_t) 8));


    /* Configure the SPI1 DMA channel, clear status and enable DMA channel */
    Sys_DMA_ChannelConfig(DMA_SPI1R_NUM, DMA_SPI1R_CFG, 1, 0, (uint32_t) &SPI1->RX_DATA,
                         (uint32_t) &spi1_rx[0]);
    Sys_DMA_ClearChannelStatus(DMA_SPI1R_NUM);
    Sys_DMA_ChannelEnable(DMA_SPI1R_NUM);


    /* Configure and start timer 0 with a period of 200 ms */
    Sys_Timer_Set_Control(0, TIMER_FREE_RUN | TIMER_PRESCALE_8 |
                          TIMER_200MS_SETTING);
    Sys_Timers_Start(SELECT_TIMER0);

    NVIC_EnableIRQ(DMA0_IRQn);


    /* Initialize the baseband and BLE stack */
    BLE_Initialize();

    /* Initialize environment */
    App_Env_Initialize();

    /* Stop masking interrupts */
    __set_PRIMASK(PRIMASK_ENABLE_INTERRUPTS);
    __set_FAULTMASK(FAULTMASK_ENABLE_INTERRUPTS);
}

/* ----------------------------------------------------------------------------
 * Function      : void App_Env_Initialize(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize application environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void App_Env_Initialize(void)
{
    /* Reset the application manager environment */
    memset(&app_env, 0, sizeof(app_env));

    /* Create the application task handler */
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    /* Initialize the custom service environment */
    CustomService_Env_Initialize();

    /* Initialize the battery service server environment */
    Bass_Env_Initialize();

    /* Initialize the battery service server environment */
    Basc_Env_Initialize();

    /* Reset the application manager environment */
    memset(&app_env, 0, sizeof(app_env));

    /* Create the application task handler */
    ke_task_create(TASK_APP, &TASK_DESC_APP);

}

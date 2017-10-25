/*
 * app_spi.h
 *
 *  Created on: Oct 4, 2017
 *      Author: fg7fww
 */

#ifndef APP_SPI_H_
#define APP_SPI_H_

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
/* SPI */
#define DMA_SPI1R_NUM					0
#define	DMA_SPI1R_RESET					0x10000001
#define DMA_SPI1R_DUMMY					0x00000005

#define DMA_SPI1R_CFG      (DMA_LITTLE_ENDIAN | \
        DMA_ENABLE | \
        DMA_DISABLE_INT_DISABLE | \
        DMA_ERROR_INT_DISABLE | \
        DMA_COMPLETE_INT_ENABLE | \
        DMA_COUNTER_INT_DISABLE | \
        DMA_START_INT_DISABLE | \
        DMA_DEST_WORD_SIZE_32 | \
        DMA_SRC_WORD_SIZE_32 | \
		DMA_SRC_SPI1 | \
        DMA_PRIORITY_0 | \
        DMA_TRANSFER_P_TO_M | \
        DMA_DEST_ADDR_INC | \
        DMA_SRC_ADDR_STATIC | \
        DMA_ADDR_LIN)

extern uint8_t spi1_rx[];
/* ----------------------------------------------------------------------------
 * Function prototype definitions
 * --------------------------------------------------------------------------*/
extern void DMA0_IRQHandler(void);

/* ----------------------------------------------------------------------------
 * Close the 'extern "C"' block
 * ------------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* APP_SPI_H_ */

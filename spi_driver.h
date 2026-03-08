#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>    // uint32_t, size_t etc.
#include <linux/errno.h>    // -EINVAL, -EIO etc.
#include <linux/printk.h>   // pr_err, pr_info

// #include <linux/proc_fs.h>
// #include <linux/slab.h>

// #include <asm/io.h>

#define SPI_PERIPH_BASE 0x7E204000
#define SPI0_BASE (SPI_PERIPH_BASE + 0U)

#define SPI_APB_CLK_HZ   250000000UL  // only reliable after locking core_freq

#define SPI_CS_CSPOL2_Pos   23U // Chip select 2 Polarity : 0 -> active Low, 1 -> active High
#define SPI_CS_CSPOL1_Pos   22U // Chip select 1 Polarity : 0 -> active Low, 1 -> active High
#define SPI_CS_CSPOL0_Pos   21U // Chip select 0 Polarity : 0 -> active Low, 1 -> active High
#define SPI_CS_RXF_Pos      20U // RX FIFO Full : 0 -> not full, 1 -> full, no further serial data will be sent/recv until u read from FIFO
#define SPI_CS_TXD_Pos      18U // TX FIFO can accept Data : 0 -> TX FIFO full and cannot accept more, 1 -> has space for at least 1 B
#define SPI_CS_RXD_Pos      17U // RX FIFO contains Data : 0 -> RX FIFO empty, 1 -> contains at least 1 B
#define SPI_CS_DONE_Pos     16U // 0 Transfer in progress or not active so TA = 0, 1 Transfer is complete, cleared by writing more data to the TX FIFO or setting TA = 0
#define SPI_CS_INTR_Pos     10U // 0 Don't generate interrupts on RX FIFO condition, 1 generate interrupt while RXR = 1
#define SPI_CS_INTD_Pos      9U // 0 Don't generate interrupts on Transfer complete, 1 generate interrupt when DONE = 1
#define SPI_CS_TA_Pos        7U // 0 transfer not active, 1 transfer active
#define SPI_CS_CSPOL_Pos     6U // 0 chip select lines are active Low, 1 chip select lines are active High
#define SPI_CS_CLEAR_RX_Pos  5U // 0 no action, 1 clear RX FIFO
#define SPI_CS_CLEAR_TX_Pos  4U // 0 no action, 1 clear TX FIFO
#define SPI_CS_CPOL_Pos      3U // Clock polarity: 0 rest of clk = 0, 1 rest of clk = 1
#define SPI_CS_CPHA_Pos      2U // 0 first CLK transition at middle of data  bit, 1 in the beginning
#define SPI_CS_CS_Pos        0U // chip select

#define SPI_CS_2             2U // chip select 2
#define SPI_CS_1             1U // chip select 1
#define SPI_CS_0             0U // chip select 0

#define CLEAR_RX_MASK        (1U << SPI_CS_CLEAR_RX_Pos)
#define CLEAR_TX_MASK        (1U << SPI_CS_CLEAR_TX_Pos)
#define DISABLE_RX_INT_MASK  (0U)
#define ENABLE_RX_INT_MASK   (1U << SPI_CS_INTR_Pos)
#define DISABLE_TX_INT_MASK  (0U)
#define ENABLE_TX_INT_MASK   (1U << SPI_CS_INTD_Pos)
#define TA_SET_MASK          (1U << SPI_CS_TA_Pos)

#define SPI_CS_ACTIVE_LOW  (0U)
#define SPI_CS_ACTIVE_HIGH (1U)

#define SPI_CPOL  (1U << SPI_CS_CPOL_Pos)
#define SPI_CPHA  (1U << SPI_CS_CPHA_Pos)
#define SPI_GET_CPOL(mode) (((mode) >> 1U) & 1U)
#define SPI_GET_CPHA(mode) (((mode) >> 0U) & 1U)


#define SPI_MODE_0   (0U)   // CPOL=0, CPHA=0 : idle low,  sample on rising  edge
#define SPI_MODE_1   (1U)   // CPOL=0, CPHA=1 : idle low,  sample on falling edge
#define SPI_MODE_2   (2U)   // CPOL=1, CPHA=0 : idle high, sample on falling edge
#define SPI_MODE_3   (3U)   // CPOL=1, CPHA=1 : idle high, sample on rising  edge

#define SPI_MODE_0   (0U)
#define SPI_MODE_1   (1U)
#define SPI_MODE_2   (2U)
#define SPI_MODE_3   (3U)


#define SPI_CLK_CDIV_Pos   0U
#define SPI_CLK_CDIV_Msk   (0xFFFFU << SPI_CLK_CDIV_Pos)  // Clock divider mask


typedef struct 
{
    volatile uint32_t CS;   // SPI Master Control and Status Register
    volatile uint32_t FIFO; // SPI Master TX and RX FIFO
    volatile uint32_t CLK;  // SPI Master Clock Divider
    volatile uint32_t DLEN; // SPI Master Data length
    volatile uint32_t LTOH; // We'll not use this (LoSSi mode)
    volatile uint32_t DC;   // We'll not use this (DMA mode)
} spi_reg_t;

#define SPI0  ((spi_reg_t*)(SPI0_BASE))

typedef struct 
{
    uint32_t CLK_freq;
    uint32_t mode;
    uint32_t Chip_select;
    uint32_t CS_polarity;
} spi_handle_t;

int spi_init(spi_handle_t* spi_handle, spi_reg_t* spi_base);
int spi_write(uint8_t byte);
int spi_write_bytes(uint8_t* byte, size_t size);
int spi_read(uint8_t* byte);
int spi_read_bytes(uint8_t* byte, size_t* size);

#endif
#include "spi_driver.h"

int spi_init(spi_handle_t* spi_handle, spi_reg_t* spi_base)
{
    if (!spi_handle || !spi_base) {
        #ifdef DEBUG
        pr_err("SPI: null pointer passed to spi_init\n");
        #endif
        return -EINVAL;
    }

    if (spi_handle->CLK_freq == 0) {
        #ifdef DEBUG
        pr_err("SPI: CLK_freq cannot be 0\n");
        #endif
        return -EINVAL;
    }

    uint32_t clock_divider = SPI_APB_CLK_HZ / spi_handle->CLK_freq;

    if (clock_divider & 1U)
        clock_divider &= 0xFFFE;

    if (clock_divider < 2U)
        clock_divider = 2U;

    if (clock_divider > SPI_APB_CLK_HZ / 2) {
        #ifdef DEBUG
        pr_err("SPI: CLK_freq cannot be reached\n");
        #endif
        return -EINVAL;
    }

    spi_base->CLK = (clock_divider << SPI_CLK_CDIV_Pos) & SPI_CLK_CDIV_Msk;
    #ifdef DEBUG
    pr_info("SPI: CLK configured — CDIV=%u actual_freq=%lu Hz\n",
            clock_divider, SPI_APB_CLK_HZ / clock_divider);
    #endif

    // clear CS register before
    spi_base->CS &= ~(SPI_CPOL | SPI_CPHA);
    if (spi_handle->mode > SPI_MODE_3) {
        #ifdef DEBUG
        pr_err("SPI: Invalid mode %u, must be 0-3\n", spi_handle->mode);
        #endif
        return -EINVAL;
    }
    
    if (SPI_GET_CPOL(spi_handle->mode)) 
        spi_base->CS |= SPI_CPOL;
    
    if (SPI_GET_CPHA(spi_handle->mode)) 
        spi_base->CS |= SPI_CPHA;

    #ifdef DEBUG
    pr_info("SPI: Mode configured succesfully : %u\n", spi_handle->mode);
    #endif

    if (spi_handle->Chip_select > SPI_CS_2) {
        #ifdef DEBUG
        pr_err("SPI: Invalid Chip select %u, must be 0-2\n", spi_handle->Chip_select);
        #endif
        return -EINVAL;
    }

    // clear before set
    spi_base->CS &= ~(0x3U << SPI_CS_CS_Pos);
    spi_base->CS |= (spi_handle->Chip_select << SPI_CS_CS_Pos);

    #ifdef DEBUG
    pr_info("SPI: Chip select configured succesfully : %u\n", spi_handle->Chip_select);
    #endif

    if (spi_handle->CS_polarity > SPI_CS_ACTIVE_HIGH) {
        #ifdef DEBUG
        pr_err("SPI: Invalid Chip select Polarity %u, must be 0-1\n", spi_handle->CS_polarity);
        #endif
        return -EINVAL;
    }

    spi_base->CS |= ((spi_handle->CS_polarity & SPI_CS_ACTIVE_HIGH) << SPI_CS_CSPOL_Pos);
    #ifdef DEBUG
    pr_info("SPI: Chip select Polarity configured succesfully : %u\n", spi_handle->CS_polarity);
    #endif

    if (spi_handle->Technique > SPI_DMA)
    {
        #ifdef DEBUG
        pr_err("SPI: Invalid Technique or not supported yet: %u\n", spi_handle->Technique);
        #endif
        return -EINVAL;
    }

    if (spi_handle->Technique == SPI_POLLING) {
        spi_disable_int(spi_base);
        #ifdef DEBUG
        pr_info("SPI: We'll use polling technique\n");
        #endif
    } else if (spi_handle->Technique == SPI_INTERUPT) {
        pr_err("SPI: Technique not supported yet: %u\n", spi_handle->Technique);
        return -EINVAL;
    } else if (spi_handle->Technique == SPI_DMA) {
        pr_err("SPI: Technique not supported yet: %u\n", spi_handle->Technique);
        return -EINVAL;
    }

    // clear FIFO's
    spi_base->CS |= (CLEAR_RX_MASK | CLEAR_TX_MASK); 
    #ifdef DEBUG
    pr_info("SPI: FIFO's cleared succesfully\n Init Done\nYou can use the peripheral the now\n");
    #endif   
    return 0;
}

int spi_disable_int(spi_reg_t* spi_base)
{   
    if (!spi_base) {
        #ifdef DEBUG
        pr_err("SPI: null pointer passed to spi_disable_int\n");
        #endif
        return -EINVAL;
    }

    spi_base->CS &= (DISABLE_RX_INT_MASK & DISABLE_TX_INT_MASK);
    return 0;
}

int spi_enable_int(spi_reg_t* spi_base)
{   
    if (!spi_base) {
        #ifdef DEBUG
        pr_err("SPI: null pointer passed to spi_disable_int\n");
        #endif
        return -EINVAL;
    }

    spi_base->CS |= (ENABLE_RX_INT_MASK | ENABLE_TX_INT_MASK);
    return 0;
}

void spi_write(spi_reg_t* spi_base, uint8_t byte)
{
    spi_base->CS |= TA_SET_MASK;
    while (!SPI_GET_TXD(spi_base->CS));
    #ifdef DEBUG
    pr_info("SPI WRITE: TX FIFO can accept data\n");
    #endif 

    spi_base->FIFO = byte;

    while (!SPI_GET_DONE(spi_base->CS));
    #ifdef DEBUG
    pr_info("SPI WRITE: Transfer Done\n");
    #endif 

    // only sending, don't care what the slave sent back during that transfer
    while (SPI_GET_RXD(spi_base->CS))
        (void)spi_base->FIFO;

    spi_base->CS &= ~(TA_SET_MASK);
}
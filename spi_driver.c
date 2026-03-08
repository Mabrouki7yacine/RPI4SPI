#include "spi_driver.h"

int spi_init(spi_handle_t* spi_handle, spi_reg_t* spi_base)
{
    if (!spi_handle || !spi_base) {
        pr_err("SPI: null pointer passed to spi_init\n");
        return -EINVAL;
    }

    if (spi_handle->CLK_freq == 0) {
        pr_err("SPI: CLK_freq cannot be 0\n");
        return -EINVAL;
    }

    uint32_t clock_divider = SPI_APB_CLK_HZ / spi_handle->CLK_freq;

    if (clock_divider & 1U)
        clock_divider &= 0xFFFE;

    if (clock_divider < 2U)
        clock_divider = 2U;

    if (clock_divider > SPI_APB_CLK_HZ / 2) {
        pr_err("SPI: CLK_freq cannot be reached\n");
        return -EINVAL;
    }

    spi_base->CLK = (clock_divider << SPI_CLK_CDIV_Pos) & SPI_CLK_CDIV_Msk;
    pr_info("SPI: CLK configured — CDIV=%u actual_freq=%lu Hz\n",
            clock_divider, SPI_APB_CLK_HZ / clock_divider);

    // clear CS register before
    spi_base->CS &= ~(SPI_CPOL | SPI_CPHA);
    if (spi_handle->mode > SPI_MODE_3) {
        pr_err("SPI: Invalid mode %u, must be 0-3\n", spi_handle->mode);
        return -EINVAL;
    }
    
    if (SPI_GET_CPOL(spi_handle->mode)) 
        spi_base->CS |= SPI_CPOL;
    
    if (SPI_GET_CPHA(spi_handle->mode)) 
        spi_base->CS |= SPI_CPHA;

    pr_info("SPI: Mode configured succesfully : %u\n", spi_handle->mode);

    if (spi_handle->Chip_select > SPI_CS_2) {
        pr_err("SPI: Invalid Chip select %u, must be 0-2\n", spi_handle->Chip_select);
        return -EINVAL;
    }

    spi_base->CS = (clock_divider << SPI_CLK_CDIV_Pos) & SPI_CLK_CDIV_Msk;
    pr_info("SPI: Chip select configured succesfully : %u\n", spi_handle->Chip_select);

    return 0;
}
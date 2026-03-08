#include "spi_driver.h"
#include "stdio.h"
#include "stdint.h"

int main(void)
{
    printf("test : 0x%x\n", (DISABLE_RX_INT_MASK & DISABLE_TX_INT_MASK));
    return 0;
}
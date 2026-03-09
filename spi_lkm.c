#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include "spi_driver.h"

#define MAX_USER_SIZE 1024

static struct proc_dir_entry *spi_driver_proc = NULL;

static char data_buffer[MAX_USER_SIZE] = {0};

static spi_reg_t* SPI0 = NULL;

ssize_t spi_driver_read(struct file *file, char __user *user, size_t size, loff_t *off)
{
	return copy_to_user(user,"Hello!\n", 7) ? 0 : 7;
}

ssize_t spi_driver_write(struct file *file, const char __user *user, size_t size, loff_t *off)
{
	memset(data_buffer, 0x0, sizeof(data_buffer));

	if (size > MAX_USER_SIZE)
	{
		size = MAX_USER_SIZE;
	}

	if (copy_from_user(data_buffer, user, size))
		return -EFAULT;

    pr_info("SPI: Data buffer:\n");
    for (int i = 0; i < size; i++)
        pr_info("0x%02x ", data_buffer[i]);

    for (int i = 0; i < size; i++)
        spi_write(SPI0, data_buffer[i]);

	return size;
}

static const struct proc_ops spi_driver_proc_fops = 
{
	.proc_read = spi_driver_read,
	.proc_write = spi_driver_write,
};


static int __init spi_driver_init(void)
{
    pr_info("SPI: Welcome to SPI driver!\n");

    SPI0 = (spi_reg_t*) ioremap(SPI0_PHYS_BASE, PAGE_SIZE);
    if (!SPI0) {
        pr_err("SPI: Failed to map SPI0 memory\n");
        return -ENOMEM;
    }
    pr_info("SPI: Successfully mapped SPI0 memory\n");

    spi_handle_t spi_handle = {
        .CLK_freq     = 1 * 1000 * 1000,
        .Chip_select  = SPI_CS_0,
        .CS_polarity  = SPI_CS_ACTIVE_LOW,
        .mode         = SPI_MODE_0,
        .Technique    = SPI_POLLING,
    };

    if (spi_init(&spi_handle, SPI0) != 0) {
        pr_err("SPI: init failed\n");
        iounmap(SPI0);
        return -EINVAL;
    }
	
    spi_driver_proc = proc_create("spi_driver", 0666, NULL, &spi_driver_proc_fops);
    if (!spi_driver_proc) {
        pr_err("SPI: Failed to create proc entry\n");
        iounmap(SPI0);
        return -ENOMEM;
    }

    pr_info("SPI: Driver loaded successfully\n");

	return 0;
}

static void __exit spi_driver_exit(void)
{
	pr_info("SPI:Leaving my driver!\n");
	iounmap(SPI0);
	proc_remove(spi_driver_proc);
	return;
}

module_init(spi_driver_init);
module_exit(spi_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MYH");
MODULE_DESCRIPTION("SPI Driver");
MODULE_VERSION("1.0");
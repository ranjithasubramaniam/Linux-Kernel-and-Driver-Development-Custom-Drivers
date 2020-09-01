// #define DEBUG
// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/serial_reg.h>
#include <linux/pm_runtime.h>
#include <asm-generic/io.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/debugfs.h>
/* Add your code here */

#define SERIAL_RESET_COUNTER 0
#define SERIAL_GET_COUNTER 1
#define SERIAL_BUFSIZE 16


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A device driver for Serial / UART device");
MODULE_AUTHOR("Suresh");


/* Global Declarations*/
struct serial_dev {
		struct miscdevice miscdev;
		void __iomem *regs;
		size_t sent_count;		
		int irq;
		char serial_buf[SERIAL_BUFSIZE];
		int serial_buf_rd;
		int serial_buf_wr;
		wait_queue_head_t serial_wait;
		spinlock_t serial_spinlock;
		struct dentry *serial_debugfs_dir;
};


static u32 reg_read(struct serial_dev *dev, int off)
{
	return readl(dev->regs + (off * 4));
}

static void reg_write(struct serial_dev *dev, int val, int off)
{
	writel(val, dev->regs + (off * 4));
}

static void uart_write(struct serial_dev *dev, char param)
{
	
	//wait until the UART_LSR_THRE is set
	while (! (reg_read(dev, UART_LSR) & UART_LSR_THRE))
	{
		cpu_relax();
	}
	
	//write the character to UART_TX
	reg_write(dev, param, UART_TX);
	
}

irqreturn_t serial_interrupt(int irq, void *devptr)
{		
		struct serial_dev *dev;
		char c;
		unsigned long flags;
		//pr_info("Called Interrupt Handler: %s\n", __func__);
		//get the device form devptr
		dev = devptr;	
		//lock
		spin_lock_irqsave(&dev->serial_spinlock, flags);
		//get a character from UART device to acknowledge the interrupt
		c = reg_read(dev, UART_RX);
		pr_debug("The character read in interrupt handler is: %c\n", c);
		//write the value in the circular buffer
		dev->serial_buf[dev->serial_buf_wr] = c;
		dev->serial_buf_wr = (dev->serial_buf_wr +1 ) % SERIAL_BUFSIZE;
		//unlock
		spin_unlock_irqrestore(&dev->serial_spinlock, flags);
		//wake up the processes waiting on the queue
		wake_up(&dev->serial_wait );
		return IRQ_HANDLED;
}

static ssize_t serial_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *offp)
{
	//get the base address of the struct serial_dev using the magic macro container_of
	// before that get the miscdev from file->private_data
	struct serial_dev *dev = container_of(file->private_data, struct serial_dev, miscdev);
    loff_t i;	
	int status;
	char *p = kmalloc(count, GFP_KERNEL);
	char *strtaddr = p;
	unsigned long flags;
	//copy the buffer from user space to kernel space
	status = copy_from_user(p, buf, count);
		
 	if(status)
	{		
		return -EFAULT;
	} 
	
	//when two process access this critical section, the output  written in our device will be incorrect
	//so acquire lock here 
	spin_lock_irqsave(&dev->serial_spinlock,flags);
	
	for (i = *offp; count > 0 ; --count, ++i, ++p)		
	{
		// if clause to rectify new line problem	
		pr_debug("The character read in write function is: %c\n", *p);
		if(*p == '\n')
		{
			uart_write(dev, '\n');
			uart_write(dev, '\r');
		}
		else
		{
			uart_write(dev, *p);		
		}
	}

	*offp = i;
	
	//assign the sent bytes count to the sent_count member in struct serial device
	dev->sent_count += p-strtaddr-1;

	// unlock here 
	spin_unlock_irqrestore(&dev->serial_spinlock,flags);

	return p-strtaddr;   
	
/* 	for (i = 0;  i < count  ; i++)
	{
		if(*(p+i)=='\n')
		{
			uart_write(dev, '\n');
			uart_write(dev, '\r');
		}
		else
		{
			uart_write(dev, *(p+i));		
		}
	}
	return count; */
	
}

static ssize_t serial_read(struct file *file, char __user *buf,
			size_t count, loff_t *offp)
{	
	//get the base address of the struct serial_dev using the magic macro container_of
	// before that get the miscdev from file->private_data
	struct serial_dev *dev = container_of(file->private_data, struct serial_dev, miscdev);
	char c;
	int status;
	unsigned long flags;
	//put this process to sleep until the condition is met
	status = wait_event_interruptible(dev->serial_wait, dev->serial_buf_rd != dev->serial_buf_wr);
	if(status)
	{
		return -EINTR;
	}	
	//lock
	spin_lock_irqsave(&dev->serial_spinlock, flags);
	//send a character to user space buffer 
	c = dev->serial_buf[dev->serial_buf_rd];	
	dev->serial_buf_rd = (dev->serial_buf_rd +1 ) % SERIAL_BUFSIZE;
	// unlock here 
	spin_unlock_irqrestore(&dev->serial_spinlock,flags);
	status = copy_to_user(buf, &c, sizeof(c));
	if(status)
	{
		pr_err("The value is not copied to user space buffer\n");
		return -EFAULT;
	}			
	
	return sizeof(c);
}


 
long serial_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	//get the base address of the struct serial_dev using the magic macro container_of
	// before that get the miscdev from file->private_data
	struct serial_dev *dev = container_of(file->private_data, struct serial_dev, miscdev);
	size_t __user *argp = (size_t __user *)arg;
	switch (cmd) {
		case SERIAL_RESET_COUNTER:
			dev->sent_count = 0;
			break;
		case SERIAL_GET_COUNTER:
			if (put_user(dev->sent_count, argp))
				return -EFAULT;
			break;
		default:
			return -ENOTTY;
	}
	return 0;
}

 
static const struct file_operations serial_fops = {
		.owner = THIS_MODULE,
		.read = serial_read,
		.write = serial_write,
		.unlocked_ioctl = serial_unlocked_ioctl,
};



static int serial_probe(struct platform_device *pdev)
{
	
	unsigned int baud_divisor;
	unsigned int uartclk;
	struct resource *res;
	struct serial_dev *dev;	
	struct miscdevice miscdev; 
	int status;
	struct dentry *counter_debugfs_file;
	
	
	pr_info("Called %s\n", __func__);
	
	//allocate the memory for dev
	dev = devm_kzalloc(&pdev->dev, sizeof(struct serial_dev), GFP_KERNEL);
	
	if (!dev)
		return -ENOMEM;
	
	//get the memory resource from kernel after parsing from DT 
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	
	if (!res)
	{
		dev_err(&pdev->dev, "no memory resource identified \n");
		return -ENODEV;
	}
		
	//get the virtual address of the base physical address of the device
	dev->regs = devm_ioremap_resource(&pdev->dev, res);
	
	if (!dev->regs) {
			dev_err(&pdev->dev, "Cannot remap registers\n");
			return -ENOMEM;
	}
	
	//retrieve the irq number from device tree
	dev->irq = platform_get_irq(pdev, 0);
	
	//initialize wait queue
	init_waitqueue_head(&dev->serial_wait);
	
	//initialize the spin lock (dynamic initialization)
	spin_lock_init(&dev->serial_spinlock);
	
	//register the interrupt handler
	status = devm_request_irq(&pdev->dev, dev->irq, serial_interrupt, IRQF_SHARED, dev_name(&pdev->dev), dev);
	if (status)
	{
			dev_err(&pdev->dev, "interrupt handler cannot be registered\n");
	}

	// Initialise sent_count variable
	dev->sent_count = 0;

	// Link pdev with misc struct
	platform_set_drvdata(pdev, dev);
	
	//power management initialization
	pm_runtime_enable(&pdev->dev);
    pm_runtime_get_sync(&pdev->dev);
	
	/* Configure the baud rate to 115200 */
	of_property_read_u32(pdev->dev.of_node, "clock-frequency", &uartclk);
	baud_divisor = uartclk / 16 / 115200;
	reg_write(dev, 0x07, UART_OMAP_MDR1);
	reg_write(dev, 0x00, UART_LCR);
	reg_write(dev, UART_LCR_DLAB, UART_LCR);
	reg_write(dev, baud_divisor & 0xff, UART_DLL);
	reg_write(dev, (baud_divisor >> 8) & 0xff, UART_DLM);
	reg_write(dev, UART_LCR_WLEN8, UART_LCR);

	/* Soft reset */
	reg_write(dev, UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT, UART_FCR);
	reg_write(dev, 0x00, UART_OMAP_MDR1);
	
	// enable the interrupts
	reg_write(dev, UART_IER_RDI, UART_IER);
	
	/* Write some characters to UART Tx*/
	// uart_write(dev, 'R');
	
	// Initialise the miscdev data struct and assign it to miscdevice inside dev struct
	miscdev.minor	= MISC_DYNAMIC_MINOR;
	miscdev.name	= devm_kasprintf(&pdev->dev, GFP_KERNEL, "serial-%x", res->start);
	miscdev.fops	= &serial_fops;
	dev->miscdev = miscdev;
	
	//register the misc device 
	status = misc_register(&dev->miscdev);
	
	// Create a debugfs entry named serial
	dev->serial_debugfs_dir = debugfs_create_dir(dev->miscdev.name, NULL);
	if (dev->serial_debugfs_dir == NULL)
	{		
		dev_err(&pdev->dev, "Debugfs directory creation failed");
	}

	// Expose the sent count value as debugfs file named counter
	counter_debugfs_file = 	debugfs_create_u32 ("counter", 0666, dev->serial_debugfs_dir, &dev->sent_count);
	if (counter_debugfs_file == NULL)
	{		
		dev_err(&pdev->dev, "Debugfs file creation failed");
	}
	
	return 0;
}

static int serial_remove(struct platform_device *pdev)
{
	
	struct serial_dev *dev;
	pr_info("Called %s\n", __func__);
    //power management disable
	pm_runtime_disable(&pdev->dev);
	//retrieve from pdev
	dev  = platform_get_drvdata(pdev);
	misc_deregister(&dev->miscdev);
	//remove the debugfs directory
	debugfs_remove_recursive(dev->serial_debugfs_dir);
	return 0;
}


/* Added code  */
static const struct platform_device_id serial_uart_id[] = {
	{"bootlin-serial", 1 },
	{}
};

MODULE_DEVICE_TABLE(platform, serial_uart_id); 


static const struct of_device_id serial_of_match[] = {
	{ .compatible = "bootlin,bootlin-serial" },
	{ }
};

MODULE_DEVICE_TABLE(of, serial_of_match);

/* End of added code */

static struct platform_driver serial_driver = {
        .driver = {
                .name = "bootlin-serial",
                .owner = THIS_MODULE,
				.of_match_table = serial_of_match,
        },
        .probe = serial_probe,
        .remove = serial_remove,
		.id_table = serial_uart_id,
};

module_platform_driver(serial_driver);

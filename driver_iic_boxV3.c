#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <asm-generic/uaccess.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/delay.h>


#define DRIVER_VERSION "V0.0.1"
#define SENSOR_NAME_XC "xc7022"
#define SENSOR_I2CADDR_XC 0x1b

#define SENSOR_NAME_IMX	"imx291"
#define SENSOR_I2CADDR_IMX 0x1a

#define DEVICE_NODE_NAME_IMX SENSOR_NAME_IMX
#define DEVICE_NODE_NAME_XC SENSOR_NAME_XC

#define HENRY_DEBUG_EN	1
#define HENRY_DEBUG_LV	1

#define henry_debbug() printk(KERN_EMERG "%s(line:%d)\n", __func__, __LINE__)

typedef struct _global_private {
	void __iomem *regcfg;
	void __iomem *regdat;
	struct i2c_client *client[2];	//0: imx291, 1: xc7022 
}global_private_t;

typedef struct _ioctl_args{
	unsigned char rdwrFlag; //0: read, 1: write
	unsigned char regH;
	unsigned char regL;
	unsigned char val;
}ioctl_args_t;

static global_private_t gInfo;

void _gpio_set_func(void __iomem * vAddr, int index, int func)
{
	int tmp = 0;

	tmp = readl(vAddr);
	tmp &= ~(0x7<<index);
	tmp |= (func<<index);
	
	writel(tmp, vAddr);
}
void _gpio_set_data(void __iomem * vAddr, int index, int val)
{
	int tmp = 0;

	tmp = readl(vAddr);
	if(val)
		tmp |= (0x1<<index);
	else
		tmp &= ~(0x1<<index);
	
	writel(tmp, vAddr);
}

int i2c_read_a16_d8(struct i2c_client *client, unsigned char regH, unsigned char regL)
{
	int ret;

	char txbuf[2] = {regH, regL};
	char rxbuf[2] = {};

	struct i2c_msg msg[2] = {
		{client->addr, 0, ARRAY_SIZE(txbuf), txbuf},
		{client->addr, I2C_M_RD, ARRAY_SIZE(rxbuf), rxbuf}
	};

	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
	if (ret < 0) {
		printk(KERN_ERR "i2c_transfer failed.\n");
		return ret;
	}else
	{
		//printk(KERN_EMERG "rxbuf[0]: %#x, rxbuf[1]: %#x\n", rxbuf[0], rxbuf[1]);
	}

	//printk(KERN_ALERT "addr:%#x\n", client->addr);
	
	return rxbuf[0];
}

int i2c_write_a16_d8(struct i2c_client *client, unsigned char regH, unsigned char regL, unsigned char val)
{
	char txbuf[3] = {regH, regL, val};

	struct i2c_msg msg[2] = {
		{client->addr, 0, ARRAY_SIZE(txbuf), txbuf},
	};

	i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));

	
	return 0;
}


int iic_sensor_open(struct inode *inode, struct file *file) 
{
	henry_debbug();

	return 0;
}

int iic_sensor_release(struct inode *inode, struct file *file) 
{
	henry_debbug();
	return 0;
}

long iic_sensor_ioctl_imx(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	ioctl_args_t* argp = (ioctl_args_t*)arg;

	switch(argp->rdwrFlag){
	case 0:
	{
		argp->val = i2c_read_a16_d8(gInfo.client[0], argp->regH, argp->regL);
		printk(KERN_EMERG "read-> [%#x%x] : %#x\n", argp->regH, argp->regL, argp->val);
		break;
	}
	case 1:
	{
		printk(KERN_EMERG "write-> [%#x%x] : %#x\n", argp->regH, argp->regL, argp->val);
		ret = i2c_write_a16_d8(gInfo.client[0], argp->regH, argp->regL, argp->val);
		break;
	}
	default:
	{
		printk(KERN_ERR "Error: file: %s, func: %s(line: %d)\n", __FILE__, __func__, __LINE__);
		break;
	}
	}
	
//	if (copy_to_user((void *)arg, &data, sizeof(data)));

	return 0;
}

long iic_sensor_ioctl_xc(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	ioctl_args_t* argp = (ioctl_args_t*)arg;

	switch(argp->rdwrFlag){
	case 0:
	{
		argp->val = i2c_read_a16_d8(gInfo.client[1], argp->regH, argp->regL);
		printk(KERN_EMERG "read-> [%#x%x] : %#x\n", argp->regH, argp->regL, argp->val);
		break;
	}
	case 1:
	{
		printk(KERN_EMERG "write-> [%#x%x] : %#x\n", argp->regH, argp->regL, argp->val);
		ret = i2c_write_a16_d8(gInfo.client[1], argp->regH, argp->regL, argp->val);
		break;
	}
	default:
	{
		printk(KERN_ERR "Error: file: %s, func: %s(line: %d)\n", __FILE__, __func__, __LINE__);
		break;
	}
	}
	
//	if (copy_to_user((void *)arg, &data, sizeof(data)));

	return 0;
}


	
static struct file_operations sensor_fops_imx = {
	.owner		= THIS_MODULE,
	.open		= iic_sensor_open,
	.release	= iic_sensor_release,
	.unlocked_ioctl = iic_sensor_ioctl_imx,
};

	
static struct file_operations sensor_fops_xc = {
	.owner		= THIS_MODULE,
	.open		= iic_sensor_open,
	.release	= iic_sensor_release,
	.unlocked_ioctl = iic_sensor_ioctl_xc,
};

static struct miscdevice sensor_miscdevice_xc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NODE_NAME_XC,
	.fops = &sensor_fops_xc,
};
	
static struct miscdevice sensor_miscdevice_imx = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NODE_NAME_IMX,
	.fops = &sensor_fops_imx,
};

int i2c_device_probe_handler(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;

	if(0 == strcmp(id->name, SENSOR_NAME_IMX))
	{
		ret = misc_register(&sensor_miscdevice_imx);
		if(ret)
		{
			printk(KERN_ERR "misc register IMX291 failed.!\n");
		}else
		{
			gInfo.client[0] = client;
			printk(KERN_CRIT "%s", __func__);
		}
	}else if(0 == strcmp(id->name, SENSOR_NAME_XC))
	{
		ret = misc_register(&sensor_miscdevice_xc);
		if(ret)
		{
			printk(KERN_ERR "misc register XC7022 failed.!\n");
		}else
		{
			gInfo.client[1] = client;
			printk(KERN_CRIT "%s", __func__);
		}
	}else
	{
		ret = -EIO;
		printk(KERN_ERR "Error: Wrong i2c_device_id.\n");
	}
	
	return ret;
}

int i2c_device_remove_handler(struct i2c_client *client)
{
	int ret = 0;

	ret = misc_deregister(&sensor_miscdevice_imx);
	if(ret)
	{
		printk(KERN_ERR "sensor imx291 remove failed.\n");
	}else
	{
		henry_debbug();
	}

	ret = misc_deregister(&sensor_miscdevice_xc);
	if(ret)
	{
		printk(KERN_ERR "sensor xc7022 remove failed.\n");
	}else
	{
		henry_debbug();
	}
	
	return 0;
}

const struct i2c_device_id i2c_device_id_list[] = {
	{SENSOR_NAME_XC, SENSOR_I2CADDR_XC},
	{SENSOR_NAME_IMX, SENSOR_I2CADDR_IMX},
	{ }
};
	
MODULE_DEVICE_TABLE(i2c, i2c_device_id_list);

static struct i2c_driver i2c_driver_obj = {
  .driver = {
  	.owner = THIS_MODULE,
	.name = SENSOR_NAME_XC,
  },
  .probe = i2c_device_probe_handler,
  .remove = i2c_device_remove_handler,
  .id_table = i2c_device_id_list,
};

int register_gpio_for_i2c(void)
{
	int ret = 0;
	//init
	gInfo.regdat = NULL;
	gInfo.regcfg = NULL;
	//get
	/*PE14->scl  ,  PE15->sda*/
	gInfo.regcfg = ioremap(0x01c20894, 4);//pe_cfg1_reg
	if(!gInfo.regcfg){
		printk(KERN_ERR "Error:ioremap for regcfg failed!\n");
		ret = -ENOMEM;
	}else
	{
		gInfo.regdat = ioremap(0x01c208a0, 4);//pe_data_reg
		if(!gInfo.regdat){
			printk(KERN_ERR "Error:ioremap for regdat failed!\n");
			ret = -ENOMEM;
		}
	}
	
	//set
	if(gInfo.regdat && gInfo.regcfg)
	{
		//SDA and SCL start as outputs
		//PE15 -> twi2_sda
		_gpio_set_func(gInfo.regcfg, 28, 3);
		_gpio_set_data(gInfo.regdat, 7, 1);
		//PE14 -> twi2_sck
		_gpio_set_func(gInfo.regcfg, 24, 3);
		_gpio_set_data(gInfo.regdat, 6, 1);
	}
	
	return ret;
}

void unregister_gpio_for_i2c(void)
{
	if(gInfo.regdat)
	{
		iounmap(gInfo.regdat);
		gInfo.regdat = NULL;
	}
	if(gInfo.regcfg)
	{
		iounmap(gInfo.regcfg);
		gInfo.regcfg = NULL;
	}
}

static int __init init_iic_sensor(void)
{
	int ret = 0;

	//
	memset(&gInfo, 0, sizeof(global_private_t));
	//

	ret = register_gpio_for_i2c();
	if(ret)
	{
		printk(KERN_ERR "register gpio for i2c failed.\n");
	}else
	{
		ret = i2c_add_driver(&i2c_driver_obj);
		if(ret)
		{
			printk(KERN_ERR "i2c add driver failed.\n");
		}
	}
	
	henry_debbug();
	return ret;
}

static void __exit exit_iic_sensor(void)
{
	if(gInfo.regdat && gInfo.regcfg)
	{
		i2c_del_driver(&i2c_driver_obj);
	}
	
	unregister_gpio_for_i2c();
	henry_debbug();
	
	return;
}

module_init(init_iic_sensor);
module_exit(exit_iic_sensor);
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
MODULE_DESCRIPTION(" I2c driver sample accessing demo @by haoxiansen@zhitongits.com.cn");

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <errno.h>

typedef struct _ioctl_args{
	unsigned char rdwrFlag; //0: read, 1: write
	unsigned char regH;
	unsigned char regL;
	unsigned char val;
}ioctl_args_t;
#include "xc7022Config.h"


static int read_regval_array(const int fd, const regval_list_t* p, const unsigned int size)
{
	int ret = 0, i =0;
	ioctl_args_t a;

	//printf("ARRAY_SIZE:%d\n", size);
	a.rdwrFlag = 0;//read
	for(i=0; i<size; i++, p++)
	{
		//printf("%#x,%#x\n", p->reg, p->val);	//test
		a.regH = ((p->reg)>>8)&0xff;
		a.regL = (p->reg)&0xff;
		a.val = p->val;
		ret = ioctl(fd, 0xff, &a);
		if (ret < 0)
		{
			ret = -EIO;
			printf("Error: ioctl failed(reg:%x, val:%x)!", p->reg, p->val);
		}
	}
	return ret;
}

static int write_regval_array(const int fd, const regval_list_t* p, const unsigned int size)
{
	int ret = 0, i =0;
	ioctl_args_t a;

	//printf("ARRAY_SIZE:%d\n", size);
	a.rdwrFlag = 1;//write
	for(i=0; i<size; i++, p++)
	{
		//printf("%#x,%#x\n", p->reg, p->val);	//test
		a.regH = ((p->reg)>>8)&0xff;
		a.regL = (p->reg)&0xff;
		a.val = p->val;
		ret = ioctl(fd, 0xff, &a);
		if (ret < 0)
		{
			ret = -EIO;
			printf("Error: ioctl failed(reg:%x, val:%x)!", p->reg, p->val);
		}
	}
	return ret;
}

static int detect_device_id_xc(int fd)
{
	ioctl_args_t a,b;
	
	//IdH ->0x71
	bzero(&a, sizeof(ioctl_args_t));
	a.rdwrFlag = 0;
	a.regH = 0xff;
	a.regL = 0xfb;

	ioctl(fd, 0xff, &a);

	//idL -> 0x60
	bzero(&b, sizeof(ioctl_args_t));
	b.rdwrFlag = 0;
	b.regH = 0xff;
	b.regL = 0xfc;
	ioctl(fd, 0xff, &b);
	printf("----xc7022 id: %#x%x\n", a.val, b.val);
	return 0;
}

static int detect_device_id_imx(int fd)
{
	ioctl_args_t a,b;
	
	//IdH ->0xa0
	bzero(&a, sizeof(ioctl_args_t));
	a.rdwrFlag = 0;
	a.regH = 0x30;
	a.regL = 0x08;

	ioctl(fd, 0xff, &a);

	//idL -> 0xb2
	bzero(&b, sizeof(ioctl_args_t));
	b.rdwrFlag = 0;
	b.regH = 0x30;
	b.regL = 0x1e;
	ioctl(fd, 0xff, &b);
	printf("----imx291 id: %#x%x\n", a.val, b.val);
	return 0;
}

int main(int argc, char* argv[])
{
	int fd[2] = {};
	ioctl_args_t a;

	fd[1] = open("/dev/xc7022", O_RDWR);
	if(fd[1] < 0)
	{
		perror("open");
		exit(1);
	}else
	{
#if 1
	//0. check i2c hw and xc7022's accessable
	detect_device_id_xc(fd[1]);
#endif

	sleep(3);
	
	//1. init xc7022 isp chip
	write_regval_array(fd[1], XC7022_default_regs, ARRAY_SIZE(XC7022_default_regs));


#if 1
		//2. enable i2c bypass function for accessing imx291 in the future
		write_regval_array(fd[1], bypass_on, ARRAY_SIZE(bypass_on));


		fd[0] = open("/dev/imx291", O_RDWR);
		if(fd[0] < 0)
		{
			perror("open imx291");
			close(fd[1]);
			exit(1);
		}else
		{
#if 1
			detect_device_id_imx(fd[0]);
#endif

			read_regval_array(fd[0], IMX291_default_regs, ARRAY_SIZE(IMX291_default_regs));
			//3. exchange i2c addr, access imx291, init imx291
			write_regval_array(fd[0], IMX291_default_regs, ARRAY_SIZE(IMX291_default_regs));

			read_regval_array(fd[0], IMX291_default_regs, ARRAY_SIZE(IMX291_default_regs));

			close(fd[0]);
		}
		//4. close i2c bypass function
		//write_regval_array(fd[1], bypass_off, ARRAY_SIZE(bypass_off));

		//5. output yuv422 1920*1080
#endif

		close(fd[1]);
	}
	
	return 0;
}

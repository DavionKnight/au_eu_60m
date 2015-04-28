#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include "omap_gpio.h"
#include "sw_iic.h"

MODULE_LICENSE("GPL");

/******************************** ��ģ��ĺ궨�� ****************************/
// pinmuxϵ�мĴ����������ַ�ʹ�С
#define PINMUX_ADDR_BASE 0x01c14120
#define PINMUX_SIZE (4*20)
// gpioϵ�мĴ����������ַ�ʹ�С
#define GPIO_ADDR_BASE 0x01e26000
#define GPIO_SIZE (0x88)
// ioctl��֧�ֵ�����
#define SET				0x80000001
#define CLR				0x80000002
#define READ_ST			0x80000003
#define DEBUG			0x80000004
#define SET_ADDR        0x80000005
// SDA��SCL���Ŷ���
#define SDA (1<<20)
#define SCL (1<<21)

#define SDA_EN (1<<8)
#define SCL_EN (1<<29)
/********************************  ����ģ���ⲿ���� ************************/


/********************************  ����ģ���ڲ����� ************************/
int scull_open(struct inode * inode, struct file * filp);
long scull_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
int scull_release(struct inode * inode, struct file * filp);
ssize_t scull_read(struct file * filp, char __user * buff, size_t count, loff_t * offp);
ssize_t scull_write(struct file * filp, const char __user * buff, size_t count, loff_t * offp);

/********************************  ģ���ڲ����� ****************************/
char * device_name = "iic_laser"; // �豸����
void * pinmux_reg;             // pinmux�Ĵ����ں˵�ַ
void * gpio_reg;               // gpio�Ĵ����ں˵�ַ
unsigned int pinmux_data = 0;  // pinmux�Ĵ�����дʹ�õı���
unsigned int gpio_data = 0;    // gpio�Ĵ�����дʹ�õı���
static int major = 0;		   // �豸�����豸��
dev_t dev;                     // �ں��е��豸���
struct cdev scull_cdev;        // �ں��е��ַ��豸
struct file_operations scull_fops = {  // ����������ʵ�ֵķ���
	.owner = THIS_MODULE,
	.unlocked_ioctl = scull_ioctl,
	.open = scull_open,
	.read = scull_read,
	.write = scull_write,
	.release = scull_release,
};
static struct class * scull_class;  // �ں��б�ʾ�豸��һ����
unsigned char slave_addr;
/*
** �������ܣ�����SDA_EN���ŷ�������/���
** ���������dir=IN ���룬 dir=OUT ���
** �����������
** ����ֵ����
** ��ע��
*/
void set_dir_sda_en(int dir)
{
	gpio_data = ioread32(ADDR(gpio_reg, gpio_t, dir23));
	if(dir == OUT){
		gpio_data &= ~SDA_EN;
	}else{
		gpio_data |= SDA_EN;
	}
	iowrite32(gpio_data, ADDR(gpio_reg, gpio_t,dir23));
}
/*
** �������ܣ�����SCL_EN���ŷ�������/���
** ���������dir=IN ���룬 dir=OUT ���
** �����������
** ����ֵ����
** ��ע��
*/
void set_dir_scl_en(int dir)
{
	gpio_data = ioread32(ADDR(gpio_reg, gpio_t, dir23));
	if(dir == OUT){
		gpio_data &= ~SCL_EN;
	}else{
		gpio_data |= SCL_EN;
	}
	iowrite32(gpio_data, ADDR(gpio_reg, gpio_t,dir23));
}
/*
** �������ܣ�SDA_EN���������ƽ
** ���������a=HIGH ����ߵ�ƽ�� a=LOW ����͵�ƽ
** �����������
** ����ֵ����
** ��ע��
*/
void tx_sda_en(unsigned char a)
{
	gpio_data = SDA_EN;
	if(a == HIGH){
		iowrite32(gpio_data, ADDR(gpio_reg, gpio_t, set_data23));
	}else{
		iowrite32(gpio_data, ADDR(gpio_reg, gpio_t, clr_data23));
	}
}
/*
** �������ܣ�SCL_EN���������ƽ
** ���������a=HIGH ����ߵ�ƽ�� a=LOW ����͵�ƽ
** �����������
** ����ֵ����
** ��ע��
*/
void tx_scl_en(unsigned char a)
{
	gpio_data = SCL_EN;
	if(a == HIGH){
		iowrite32(gpio_data, ADDR(gpio_reg, gpio_t, set_data23));
	}else{
		iowrite32(gpio_data, ADDR(gpio_reg, gpio_t, clr_data23));
	}
}
/*
** �������ܣ�����SDA���ŷ�������/���
** ���������dir=IN ���룬 dir=OUT ���
** �����������
** ����ֵ����
** ��ע��
*/
void set_dir_sda(int dir)
{
	gpio_data = ioread32(ADDR(gpio_reg, gpio_t, dir01));
	if(dir == OUT){
		gpio_data &= ~SDA;
	}else{
		gpio_data |= SDA;
	}
	iowrite32(gpio_data, ADDR(gpio_reg, gpio_t,dir01));
}
/*
** �������ܣ�����SCL���ŷ�������/���
** ���������dir=IN ���룬 dir=OUT ���
** �����������
** ����ֵ����
** ��ע��
*/
void set_dir_scl(int dir)
{
	gpio_data = ioread32(ADDR(gpio_reg, gpio_t, dir01));
	if(dir == OUT){
		gpio_data &= ~SCL;
	}else{
		gpio_data |= SCL;
	}
	iowrite32(gpio_data, ADDR(gpio_reg, gpio_t,dir01));
}
/*
** �������ܣ�5us��ʱ
** �����������
** �����������
** ����ֵ����
** ��ע��
*/
void delay5us(void)
{
	udelay(5);
}
/*
** �������ܣ�6us��ʱ
** �����������
** �����������
** ����ֵ����
** ��ע��
*/
void delay6us(void)
{
	udelay(6);
}
/*
** �������ܣ�SDA���������ƽ
** ���������a=HIGH ����ߵ�ƽ�� a=LOW ����͵�ƽ
** �����������
** ����ֵ����
** ��ע��
*/
void tx_sda(unsigned char a)
{
	tx_sda_en(HIGH);// ʹ��CPLD��SDA�ķ���ARM->LASER
	set_dir_sda(OUT);
	gpio_data = SDA;
	if(a == HIGH){
		iowrite32(gpio_data, ADDR(gpio_reg, gpio_t, set_data01));
	}else{
		iowrite32(gpio_data, ADDR(gpio_reg, gpio_t, clr_data01));
	}
}
/*
** �������ܣ���ȡSDA����״̬
** �����������
** �����������
** ����ֵ��HIGH ����ߵ�ƽ�� LOW ����͵�ƽ
** ��ע��
*/
unsigned char rx_sda(void)
{

	tx_sda_en(LOW);// ʹ��CPLD��SDA�ķ���ARM<-LASER
	set_dir_sda(IN);
	udelay(5);
	gpio_data = ioread32(ADDR(gpio_reg, gpio_t, in_data01));
	if(gpio_data & SDA){
		return HIGH;
	}else{
		return LOW;
	}
}
/*
** �������ܣ�SCL���������ƽ
** ���������a=HIGH ����ߵ�ƽ�� a=LOW ����͵�ƽ
** �����������
** ����ֵ����
** ��ע��
*/
void tx_scl(unsigned char a)
{
	tx_scl_en(HIGH);// ʹ��CPLD��SCL�ķ���ARM->LASER
	set_dir_scl(OUT);
	gpio_data = SCL;
	if(a == HIGH){
		iowrite32(gpio_data, ADDR(gpio_reg, gpio_t, set_data01));
	}else{
		iowrite32(gpio_data, ADDR(gpio_reg, gpio_t, clr_data01));
	}
}
/*
** �������ܣ���ȡSCL����״̬
** �����������
** �����������
** ����ֵ��HIGH ����ߵ�ƽ�� LOW ����͵�ƽ
** ��ע��
*/
unsigned char rx_scl(void)
{
	tx_scl_en(LOW);// ʹ��CPLD��SCL�ķ���ARM<-LASER
	set_dir_scl(IN);
	udelay(5);
	gpio_data = ioread32(ADDR(gpio_reg, gpio_t, in_data01));
	if(gpio_data & SCL){
		return HIGH;
	}else{
		return LOW;
	}
}
/*
** �������ܣ��������غ���
** �����������
** �����������
** ����ֵ�� 0=�ɹ� ����=ʧ��
** ��ע��
*/
static int hello_init(void)
{
	int result = 0;
	int err = 0;
	struct device * device;

	if(major){// ��̬�����豸��ŷ�Χ
		dev = MKDEV(major, 0);	
		result = register_chrdev_region(dev, 1, device_name);
	}else{
		// ��̬�����豸��ŷ�Χ
		result = alloc_chrdev_region(&dev, 0, 1, device_name);
		major = MAJOR(dev);
	}
	if(result < 0){
		printk(KERN_WARNING "scull: can't get major %d\n", major);
		return result;
	}
	printk("%s major is %d\n", device_name, major);
	// �����ʼ��struct cdev��struct file_operations
	cdev_init(&scull_cdev, &scull_fops);
	scull_cdev.owner = THIS_MODULE;
	scull_cdev.ops = &scull_fops;
	// ���豸��ӵ�ϵͳ��
	err = cdev_add(&scull_cdev, dev, 1);
	if(err){
		printk(KERN_NOTICE "Error %d adding %s", err, device_name);
		return -1;
	}
	// Ϊ�豸����һ����
	scull_class = class_create(THIS_MODULE, device_name);
	if(IS_ERR(scull_class)){
		printk(KERN_NOTICE "Error faild in creating class.\n");
		return -1;
	}
	// ������Ӧ�豸�ڵ�
	device = device_create(scull_class, NULL, dev, NULL, device_name);
	if(IS_ERR(device)){
		printk(KERN_NOTICE "error faild in device_create.\n");
		return -2;
	}
	// �ڴ�ӳ�䣬�����豸�������ַ
	// PINMUX
	pinmux_reg = (unsigned int *)ioremap(PINMUX_ADDR_BASE, PINMUX_SIZE); // pinmuxӳ��
	printk("pinmux addr is 0x%08x.\n", (unsigned int)pinmux_reg);
	pinmux_data = ioread32(ADDR(pinmux_reg, pinmux_t, pinmux4)); // SDA SCL
	pinmux_data &= ~(0xff<<8);
	pinmux_data |= (0x88<<8);
	iowrite32(pinmux_data, ADDR(pinmux_reg, pinmux_t, pinmux4));

	pinmux_data = ioread32(ADDR(pinmux_reg, pinmux_t, pinmux7)); // SCL_EN
	pinmux_data &= ~(0xf<<8);
	pinmux_data |= (0x8<<8);
	iowrite32(pinmux_data, ADDR(pinmux_reg, pinmux_t, pinmux7));

	pinmux_data = ioread32(ADDR(pinmux_reg, pinmux_t, pinmux5)); // SDA_EN
	pinmux_data &= ~(0xf<<28);
	pinmux_data |= (0x8<<28);
	iowrite32(pinmux_data, ADDR(pinmux_reg, pinmux_t, pinmux5));

	// GPIO
	gpio_reg = ioremap(GPIO_ADDR_BASE, GPIO_SIZE); // gpioӳ��
	printk("gpio addr is 0x%08x.\n", (unsigned int)gpio_reg);
	return 0;
}
/*
** �������ܣ�����OPEN����
** �����������
** �����������
** ����ֵ�� 0=�ɹ� ����=ʧ��
** ��ע��
*/
int scull_open(struct inode * inode, struct file * filp)
{
	set_dir_scl_en(OUT);
	set_dir_sda_en(OUT);
	return 0;
}
/*
** �������ܣ�����IOCTL����
** ���������cmd=���� arg=����
** �����������
** ����ֵ�� 0=�ɹ� ����=ʧ��
** ��ע��
*/
long scull_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
	unsigned char st;
	switch(cmd){
		case SET_ADDR:
			slave_addr = (unsigned char)arg;
			break;
		case SET: // ����SDA��SCL����ߵ�ƽ
			if(arg == SDA){
				printk("set sda 1.\n");
				tx_sda(1);
			}else{
				printk("set scl 1.\n");
				tx_scl(1);
			}
			break;
		case CLR: // ����SDA��SCL����͵�ƽ
			if(arg == SDA){
				printk("clr sda 0.\n");
				tx_sda(0);
			}else{
				printk("clr scl 0.\n");
				tx_scl(0);
			}
			break;
		case READ_ST: // ������״̬
			if(arg == SDA){
				st = rx_sda();
				printk("sda = %d.\n", st);
			}else{
				st = rx_scl();
				printk("scl = %d.\n", st);
			}
			break;
		default:
			break;
	}
	return 0;
}
/*
** �������ܣ�����READ����
** ���������count=���ȡ���ݳ���
** ���������buff=�û��ռ����ݻ�����
** ����ֵ�� 0=�ɹ� ����=ʧ��
** ��ע��
*/
ssize_t scull_read(struct file * filp, char __user * buff, size_t count, loff_t * offp)
{
	unsigned char tbuf[256];
	int i = 0;
	unsigned char tmp = 0;
	
	for(i = 0; i < 256; i++){
		tbuf[i] = 0;
	}
	iic_start();
	tmp = slave_addr|0x1;
	iic_write_bytes(&tmp, 1);
	iic_read_bytes(tbuf, count);
	iic_stop();
	if(copy_to_user(buff, tbuf, count)){
		return -1;
	}
	return 0;
}
/*
** �������ܣ�����WRITE����
** ���������buff=�û��ռ����ݻ�����, count=��д�����ݳ���
** �����������
** ����ֵ�� 0=�ɹ� ����=ʧ��
** ��ע��
*/
ssize_t scull_write(struct file * filp, const char __user * buff, size_t count, loff_t * offp)
{
	unsigned char tbuf[256];
	unsigned char tmp = 0;
	
	if(count > 255){
		printk("write error. buff is too long!!!\n");
		return -2;
	}
	if(copy_from_user(tbuf, buff, count)){
		return -1;
	}
	iic_start();
	tmp = slave_addr;
	iic_write_bytes(&tmp, 1);
	if(iic_write_bytes(tbuf, count)){
		return -1;
	}
	iic_stop();
	return 0;
}
/*
** �������ܣ�����CLOSE����
** �����������
** �����������
** ����ֵ�� 0=�ɹ� ����=ʧ��
** ��ע��
*/
int scull_release(struct inode * inode, struct file * filp)
{
	return 0;
}
/*
** �������ܣ�����ж�غ���
** �����������
** �����������
** ����ֵ�� 0=�ɹ� ����=ʧ��
** ��ע��
*/
static void hello_exit(void)
{
	// ��ϵͳ���Ƴ�����ַ��豸
	cdev_del(&scull_cdev);
	// ɾ���豸�ڵ�
	device_destroy(scull_class, dev);
	// �ͷ��豸�ڵ���
	class_destroy(scull_class);
	// �ͷ��豸���
	unregister_chrdev_region(dev, 1);
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);

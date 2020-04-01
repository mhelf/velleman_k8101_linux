#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/slab.h>

#define BULK_OUT 0x02
#define CTL_IN 0x81
#define CTL_OUT 0x00

#define CLEAR 0x01
#define PIXEL 0x02
#define RECT 0x03
#define LINE 0x04
#define LIGHT 0x05
#define SOUND  0x06
#define TEXT  0x07
#define CONTRAST 0x08
#define INVERT 0x09

/*
 * Struct holds data and attributes of the module needs to build usb-messages.
 */
struct usb_display{
	struct usb_device *ctl;
	struct usb_device *data;
	unsigned char functioncode;
	unsigned char posx;
	unsigned char posy;
	unsigned char width;
	unsigned char height;
	unsigned char lightduration;
	unsigned char sound_number;
	unsigned char is_invert;
	unsigned char contrast;
	unsigned char* text_msg;
	unsigned char text_size;
	unsigned int text_length;
};
static struct usb_display disp_object;
static struct usb_display *display = &disp_object;

/*
 * Calculates the checksum of one usb-message
 */
static unsigned char check_sum(unsigned char* input,int count)
{
	int i;
	int result = 0;
		
	for(i = 0;i<count;i++)
	{
		result += input[i];
	}
	return result % 256;
}

/*
 * Sends the initial message to wake up the K8101
 */
static int send_init_message(void)
{
	unsigned char *buf;	
	buf = "\x80\x25\0\0\0\0\x08";

	if(usb_control_msg(display->ctl,usb_sndctrlpipe(display->ctl,0x0),0x20,0x21,0,0,buf,7,2000)<0)
		return 1;	
	
	printk(KERN_INFO "K8101 init msg sent!...\n");
	return 0;
}

/*
 * Sends a generic bulk message contained in buf to the endpoint 0x02 of the device.
 */
static void send_bulk(unsigned char *buf,int size)
{
	int act_length;
	if(usb_bulk_msg(display->data, usb_sndbulkpipe(display->data, 0x02),buf, size, &act_length,1000) < 0)
		printk(KERN_ERR "K8101 sending bulk message failed\n");
	if(act_length != size)
		printk(KERN_ERR "K8101 Wrong sized bulk message sent\n");
}

/*
 * Sends a draw_pixel message to the display using the attributes x and y.
 */
static void draw_pixel(void)
{	
	unsigned char buf[8];
	buf[0] = 0xAA;
	buf[1] = 0x08;
	buf[2] = 0x00;
	buf[3] = 0x09;
	buf[4] = display->posx;
	buf[5] = display->posy;
	buf[6] = 0x11+display->posx+display->posy;
	buf[7] = 0x55;
	
	send_bulk(buf,8);
}

/*
 * Sends a draw_rect message to the display using attributes x,y, width and height.
 */
static void draw_rect(void)
{
	unsigned char buf[10];
	buf[0] = 0xAA;
	buf[1] = 0x10;
	buf[2] = 0x00;
	buf[3] = 0x07;
	buf[4] = display->posx;
	buf[5] = display->posy;
	buf[6] = display->width;
	buf[7] = display->height;
	buf[8] = 0x17+display->posx+display->posy+display->width+display->height;
	buf[9] = 0x55;
	
	send_bulk(buf,10);
}

/*
 * Sends a draw_line message to the display using using x and y as 
 * start point and width and height are used as endpoint.
 */
static void draw_line(void)
{
	unsigned char buf[10];
	buf[0] = 0xAA;
	buf[1] = 0x10;
	buf[2] = 0x00;
	buf[3] = 0x12;
	buf[4] = display->posx;
	buf[5] = display->posy;
	buf[6] = display->width;
	buf[7] = display->height;
	buf[8] = 0x22+display->posx+display->posy+display->width+display->height;
	buf[9] = 0x55;
	
	send_bulk(buf,10);
}

/*
 * Sends a message to the display which enables the backlight using the attribute lightduration.
 */
static void backlight(void)
{
	unsigned char buf[7];
	buf[0] = 0xAA;
	buf[1] = 0x07;
	buf[2] = 0x00;
	buf[3] = 0x14;
	buf[4] = display->lightduration;
	buf[5] = 0x1B+display->lightduration;
	buf[6] = 0x55;
	
	send_bulk(buf,7);
}

/*
 * Plays a sound for sound_number times.
 */
static void play_sound(void)
{
	unsigned char buf[7];
	buf[0] = 0xAA;
	buf[1] = 0x07;
	buf[2] = 0x00;
	buf[3] = 0x06;
	buf[4] = display->sound_number;
	buf[5] = display->sound_number+0x06+0x07;
	buf[6] = 0x55;

	send_bulk(buf,7);
}

/*
 * Sets the contrast to the value of the attribute contained in display object.
 */
static void contrast(void)
{
	unsigned char buf[7];
	buf[0] = 0xAA;
	buf[1] = 0x07;
	buf[2] = 0x00;
	buf[3] = 0x11;
	buf[4] = display->contrast;
	buf[5] = display->contrast+0x07+0x11;
	buf[6] = 0x55;

	send_bulk(buf,7);
}
/*
 * Inverts the screen.
 */
static void invert(void)
{
	unsigned char buf[7];
	buf[0] = 0xAA;
	buf[1] = 0x07;
	buf[2] = 0x00;
	buf[3] = 0x15;
	buf[4] = 0x00;
	buf[5] = display->is_invert+0x15+0x07;
	buf[6] = 0x55;

	send_bulk(buf,7);
}

/*
 * Sends a message to draw a text. The buffer is copied from display->text_msg. Byte 1 text_length is added to 2 and 7
 * because this byte contains the length of the whole message so 7 is the message before the buffer 2 after the buffer.
 */
static void send_text(void)
{
	display->text_msg[0] = 0xAA;
	display->text_msg[1] = display->text_length+2+7;
	display->text_msg[2] = 0x00;
	display->text_msg[3] = display->text_size;
	display->text_msg[4] = display->posx;
	display->text_msg[5] = display->posy;
	display->text_msg[6] = display->width;

	display->text_msg[display->text_length+9] = 0x55;
	display->text_msg[display->text_length+8] = check_sum(display->text_msg+1,display->text_length+6);
	display->text_msg[display->text_length+7] = 0;
	
	send_bulk(display->text_msg,display->text_length+2+7);
	kfree(display->text_msg);
	display->text_msg = NULL;
	display->text_length = 0;
}
/*
 * Sends the message which clears the screen.
 */
static void clear(void)
{

	unsigned char buf[6];
	buf[0] = 0xAA;
	buf[1] = 0x06;
	buf[2] = 0x00;
	buf[3] = 0x02;
	buf[4] = 0x08;
	buf[5] = 0x55;
	send_bulk(buf,6);
}
/*
 * Decides which function must be called depending on which value display->functioncode was set to.
 */
static void do_function(void)
{	
	switch(display->functioncode){
		case PIXEL: draw_pixel();break;
		case CLEAR: clear();break;
		case RECT: draw_rect();break;
		case LINE: draw_line();break;
		case LIGHT: backlight();break;
		case SOUND: play_sound();break;
		case TEXT: send_text();break;
		case CONTRAST: contrast();break;
		case INVERT: invert();break;	
	}
}

/*
 * Function to show attributes to the user-space.
 */
static ssize_t attr_show(struct device *dev, struct device_attribute *attr,char *buf)
{
		int size;
		unsigned char var;
		var = 0;

		if (strcmp(attr->attr.name, "functioncode") == 0)
			var = display->functioncode;
		else if (strcmp(attr->attr.name, "posx") == 0)
			var = display->posx;
		else if(strcmp(attr->attr.name, "posy") == 0)
			var = display->posy;
		else if(strcmp(attr->attr.name, "width") == 0)
			var = display->width;
		else if(strcmp(attr->attr.name, "height") == 0)
			var = display->height;
		else if(strcmp(attr->attr.name, "lightduration") == 0)
			var = display->lightduration;
		else if(strcmp(attr->attr.name, "textsize") == 0)
			var = display->text_size;
		else if(strcmp(attr->attr.name, "text") == 0)
		{
			if(display->text_msg != NULL)
			{
				memcpy(buf, display->text_msg, PAGE_SIZE - 1);
				return display->text_length+9+7+2;
			}	
			return 0;
		}
		else if(strcmp(attr->attr.name, "soundnumber") == 0)
			var = display->sound_number;
		else if(strcmp(attr->attr.name, "contrast") == 0)
			var = display->contrast;
		else if(strcmp(attr->attr.name, "isinverted") == 0)
			var = display->is_invert;

		size = sprintf(buf, "%c\n", var);
		return size;
}


/*
 * Function to get attributes from user-space.
 */
static ssize_t attr_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
        unsigned char var;
	var = 0;

        sscanf(buf, "%c", &var);

	if (strcmp(attr->attr.name, "functioncode") == 0)
	{
		display->functioncode = var;
		do_function();
	}
	else if (strcmp(attr->attr.name, "posx") == 0)
		display->posx = var;
	else if(strcmp(attr->attr.name, "posy") == 0)
		display->posy = var;
	else if(strcmp(attr->attr.name, "width") == 0)
		display->width = var;
	else if(strcmp(attr->attr.name, "height") == 0)
		display->height = var;
	else if(strcmp(attr->attr.name, "lightduration") == 0)
		display->lightduration = var;
	else if(strcmp(attr->attr.name, "textsize") == 0)
		display->text_size = var;
	else if(strcmp(attr->attr.name, "text") == 0)
	{
		display->text_length = count;
		display->text_msg = kmalloc(display->text_length+9+7+2,GFP_KERNEL);
		memcpy((display->text_msg+7), buf,count);
	}
	else if(strcmp(attr->attr.name, "soundnumber") == 0)
		display->sound_number = var;
	else if(strcmp(attr->attr.name, "contrast") == 0)
		display->contrast = var;
	else if(strcmp(attr->attr.name, "isinverted") == 0)
		display->is_invert = var;

	return count;
}

/*
 * Initialization of the device attributes located in sysfs.
 */
static DEVICE_ATTR(functioncode, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(posx, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(posy, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(width, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(height, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(lightduration, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(soundnumber, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(text, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(textsize, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(contrast, S_IRUGO | S_IWUSR, attr_show, attr_store);
static DEVICE_ATTR(isinverted, S_IRUGO | S_IWUSR, attr_show, attr_store);

/*
 * Function which is called when connecting the K8101. K8101 has 2 devices which means it also needs 2 seperated structs.
 * 1 for the ctl interface and 1 for the bulk interface.
 */
static int k8101_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	
	int result;	
	result = 0;

	if(display->ctl == NULL){

		display->ctl = usb_get_dev(interface_to_usbdev(interface));

	
	}else if(display->data == NULL){

		display->data = usb_get_dev(interface_to_usbdev(interface));

		result = device_create_file(&interface->dev, &dev_attr_functioncode);
		result = device_create_file(&interface->dev, &dev_attr_posx);
		result = device_create_file(&interface->dev, &dev_attr_posy);
		result = device_create_file(&interface->dev, &dev_attr_width);		
		result = device_create_file(&interface->dev, &dev_attr_height);	
		result = device_create_file(&interface->dev, &dev_attr_lightduration);
		result = device_create_file(&interface->dev, &dev_attr_soundnumber);	
		result = device_create_file(&interface->dev, &dev_attr_text);
		result = device_create_file(&interface->dev, &dev_attr_textsize);
		result = device_create_file(&interface->dev, &dev_attr_contrast);
		result = device_create_file(&interface->dev, &dev_attr_isinverted);		
		

		if(result){
			goto err;
		}else{
			if(send_init_message())
				goto err;
			clear();
		}

	}

	printk(KERN_INFO "K8101 successfull connected...\n");
	return 0;

err:
	device_remove_file(&interface->dev, &dev_attr_functioncode);
	device_remove_file(&interface->dev, &dev_attr_posx);
	device_remove_file(&interface->dev, &dev_attr_posy);
	device_remove_file(&interface->dev, &dev_attr_width);		
	device_remove_file(&interface->dev, &dev_attr_height);	
	device_remove_file(&interface->dev, &dev_attr_lightduration);
	device_remove_file(&interface->dev, &dev_attr_soundnumber);		
	device_remove_file(&interface->dev, &dev_attr_text);
	device_remove_file(&interface->dev, &dev_attr_textsize);
	device_remove_file(&interface->dev, &dev_attr_contrast);
	device_remove_file(&interface->dev, &dev_attr_isinverted);

	usb_put_dev(display->ctl);
	usb_put_dev(display->data);
	display->data = NULL;	
	display->ctl = NULL;	

	printk(KERN_ERR "K8101 connection failed...\n");
	return result;


}
/*
 * Gets called when K8101 is disconnected from the system. Cleans up sysfs.
 */
static void k8101_disconnect(struct usb_interface *interface)
{



	if(display->ctl != NULL)
	{
		usb_put_dev(display->ctl);
		display->ctl = NULL;		
	}
	else if(display->data != NULL)
	{
		usb_put_dev(display->data);
		display->data = NULL;	
		device_remove_file(&interface->dev, &dev_attr_functioncode);
		device_remove_file(&interface->dev, &dev_attr_posx);
		device_remove_file(&interface->dev, &dev_attr_posy);
		device_remove_file(&interface->dev, &dev_attr_width);		
		device_remove_file(&interface->dev, &dev_attr_height);	
		device_remove_file(&interface->dev, &dev_attr_lightduration);	
		device_remove_file(&interface->dev, &dev_attr_soundnumber);	
		device_remove_file(&interface->dev, &dev_attr_text);
		device_remove_file(&interface->dev, &dev_attr_textsize);
		device_remove_file(&interface->dev, &dev_attr_contrast);
		device_remove_file(&interface->dev, &dev_attr_isinverted);
		printk(KERN_INFO "K8101 successfull disconnected...\n");
	}


	
	
	
}

static struct usb_device_id k8101_table[] =
{
	{ USB_DEVICE(0x10cf, 0x8101) },
	{}
};
MODULE_DEVICE_TABLE (usb, k8101_table);

static struct usb_driver k8101_driver =
{
	.name = "velleman_k8101_driver",
	.probe = k8101_probe,
	.disconnect = k8101_disconnect,
	.id_table = k8101_table,
};

static int __init k8101_init(void)
{
	int result;
	result = usb_register(&k8101_driver);
	if(result != 0)
	{
		printk(KERN_ERR "K8101 Init failed\n");
	}
	else
	{
		printk(KERN_INFO "K8101 successfull initialized\n");
	}

	return result;
}

static void __exit k8101_exit(void)
{
	usb_deregister(&k8101_driver);
}

module_init(k8101_init);
module_exit(k8101_exit);
MODULE_AUTHOR("Marius Helf");
MODULE_LICENSE("GPL");


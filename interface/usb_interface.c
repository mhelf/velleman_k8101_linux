#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "usb_interface.h"

static int fd;
static char *sysfs_path;
static int path_length;

void start()
{
	path_length = 0;
	FILE *fp;
  	char path[1000];
	fp = popen("/usr/bin/find /sys/bus/usb/drivers/velleman_k8101_driver/* -name *1.1", "r");
	if (fp == NULL)
	{
		printf("Failed to run command! Tool find is installed?\n" );
		exit(EXIT_FAILURE);
	}
	if(fgets(path, sizeof(path)-1, fp) == NULL)
	{
		printf("Error reading input stream! K8101 is plugged in?\n");
		exit(EXIT_FAILURE);
	}
	pclose(fp);

	path_length = strlen(path);
	if(!(sysfs_path = malloc(path_length)))
	{
		printf("Failed to allocate space!\n");
		exit(EXIT_FAILURE);

	}
	strncpy(sysfs_path,path,path_length-1);
}

void end()
{
	free(sysfs_path);
}

void write_sysfs_file(unsigned char value,char *file)
{
	char full_path[path_length+strlen(file)];
	strcpy(full_path,sysfs_path);
	strcat(full_path,"/");
	strcat(full_path,file);
	if((fd = open(full_path, O_WRONLY)) == -1)
	{
		printf("open sysfs file failed %s\n",file);
		exit(EXIT_FAILURE);
	}
	if(write (fd, &value, 1) == -1)
	{
		printf("write sysfs file failed\n");
		exit(EXIT_FAILURE);
	}
	if(close(fd) == -1)
	{
		perror("close sysfs file failed");
		exit(EXIT_FAILURE);
	}
}

void write_buf_sysfs_file(unsigned char *value,char *file)
{

	char full_path[path_length+strlen(file)];
	strcpy(full_path,sysfs_path);
	strcat(full_path,"/");
	strcat(full_path,file);
	if((fd = open(full_path, O_WRONLY)) == -1)
	{
		printf("open sysfs file failed %s\n",file);
		exit(EXIT_FAILURE);
	}
	if(write (fd, value, strlen(value)) == -1)
	{
		printf("write sysfs file failed\n");
		exit(EXIT_FAILURE);
	}
	if(close(fd) == -1)
	{
		perror("close sysfs file failed");
		exit(EXIT_FAILURE);
	}
}

void draw_pixel(unsigned char x, unsigned char y)
{
	write_sysfs_file(x,POSX);
	write_sysfs_file(y,POSY);
	write_sysfs_file(PIXEL,FUNCTION);
}

void draw_rectangle(unsigned char x,unsigned char y, unsigned char width, unsigned char height)
{
	write_sysfs_file(x,POSX);
	write_sysfs_file(y,POSY);
	write_sysfs_file(width,WIDTH);
	write_sysfs_file(height,HEIGHT);
	write_sysfs_file(RECT,FUNCTION);
}

void draw_line(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2)
{
	write_sysfs_file(x1,POSX);
	write_sysfs_file(y1,POSY);
	write_sysfs_file(x2,WIDTH);
	write_sysfs_file(y2,HEIGHT);
	write_sysfs_file(LINE,FUNCTION);
}

void draw_text(char *text,unsigned char x, unsigned char y, unsigned char width,unsigned char text_size)
{	
	write_sysfs_file(x,POSX);
	write_sysfs_file(y,POSY);
	write_sysfs_file(width,WIDTH);
	write_sysfs_file(text_size,TEXT_SIZE_FILE);
	write_buf_sysfs_file(text,TEXT_FILE);
	write_sysfs_file(TEXT,FUNCTION);
}

void clear_all()
{	
	write_sysfs_file(CLEAR,FUNCTION);
}

void sound(unsigned char sound_number)
{
	write_sysfs_file(sound_number,SOUND_NUMBER);
	write_sysfs_file(BEEP,FUNCTION);
}

void backlight_on(unsigned char seconds)
{	
	write_sysfs_file(seconds,LIGHT_DURATION);
	write_sysfs_file(LIGHT,FUNCTION);
}

void invert(unsigned char is_inverted)
{
	write_sysfs_file(is_inverted,INVERT_FILE);
	write_sysfs_file(INVERT,FUNCTION);
}

void contrast(unsigned char value)
{
	write_sysfs_file(value,CONTRAST_FILE);
	write_sysfs_file(CONTRAST,FUNCTION);
}



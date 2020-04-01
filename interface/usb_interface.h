
#ifndef USB_INTERFACE_H
#define USB_INTERFACE_H

//Sysfs attribute file names
#define POSX "posx"
#define POSY "posy"
#define WIDTH "width"
#define HEIGHT "height"
#define LIGHT_DURATION "lightduration"
#define SOUND_NUMBER "soundnumber"
#define FUNCTION "functioncode"
#define TEXT_FILE "text"
#define TEXT_SIZE_FILE "textsize"
#define CONTRAST_FILE "contrast"
#define INVERT_FILE "isinverted"

//Function codes
#define CLEAR 0x01
#define PIXEL 0x02
#define RECT 0x03
#define LINE 0x04
#define LIGHT 0x05
#define BEEP  0x06
#define TEXT  0x07
#define CONTRAST 0x08
#define INVERT 0x09

//Text Sizes
#define TEXT_SIZE_SMALL 0x05
#define TEXT_SIZE_BIG 0x04

/*
 * Draws a pixel at position x/y
 */
void draw_pixel(unsigned char x, unsigned char y);

/*
 * Draws a rectangle at position x/y with given width and height
 */
void draw_rectangle(unsigned char x,unsigned char y, unsigned char width, unsigned char height);

/*
 * Draws a line between x1/y1 and x2/y2
 */
void draw_line(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2);

/*
 * Draws a text at position x/y with given width and size
 */
void draw_text(char *text,unsigned char x, unsigned char y, unsigned char width,unsigned char text_size);

/*
 * Enables the backlight of the display for seconds
 */
void backlight_on(unsigned char seconds);

/*
 * Forces the display to beep for sound_number times
 */
void sound(unsigned char sound_number);

/*
 * Sets to contrast to a specific value
 */
void contrast(unsigned char value);

/*
 * Inverts the display
 */
void invert(unsigned char is_inverted);

/*
 * Clears the current screen buffer and removes all drawn elements
 */
void clear_all();

/*
 * Writes value to sysfs attribute file
 */
void write_sysfs_file(unsigned char value,char *file);

/*
 * Writes a whole buffer to sysfs attribute file
 */
void write_buf_sysfs_file(unsigned char *value,char *file);

/*
 * Must be called before a program can use this lib. Determines the path of the attribute files in sysfs.
 */
void start();

/*
 * Must be called when a programm finished using this lib. Releases resources.
 */
void end();


#endif /* USB_INTERFACE_H */

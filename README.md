# Velleman K8101 Linux Kernel module and user-space-interface

Table of contents:
1. How to install the software
2. How to use the software
3. Overview directory structure 
4. Kernel module
	5.1 USB protocoll
	5.2 Code documentation
5. User interface library
	6.1 Code documentation
	6.2 How to link with other programms
6. Test application: A simple game
	7.1 Manual
7. Difficulties while developing
8. Extendability
9. License

## 2. How to install the software

The main Makefile is placed in the root directory of the project.
It offers some commands to build the whole project at once.

To install the kernel module and compile all the project simply do the
following steps:

### Navigate to the projects directory
cd path/to/project

### Compile the project
make

### Install the kernel module
make install

Additionally the Makefile offers commands for uninstalling the module 
and cleaning the source folders.

make removemod
make uninstall
make clean


## 3. How to use the software

You can simply try out the functionality of the kernel module by 
playing the game located in the game folder.

The root Makefile also offers a command for starting the game.

Simply type make startgame

In section 6.2 see how you can use the library found in the
interface directory for your own programms which need access to 
the kernel module.


## 4. Overview directory structure 

### Root
The root directory contains the Makefile which gives you the
possibility to build the project from a central place without
changing the directories.

### Module
Contains the kernel module source (k8101_driver.c) and the 
Makefile for compiling it.

### Interface
Contains the user interface source files and the Makefile
(usb_interface.c, usb_interface.h)

### Game
Contains the games source files and it's Makefile


## 5. Kernel module

The kernel module is the part of the project which directly communicates
with the Velleman K8101 display it sends the Usb messages depending on 
the input given by the user interface.

### 5.1 USB protocoll

The K8101 has 2 Interfaces. The first one contains an interrupt endpoint
which is used to get asynchronous information from the device.

The second one is data interface with 2 bulk endpoints. 1 for receiving
and the other on for sending data.

The messages sent to the display vary. Some are quite big (over 170 bytes)
but the majority is quite small like 7-10 bytes.

Every message starts with the byte 0xAA and ends with 0x55.

Between these two bytes are different parameters for the specific 
display-function.

For example the message for sending a "draw_pixel" command is:

AA 08 00 09 0A 0A 25 55

0x08 and 0x09 are function specific bytes which basically determine which
function the message should trigger.

The two 0x0A bytes are the hex form of 10. They make up the x and y position
of the to drawn pixel.

The last byte before the end byte is 0x25 which is the checksum.

The checksum formula is the sum of all bytes between 0xAA and 0x55 modulo 256



### 5.2 Code documentation

See the documentation in the source files.


## 6. User interface library

This part of the project is the communicator between user-level and the
kernel module.

It is a static library which can be linked with any other programm.

It offers functions which can be used inside a user level programm that 
wants to controll the usb display.


### 6.1 Code documentation

See the documentation in the source files.

	
### 6.2 How to link with other programms

Just use the code of the Makefile located in the game directory and make
it fit your own user level programm. 

Make sure to create your programm in the root directory of the project.
To keep it organized :D


## 7. Test application: A simple game

For testing the interface and the module i built a simple game which
is linked to the library and uses it's functions.


### 7.1 Manual

With a little power of imagination you could feel like flying a plane 
inside of a cave or a tunnel avoiding obstacles.

Pressing the arrow up key you rise up and releasing it the gravity will
do it's job.

The game is over when you hit an obstacle.


## 8. Difficulties while developing

I had some difficulties and problems while developing the project.

First of all it already was quite a job to connect the display since 
no computer tried wanted to detect the display. The solution was using an old
usb hub to connect it to.

Then it was hard to decode the protocoll using a virtual machine with
windows installed because there already has been a driver for windows.

Wireshark helped me to fetch the usb-data the windows driver have sent
to the K8101.

The hardest part of decoding was the initialisation message. The windows
programm seemed to sent 100 useless messages. To pick the right one was
a lot trial and error.

10 Kernel panics later i could draw my first pixel onto the display.
After that it was easy to implement the other functions.

I only could not implement the integrated button of the display until 
the deadline of the project because of it's different and more complex
asynchronous protocoll.

Maybe i will implement that later on.


## 9. Extendability

You are free to extend the project with your own programms. Like i mentioned
in section 6.2 you can simply use the library.

I wrote it in section 8 that i did not finish the integrated button. So the
project is extendable at this point as well.

## 10. License

This project is licensed under GPL V3
You are freely allowed to use it in any form.
http://www.gnu.org/licenses/gpl.html


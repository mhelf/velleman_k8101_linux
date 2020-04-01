libusbinterface.a:	usb_interface.o
	@echo "creating libusbinterface.a"
	ar rcs -o libusbinterface.a usb_interface.o
	sudo chown root libusbinterface.a
	sudo chmod u+s	libusbinterface.a
usb_interface.o:	usb_interface.c usb_interface.h
		@echo "creating usb_interface.o"
		gcc -c -fPIC usb_interface.c
clean:
	rm -rf *.a *.o

CFLAGS=-Wall
LIBDIR=../interface/
LIB=$(LIBDIR)libusbinterface.a
LIBOBJ=usb_interface.o

game: 	game.c game.h $(LIBDIR)usb_interface.h $(LIB)
	@echo "creating game"
	gcc $(CFLAGS) -o game game.c -I$(LIBDIR) $(LIB) -lrt -lncurses
	sudo chown root game
	sudo chmod u+s	game
$(LIB):	
	cd $(LIBDIR);make;
clean:
	rm -rf *.a *.o game
	cd $(LIBDIR);make clean;

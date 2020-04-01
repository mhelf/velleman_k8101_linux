help:
	@echo 'make all- Make module, game and interface';
	@echo 'make install - make all and install the module';
	@echo 'make uninstall - make clean and remove module';
	@echo 'make startgame - make all, install the module and start the test game';
	@echo 'make clean - remove all except source';
	@echo 'make removemod - remove module from system';
all:
	cd ./game;make;
	cd ./interface;make;
	cd ./module;sudo make;
install:	all
	sudo insmod ./module/k8101_driver.ko
uninstall:	clean
	sudo rmmod k8101_driver
removemod:
	sudo rmmod k8101_driver
startgame:
	cd ./game;./game;
clean:
	cd ./game;make clean;
	cd ./interface;make clean;
	cd ./module;make clean;


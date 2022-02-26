KDIR := /usr/src/linux-headers-$(shell uname -r)
PWD  := $(shell pwd)

obj-m := simple.o

all:
	make -C "$(KDIR)" M="$(PWD)" modules

install:
	make -C "$(KDIR)" M="$(PWD)" modules_install

clean:
	make -C "$(KDIR)" M="$(PWD)" clean

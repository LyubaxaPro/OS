<<<<<<< HEAD
ifneq ($(KERNELRELEASE),)
	obj-m := fort_n.o
else
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build 
	PWD = $(shell pwd)
=======
# Если KERNELRELEASE определён, значит вызов сделан из
# системы сборки ядра и можно использовать её язык.
ifneq ($(KERNELRELEASE),)
	obj-m := fort_n.o # требуется собрать один модуль из объектного файла main.o. Результирующий модуль после сборки из объектного файла именуется как main.ko.
# Иначе вызов сделан прямо из командной
# строки; вызвать систему сборки ядра.
else
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build # дерево исходных текстов ядра
	PWD = $(shell pwd)

#  вызов в более широком контексте системы сборки ядра

	# -C - смена своего каталога на указанный опцией (т.е на каталог исходных кодов ядра)
	# там находит Makefile верхнего уровня ядра

	# M= заставляет вернуться обратно в директорию исходников модуля, прежде чем попытаться построить целевой модуль
	#ссылается на список модулей в obj-m
>>>>>>> 3c69c8ed2911776c36c8920831dc105a30eb2e7e
default: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	make clean

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order
	@rm -f .*.*.cmd *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions

disclean: clean
	@rm *.ko *.symvers
<<<<<<< HEAD
endif 
=======
endif 
>>>>>>> 3c69c8ed2911776c36c8920831dc105a30eb2e7e

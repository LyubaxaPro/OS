//. Вызывающий модуль ядра Linux

// модуль не будет подгружен к ядру, но это произойдёт уже после выполнения кода инициализирующей функции
// так как модуль по замыслу не загружается, то может не иметь функции выгрузки
#include <linux/init.h> 
#include <linux/module.h> 
#include "md.h"

MODULE_LICENSE( "GPL" ); 
MODULE_AUTHOR( "Oleg Tsiliuric <olej@front.ru>" ); 
static int __init md_init( void ) { 
   printk( "+ module md3 start!\n" ); 
   printk( "+ data string exported from md1 : %s\n", md1_data ); 
   printk( "+ string returned md1_proc() is : %s\n", md1_proc() ); 

   // ошибка инициализации модуля
   return -1; 
} 
module_init( md_init );
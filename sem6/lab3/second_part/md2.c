//. Вызывающий модуль ядра Linux


// Модуль md2, использующий экспортируемое имя, связывается с этим именем по абсолютному адресу. 
#include <linux/init.h> 
#include <linux/module.h> 
#include "md.h" 

MODULE_LICENSE( "GPL" ); 
MODULE_AUTHOR( "Oleg Tsiliuric <olej@front.ru>" ); 

static int __init md_init( void ) { 
   printk( "+ module md2 start!\n" ); 
   printk( "+ data string exported from md1 : %s\n", md1_data ); 
   printk( "+ string returned md1_proc() is : %s\n", md1_proc() ); 

    // не компилируется 
   //printk( "+ module md2 use local from md1: %s\n", md1_local());

   //printk( "+ module md2 use noexport from md1: %s\n", md1_noexport());
   return 0; 
} 

static void __exit md_exit( void ) { 
   printk( "+ module md2 unloaded!\n" ); 
} 

module_init( md_init ); 
module_exit( md_exit );
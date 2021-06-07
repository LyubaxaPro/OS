#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>

#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prokhorova");

#define MY_VFS_MAGIC_NUM 0x13131313
#define SLAB_NAME "my_vfs_cache"

//Структура kmem_cache содержит данные, относящиеся к конкретным CPU-модулям, набор настроек (доступных через файловую систему proc),
//статистических данных и элементов, необходимых для управления кэшем slab.
struct kmem_cache *cache = NULL;
static void **cache_mem_area = NULL;

// Функция создания вызывается при размещении каждого элемента
static void func_init(void *p)
{
    *(int *)p = (int)p;
}

// для кеширования inode
static struct my_vfs_inode
{
    int i_mode;
    unsigned long i_ino;
} my_vfs_inode;

static struct inode *my_vfs_make_inode(struct super_block *sb, int mode)
{
    // новая структура inode
    struct inode *ret = new_inode(sb);
    if (ret)
    {
        //директория inode NULL
        inode_init_owner(ret, NULL, mode);
        ret->i_size = PAGE_SIZE;
        //current_time(ret) размещает новую структуру inode (системным вызовом 
        //new_inode()) и заполняет ее значениями: размером и временами 
        //(citme, atime, mtime). Повторимся, аргумент mode определяет не 
        //только права доступа к файлу, но и его тип - регулярный файл или каталог.
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
        ret->i_private = &my_vfs_inode;
    }

    printk(KERN_INFO "! Создана struct inode\n");
    return ret;
}

// вывод строки перед уничтожением суперблока
static void my_vfs_put_super(struct super_block *sb)
{
    printk(KERN_INFO "! Вызвана функция уничтожения суперблока суперблока\n");
}

// Операции структуры суперблок
static struct super_operations const my_vfs_sup_ops = {
        .put_super = my_vfs_put_super,
        // заглушки из libfs
        .statfs = simple_statfs,
        .drop_inode = generic_delete_inode,
};

// инициализация суперблока
static int my_vfs_fill_sb(struct super_block *sb, void *data, int silent)
{
///////////////////// инициализировать поля структуры суперблока /////////////////////////
    sb->s_blocksize = PAGE_SIZE;  //Размер суперблока в байтах
    sb->s_blocksize_bits = PAGE_SHIFT; //Размер суперблока в битах 

    //магическое число, по которому драйвер файловой системы может проверить,
    //что на диске хранится именно та самая файловая система, а не что-то еще
    //или прочие данные
    sb->s_magic = MY_VFS_MAGIC_NUM;

    //операции суперблока
    sb->s_op = &my_vfs_sup_ops;

///////////////////// построить корневой каталог ФС/////////////////////////
    // создать inode каталога ФС
    // mode задает разрешения на создаваемый файл и его тип 
    // маска S_IFDIR говорит функции, что мы создаем каталог
    struct inode *root_inode = my_vfs_make_inode(sb, S_IFDIR | 0755); //7 - все разрешено 5 - чтение и выполнение  владелец-член группы-другие
    if (!root_inode)
    {
        printk(KERN_ERR "! Ошибка my_vfs_make_inode\n");
        return -ENOMEM;
    }

    //Файловые и inode-операции, которые мы назначаем новому inode, взяты из libfs, т.е. предоставляются ядром.

    root_inode->i_op = &simple_dir_inode_operations; 
    root_inode->i_fop = &simple_dir_operations; 

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root)
    {
        printk(KERN_ERR "! Ошибка d_make_root\n");
        iput(root_inode);  
        return -ENOMEM;
    }
    else
        printk(KERN_INFO "! Создан корневой каталог виртуальной файловой системы\n");

    printk(KERN_INFO "! Суперблок проинициализирован");
    return 0;
}

// вызывается при монтировании ФС:
// должна вернуть структуру, описывающую корневой каталог ФС
static struct dentry *my_vfs_mount(struct file_system_type *type, int flags, const char *dev, void *data)
{
    // создается виртуальная ФС, несвязанная с устройстовом
    // my_vfs_fill_sb - указатель на функцию, которая будет вызвана из mount_nodev для создания суперблока
    //nodev - Не различает файловые системы символьно-специальных и блочно-специальных устройств.
    struct dentry *const root_dentry = mount_nodev(type, flags, data, my_vfs_fill_sb);

    if (IS_ERR(root_dentry))
        printk(KERN_ERR "! Ошибка  mount_nodev\n");
    else
        printk(KERN_INFO "! Виртуальная файловая система была монтирована\n");

    // вернуть корневой каталог
    return root_dentry;
}

// Описание создаваемой ФС
static struct file_system_type my_vfs_type = {

    //поле owner отвечает за счетчик ссылок на модуль, чтобы его нельзя было случайно выгрузить. 
    //если файловая система была подмонтирована, то выгрузка модуля может привести к краху, 
    //но счетчик ссылок не позволит выгрузить модуль пока он используется, т.е. пока мы не размонтируем файловую систему.
    .owner = THIS_MODULE,
             
    //поле name хранит название файловой системы. Именно это название будет использоваться при ее монтировании.                                                            
    .name = "my_vfs",

    //mount и kill_sb два поля хранящие указатели на функции.

    //mount вызывается при монтировании ФС 
    .mount = my_vfs_mount,

    //Вызывается при размонтировании ФС
    .kill_sb = kill_litter_super,
};

// Инициализация модуля
static int __init my_vfs_init(void)
{
    // Регистрация файловой ситемы
    int ret = register_filesystem(&my_vfs_type);
    if (ret != 0)
    {
        printk(KERN_ERR "! Ошибка register_filesystem\n");
        return ret;
    }

    // Выделение непр области в физической памяти
    cache_mem_area = (void **)kmalloc(sizeof(void *), GFP_KERNEL);
    if (!cache_mem_area)
    {
        printk(KERN_ERR "! Ошибка cache_mem_area\n");
        kfree(cache_mem_area);
        return -ENOMEM;
    }

    // создание слаб кеша
    // 0 - смещение первого элемента от начала кэша (для выравнивания)
    //SLAB_HWCACHE_ALIGN — расположение каждого элемента в слабе должно
    //выравниваться по строкам процессорного кэша, это может существенно поднять производительность, но непродуктивно расходуется память;

    cache = kmem_cache_create(SLAB_NAME, sizeof(void *), 0, SLAB_HWCACHE_ALIGN, func_init);  
    if (!cache)
    {
        printk(KERN_ERR "! Ошибка kmem_cache_create\n");
        kmem_cache_destroy(cache);
        kfree(cache_mem_area);
        return -ENOMEM;
    }


    // запрос объекта
    if (NULL == ((*cache_mem_area) = kmem_cache_alloc(cache, GFP_KERNEL)))
    {
        printk(KERN_ERR "! Ошибка kmem_cache_alloc\n");
        kmem_cache_free(cache, *cache_mem_area);
        kmem_cache_destroy(cache);
        kfree(cache_mem_area);
        return -ENOMEM;
    }

    printk(KERN_INFO "! Модуль виртуальной файловой системы загружен");
    return 0;
}

static void __exit my_vfs_exit(void)
{
    kmem_cache_free(cache, *cache_mem_area);
    kmem_cache_destroy(cache);
    kfree(cache_mem_area);

    if (unregister_filesystem(&my_vfs_type) != 0)
    {
        printk(KERN_ERR "! Ошибка unregister_filesystem\n");
    }

    printk(KERN_INFO "! Модуль виртуальной файловой системы выгружен \n");
}

module_init(my_vfs_init);
module_exit(my_vfs_exit);








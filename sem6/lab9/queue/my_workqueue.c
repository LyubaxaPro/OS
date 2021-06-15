#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/lockdep.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/unistd.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prokhorova");

#define IRQ_NUM 1
#define IRQ_NAME "keyboard"
#define WQ_NAME "keyboard_wq"
#define ITER_COUNT 100000000

// Структура, описывающая очередь работ
static struct workqueue_struct *my_wq;

typedef struct
{
  struct work_struct work;
  int work_num; // номер работы
} my_work_t;

// Структуры, описывающие две работы
// (обработчики нижней половины прерывания в очереди работ)
static my_work_t *work_1;
static my_work_t *work_2;

static int counter = 0;

// Mutex
struct mutex my_mutex;

static int my_proc_show(struct seq_file *m, void *v)
{
  //work_pending - можно выяснить приостановлен ли элемент work (еще не обработан обработчиком)
  seq_printf(m, "!  Режим пользователя: is work_1 pending - %d, is work_1 pending - %d, shared counter - %d\n",
             work_pending(&(work_1->work)), work_pending(&(work_2->work)), counter);

  return 0;
}

static int my_proc_open(struct inode *inode, struct file *file)
{
  printk(KERN_INFO "! вызвана функция my_proc_open\n");
  return single_open(file, my_proc_show, NULL);
}

static struct file_operations proc_tasklet_ops = {
    .open = my_proc_open,
    .release = single_release,
    .read = seq_read,
};

void my_bottom_half(struct work_struct *work)
{
  //блокировать мьютекс
  mutex_lock(&my_mutex);
  printk(KERN_INFO "!MUTEX заблокирован\n");

  my_work_t *my_work = (my_work_t *)work;

  // какая работа (поток) вошла в крит секцию?
  //pending - 1 если еще не была выполнена, 0 иначе
  printk(KERN_INFO "! work_%d получила монопольный доступ (work_pending=%d);\n", my_work->work_num, work_pending(&(my_work->work)));

  // другая работа заблокирована в это время?
  if (my_work->work_num == 1) //
    printk(KERN_INFO "! work_2 - work_pending имеет значение %d\n", work_pending(&(work_2->work)));
  else
    printk(KERN_INFO "! work_1 - work_pending имеет значение %d\n", work_pending(&(work_1->work)));

  int t = 1, i = 0;
  for (; i < ITER_COUNT; i++)
    t *= i;
  counter++;
  printk(KERN_INFO "! Разделяемая переменая увеличина очередью работ %d = %d\n", my_work->work_num, counter);

  mutex_unlock(&my_mutex);
  printk(KERN_INFO "!MUTEX разблокирован\n");
}

// TOP HALF
// в верхней половине выполняется
// динамическая инициализация и поставновка работ в очередь работ
irqreturn_t my_irq_handler(int irq_num, void *dev_id)
{
  if (irq_num == IRQ_NUM)
  {
    
    if (work_1)
      //queue_work постановка работы в очередь работ
      queue_work(my_wq, (struct work_struct *)work_1);
    if (work_2)
      queue_work(my_wq, (struct work_struct *)work_2);

    return IRQ_HANDLED; // прерывание обработано
  }
  return IRQ_NONE; // прерывание не обработано
}

static int __init my_module_init(void)
{
  struct proc_dir_entry *entry;
  // создание seq
  entry = proc_create("wqueue", S_IRUGO | S_IWUGO, NULL, &proc_tasklet_ops); 
  if (!entry)
  {
    printk(KERN_INFO "! Ошибка proc_create\n");
    return -ENOMEM;
  }
  printk(KERN_INFO "! seq file создан\n");

  // Регистрация обработчика прерывания
  //Драйверы регистрируют обработчик аппаратного прерывания и разрешают определенную линию irq посредством функции

  // irq – номер прерывания, *handler –указатель на обработчик прерывания, irqflags – флаги,
  //  devname – ASCII текст, представляющий устройство, связанное с прерыванием, dev_id – используется
  //   прежде всего для разделения (shared) линии прерывания.
  // IRQF_SHAREDразрешает разделение irq несколькими устройствами*/

  if (request_irq(IRQ_NUM, my_irq_handler, IRQF_SHARED, IRQ_NAME, my_irq_handler))      
  {
    printk(KERN_ERR "! ошибка request_irq\n");
    return -ENOMEM;
  }
  printk(KERN_INFO "! IRQ handler зарегистрирован\n");

  //выделяем память 
  //GFP_KERNEL вызывающая функция выполняет системный вызов. Если не будет хватать памяти, то вызваемый процесс будет заблокирован пока память не появится
  work_1 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);
  work_2 = (my_work_t *)kmalloc(sizeof(my_work_t), GFP_KERNEL);

  if (work_1)
  {
    //Если требуется задать структуру work_struct динамически, то необходимо использовать INIT_WORK
    INIT_WORK((struct work_struct *)work_1, my_bottom_half);
    work_1->work_num = 1;
  }
  else
  {
    printk(KERN_ERR "! ошибка kalloc\n");
    return -ENOMEM;
  }

  if (work_2)
  {
    INIT_WORK((struct work_struct *)work_2, my_bottom_half);
    work_2->work_num = 2;
  }
  else
  {
    printk(KERN_ERR "! ошибка kalloc\n");
  }

  // создание очереди работ
  // create_workqueue - обертка над alloc_workqueue
  // #define create_workqueue(name)						\
	//    alloc_workqueue("%s", __WQ_LEGACY | WQ_MEM_RECLAIM, 1, (name))
  // https://elixir.bootlin.com/linux/v5.8/source/kernel/workqueue.c#L4238
  my_wq = create_workqueue(WQ_NAME);
  if (!my_wq)
  {
    free_irq(IRQ_NUM, my_irq_handler);
    printk(KERN_ERR "! Ошибка create_workqueue\n");
    return -ENOMEM;
  }
  printk(KERN_INFO "! Рабочая очередь создана\n");
 // Функция mutex_init () инициализирует мьютекс, на который ссылается mp, с типом, указанным в type . 
 // После успешной инициализации состояние мьютекса становится инициализированным и разблокированным.
  mutex_init(&my_mutex);

  printk(KERN_INFO "! модуль загружен\n");
  return 0;
}

// Выход загружаемого модуля
static void __exit my_module_exit(void)
{
  // Освобождение линии прерывания
  free_irq(IRQ_NUM, my_irq_handler);

  // Удаление очереди работ
  flush_workqueue(my_wq);
  destroy_workqueue(my_wq);
  mutex_destroy(&my_mutex);

  if (work_1)
    kfree(work_1);

  if (work_2)
    kfree(work_2);

  remove_proc_entry("wqueue", NULL);
  printk(KERN_INFO "! модуль выгружен\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
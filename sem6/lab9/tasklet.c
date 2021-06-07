#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h> 
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prohorova");

#define KEYB_IRQ 1
#define PROC_FILE_NAME "tasklet"

static int counter = 0;

static struct proc_dir_entry *proc_file;
char my_tasklet_data[] = "KEYBOARD IRQ";

// struct tasklet_struct

// {

// struct tasklet_struct *next; /* указатель на следующий тасклет в списке */

// unsigned long state; /* состояние тасклета */

// atomic_t count; /* счетчик ссылок */

// void (*func) (unsigned long); /* функция-обработчик тасклета*/

// unsigned long data; /* аргумент функции-обработчика тасклета */

// )
struct tasklet_struct my_tasklet;

static int proc_tasklet_show(struct seq_file *m_seq, void *v)
{
    printk(KERN_INFO "!tasklet!: proc_tasklet_show\n");
    int error = 0;
    seq_printf(m_seq, "!tasklet!: counter: %d, state: %ld, count: %d, data: %ld\n", 
      counter, my_tasklet.state, my_tasklet.count, my_tasklet.data); 
    return error;
}

static int proc_tasklet_open(struct inode *inode, struct file *file)
{
   printk(KERN_INFO "!tasklet!: proc_tasklet_open\n");
   return single_open(file, proc_tasklet_show, NULL);
}

static struct file_operations proc_tasklet_ops = {
  .open = proc_tasklet_open,
  .release = single_release,
  .read = seq_read
};


void my_tasklet_func(unsigned long data)
{
	// state - состояние тасклета
	// счетчик ссылок
	// data - аргумент ф-ии обработчика тасклета
  printk(KERN_INFO "!tasklet!: counter: %d, state: %ld, count: %d, data: %ld\n",
    counter, my_tasklet.state, my_tasklet.count, my_tasklet.data);
}

// Обработчик прерывания
irqreturn_t irq_handler(int irq, void *dev)
{
  if(irq == KEYB_IRQ)
  {
////////////////// Постановка тасклета в очередь на выполнение////////////////////////////////////////
    ++counter;
    printk(KERN_INFO "!tasklet!: counter: %d, state: %ld, count: %d, data: %ld\n",
    counter, my_tasklet.state, my_tasklet.count, my_tasklet.data);
    tasklet_schedule(&my_tasklet);
    printk(KERN_INFO "!tasklet!: counter: %d, state: %ld, count: %d, data: %ld\n",
    counter, my_tasklet.state, my_tasklet.count, my_tasklet.data);

    // прерывание обработано
    return IRQ_HANDLED;
  }
  else
    // прерывание не обработано
    return IRQ_NONE;
}

// Инициализация модуля
static int __init my_module_init(void)
{
  printk(KERN_INFO "!tasklet!: MODULE loaded!\n");

  // номер irq
  // указ-ль на наш обработчик прерывания
  // флаг разделение(совместное использование) линии IRQ с другими устройствами
  // имя устройства, связ. с прерыванием
  // идентификатор устройства, нужен для для разделения линии прер-я, отключения с линии прерваний с помощью free_irq
  ///////Драйверы регистрируют обработчик аппаратного прерывания и разрешают определенную линию irq /////////////////////
  int ret = request_irq(KEYB_IRQ, (irq_handler_t)irq_handler, IRQF_SHARED,
				"keyboard", (void *)(irq_handler));

  if (ret != 0)
  {
    printk(KERN_ERR "!tasklet!: KEYBOARD IRQ handler wasn't registered");
    return ret;
  }

  // инициализирует структуру tasklet_struct в соответствие с данными, предоставленными пользователем.
  // my_tasklet_func - обработчик тасклета
  // знач-е 3его пар-ра - ф-я обработчик
  tasklet_init(&my_tasklet, my_tasklet_func, (void *)(irq_handler));
  printk(KERN_INFO "!tasklet!: KEYBOARD IRQ handler was registered successfully");

  proc_file = proc_create_data(PROC_FILE_NAME, S_IRUGO | S_IWUGO, NULL, &proc_tasklet_ops, NULL);

  if (!proc_file) 
      return -ENOMEM;

  return ret;
}

// Выход загружаемого модуля
static void __exit my_module_exit(void)
{
  // Освобождение линии irq от указанного обработчика
  // Данные по указателю dev (2ой пар-р) требуются для удаления только конкретного устройства. 
  free_irq(KEYB_IRQ, (void *)(irq_handler));

  // Удаление тасклета
  tasklet_disable(&my_tasklet);
  // ждет заверш-я тасклета и удаляет тасклет из очереди на выполнение
  tasklet_kill(&my_tasklet);

  if (proc_file)
    remove_proc_entry(PROC_FILE_NAME, NULL);

  printk(KERN_INFO "!tasklet!: MODULE unloaded!\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
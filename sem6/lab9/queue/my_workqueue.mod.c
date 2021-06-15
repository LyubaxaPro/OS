#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xc79d2779, "module_layout" },
	{ 0x3e230885, "single_release" },
	{ 0xfee7c6cb, "seq_read" },
	{ 0xb7a8281, "remove_proc_entry" },
	{ 0x37a0cba, "kfree" },
	{ 0x8c03d20c, "destroy_workqueue" },
	{ 0x42160169, "flush_workqueue" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xdf9208c0, "alloc_workqueue" },
	{ 0x26c2e0b5, "kmem_cache_alloc_trace" },
	{ 0x8537dfba, "kmalloc_caches" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0x4c684ef, "proc_create" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0xe9a8fe46, "seq_printf" },
	{ 0x9875a8e1, "single_open" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "8EA8EEDC1251FD61024D0A1");

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
	{ 0x82072614, "tasklet_kill" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xa47bc098, "proc_create_data" },
	{ 0x9545af6d, "tasklet_init" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0xfaef0ed, "__tasklet_schedule" },
	{ 0xe9a8fe46, "seq_printf" },
	{ 0x9875a8e1, "single_open" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "923EA8D0266F3ABA9D1D793");

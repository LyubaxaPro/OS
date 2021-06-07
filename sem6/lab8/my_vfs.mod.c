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
	{ 0x5b4537ed, "simple_statfs" },
	{ 0xa6ebe15d, "generic_delete_inode" },
	{ 0xc62005e4, "kill_litter_super" },
	{ 0x3fe9fbf1, "unregister_filesystem" },
	{ 0x83c66b, "kmem_cache_destroy" },
	{ 0x5f5f1453, "kmem_cache_free" },
	{ 0x80299be8, "kmem_cache_alloc" },
	{ 0xb776a08b, "kmem_cache_create" },
	{ 0x37a0cba, "kfree" },
	{ 0x26c2e0b5, "kmem_cache_alloc_trace" },
	{ 0x8537dfba, "kmalloc_caches" },
	{ 0xa407be02, "register_filesystem" },
	{ 0x13da636d, "iput" },
	{ 0xa04a8ab9, "d_make_root" },
	{ 0x58bcd4ff, "simple_dir_operations" },
	{ 0xef056e54, "simple_dir_inode_operations" },
	{ 0xf1e72618, "current_time" },
	{ 0x1a7ee8f7, "inode_init_owner" },
	{ 0x257c1a8d, "new_inode" },
	{ 0x8af0bb4f, "mount_nodev" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "67C629849A67B0F912D7161");

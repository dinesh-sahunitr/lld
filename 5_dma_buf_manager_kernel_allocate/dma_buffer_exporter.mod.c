#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbd7aecd6, "dma_mmap_attrs" },
	{ 0x92997ed8, "_printk" },
	{ 0x71d5fefa, "dma_free_attrs" },
	{ 0x37a0cba, "kfree" },
	{ 0x607a5c68, "device_destroy" },
	{ 0x6775d5d3, "class_destroy" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xdcb764ad, "memset" },
	{ 0x7a6db5b5, "kmalloc_caches" },
	{ 0x5443de3e, "__kmalloc_cache_noprof" },
	{ 0x4405da6, "dma_alloc_attrs" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x578aad01, "dma_buf_export" },
	{ 0xa870dfc7, "dma_buf_fd" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x418c10ec, "__register_chrdev" },
	{ 0x59c02473, "class_create" },
	{ 0x6e26cac4, "device_create" },
	{ 0xc3006ab8, "dma_set_mask" },
	{ 0x71fc90ad, "dma_set_coherent_mask" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "EDEC0BEBEFD3A4F1AF05ED7");

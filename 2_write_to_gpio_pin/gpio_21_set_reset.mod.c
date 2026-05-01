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
	{ 0xedc03953, "iounmap" },
	{ 0x607a5c68, "device_destroy" },
	{ 0x6775d5d3, "class_destroy" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x418c10ec, "__register_chrdev" },
	{ 0xaf56600a, "arm64_use_ng_mappings" },
	{ 0x40863ba1, "ioremap_prot" },
	{ 0x59c02473, "class_create" },
	{ 0x6e26cac4, "device_create" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xdcb764ad, "memset" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "3386CD050E31C85204F4BE5");

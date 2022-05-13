#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x28950ef1, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x2cb61da5, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0x99487493, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x196103b4, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x4cce8b07, __VMLINUX_SYMBOL_STR(_dev_info) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0x7e5df8e3, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x42c8de35, __VMLINUX_SYMBOL_STR(ioremap_nocache) },
	{ 0xbe4a1520, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0xd795224, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0xc3fc2f, __VMLINUX_SYMBOL_STR(pci_request_regions) },
	{ 0x7ae5ad74, __VMLINUX_SYMBOL_STR(sme_active) },
	{ 0x7a7f7d68, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0x59d5a7f7, __VMLINUX_SYMBOL_STR(dma_set_mask) },
	{ 0x46734db7, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x1c3e657e, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0xcf73ce21, __VMLINUX_SYMBOL_STR(pci_release_regions) },
	{ 0x450c190, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0xedc03953, __VMLINUX_SYMBOL_STR(iounmap) },
	{ 0xacbfe419, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010EEd00009038sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "4C12F59AEEAB8C122C57FC2");
MODULE_INFO(rhelversion, "7.7");
#ifdef RETPOLINE
	MODULE_INFO(retpoline, "Y");
#endif
#ifdef CONFIG_MPROFILE_KERNEL
	MODULE_INFO(mprofile, "Y");
#endif

/* Quick and dirty WZAB PCIe Bus Mastering device driver
 * 
 * Copyright (C) 2016 by Wojciech M. Zabolotny
 * wzab<at>ise.pw.edu.pl
 * Significantly based on multiple drivers included in
 * sources of Linux
 * Therefore this source is licensed under GPL v2
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>
MODULE_LICENSE("GPL v2");
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>

//PCI IDs below are not registred! Use only for experiments!
#define PCI_VENDOR_ID_WZAB 0x10EE
#define PCI_DEVICE_ID_WZAB_BM1 0x9038
//Address layout at the PCI side:
//RES0 - PER_REGS (1MB)
//RES1 - AXI_CTL (16KB)
//RES2 - BRAM 4MB (really 1MB)

static DEFINE_PCI_DEVICE_TABLE(tst1_pci_tbl) = {
    {PCI_VENDOR_ID_WZAB, PCI_DEVICE_ID_WZAB_BM1, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
    {
        0,
    }};
MODULE_DEVICE_TABLE(pci, tst1_pci_tbl);

#define SUCCESS 0
#define DEVICE_NAME "wzab_bm1"

//Global variables used to store information about WZAB_BM1
//This must be changed, if we'd like to handle multiple WZAB_BM1 instances
static volatile uint32_t *fmem = NULL;  //Pointer to registers area
static volatile uint32_t *fmem2 = NULL; //Pointer to registers area
#define N_OF_RES (3)
//If 64-bit bars are used:
//static int res_nums[N_OF_RES]={0,2};
//If 32-bit bars are used:
static int res_nums[N_OF_RES] = {0, 1, 2};

#define RES_REGS (2)
#define PER_REGS (1)

static resource_size_t mmio_start[N_OF_RES], mmio_end[N_OF_RES],
    mmio_flags[N_OF_RES], mmio_len[N_OF_RES];
#define DMA_SIZE (4 * 1024 * 1024)

//It is a dirty trick, but we can service only one device :-(
static struct pci_dev *my_pdev = NULL;

dev_t my_dev = 0;
static struct class *class_my_tst = NULL;

/* Cleanup resources */
void tst1_remove(struct pci_dev *pdev)
{
  if (my_dev && class_my_tst)
  {
    device_destroy(class_my_tst, my_dev);
  }
  if (fmem)
  {
    iounmap(fmem);
    fmem = NULL;
  }
  if (fmem2)
  {
    iounmap(fmem2);
    fmem2 = NULL;
  }

  unregister_chrdev_region(my_dev, 1);
  if (class_my_tst)
  {
    class_destroy(class_my_tst);
    class_my_tst = NULL;
  }
  pci_release_regions(pdev);
  pci_disable_device(pdev);
  //printk("<1>drv_tst1 removed!\n");
  if (my_pdev == pdev)
  {
    printk(KERN_INFO "Device %p removed !\n", pdev);
    my_pdev = NULL;
  }
}

static int tst1_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{

  int res = 0;
  int i;
  if (my_pdev)
  {
    //We can't handle more than one device
    printk(KERN_INFO "The driver handles already one device: %p\n", my_pdev);
    return -EINVAL;
  }
  res = pci_enable_device(pdev);
  if (res)
  {
    dev_err(&pdev->dev, "Can't enable PCI device, aborting\n");
    res = -ENODEV;
    goto err1;
  }
  for (i = 0; i < N_OF_RES; i++)
  {
    mmio_start[i] = pci_resource_start(pdev, res_nums[i]);
    mmio_end[i] = pci_resource_end(pdev, res_nums[i]);
    mmio_flags[i] = pci_resource_flags(pdev, res_nums[i]);
    mmio_len[i] = pci_resource_len(pdev, res_nums[i]);
    printk(KERN_INFO "Resource: %d start:%llx, end:%llx, flags:%llx, len=%llx\n",
           i, mmio_start[i], mmio_end[i], mmio_flags[i], mmio_len[i]);
    if (!(mmio_flags[i] & IORESOURCE_MEM))
    {
      dev_err(&pdev->dev, "region %i not an MMIO resource, aborting\n", i);
      res = -ENODEV;
      goto err1;
    }
  }
  if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(64)))
  {
    if (pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64)))
    {
      dev_info(&pdev->dev,
               "Unable to obtain 64bit DMA for consistent allocations\n");
      goto err1;
    }
  }
  res = pci_request_regions(pdev, DEVICE_NAME);
  if (res)
    goto err1;
  pci_set_master(pdev);
  /* Let's check if the register block is read correctly */
  fmem = ioremap(mmio_start[RES_REGS], mmio_len[RES_REGS]);
  if (!fmem)
  {
    printk("<1>Mapping of memory for %s registers failed\n",
           DEVICE_NAME);
    res = -ENOMEM;
    goto err1;
  }

  //Create the class
  class_my_tst = class_create(THIS_MODULE, "my_enc_class");
  if (IS_ERR(class_my_tst))
  {
    printk(KERN_ERR "Error creating my_tst class.\n");
    res = PTR_ERR(class_my_tst);
    goto err1;
  }
  /* Alocate device number */
  res = alloc_chrdev_region(&my_dev, 0, 1, DEVICE_NAME);
  if (res)
  {
    printk("<1>Alocation of the device number for %s failed\n",
           DEVICE_NAME);
    goto err1;
  };

  device_create(class_my_tst, NULL, my_dev, NULL, "my_bm%d", MINOR(my_dev));
  printk(KERN_INFO "%s The major device number is %d.\n",
         "Registeration is a success.",
         MAJOR(my_dev));
  printk(KERN_INFO "Registred device at: %p\n", pdev);
  my_pdev = pdev;
  return 0;
err1:
  if (fmem)
  {
    iounmap(fmem);
    fmem = NULL;
  }
  if (fmem2)
  {
    iounmap(fmem2);
    fmem2 = NULL;
  }
  return res;
}

static struct pci_driver tst1_pci_driver = {
    .name = DEVICE_NAME,
    .id_table = tst1_pci_tbl,
    .probe = tst1_probe,
    .remove = tst1_remove,
};

static int __init tst1_init_module(void)
{
  /* when a module, this is printed whether or not devices are found in probe */
#ifdef MODULE
  //  printk(version);
#endif
  return pci_register_driver(&tst1_pci_driver);
}

static void __exit tst1_cleanup_module(void)
{
  pci_unregister_driver(&tst1_pci_driver);
}

module_init(tst1_init_module);
module_exit(tst1_cleanup_module);

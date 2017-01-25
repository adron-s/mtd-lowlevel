#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/route.h>
#include <linux/pkt_sched.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_tcpudp.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/if_arp.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/err.h>

#define DEBUG 1
#include "debug.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Sergeev <sergey.sergeev@yapic.net>");
MODULE_DESCRIPTION("kernel mtd_lowlevel");

//номер mtd. например для /dev/mtd1 это 1
static int mtd_index = -1;
module_param(mtd_index, int, 0);
//если 1 то забедует весь раздел
static int debug_bad = 0;
module_param(debug_bad, int, 0);

//*********************************************************
//выполняется при загрузке модуля
static int __init mtd_lowlevel_module_init(void){
  struct mtd_info *mtd = NULL;
  struct mtd_info *master = NULL;
  struct nand_chip *chip = NULL;

  if(mtd_index == -1){
    printk(KERN_ERR "%s: no needed options. please pass: <mtd_index> [debug_bad = 1]\n", __func__);
    goto end;
  }
  //получим устройство
  mtd = get_mtd_device(NULL, mtd_index);
  if(IS_ERR(mtd)){
    printk(KERN_ERR "Can't find mtd with index := '%d'!\n", mtd_index);
    mtd = NULL;
    return -ENODEV;
  }
  printk(KERN_DEBUG "Ok. Device mtd%d, name = '%s' successfully found.\n",
         mtd->index,  mtd->name);
  //получим настоящий mtd_info в котором заполенны все поля(в частности mtd->priv)
  master = mtdpart_get_master(mtd);
  //part->master это уже настоящий mtd_info нашего устройства а не фейк для публики каким был результат get_mtd_device
  chip = master->priv;
  printk(KERN_INFO "chip = 0x%p\n", chip);
  if(chip == NULL){
    printk(KERN_ERR "can't get chip struct from part->master!\n");
    goto end;
  }

  //наебываем ядро чтобы разрешить 
  if(!debug_bad && chip->bbt){
    void *old = chip->bbt;
    //creating fake bbt
    int len = master->size >> (chip->bbt_erase_shift + 2); //len скипировано с nand_bbt.c
    chip->bbt = kzalloc(len, GFP_KERNEL);
    printk(KERN_INFO "Go to sleep...grace time for protect old bbt\n");
    //подготовимся к schedule_timeout-у
    set_current_state(TASK_INTERRUPTIBLE);
    //отпускаем ядро погулять :-). grace time чтобы никто не использован старый bbt
    schedule_timeout(HZ * 5);
    printk("Sleep done. Freeing old bbt\n");
    kfree(old);
    printk(KERN_INFO "reset chip->bbt done. Now you can do: mtd erase %s\n", mtd->name);
  }
  if(debug_bad){ //ручное побайтовое бэдование. для проверки.
    typeof(mtd->size) a;
    printk(KERN_INFO "Marking all size(%llu) of /dev/mtd%d as bad bad block :-)\n", mtd->size, mtd->index);
    for(a = 0; a < mtd->size; a++){
      mtd_block_markbad(mtd, a);
    }
  }
end:
  if(mtd)
    put_mtd_device(mtd);
  return -ENOMEM;
}//--------------------------------------------------------

//*********************************************************
//выполняется при выгрузке модуля
static void __exit mtd_lowlevel_module_exit (void){
}//--------------------------------------------------------

module_init(mtd_lowlevel_module_init);
module_exit(mtd_lowlevel_module_exit);

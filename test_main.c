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
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/err.h>
#include <linux/math64.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <linux/preempt.h>
#include <linux/mtd/rawnand.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Sergeev <sergey.sergeev@yapic.net>");
MODULE_DESCRIPTION("kernel test");

//*********************************************************
static int __init test_m_module_init(void){
	struct mtd_info *mtd;
	struct mtd_info *master = NULL;
	struct nand_chip *chip = NULL;
	const char part[ ] = "kernel";
	u32 offset = 0, mb_count = 0;
	u32 max_offset = 0x003c0000;

	mtd = get_mtd_device_nm(part);
	if (IS_ERR(mtd))
		return -ENODEV;

	pr_info("%s: OWL: mtd->name: %s, mtd->parent: 0x%px\n", __func__, mtd->name, mtd->parent);

	//this code allows you to remove previously marked bad blocks
	if (1) {
		//get the real mtd_info in which all fields are filled
		master = mtd_get_master(mtd);
		//master is already a real mtd_info of our device and not a fake for the public what was the result of get_mtd_device
		chip = mtd_to_nand(master);
		pr_info("chip = 0x%p\n", chip);
		if (chip == NULL) {
			pr_err("can't get chip struct from part->master!\n");
			goto end;
		}

		if (chip->bbt){
			void *old = chip->bbt;
			//creating fake bbt
			int len = master->size >> (chip->bbt_erase_shift + 2); //len is copied from nand_bbt.c
			chip->bbt = kzalloc(len, GFP_KERNEL);
			pr_info("Go to sleep...grace time for protect old bbt\n");
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(HZ * 5);
			pr_info("Sleep done. Freeing old bbt\n");
			kfree(old);
			pr_info("reset chip->bbt done. Now you can do: mtd erase %s\n", mtd->name);
  	}
	}

	//this code allows you to create(mark) the new bad blocks
	if (0) {
		while (offset < max_offset) {
			mtd_block_markbad(mtd, offset);
			pr_info("Mark block 0x%x as BAD\n", offset);
			offset += 0x20000 * 4;
			if (mb_count++ > 5)
				break;
		}
	}

end:
	if (mtd)
		put_mtd_device(mtd);
	return -ENOMEM;
}//--------------------------------------------------------

//*********************************************************
static void __exit test_m_module_exit(void){

}//--------------------------------------------------------

module_init(test_m_module_init);
module_exit(test_m_module_exit);

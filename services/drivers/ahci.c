#include "ahci.h"
#include "ahci_def.h"
#include "../hardware/pci.h"
#include "../../kernel/paging.h"
#include "cpu_timer.h"

volatile HBA_MEM_t * ahci_bar;
u8 count = 0;

void stop_cmd(HBA_PORT_t * port){
	port->cmd &= ~HBA_PxCMD_ST;
	port->cmd &= ~HBA_PxCMD_FRE;
	while (port->cmd & (HBA_PxCMD_FR | HBA_PxCMD_CR));
}

void start_cmd(HBA_PORT_t * port){
	while (port->cmd & HBA_PxCMD_CR);
	port->cmd |= HBA_PxCMD_ST;
	port->cmd |= HBA_PxCMD_FRE;
}
 
void port_rebase(HBA_PORT_t * port, u8 port_num){
	u32 base = AHCI_BASE;
	port->cmd_lower = base + (port_num * 0x400);
	port->cmd_upper = 0;
	mset(port->cmd_lower, 0, 0x400);
	base += 0x8000; // 0x400 * 32
	
	port->fis_base_lower = base + (port_num * 0x100);
	port->fis_base_upper = 0;
	mset(port->fis_base_lower, 0, 0x100);
	base += 0x2000; // 0x100 * 32;
	
	HBA_CMD_HEADER_t * cmd_header = port->cmd_lower;
	u32 cmd_tbls = base + (port_num * 0x2000);
	for (u8 i = 0; i < 32; i++){
		cmd_header[i].phy_reg_desc_tbl_len = 0;
		cmd_header[i].port_mul_port = 0;
		// cmd_headers: 0x400 = 32*32
		// CMDTBL: 0x2000 = 0x100*32
		cmd_header[i].cmd_tbl_desc_lower = cmd_tbls + (i * 0x100);
		cmd_header[i].cmd_tbl_desc_upper = 0;
	}
	mset(cmd_tbls, 0, 0x2000);
	print_kv_hex("Memory for one port", 0x400 + 0x100 + 0x2000);
}

void software_reset(HBA_PORT_t * port){
	stop_cmd(port);
	
	port->task_file_data &= ~0b10001000;
	
	kprint("Command engine restart...\n");
	port->cmd |= HBA_PxCMD_ST;
	
	HBA_CMD_HEADER_t * cmd_header = port->cmd_lower;
	
	kprint("CMD0 creation...\n");
	cmd_header[0].length = sizeof(FIS_REG_H2D_t) / sizeof(u32);
	cmd_header[0].direction = D2H;
	cmd_header->phy_reg_desc_tbl_len = 0;
	cmd_header[0].reset = 1;
	cmd_header[0].clear = 1;
	
	HBA_CMD_TBL_t * cmd_tbl0 = cmd_header[0].cmd_tbl_desc_lower;
	FIS_REG_H2D_t * cmd_fis0 = &cmd_tbl0->cmd_fis;
	cmd_fis0->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis0->cmd_or_ctrl = CTRL;
	cmd_fis0->control = 0b100; // SRST bit
	
	kprint("CMD1 creation...\n");
	cmd_header[1].length = sizeof(FIS_REG_H2D_t) / sizeof(u32);
	cmd_header[1].direction = D2H;
	cmd_header[1].phy_reg_desc_tbl_len = 0;
	cmd_header[1].reset = 0;
	cmd_header[1].clear = 0;
	
	HBA_CMD_TBL_t * cmd_tbl1 = cmd_header[1].cmd_tbl_desc_lower;
	FIS_REG_H2D_t * cmd_fis1 = &cmd_tbl1->cmd_fis;
	cmd_fis1->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis1->cmd_or_ctrl = CTRL;
	cmd_fis1->control = 0;
	
	kprint("CI double toggle...\n");
	port->cmd_issue = 0b11;

	kprint("Monitoring CMD\n");
	while (port->cmd_issue){
		kprint("\r");
		print_bin(port->cmd_issue);
		kprint(" ");
		print_bin(port->sata_active);
		if (port->interrupt_status & HBA_PxIS_TFES){
			kprint("\nSoft reset error !");
			break;
		}
	}
	
	kprint("\nSRST toggle commands succeeded !\n\n");
	
	print_kv_bin("sact", port->sata_active);
	print_kv_bin("ci", port->cmd_issue);
}

void port_reset(HBA_PORT_t * port){
	stop_cmd(port);
	//kprint("stop_cmd\n");
	port->sata_control &= ~0b1110;
	sleep(2);
	//port->sata_control &= ~0b1111;
	//kprint("SCTL cleared\n");
	while ((port->sata_status & 0b1111) != 0b11);
	port->sata_error = ~(u32)0;
}

#define dbg_cp if(0)

u8 read_sector(HBA_PORT_t * port, u32 start_low, u32 start_high, char * buf, u32 timeout_ms){
	
	u32 phy_buf = ext_getphyaddr((void *)buf);
	u32 count = 1;
	
	port->interrupt_status = (u32) -1;
	u8 cmd_slots = (ahci_bar->host_capabilities >> 8) & 0xf;
	print_kv_bin("read/cmd_slots", cmd_slots);
	print_kv_bin("sact", port->sata_active);
	u32 slots = port->sata_active;
	u8 slot = 0;
	while (slot < cmd_slots){
		if ((slots >> slot) & 1) break;
		else if (++slot == cmd_slots) return ERR_NO_SLOT_AVAILABLE;
	}
	print_kv("slot", slot);
	dbg_cp kprint("cp1\n");
	
	HBA_CMD_HEADER_t * cmd_header = port->cmd_lower + sizeof(HBA_CMD_HEADER_t) * slot;
	cmd_header->length = sizeof(FIS_REG_H2D_t) / sizeof(u32);
	cmd_header->direction = D2H;
	u16 tbl_len = ((count-1) >> 4) & 0xffff;
	cmd_header->phy_reg_desc_tbl_len = tbl_len + 1;
	dbg_cp kprint("cp2\n");
	
	HBA_CMD_TBL_t * cmd_tbl = cmd_header->cmd_tbl_desc_lower;
	mset(cmd_tbl, 0, tbl_len * sizeof(HBA_PRDT_ENTRY_t) + sizeof(HBA_CMD_TBL_t));
	//dbg_cp kprint("cp3\n");
	
	u16 i = 0;
	while (i < tbl_len){
		cmd_tbl->prdt_entry[i].data_lower = phy_buf & 0xffffffff;
		cmd_tbl->prdt_entry[i].data_upper = phy_buf / 0xffffffff;
		cmd_tbl->prdt_entry[i].byte_count = 8 * 1024 - 1; // 8K bytes - 1
		cmd_tbl->prdt_entry[i++].int_on_compl = 1;
		buf += 0x1000; // 4K words
		count -= 16; // 16 sectors
	}
	//dbg_cp kprint("cp4\n");
	
	cmd_tbl->prdt_entry[i].data_lower = phy_buf & 0xffffffff;
	cmd_tbl->prdt_entry[i].data_upper = phy_buf / 0xffffffff;
	cmd_tbl->prdt_entry[i].byte_count = (count << 9) - 1; // 1 sector = 512 bytes
	cmd_tbl->prdt_entry[i].int_on_compl = 1;
	dbg_cp kprint("cp5\n");
	
	FIS_REG_H2D_t * cmd_fis = &cmd_tbl->cmd_fis;
	cmd_fis->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis->cmd_or_ctrl = CMD;
	cmd_fis->command = ATA_CMD_READ_DMA_EX;
	dbg_cp kprint("cp6\n");
	
	cmd_fis->lba0 = start_low & 0xff;
	cmd_fis->lba1 = start_low / 0xff;
	cmd_fis->lba2 = start_low / 0xffff;
	cmd_fis->device = 0x40; // LBA mode
	
	cmd_fis->lba3 = start_low / 0xffffff;
	cmd_fis->lba4 = start_high & 0xff;
	cmd_fis->lba5 = start_high / 0xff;
	
	cmd_fis->count_low = count & 0xff;
	cmd_fis->count_high = count / 0xff;
	dbg_cp kprint("cp7\n");
	
	u32 init_time = get_tick();
	while (port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)){
		if (get_tick() - init_time > timeout_ms){
			kprint("error\n");
			return ERR_TIMEOUT;
		}
	}
	dbg_cp kprint("cp8\n");
	
	u32 slot_mask = 1 << slot;
	port->cmd_issue |= slot_mask;
	dbg_cp kprint("cp9\n");
	
	while (port->cmd_issue & slot_mask){
		if (port->interrupt_status & HBA_PxIS_TFES) return ERR_DISK;
	}
	dbg_cp kprint("cp10\n");
	
	// Check again
	if (port->interrupt_status & HBA_PxIS_TFES) return ERR_DISK;
	kprint("end\n");
	
	return 0;
}

void list_drives(){
	u32 pi = ahci_bar->port_implemented;
	print_kv_bin("Ports", pi);
	for (u8 i = 0; i < 32; i++){
		if ((pi >> i) & 1){
			HBA_PORT_t * port = &ahci_bar->ports[i];
			port_reset(port);
			if ((port->sata_status & 0xf) != 3 && ((port->sata_status >> 8) & 0xf) != 1) continue;
			
			// TRY
			stop_cmd(port);
			port_rebase(port, i);
			/*start_cmd(port);
			kprint("Port rebased\n");
			
			port_reset(port);
			kprint("reset 0\n");
			software_reset(port);
			kprint("reset 1\n");
			
			stop_cmd(port);*/
			port->sata_active = 0b1;
			start_cmd(port);
			
			// nTRY
			
			u32 signature = port->signature;
			if (signature == SIG_ATAPI) kprint("SATAPI");
			else if (signature == SIG_SEMB) kprint("SEMB");
			else if (signature == SIG_PM) kprint("PM");
			else kprint("SATA");
			print_kv(" drive @p", i);
			char * buf = malloc(512, false);
			u8 result = read_sector(port, 0, 0, buf, 1000);
			print_kv("return value", result);
			char * lol = "_";
			for (u16 i = 0; i < 10; i++){
				lol[0] = buf[i];
				kprint(lol);
			}
			free(buf);
		}
	}
}

void initialize_ahci_device(pci_info_t * pci){
	ahci_bar = pci_get_bar(pci, 5);
	print_kv_hex("AHCI Base Address", ahci_bar);
	ext_id_map(ahci_bar, ahci_bar + sizeof(HBA_MEM_t)); // todo: replace 0x1100 with sizeof()
	ahci_bar->bios_handoff_cs |= 0b10;
	ahci_bar->bios_handoff_cs |= 0b1000;
	/*ahci_bar->global_host_control |= GHC_RST;
	while(ahci_bar->global_host_control & GHC_RST);*/
	ahci_bar->global_host_control |= GHC_RST;
	ahci_bar->global_host_control |= GHC_AE;
	ahci_bar->global_host_control &= ~GHC_IE;
	list_drives();
}

void ahci_test(){
	pci_info_t * pci_tree = pci_get_tree();
	while (pci_tree){
		if (pci_tree->base_class == 1 && pci_tree->sub_class == 6 && pci_tree->interface == 1){
			initialize_ahci_device(pci_tree);
		}
		pci_tree = pci_tree->next;
	}
}

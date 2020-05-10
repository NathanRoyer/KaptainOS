#define SIG_ATA 0x101
#define SIG_ATAPI 0xeb140101
#define SIG_SEMB 0xc33c0101
#define SIG_PM 0x96690101

#define H2D 1
#define D2H 0
#define FIS_D2H 1
#define FIS_H2D 0
#define CMD 1
#define CTRL 0

#define FIS_TYPE_REG_H2D 0x27 // host to device
#define FIS_TYPE_REG_D2H 0x34 // device to host
#define FIS_TYPE_DMA_ACT 0x39 // DMA activate FIS - device to host
#define FIS_TYPE_DMA_SETUP 0x41 // DMA setup FIS - bidirectional
#define FIS_TYPE_DATA 0x46 // Data FIS - bidirectional
#define FIS_TYPE_BIST 0x58 // BIST activate FIS - bidirectional
#define FIS_TYPE_PIO_SETUP 0x5F // PIO setup FIS - device to host
#define FIS_TYPE_DEV_BITS 0xA1 // Set device bits FIS - device to host

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define HBA_PxIS_TFES (1 << 30)
#define GHC_AE (1 << 31)
#define GHC_RST 1
#define GHC_IE 2
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_FR 0x4000
#define HBA_PxCMD_CR 0x8000
#define AHCI_BASE 0x400000

typedef struct FIS_REG_H2D {
	u8 fis_type;
	u8 port_mul_port:4;
	u8 rsv0:3;
	u8 cmd_or_ctrl:1;
	u8 command;
	u8 feature_low;
	
	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 device;
	
	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 feature_high;
	
	u8 count_low;
	u8 count_high;
	u8 isoch_cmd_compl;
	u8 control;
	
	u8 rsv1[4];
} FIS_REG_H2D_t;

typedef struct FIS_REG_D2H {
	u8 fis_type;
	u8 port_mul_port:4;
	u8 rsv0:2;
	u8 interrupt:1;
	u8 rsv1:1;
	
	u8 status;
	u8 error;
	
	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 device;
	
	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 rsv2;
	
	u8 count_low;
	u8 count_high;
	u8 rsv3[2];
	
	u8 rsv4[4];
} FIS_REG_D2H_t;

typedef struct FIS_DATA {
	u8 fis_type;
	u8 port_mul_port:4;
	u8 rsv0:4;
	u8 rsv1[2];
	u32 data[1];
} FIS_DATA_t;

typedef struct FIS_PIO_SETUP {
	u8 fis_type;
	u8 port_mul_port:4;
	u8 rsv0:1;
	u8 direction:1;
	u8 interrupt:1;
	u8 rsv1:1;
	
	u8 status;
	u8 error;
	
	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 device;
	
	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 rsv2;
	
	u8 count_low;
	u8 count_high;
	u8 rsv3;
	u8 e_status;
	
	u16 transfer_count;
	u8 rsv4[2];
} FIS_PIO_SETUP_t;

typedef struct FIS_DMA_SETUP {
	u8 fis_type;
	u8 port_mul_port:4;
	u8 rsv0:1;
	u8 direction:1;
	u8 interrupt:1;
	u8 auto_activate:1;
	
	u8 rsv1[2];
	
	u64 dma_buffer_id;
	
	u32 rsv2;
	
	u32 dma_buffer_offset;
	u32 transfer_count;
	
	u32 rsv3;
} FIS_DMA_SETUP_t;

// been a pain to find info about this one:
typedef struct FIS_DEV_BITS {
	u8 fis_type;
	u8 port_mul_port:4;
	u8 rsv0:2;
	u8 interrupt:1;
	u8 rsv1:1;
	
	u8 status;
	u8 error;
	
	u32 sata_active;
} FIS_DEV_BITS_t;

typedef struct HBA_PRDT_ENTRY {
	u32 data_lower;
	u32 data_upper;
	u32 rsv0;
 
	u32 byte_count:22;
	u32 rsv1:9;
	u32 int_on_compl:1;
} HBA_PRDT_ENTRY_t;

typedef struct HBA_CMD_TBL {
	u8 cmd_fis[64];
	u8 atapi_cmd[16];
	u8 rsv[48];
	HBA_PRDT_ENTRY_t prdt_entry[1];
} HBA_CMD_TBL_t;

typedef struct HBA_CMD_HEADER {
	u8 length:5;
	u8 atapi:1;
	u8 direction:1;
	u8 prefetchable:1;
	u8 reset:1;
	u8 bist:1;
	u8 clear:1;
	u8 rsv0:1;
	u8 port_mul_port:4;
	u16 phy_reg_desc_tbl_len;
	
	volatile u32 phy_reg_desc_byte_count;
	
	u32 cmd_tbl_desc_lower;
	u32 cmd_tbl_desc_upper;
	
	u32 rsv1[4];
} HBA_CMD_HEADER_t;

typedef volatile struct HBA_FIS {
	FIS_DMA_SETUP_t dma_setup;
	u8 pad0[4];
	
	FIS_PIO_SETUP_t pio_setup;
	u8 pad1[12];
	
	FIS_REG_D2H_t d2h;
	u8 pad2[4];
	
	FIS_DEV_BITS_t sdb;
	
	u8 unknown[64];
	u8 rsv0[96];
} HBA_FIS_t;
 
typedef volatile struct HBA_PORT {
	u32 cmd_lower;
	u32 cmd_upper;
	u32 fis_base_lower;
	u32 fis_base_upper;
	u32 interrupt_status;
	u32 interrupt_enable;
	u32 cmd;
	u32 rsv0;
	u32 task_file_data;
	u32 signature;
	u32 sata_status;
	u32 sata_control;
	u32 sata_error;
	u32 sata_active;
	u32 cmd_issue;
	u32 sata_notif;
	u32 fis_switch_control;
	u32 rsv1[11];
	u32 vendor[4];
} HBA_PORT_t;

typedef volatile struct HBA_MEM {
	u32 host_capabilities;
	u32 global_host_control;
	u32 interrupt_status;
	u32 port_implemented;
	u32 version;
	u32 cmd_compl_coal_ctrl;
	u32 cmd_compl_coal_ports;
	u32 encl_management_location;
	u32 encl_management_control;
	u32 cap2host_capabilities_ext;
	u32 bios_handoff_cs;
 
	u8 rsv[0x74];
 
	u8 vendor[0x60];
 
	HBA_PORT_t ports[32];
} HBA_MEM_t;

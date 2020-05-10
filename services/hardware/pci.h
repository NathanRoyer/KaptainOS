#ifndef PCI_H
#define PCI_H

#include "../../utils/types.h"

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

typedef struct pci_info pci_info_t;

typedef struct pci_info {
	u16 vendor_id, device_id;
	u8 base_class, sub_class, interface, header_type;
	pci_info_t * parent;
	u32 address;
	pci_info_t * next;
} pci_info_t;

void init_pci();
u32 pci_get_bar(pci_info_t * pci, u8 i);
pci_info_t * pci_get_tree();

#endif

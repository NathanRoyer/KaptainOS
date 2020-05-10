#include "pci.h"

#define _vendor_id(a) pcicfg_in_w(a, 0)
#define _device_id(a) pcicfg_in_w(a, 0x2)
#define _base_class(a) pcicfg_in_b(a, 0xb)
#define _sub_class(a) pcicfg_in_b(a, 0xa)
#define _header_type(a) pcicfg_in_b(a, 0xe)
#define _secondary_bus(a) pcicfg_in_b(a, 0x19)
#define _interface(a) pcicfg_in_b(a, 0x9)

pci_info_t * pci_list = 0;
pci_info_t * pci_list_end = 0;

u32 pcicfg_in_l(u32 address, u8 offset){
	io_out_l(PCI_CONFIG_ADDR, address | (offset & 0xfc));
	return io_in_l(PCI_CONFIG_DATA);
}

u16 pcicfg_in_w(u32 address, u8 offset){
	io_out_l(PCI_CONFIG_ADDR, address | (offset & 0xfc));
	return io_in_w(PCI_CONFIG_DATA + (offset & 0b10));
}

u8 pcicfg_in_b(u32 address, u8 offset){
	io_out_l(PCI_CONFIG_ADDR, address | (offset & 0xfc));
	return io_in_b(PCI_CONFIG_DATA + (offset & 0b11));
}

u32 pcicfg_address(u32 bus, u32 device, u32 function){
	return 0x80000000 | (bus << 16) | (device << 11) | (function << 8);
}

u32 pci_get_bar(pci_info_t * pci, u8 i){
	return pcicfg_in_l(pci->address, i*4 + 0x10);
}

u32 query_function(u32 bus, u32 device, u32 function, pci_info_t * parent){
	u32 address = pcicfg_address(bus, device, function);
	
	pci_info_t * pci = malloc(sizeof(pci_info_t), false);
	
	pci->vendor_id = _vendor_id(address);
	if (pci->vendor_id == 0xffff) return;
	pci->device_id = _device_id(address);
	
	pci->base_class = _base_class(address);
	pci->sub_class = _sub_class(address);
	pci->interface = _interface(address);
	pci->header_type = _header_type(address);
	pci->address = address;
	pci->parent = parent;
	pci->next = 0;
	
	if (pci_list == 0) pci_list = pci;
	if (pci_list_end) pci_list_end->next = pci;
	pci_list_end = pci;
	
	if (pci->base_class == 6 && pci->sub_class == 4){
		u32 bus2 = _secondary_bus(address);
		list_bus(bus2, pci);
	}
	
	if (function == 0 && (pci->header_type & 0x80) != 0){
		for (function = 1; function < 8; function++) query_function(bus, device, function, pci);
	}
}

void list_bus(u32 bus, pci_info_t * parent){
	for (u32 device = 0; device < 32; device++){
		query_function(bus, device, 0, parent);
	}
}

void init_pci(){
	list_bus(0, 0);
}

pci_info_t * pci_get_tree(){
	return pci_list;
}

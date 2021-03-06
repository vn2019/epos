#include "cpu.h"
#include "global.h"

#define PCI_CONFIG_ADDR      0xCF8
#define PCI_CONFIG_DATA      0xCFC
#define PCI_BAR_0            0x10

struct pci_device_conf {
	uint16_t vendor_id;
	uint16_t product_id;
	uint16_t command;
	uint16_t status;
	uint8_t rev;
	uint8_t prog_if;
	uint8_t sub_class;
	uint8_t class_code;
	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;

	//if header_type&7f == 0
	uint32_t bars[6];
	uint32_t reserved[2];
	uint32_t romaddr;
	uint32_t reserved2[2];
	uint8_t intr_line;
	uint8_t intr_pin;
	uint8_t min_grant;
	uint8_t max_latency;
	uint8_t data[192];
} __attribute__((packed));

#define MAKE_BSF(bus,slot,func) (uint32_t)(((bus)<<8)|((slot)<<3)|(func))
#define GET_BUS(x)  (((x)>>8)&0xff)
#define GET_SLOT(x)  (((x)>>3)&0x1f)
#define GET_FUNC(x)  ((x)&0x7)

#define PCI_CONF_ADDR(bus,slot,func,reg) \
        (0x80000000|(MAKE_BSF(bus,slot,func)<<8)|((uint32_t)(reg&0xfc)))

struct pci_device {
	uint32_t bsf;//bus, slot, func
	struct pci_device_conf conf;
};

static int nr_pci_device = 0;
static struct pci_device pci_devices[16];/*XXX*/

static uint32_t pci_read(uint32_t addr){
	outportl(PCI_CONFIG_ADDR, addr);
	return inportl(PCI_CONFIG_DATA);
}

static void pci_write(uint32_t addr, uint32_t data){
	outportl(PCI_CONFIG_ADDR, addr);
	outportl(PCI_CONFIG_DATA, data);
}

static void pci_device_read_config(struct pci_device_conf *conf,
                                   uint32_t bus, uint32_t slot, uint32_t func)
{
	uint32_t *p = (uint32_t *)conf;
	uint32_t reg;
	for(reg = 0; reg < 0x40; reg+=4) {
		*p=pci_read(PCI_CONF_ADDR(bus, slot, func, reg));
		p++;
	}
}

static void pci_bus_scan(uint32_t bus)
{
	struct pci_device_conf yyy;
	uint32_t slot, func;

	for(slot = 0; slot < 0x20; slot++){
		pci_device_read_config(&yyy, bus, slot, 0);

		if(yyy.vendor_id == 0xFFFF)
			continue;
#if VERBOSE
		printk("PCI: bus %d slot %d func %d, VID=0x%04x PID=0x%04x\r\n",
               bus, slot, 0, yyy.vendor_id, yyy.product_id);
#endif
		pci_devices[nr_pci_device].bsf = MAKE_BSF(bus, slot, 0);
		pci_devices[nr_pci_device].conf = yyy;
		nr_pci_device++;

		if((yyy.header_type & 0x80) != 0) {
			//This device has multiple functions
			for(func = 1; func < 8; func++) {
				pci_device_read_config(&yyy, bus, slot, func);

				if(yyy.vendor_id == 0xFFFF)
					continue;
#if VERBOSE
				printk("     bus %d slot %d func %d, VID=0x%04x PID=0x%04x\r\n",
                       bus, slot, func, yyy.vendor_id, yyy.product_id);
#endif
				pci_devices[nr_pci_device].bsf = MAKE_BSF(bus, slot, func);
				pci_devices[nr_pci_device].conf = yyy;

				nr_pci_device++;
			}
		}
	}
}

static uint32_t _pci_get_bar_addr(uint32_t bus, uint32_t slot)
{
	return pci_read(PCI_CONF_ADDR(bus, slot, 0, PCI_BAR_0));
}

static uint32_t _pci_get_bar_size(uint32_t bus, uint32_t slot)
{
	uint32_t old = _pci_get_bar_addr(bus, slot);
	outportl(PCI_CONFIG_DATA, 0xFFFFFFFF);
	uint32_t size = inportl(PCI_CONFIG_DATA);
	outportl(PCI_CONFIG_DATA, old);
	return (~size)+1;
}

static struct pci_device *get_device(uint16_t vendor, uint16_t product)
{
	int i;
	for(i = 0; i < nr_pci_device; i++) {
		if(pci_devices[i].conf.vendor_id  == vendor &&
           pci_devices[i].conf.product_id == product)
			return &pci_devices[i];
	}

	return NULL;
}

void pci_init()
{
	uint32_t bus;
	nr_pci_device = 0;

	for(bus = 0; bus < 0x100; bus++) {
		if(pci_read(PCI_CONF_ADDR(bus, 0, 0, 0)) == 0xFFFFFFFF)
			continue;

		pci_bus_scan(bus);
	}
}

uint32_t pci_get_bar_addr(uint16_t vendor, uint16_t product)
{
	struct pci_device *dev = get_device(vendor, product);
	if(dev != NULL)
		return _pci_get_bar_addr(GET_BUS(dev->bsf), GET_SLOT(dev->bsf));

	return -1;
}

uint32_t pci_get_bar_size(uint16_t vendor, uint16_t product)
{
	struct pci_device *dev = get_device(vendor, product);
	if(dev != NULL)
		return _pci_get_bar_size(GET_BUS(dev->bsf), GET_SLOT(dev->bsf));

	return -1;
}

uint8_t pci_get_intr_line(uint16_t vendor, uint16_t product)
{
	struct pci_device *dev = get_device(vendor, product);
	if(dev != NULL)
		return dev->conf.intr_line;

	return 0xff;
}
#include <stdint.h>

#define SRAM_ADDR		0x3e801000
#define SIZE			0x4800UL
#define SDHC_ADDR		0x25D020
#define SCI_FAIL		0x33
#define SCI_OK			0xF0

typedef uint32_t card_data_read_t(uint32_t *dest_ptr, uint32_t len,
		uint32_t offset);

#define ROM_CARD_DATA_READ	((uint32_t *) 0x00003ebf)

int Locate_KeyImageFile(uint32_t *file1, uint32_t *file2)
{
	if (((card_data_read_t *)ROM_CARD_DATA_READ)((uint32_t *)SRAM_ADDR,
				SIZE, SDHC_ADDR) == 0)
		return SCI_FAIL;

	*file1 = 0x3e805000;
	*file2 = 0x3e805000;
	
	return SCI_OK;
}

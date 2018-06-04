


//These defines are timed specific to a series of if statements and will need to be changed
//to compensate for different writing algorithms than the one in neopixel.c

// nop=62.5ns
#define NEOPIXEL_SEND_ONE	NRF_GPIO->OUTSET = (1UL << PIN); \
		__ASM ( \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
		); \
		NRF_GPIO->OUTCLR = (1UL << PIN); \
		__ASM ( \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
		);

// modif pour avoir le bon ON-time
#define NEOPIXEL_SEND_ZERO \
	    NRF_GPIO->OUTSET = (1UL << PIN); \
	    __ASM ( \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
		); \
		NRF_GPIO->OUTCLR = (1UL << PIN);  \
		__ASM ( \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
				" NOP\n\t" \
		);


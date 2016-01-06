/*
 * Arduino Yellow LED - PB27
 */ 

// Interrupt on PB26 - Arduino PIN22

#include "sam.h"

#define LEDON (PIOB->PIO_SODR = PIO_PB27)
#define LEDOFF (PIOB->PIO_CODR = PIO_PB27)
#define LEDTOGGLE ((PIOB->PIO_ODSR & PIO_PB27) ? LEDOFF : LEDON)

char TXBUFFER[64]; // UART PDC Transmit buffer

void sendText(char *text)
{
	char *src = text;
	char *dst = TXBUFFER;
	
	// copy to buffer - MUST be in SRAM as PDC is not connected to Flash
	while ((*dst++ = *src++)); 
	
	uint32_t size = dst - TXBUFFER - 1; // calculate count of copied characters minus trailing '\0'
	
	while (UART->UART_TCR); // wait until previous transmission is finished

	UART->UART_TPR = (uint32_t)TXBUFFER; // set Trasmission pointer in PDC register
	UART->UART_TCR = size; // set count of characters to be sent; starts transmission (since UART_PTCR_TXTEN is already set)
}

void TC0_Handler(void)
{	
	// read status from TC0 status register
	TC0->TC_CHANNEL[0].TC_SR;
	
	LEDTOGGLE;	
}

void UART_Handler(void)
{	
	if (UART->UART_SR & UART_SR_RXRDY)
	{
		uint8_t in = UART->UART_RHR; // Read in received byte		
		UART->UART_CR = UART_CR_RSTSTA; // Clear receiver errors (if any)
		
		if (in == 'a')
		{
			// Disable TC clock
			TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS;			
			
			// Set compare value to about 1/3rd of a sec
			TC0->TC_CHANNEL[0].TC_RC = 32000 / 3;

			// Reset counter (SWTRG) and enable counter clock (CLKEN)
			TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
		}
		else if (in == 'b')
		{
			// Disable TC clock
			TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS;
			
			// Set compare value to about a sec
			TC0->TC_CHANNEL[0].TC_RC = 32000;

			// Reset counter (SWTRG) and enable counter clock (CLKEN)
			TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
		}		
	}	
}

void PIOB_Handler(void)
{
	// Save all triggered interrupts
	uint32_t status = PIOB->PIO_ISR;
	
	if (status & PIO_PB26)
	{		
  		sendText("Interrupt (Falling Edge PB26)\n");
		  
		LEDTOGGLE;
	}
}

void configure_tc(void)
{
	// Enable TC0 (27 is TC0)
	PMC->PMC_PCER0 = 1 << ID_TC0;
	
	// Disable TC clock
	TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS;
	
	// Disable interrupts
	TC0->TC_CHANNEL[0].TC_IDR = 0xFFFFFFFF;
	
	// Clear status register
	TC0->TC_CHANNEL[0].TC_SR;
	
	// Set TC0 Mode: Compare C and Clock5 (slow clock)
	TC0->TC_CHANNEL[0].TC_CMR = TC_CMR_CPCTRG | TC_CMR_TCCLKS_TIMER_CLOCK5;
	
	// Set Compare Value in RC register
	TC0->TC_CHANNEL[0].TC_RC = 64000; // note: RC oscillator is around 32kHz
	
	// Enable interrupt on RC compare
	TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;

	// Enable interrupt in NVIC
	NVIC_EnableIRQ(TC0_IRQn);
	
	// Reset counter (SWTRG) and start counter clock (CLKEN)
	TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
	
}

void configure_led_io(void)
{
	// Enable IO
	PIOB->PIO_PER = PIO_PB27;
	
	// Set to output
	PIOB->PIO_OER = PIO_PB27;
	
	// Disable pull-up
	PIOB->PIO_PUDR = PIO_PB27;	
}

void configure_int(void)
{
	// Enable Clock for PIOB - needed for sampling falling edge
	PMC->PMC_PCER0 = 1 << ID_PIOB;
	
	// Enable IO pin control
	PIOB->PIO_PER = PIO_PB26;
	
	// Disable output (set to High Z)
	PIOB->PIO_ODR = PIO_PB26;
	
	// Enable pull-up
	PIOB->PIO_PUER = PIO_PB26;
	
	
	// Enable Glitch/Debouncing filter
	PIOB->PIO_IFER = PIO_PB26;
	
	// Select Debouncing filter 
	PIOB->PIO_DIFSR = PIO_PB26;
	
	// Set Debouncing clock divider	
	PIOB->PIO_SCDR = 0x4FF;
	
	
	// Select additional detection mode (for single edge detection)
	PIOB->PIO_AIMER = PIO_PB26;
	
	// The interrupt source is an Edge detection event.
	PIOB->PIO_ESR = PIO_PB26;
	
	// The interrupt source is set to a Falling Edge detection
	PIOB->PIO_FELLSR = PIO_PB26;
	
	
	// Enables the Input Change Interrupt on the I/O line.
	PIOB->PIO_IER = PIO_PB26;
	
	// Enable Interrupt Handling in NVIC
	NVIC_EnableIRQ(PIOB_IRQn);
}

void configure_wdt(void)
{
	WDT->WDT_MR = 0x00000000; // disable WDT
}

void configure_uart(void)
{
	// Enable Clock for UART
	PMC->PMC_PCER0 = 1 << ID_UART;
	
	// Set pin in peripheral mode
	PIOA->PIO_PDR = PIO_PA8; 
	PIOA->PIO_PDR = PIO_PA9;
	
	// Select peripheral A
	PIOA->PIO_ABSR &= !PIO_PA8;
	PIOA->PIO_ABSR &= !PIO_PA9;
	
	// Enable pull-ups
	PIOA->PIO_PUER = PIO_PA8;
	PIOA->PIO_PUER = PIO_PA9;
	
	// UART clock divider (around 115200)
	UART->UART_BRGR = 84000000 / 115200 / 16;
	
	// UART mode 8n1
	UART->UART_MR = UART_MR_PAR_NO;
	
	// Enable Receive and Transmit
	UART->UART_CR = UART_CR_TXEN | UART_CR_RXEN;
	
	// Enable UART interrupt
	UART->UART_IER = UART_IER_RXRDY ;//| UART_IER_TXRDY;
	
	// Enable UART Interrupt Handling in NVIC
	NVIC_EnableIRQ(UART_IRQn);
	
	// Enable Peripheral DMA Controller Transmission
	UART->UART_PTCR = UART_PTCR_TXTEN;
}

int main(void)
{
	/* Initialize the SAM system */
	SystemInit();
	//SystemCoreClockUpdate();
	configure_led_io();
	configure_tc();
	configure_wdt();
	configure_uart();
	configure_int();

	while (1)
	{
		//TODO:: Please write your application code
	}
}

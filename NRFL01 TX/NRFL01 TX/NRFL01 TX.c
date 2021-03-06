/*
nrf24l01 lib sample

copyright (c) Arnab Das Ar TEch MD

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>

//define debug print enable or disable on uart
#define DEBUGENABLED 1

//include uart
#if DEBUGENABLED == 1
#include <avr/serial.h>
#endif

//include nrf24l01
#include <nrf24l01/nrf24l01.h>

//role definitions
#define ROLETX 1
#define ROLERX 0

uint8_t nrf24l01_writestr(char *data)
{
	uint8_t i = 0;
	uint8_t ret = 0;

	//set tx mode
	nrf24l01_setTX();

	//write data
	nrf24l01_CSNlo; //low CSN
	spi_writereadbyte(NRF24L01_CMD_W_TX_PAYLOAD);
	for (i=0; i<NRF24L01_PAYLOAD; i++)
	spi_writereadbyte(data[i]);
	nrf24l01_CSNhi; //high CSN

	//start transmission
	nrf24l01_CEhi; //high CE
	_delay_us(15);
	nrf24l01_CElo; //low CE

	//stop if max_retries reached or send is ok
	do {
		_delay_us(10);
	}
	while( !(nrf24l01_getstatus() & (1<<NRF24L01_REG_MAX_RT | 1<<NRF24L01_REG_TX_DS)) );

	if(nrf24l01_getstatus() & 1<<NRF24L01_REG_TX_DS)
	ret = 1;

	//reset PLOS_CNT
	nrf24l01_writeregister(NRF24L01_REG_RF_CH, NRF24L01_CH);

	//power down
	nrf24l01_writeregister(NRF24L01_REG_CONFIG, nrf24l01_readregister(NRF24L01_REG_CONFIG) & ~(1<<NRF24L01_REG_PWR_UP));

	//set rx mode
	nrf24l01_setRX();

	return ret;
}

//main here
int main(void) {
	uint8_t txrxrole = 0; // 1 transmitter 0 receiver
	uint8_t i = 0;

	//nrf24l01 variables
	char bufferout[NRF24L01_PAYLOAD];
	uint8_t bufferin[NRF24L01_PAYLOAD];

	#if DEBUGENABLED == 1
	//init uart
	USART_Init(103);
	#endif

	//init nrf24l01
	nrf24l01_init();

	//init interrupt
	sei();

	txrxrole = ROLETX;         /////////////////////////////////////////////////

	#if DEBUGENABLED == 1
	if(txrxrole == ROLETX)
	uart_putc('T');
	else if(txrxrole == ROLERX)
	uart_putc('R');
	#endif

	//setup buffer
	/*for(i=0; i<sizeof(bufferout); i++)
	bufferout[i] = i+'a';
	for(i=0; i<sizeof(bufferin); i++)
	bufferin[i] = 0;*/
	
	//sending buffer addresses
	uint8_t sendpipe = 0;
	uint8_t addrtx0[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP0;
	uint8_t addrtx1[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP1;
	uint8_t addrtx2[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP2;
	uint8_t addrtx3[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP3;
	uint8_t addrtx4[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP4;
	uint8_t addrtx5[NRF24L01_ADDRSIZE] = NRF24L01_ADDRP5;

	#if DEBUGENABLED == 1 && NRF24L01_PRINTENABLE == 1
	nrf24l01_printinfo(uart_puts, uart_putc);
	#endif

	//main loop
	for(;;)
	{
		if(txrxrole == ROLETX)
		{//Tx
				#if DEBUGENABLED == 1
				char pipebuffer[5];
				uart_puts("sending data, on pipe ");
				itoa(sendpipe, pipebuffer, 10);
				uart_puts(pipebuffer);
				uart_puts("... ");
				#endif

				if(sendpipe == 0){
					//set tx address for pipe 0
					nrf24l01_settxaddr(addrtx0);
					} else if(sendpipe == 1) {
					//set tx address for pipe 1
					nrf24l01_settxaddr(addrtx1);
					} else if(sendpipe == 2) {
					//set tx address for pipe 2
					nrf24l01_settxaddr(addrtx2);
					} else if(sendpipe == 3) {
					//set tx address for pipe 3
					nrf24l01_settxaddr(addrtx3);
					} else if(sendpipe == 4) {
					//set tx address for pipe 4
					nrf24l01_settxaddr(addrtx4);
					} else if(sendpipe == 5) {
					//set tx address for pipe 5
					nrf24l01_settxaddr(addrtx5);
				}

				//write buffer
				uint8_t writeret = nrf24l01_writestr("Arnab 101");

				#if DEBUGENABLED == 1
				if(writeret == 1)
				uart_puts("ok\r\n");
				else
				uart_puts("failed\r\n");
				#endif

				sendpipe++;
				sendpipe%=6;
				
				#if DEBUGENABLED == 0
			}
			#endif
			#if DEBUGENABLED == 1
			_delay_ms(1000);
			#endif
			}
			
			 else if(txrxrole == ROLERX) 
			 {//Rx
			uint8_t pipe = 0;
			if(nrf24l01_readready(&pipe)) { //if data is ready

				#if DEBUGENABLED == 1
				char pipebuffer[5];
				uart_puts("getting data, on pipe ");
				itoa(pipe, pipebuffer, 10);
				uart_puts(pipebuffer);
				uart_puts("\r\n");
				#endif

				//read buffer
				nrf24l01_read(bufferin);

				#if DEBUGENABLED == 1
				uint8_t samecheck = 1;
				uart_puts("  data: ");
				for(i=0; i<sizeof(bufferin); i++) {
					if(bufferin[i] != bufferout[i])
					samecheck = 0;
					uart_putc(bufferin[i]);
				}
				uart_puts("\r\n");
				if(samecheck)
				uart_puts("  check ok\r\n");
				else
				uart_puts("  check fails\r\n");
				for(i=0; i<sizeof(bufferin); i++)
				bufferin[i] = 0;
				#endif
			}
			_delay_ms(10);

		}
	}
}





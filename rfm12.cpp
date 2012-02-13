/*
	rfm12.cpp
	rfm12 radio module
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#include "rfm12.h"
#include <avr/interrupt.h>
#include <string.h>
#include <util/crc16.h>
#include "button.h"

#define SPI_PORT	PORTB
#define SPI_DDR		DDRB
#define NSEL		PB2
#define MOSI		PB3
#define SCK			PB5

//Power management register
#define dc	0	//disable clock out
#define ew	1	//enable wake up timer
#define eb	2	//low bat
#define ex	3	//crystal osc
#define es	4	//synth
#define et	5	//PLL, PA, TX
#define ebb	6	//Baseband
#define er	7	//rx chain

#define FWORD ((uint16_t) ((F0 - 430) * 400))

#if F_CPU == 1000000
#define RFMCLK 0
#elif (F_CPU == 1250000)
#define RFMCLK 1
#elif (F_CPU == 1660000)
#define RFMCLK 2
#elif (F_CPU == 2000000)
#define RFMCLK 3
#elif (F_CPU == 2500000)
#define RFMCLK 4
#elif (F_CPU == 3330000)
#define RFMCLK 5
#elif (F_CPU == 5000000)
#define RFMCLK 6
#elif (F_CPU == 10000000)
#define RFMCLK 7
#endif

#ifndef RFMCLK
#define RFMCLK 0
#endif

Rfm12 rfm12;

volatile uint8_t rxflag;

Rfm12::Rfm12(void)
{
	//Initializing SPI port
	SPCR = 1<<SPE | 1<<MSTR;
	SPI_PORT |= 1<<NSEL;
	SPI_DDR |= 1<<NSEL | 1<<MOSI | 1<<SCK;
	PORTB |= 1<<PB1; //IRQ pullup
	
	//PORTC |= 1<<PC0; //FSK pullup, not neccessary for RFM12B
	wakeupFlag = false;
	rx2flag = false;
	
	//if internal clock is used, clock pin of RFM12 is disabled to save energy
	#if USE_RFM_CLOCK == 1
		pwr = 1<<ex | 1<<eb | 1<<ew | 0<<dc; //RFM12 power control (crystal and low bat detector)
	#else
		pwr = 0<<ex | 1<<eb | 1<<ew | 1<<dc; //RFM12 power control (crystal and low bat detector)
	#endif
	
	rfm12.Trans(0x0000); //clear pending IRQ, otherwise xtal cannot switched off
	rfm12.Trans(0x8200 | pwr); //set power
}

void Rfm12::Init(void)
{
	memset((void *) l1_txbuf, 0xAA, PREAMBLE_LEN);
	l1_txbuf[PREAMBLE_LEN] = 0x2D; //sync pattern
	l1_txbuf[PREAMBLE_LEN + 1] = 0xD4; //sync pattern
	
	//Initialize RFM12, refer to RFM12 datasheet for details
	Trans(0x0000);
	Trans(0x80D7); //Chap 1.: enable data register & fifo, 433 MHz band, 12 pF crystal load
	Trans(0x8200 | pwr); //power control
	Trans(0xA000 | FWORD); //Chap 3.: Set frequency
	Trans(0xC688); //Chap 4.: Data rage, 4,79 kbit/s
	Trans(0x94A9); //Chap 5.: Receiver control, VDI output, VDI fast response, rx bandwidth 134 kHz, LNA gain -6 dB, RSSI thershold -97 dBm)
	Trans(0xC2AB); //Chap 6.: Data filter. Auto lock, slow mode, digital filter, 
	Trans(0xCA81); //Chap 7.: FIFO and reset mode. 8 bit fifo level, 
	Trans(0xC400 | 3 << 6 | 3 << 4 | 7); //Chap 9.: AFC, auto keep, -10 kHz to 7.5 kHz range, 
	Trans(0x9800 | 0<<8 | 5 << 4 | 4); //chap 10.: mod polarity, TX deviation +/- 90 kHz deviation, output power
	Trans(0xE000); //Chap 12.: disable wake up timer
	Trans(0xC800); //Chap 13.: disable low duty cycle
	Trans(0xC000 | RFMCLK<<5 | VBAT_MIN_VAL); //chap 14.: 2.5 MHz clock
	
	Trans(0x0000);
	
	PCICR |= 1<<PCIE0; //Enable pin change interrupt 0
	PCMSK0 |= 1<<PCINT1; //pin change mask, PCINT 1
}

bool Rfm12::Execute(void)
{
	bool result = false;
	if (rxflag)
	{//frame received
		result = true;
		rxflag = 0;
		
		uint16_t crc;
		
		crc = calc_crc((uint8_t *) l1_rxbuf, l1_rxdatalen);
		
		if (crc == 0)
			rx2flag = true;
		else
			SetupRX();
	}
	
	if (wakeupFlag)
		result = true;
	
	return result;
}

uint8_t *Rfm12::GetRX(uint8_t *len)
{
	if (rx2flag)
	{
		rx2flag = false;
		*len = l1_rxdatalen - 2;
		return (uint8_t *) l1_rxbuf;
	}
	else
		return 0;
}

bool Rfm12::WakeupFlagged(void)
{
	bool result = wakeupFlag;
	wakeupFlag = false;
	return result;
}

uint16_t Rfm12::Trans(uint16_t cmd)
{
	uint16_t res;
	SPI_PORT &= ~(1<<NSEL);
	
	SPDR = cmd >> 8;
	loop_until_bit_is_set(SPSR, SPIF);
	
	res = SPDR << 8;
	SPDR = cmd & 0xFF;
	loop_until_bit_is_set(SPSR, SPIF);

	res |= SPDR;
	SPI_PORT |= 1<<NSEL;
	return res;
}

void Rfm12::SetupRX(void)
{
	PCICR &= ~(1<<PCIE0); //pin change interrupt 0
	pwr |= 1<<er;
	pwr &= ~(1<<et);
	
	Trans(0x8200 | pwr); //RX mode
	Trans(0xCA81); //Reset FIFO
	Trans(0xCA83); //Reset FIFO
	mode = 0;
	PCICR |= 1<<PCIE0; //Enable pin change interrupt 0
}

void Rfm12::SetupTX(void)
{
	PCICR &= ~(1<<PCIE0); //Enable pin change interrupt 0
	pwr |= 1<<et;
	pwr &= ~(1<<er);
	Trans(0x8200 | pwr); //TX MODE
	mode = 2;
	PCICR |= 1<<PCIE0; //Enable pin change interrupt 0
}

void Rfm12::SetWakeupTimer(uint32_t ms)
{
	uint8_t r = 0;
	
	while (ms > 255)
	{
		ms >>= 1;
		r++;
	}
	
	PCICR &= ~(1<<PCIE0); //disable pin change interrupt 0
	
	Trans(0x8200 | (pwr & ~(1<<ew)));
	Trans(0xE000 | r<<8 | (ms & 0xFF));
	Trans(0x8200 | pwr);
		
	PCICR |= 1<<PCIE0; //Enable pin change interrupt 0
}

void Rfm12::OnIRQ(void)
{
	uint16_t status;
	status = Trans(0x0000);
	
	if (status & 0x8000)
	{
		switch (mode)
		{
			case 0:
				lfsr = 0x1FF;
				l1_rxdatalen = Trans(0xB000) ^ lfsr;
				l1_rxlen = (l1_rxdatalen + 7) & 0xFFF8;
				l1_rxpos = 0;
				ShakeLFSR();
				mode = 1;
				break;
				
			case 1:
				l1_rxbuf[l1_rxpos++] = Trans(0xB000) ^ lfsr;
				ShakeLFSR();
				if (--l1_rxlen == 0)
				{
					Trans(0xCA81); //Stop FIFO
					rxflag = 1;
				}
				break;
				
			case 2:
				if (l1_txpos >= l1_txlen)
					SetupRX();
				else
					Trans(0xB800 + l1_txbuf[l1_txpos++]);
				break;
		}
	}
	
	if (status & 0x1000)
		wakeupFlag = true;
		
	if (status & 0x0400) //low battery
		button.Shutdown(true);
}

void Rfm12::L1Send(uint8_t *data, uint8_t len)
{
	uint8_t i;
	uint8_t blocklen = (len + 2 + 7) & 0xFFF8; //data len + 2 bytes crc  rounded up to 8 byte blocks
	uint16_t crc;
	
	uint8_t *l1_ptr = (uint8_t *) l1_txbuf + PREAMBLE_LEN + 2;
	*l1_ptr++ = len + 2; //copy len byte (data + crc) after preamble & sync pattern
	memcpy((void *) l1_ptr, data, len); //copy data after preamble, sync pattern & length byte
	l1_ptr += len;
	crc = calc_crc(data, len);
	//append CRC after data
	*l1_ptr++ = crc & 0xFF;
	*l1_ptr++ = crc >> 8;
	
	//pad rest of the block if data does not fill the (last) block completely
	for (i=0; i < blocklen - len - 2; i++)
		*l1_ptr++ = 0xAA;
		
	//data whitening starting at len byte
	lfsr = 0x1FF;
	for (i=0; i<blocklen + 1; i++)
	{
		l1_txbuf[PREAMBLE_LEN + 2 + i] ^= lfsr;
		ShakeLFSR();
	}
	
	l1_txpos = 0;
	l1_txlen = PREAMBLE_LEN + 2 + 1 + blocklen + 1; //preamble, sync pattern, len byte, data (incl. crc), postamble
	SetupTX();
}

void Rfm12::ShakeLFSR(void)
{
	uint8_t i;
	for (i=0; i<8; i++)
	{
		lfsr |= ((lfsr << 9) ^ (lfsr << 4)) & 0x200;
		lfsr >>= 1;
	}
}

uint16_t Rfm12::calc_crc(uint8_t *data, uint8_t len)
{
	uint16_t crc = 0xFFFF;
	while (len--)
		crc = _crc_ccitt_update(crc, *data++);
	return crc;
}

ISR (SIG_PIN_CHANGE0)
{
	if (bit_is_set(PINB, PB1))
		return;
		
	rfm12.OnIRQ();	
}
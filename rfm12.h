/*
	rfm12.h
	header file for rfm12 class
	Stefan Seegel, post@seegel-systeme.de
	feb 2011
	http://opensource.org/licenses/gpl-license.php GNU Public License
*/
#ifndef rfm12_h
#define rfm12_h

#include "task.h"

#define MAX_DATA_LEN	24
#define PREAMBLE_LEN	5
#define RX_BUF_LEN		((MAX_DATA_LEN + 2 + 7) & 0xFFF8)
#define TX_BUF_LEN		PREAMBLE_LEN + 2 + 1 + RX_BUF_LEN + 1 //preamble + sync + len + data + postamble

#define VBAT_MIN 3.2
#define F0 434.750 //Centerfrequency / Mhz 

#define VBAT_MIN_VAL (((uint8_t) ((VBAT_MIN - 2.2) * 10)) & 0x1F)

class Rfm12: public Task
{
	private:
		volatile uint8_t mode;
		volatile uint8_t l1_txbuf[TX_BUF_LEN];
		volatile uint8_t l1_txpos;
		volatile uint8_t l1_txlen;
		volatile uint8_t l1_rxbuf[RX_BUF_LEN];
		volatile uint8_t l1_rxlen;
		volatile uint8_t l1_rxdatalen;
		volatile uint8_t l1_rxpos;
		volatile bool wakeupFlag;
		uint16_t lfsr;
		void SetupTX(void);
		void ShakeLFSR(void);
		uint16_t calc_crc(uint8_t *data, uint8_t len);
		bool rx2flag;
		uint8_t pwr;
	protected:
		bool Execute(void);
	public:
		Rfm12(void);
		void Init(void);
		void OnIRQ(void);
		void L1Send(uint8_t *data, uint8_t len);
		void SetWakeupTimer(uint32_t ms);
		bool WakeupFlagged(void);
		void SetupRX(void);
		uint8_t *GetRX(uint8_t *len);
		uint16_t Trans(uint16_t cmd);
};

extern Rfm12 rfm12;
#endif
/*************************************
**	main.c							**
**									**
**	Created	: 2014/04/03 11:30:45	**
**	Author	: O.aoki				**
**************************************/

#define __ATtiny85__		1						// チップ環境を"ATtiny85"としてコンパイルする

#define I2C_SLAVE_ADDRESS	0x6A					// Original Slave Address

#define BitRst(addr, bit)	addr &= ~(1<<bit)		// Global : addrのbit番目をLowに
#define BitSet(addr, bit)	addr |=  (1<<bit)		// Global : addrのbit番目をHighに
#define BitTgl(addr, bit)	addr ^=  (1<<bit)		// Global : addrのbit番目を反転

#define	LED0					4
#define	SetLED0				BitRst(PORTB, LED0)		// Global : LED0を点ける
#define	RstLED0				BitSet(PORTB, LED0)		// Global : LED0を消す
#define	TglLED0				BitTgl(PORTB, LED0)		// Global : LED0を反転

/* ---- テスト用 ---- */
#define	LED1					3
#define	SetLED1				BitRst(PORTB, LED1)		// Global : LED1を点ける
#define	RstLED1				BitSet(PORTB, LED1)		// Global : LED1を消す
#define	TglLED1				BitTgl(PORTB, LED1)		// Global : LED1を反転

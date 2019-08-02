/*************************************
**	main.c							**
**									**
**	Created	: 2014/04/03 11:30:45	**
**	Author	: O.aoki				**
**************************************/

#define __ATtiny85__		1						// �`�b�v����"ATtiny85"�Ƃ��ăR���p�C������

#define I2C_SLAVE_ADDRESS	0x6A					// Original Slave Address

#define BitRst(addr, bit)	addr &= ~(1<<bit)		// Global : addr��bit�Ԗڂ�Low��
#define BitSet(addr, bit)	addr |=  (1<<bit)		// Global : addr��bit�Ԗڂ�High��
#define BitTgl(addr, bit)	addr ^=  (1<<bit)		// Global : addr��bit�Ԗڂ𔽓]

#define	LED0					4
#define	SetLED0				BitRst(PORTB, LED0)		// Global : LED0��_����
#define	RstLED0				BitSet(PORTB, LED0)		// Global : LED0������
#define	TglLED0				BitTgl(PORTB, LED0)		// Global : LED0�𔽓]

/* ---- �e�X�g�p ---- */
#define	LED1					3
#define	SetLED1				BitRst(PORTB, LED1)		// Global : LED1��_����
#define	RstLED1				BitSet(PORTB, LED1)		// Global : LED1������
#define	TglLED1				BitTgl(PORTB, LED1)		// Global : LED1�𔽓]

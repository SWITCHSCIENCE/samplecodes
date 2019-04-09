/*************************************
**	main.c							**
**									**
**	Created	: 2014/04/03 11:30:45	**
**	Author	: O.aoki				**
**************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "I2Cslave.h"
#include "USI_TWI_Slave.h"

/*---------------------------------------------------
	Routines for A/D converter
*/
/* --== Initialize ==-- */
void	ADC_Init( unsigned char adcha ) {
	ADCSRA |= _BV(ADEN);							// A/Dを許可
	ADCSRA |= _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);		// ADCクロック128分周
	BitSet(ADMUX,ADLAR);							// 左詰めで格納。10bit中の上位8bit取り出し
	ADMUX |= adcha;									// adch をAD入力chとする
}

/* --== Read Value ==-- */
unsigned char	ADC_Read( unsigned char adcha ) {
	ADMUX |= adcha;									// adch をAD入力chとする

	BitSet(ADCSRA,ADIF);							// 変換完了フラグをリセット
	BitSet(ADCSRA,ADSC);							// 変換開始。ADSC(A/D変換開始レジスタ)

	loop_until_bit_is_set(ADCSRA,ADIF);				// 変換完了フラグが1になるまで待機

	return ( ADCH );								// ad_dataにデータの上位8ビットを格納
}

/*---------------------------------------------------
	Main Routine
*/
int	main( void ) {
	/* ******* 初期化開始 ******* */
	cli();						// 初期処理中は割り込み禁止

	// ---- ポートＢ初期化
	BitSet(MCUCR, PUD);
	DDRB  = 0x10;				// Set to ouput
	PORTB = 0x10;				// Startup pattern
	// ---- ADC初期設定
	ADC_Init(3);
	// ---- USIモジュール初期化
	USI_TWI_Slave_Initialise( I2C_SLAVE_ADDRESS );

	sei();						// 割込み許可
	/* ******* 初期化終了 ******* */

	while (1) {
		Put_AD0_Data(ADC_Read(3));
	}
}

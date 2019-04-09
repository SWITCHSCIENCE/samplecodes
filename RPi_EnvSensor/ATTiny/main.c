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
	ADCSRA |= _BV(ADEN);							// A/D������
	ADCSRA |= _BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);		// ADC�N���b�N128����
	BitSet(ADMUX,ADLAR);							// ���l�߂Ŋi�[�B10bit���̏��8bit���o��
	ADMUX |= adcha;									// adch ��AD����ch�Ƃ���
}

/* --== Read Value ==-- */
unsigned char	ADC_Read( unsigned char adcha ) {
	ADMUX |= adcha;									// adch ��AD����ch�Ƃ���

	BitSet(ADCSRA,ADIF);							// �ϊ������t���O�����Z�b�g
	BitSet(ADCSRA,ADSC);							// �ϊ��J�n�BADSC(A/D�ϊ��J�n���W�X�^)

	loop_until_bit_is_set(ADCSRA,ADIF);				// �ϊ������t���O��1�ɂȂ�܂őҋ@

	return ( ADCH );								// ad_data�Ƀf�[�^�̏��8�r�b�g���i�[
}

/*---------------------------------------------------
	Main Routine
*/
int	main( void ) {
	/* ******* �������J�n ******* */
	cli();						// �����������͊��荞�݋֎~

	// ---- �|�[�g�a������
	BitSet(MCUCR, PUD);
	DDRB  = 0x10;				// Set to ouput
	PORTB = 0x10;				// Startup pattern
	// ---- ADC�����ݒ�
	ADC_Init(3);
	// ---- USI���W���[��������
	USI_TWI_Slave_Initialise( I2C_SLAVE_ADDRESS );

	sei();						// �����݋���
	/* ******* �������I�� ******* */

	while (1) {
		Put_AD0_Data(ADC_Read(3));
	}
}

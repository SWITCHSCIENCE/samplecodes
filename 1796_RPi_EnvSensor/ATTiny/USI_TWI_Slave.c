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

/*------------------------------------------------------
	! Static Variables
 */
static volatile unsigned char TWI_slaveAddress;
static volatile unsigned char Exe_Cmd;
static volatile unsigned char Cmd_Data;
static volatile unsigned char AD0_Data;
static volatile unsigned char USI_TWI_Overflow_State;

/*------------------------------------------------------
	USI Non-Interrupt Sub Routines
 */
/* -=-=-=-= Initialise USI for TWI Slave mode. =-=-=-=- */
void USI_TWI_Slave_Initialise( unsigned char TWI_ownAddress ) {
	TWI_slaveAddress = TWI_ownAddress;
	Exe_Cmd = CMD_IDLE;
	Cmd_Data = 0;
	SET_USI_TO_TWI_START_CONDITION_MODE();
}

/* -=-=-=-= Hold A/D conv. value =-=-=-=- */
void Put_AD0_Data( unsigned char dat) {
	AD0_Data = dat;
}

/* -=-=-=-= USI stop condition Checker =-=-=-=- */
unsigned char USI_TWI_Stop_Condition_Check( void ) {
	unsigned char	ret_condition;

	if (USISR & (1<<USIPF)) {
		SET_USI_TO_TWI_START_CONDITION_MODE();
		ret_condition = 0xFF;
	} else {
		ret_condition = 0;
	}
	return (ret_condition);
}

/* -=-=-=-= TWI-Command Analyzer =-=-=-=- */
void TWI_Command_Analyzer( unsigned char cmd ) {
	switch ( cmd ) {
		case CMD_SET_LED0:
			SetLED0;
			Exe_Cmd = CMD_IDLE;
			break;
		case CMD_RST_LED0:
			RstLED0;
			Exe_Cmd = CMD_IDLE;
			break;
		case CMD_GET_AD0:
			Cmd_Data = AD0_Data;
			break;
		default :
			break;
	}
}

/*------------------------------------------------------
	USI Interrupt Sub Routines
 */
/* -=-=-=-= USI start condition ISR =-=-=-=- */
ISR(SIG_USI_START) {
	// ---- I2C�n���h�V�F�C�N�̊J�n�M�������o�����Ƃ���USI����������
    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
	// ---- �X�g�b�v�R���f�B�V�����̃t���O���N���A����
	USISR	=  (0<<USI_START_COND_INT)|(0<<USIOIF)|(1<<USIPF)|(0<<USIDC)|
			   (0x0<<USICNT0);
	SET_SDA_IN();
	while (1) {
		// ---- �X�^�[�g�R���f�B�V�������"SCL"��"0"�ɂȂ�̂�҂�
		if ((PIN_USI & (1<<PORT_USI_SCL)) == 0) {
			// ---- �X�^�[�g�R���f�B�V�������荞�݂ƃI�[�o�[�t���[���荞�݂�L���ɂ���
		    USICR	=  (1<<USISIE)|(1<<USIOIE)|
					   (1<<USIWM1)|(1<<USIWM0)|
					   (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|
					   (0<<USITC);
			// ---- ���ׂẴt���O���N���A���A8�r�b�g��M���鏀��������
		    USISR	=  (1<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|
					   (0x0<<USICNT0);
			break;
		}
		// ---- �X�g�b�v�R���f�B�V���������o�����狭��������
		if ((USISR & (1<<USIPF)) == (1<<USIPF)) {
			SET_USI_TO_TWI_START_CONDITION_MODE();
			break;
		}
	}
}

/* -=-=-=-= USI counter overflow ISR =-=-=-=- */
ISR(SIG_USI_OVERFLOW) {
	unsigned char	USIdat;

	USIdat = USIDR;
	switch (USI_TWI_Overflow_State) {
		/* -=-=-=-= Address mode =-=-=-=- */
		// ---- �A�h���X�̌��o��"ACK"�̏���
		case USI_SLAVE_CHECK_ADDRESS :
				if ((USIdat == 0) || (( USIdat>>1 ) == TWI_slaveAddress)) {
					// ---- �A�h���X��v -> �������s
					if ( USIdat & 0x01 ) {
						// ---- �f�[�^���[�h���� (data from Slave to Master)
						if (Exe_Cmd >= 0xC0) {
							// ---- ���[�h�R�}���h�n�Ȃ� -> �f�[�^��ԐM������
							USI_TWI_Overflow_State = USI_SLAVE_SEND_DATA;
							SET_USI_TO_SEND_ACK();
						} else {
							// ---- �R�}���h�s��v -> �����I���i�o�X������Ԃցj
						    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
							SET_USI_TO_TWI_START_CONDITION_MODE();
						}
					} else {
						// ---- �f�[�^���C�g���� (data from Master to Slave)
						USI_TWI_Overflow_State = USI_SLAVE_REQUEST_DATA;
						SET_USI_TO_SEND_ACK();
					}
				} else {
					// ---- �A�h���X�s��v -> �����I���i�o�X������Ԃցj
				    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
					SET_USI_TO_TWI_START_CONDITION_MODE();
				}
				break;

		/* -=-=-=-= Slave Transmit mode =-=-=-=- */
		// ---- �}�X�^�[���o�����X�|���X�iACK/NACK�j�̃`�F�b�N
		case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
				if ( USIdat & 0x01 ) {
					// ---- ����"NACK"���A���Ă��Ă����珈���I���i�o�X������Ԃցj
				    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
					SET_USI_TO_TWI_START_CONDITION_MODE();
					return;
				}
				// ---- "ACK"���A���Ă��Ă�����i"NACK"�Ŗ����Ȃ�j�f�[�^�ԐM�����Ɉڍs
				// ---- "USI_SLAVE_SEND_DATA"�X�e�[�g�ɂ����̂�"break"���Ȃ�

		// ---- �f�[�^���}�X�^�ɕԐM����
		case USI_SLAVE_SEND_DATA:
				if (Exe_Cmd == CMD_GET_AD0) {
					Exe_Cmd = CMD_IDLE;
					USIDR = AD0_Data;
				}
				USI_TWI_Overflow_State = USI_SLAVE_PREPARE_REPLY_FROM_SEND_DATA;
				SET_USI_TO_SEND_DATA();
				break;

		// ---- �ԐM�f�[�^�ɑ΂���"ACK"�̎�M����
		case USI_SLAVE_PREPARE_REPLY_FROM_SEND_DATA:
				USI_TWI_Overflow_State = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
				SET_USI_TO_READ_ACK();
				break;

		/* -=-=-=-= Slave Receive mode =-=-=-=- */
		// ---- �f�[�^����M���鏀��
		case USI_SLAVE_REQUEST_DATA:
				USI_TWI_Overflow_State = USI_SLAVE_GET_DATA_AND_SEND_ACK;
				SET_USI_TO_READ_DATA();
				break;

		// ---- ��M�����f�[�^���o�b�t�@�Ɏ�荞��"ACK"��Ԃ�
		case USI_SLAVE_GET_DATA_AND_SEND_ACK:
				Exe_Cmd = USIdat;
				USI_TWI_Overflow_State = USI_SLAVE_REQUEST_DATA;
				SET_USI_TO_SEND_ACK();
				TWI_Command_Analyzer(Exe_Cmd);
				break;

		/* -=-=-=-= etc. =-=-=-=- */
		default :
				break;
	}
}

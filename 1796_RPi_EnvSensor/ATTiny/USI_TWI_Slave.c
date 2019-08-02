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
	// ---- I2Cハンドシェイクの開始信号を検出したときのUSI初期化処理
    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
	// ---- ストップコンディションのフラグをクリアする
	USISR	=  (0<<USI_START_COND_INT)|(0<<USIOIF)|(1<<USIPF)|(0<<USIDC)|
			   (0x0<<USICNT0);
	SET_SDA_IN();
	while (1) {
		// ---- スタートコンディション後の"SCL"が"0"になるのを待つ
		if ((PIN_USI & (1<<PORT_USI_SCL)) == 0) {
			// ---- スタートコンディション割り込みとオーバーフロー割り込みを有効にする
		    USICR	=  (1<<USISIE)|(1<<USIOIE)|
					   (1<<USIWM1)|(1<<USIWM0)|
					   (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|
					   (0<<USITC);
			// ---- すべてのフラグをクリアし、8ビット受信する準備をする
		    USISR	=  (1<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|
					   (0x0<<USICNT0);
			break;
		}
		// ---- ストップコンディションを検出したら強制初期化
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
		// ---- アドレスの検出と"ACK"の処理
		case USI_SLAVE_CHECK_ADDRESS :
				if ((USIdat == 0) || (( USIdat>>1 ) == TWI_slaveAddress)) {
					// ---- アドレス一致 -> 処理実行
					if ( USIdat & 0x01 ) {
						// ---- データリード判定 (data from Slave to Master)
						if (Exe_Cmd >= 0xC0) {
							// ---- リードコマンド系なら -> データを返信処理へ
							USI_TWI_Overflow_State = USI_SLAVE_SEND_DATA;
							SET_USI_TO_SEND_ACK();
						} else {
							// ---- コマンド不一致 -> 処理終了（バス初期状態へ）
						    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
							SET_USI_TO_TWI_START_CONDITION_MODE();
						}
					} else {
						// ---- データライト判定 (data from Master to Slave)
						USI_TWI_Overflow_State = USI_SLAVE_REQUEST_DATA;
						SET_USI_TO_SEND_ACK();
					}
				} else {
					// ---- アドレス不一致 -> 処理終了（バス初期状態へ）
				    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
					SET_USI_TO_TWI_START_CONDITION_MODE();
				}
				break;

		/* -=-=-=-= Slave Transmit mode =-=-=-=- */
		// ---- マスターが出すレスポンス（ACK/NACK）のチェック
		case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
				if ( USIdat & 0x01 ) {
					// ---- もし"NACK"が帰ってきていたら処理終了（バス初期状態へ）
				    USI_TWI_Overflow_State = USI_SLAVE_CHECK_ADDRESS;
					SET_USI_TO_TWI_START_CONDITION_MODE();
					return;
				}
				// ---- "ACK"が帰ってきていたら（"NACK"で無いなら）データ返信処理に移行
				// ---- "USI_SLAVE_SEND_DATA"ステートにいくので"break"しない

		// ---- データをマスタに返信する
		case USI_SLAVE_SEND_DATA:
				if (Exe_Cmd == CMD_GET_AD0) {
					Exe_Cmd = CMD_IDLE;
					USIDR = AD0_Data;
				}
				USI_TWI_Overflow_State = USI_SLAVE_PREPARE_REPLY_FROM_SEND_DATA;
				SET_USI_TO_SEND_DATA();
				break;

		// ---- 返信データに対する"ACK"の受信準備
		case USI_SLAVE_PREPARE_REPLY_FROM_SEND_DATA:
				USI_TWI_Overflow_State = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
				SET_USI_TO_READ_ACK();
				break;

		/* -=-=-=-= Slave Receive mode =-=-=-=- */
		// ---- データを受信する準備
		case USI_SLAVE_REQUEST_DATA:
				USI_TWI_Overflow_State = USI_SLAVE_GET_DATA_AND_SEND_ACK;
				SET_USI_TO_READ_DATA();
				break;

		// ---- 受信したデータをバッファに取り込み"ACK"を返す
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

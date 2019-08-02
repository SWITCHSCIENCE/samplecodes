/*************************************
**	main.c							**
**									**
**	Created	: 2014/04/03 11:30:45	**
**	Author	: O.aoki				**
**************************************/

//! Prototypes
void			USI_TWI_Slave_Initialise( unsigned char );
void			Put_AD0_Data( unsigned char );
unsigned char	USI_TWI_Stop_Condition_Check( void );
void			TWI_Command_Analyzer( unsigned char );

#define TRUE                1
#define FALSE               0

typedef     unsigned char       uint8_t;

// Sample TWI commands
#define CMD_IDLE				0x00
#define CMD_SET_LED0			0x80
#define CMD_RST_LED0			0x81
#define CMD_GET_AD0				0xC0


//! Driver state defines
#define USI_SLAVE_CHECK_ADDRESS                (0x00)
#define USI_SLAVE_SEND_DATA                    (0x01)
#define USI_SLAVE_PREPARE_REPLY_FROM_SEND_DATA (0x02)
#define USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA   (0x03)
#define USI_SLAVE_REQUEST_DATA                 (0x04)
#define USI_SLAVE_GET_DATA_AND_SEND_ACK        (0x05)


//! Device dependent defines
#if defined(__AT90tiny26__) | defined(__ATtiny26__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB0
    #define PORT_USI_SCL        PORTB2
    #define PIN_USI_SDA         PINB0
    #define PIN_USI_SCL         PINB2
    #define USI_START_COND_INT  USISIF
    #define USI_START_VECTOR    USI_STRT_vect
    #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

#if defined(__AT90Tiny2313__) | defined(__ATtiny2313__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB5
    #define PORT_USI_SCL        PORTB7
    #define PIN_USI_SDA         PINB5
    #define PIN_USI_SCL         PINB7
    #define USI_START_COND_INT  USISIF
    #define USI_START_VECTOR    USI_STRT_vect
    #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

#if defined(__ATtiny25__) | defined(__ATtiny45__) | defined(__ATtiny85__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PB0
    #define PORT_USI_SCL        PB2
    #define PIN_USI_SDA         PINB0
    #define PIN_USI_SCL         PINB2
    #define USI_START_COND_INT  USISIF
    #define USI_START_VECTOR    USI_START_vect
    #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

#if defined(__AT90Mega165__) | defined(__ATmega165__)  | \
    defined(__ATmega325__)   | defined(__ATmega3250__) | \
    defined(__ATmega645__)   | defined(__ATmega6450__) | \
    defined(__ATmega329__)   | defined(__ATmega3290__) | \
    defined(__ATmega649__)   | defined(__ATmega6490__)
    #define DDR_USI             DDRE
    #define PORT_USI            PORTE
    #define PIN_USI             PINE
    #define PORT_USI_SDA        PORTE5
    #define PORT_USI_SCL        PORTE4
    #define PIN_USI_SDA         PINE5
    #define PIN_USI_SCL         PINE4
    #define USI_START_COND_INT  USISIF
    #define USI_START_VECTOR    USI_START_vect
    #define USI_OVERFLOW_VECTOR USI_OVERFLOW_vect
#endif

#if defined(__AT90Mega169__) | defined(__ATmega169__)
    #define DDR_USI             DDRE
    #define PORT_USI            PORTE
    #define PIN_USI             PINE
    #define PORT_USI_SDA        PORTE5
    #define PORT_USI_SCL        PORTE4
    #define PIN_USI_SDA         PINE5
    #define PIN_USI_SCL         PINE4
    #define USI_START_COND_INT  USISIF
    #define USI_START_VECTOR    USI_STRT_vect
    #define USI_OVERFLOW_VECTOR USI_OVF_vect
#endif

/*------------------------------------------------------
	! Functions implemented as macros
 */

#define SET_SCL_OUT()			{DDR_USI  |=  (1<<PORT_USI_SCL);}
#define SET_SCL_IN()			{DDR_USI  &= ~(1<<PORT_USI_SCL);}
#define SET_SDA_OUT()			{DDR_USI  |=  (1<<PORT_USI_SDA);}
#define SET_SDA_IN()			{DDR_USI  &= ~(1<<PORT_USI_SDA);}
#define SET_SCL_HIGH()			{PORT_USI |=  (1<<PORT_USI_SCL);}
#define SET_SCL_LOW()			{PORT_USI &= ~(1<<PORT_USI_SCL);}
#define SET_SDA_HIGH()			{PORT_USI |=  (1<<PORT_USI_SDA);}
#define SET_SDA_LOW()			{PORT_USI &= ~(1<<PORT_USI_SDA);}

// ---- 受信データに対する"ACK"を返信
#define SET_USI_TO_SEND_ACK()																												\
{																																			\
	SET_SDA_OUT();																															\
	USIDR	 =  0;															/* Prepare ACK												*/	\
	USISR	 =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|	/* Clear all flags, except Start Cond						*/	\
				(0x0E<<USICNT0);											/* 1ビット（2カウント）送信したらオーバーフロー割り込み発生	*/	\
}

// ---- 返信データに対する"ACK"の受信準備
#define SET_USI_TO_READ_ACK()																												\
{																																			\
	SET_SDA_IN();																															\
	USIDR	 =  0;															/* Prepare ACK												*/	\
	USISR	 =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|	/* Clear all flags, except Start Cond						*/	\
				(0x0E<<USICNT0);											/* 1ビット（2カウント）受信したらオーバーフロー割り込み発生	*/	\
}

// ---- データを返信（データは事前に準備）
#define SET_USI_TO_SEND_DATA()																													\
{																																				\
	SET_SDA_OUT();																																\
	USISR    =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|	/* Clear all flags, except Start Cond							*/	\
                (0x0<<USICNT0);												/* 8ビット（16カウント）送信したらオーバーフロー割り込み発生	*/	\
}

// ---- データの受信準備
#define SET_USI_TO_READ_DATA()																													\
{																																				\
	SET_SDA_IN();																																\
    USISR    =  (0<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|	/* Clear all flags, except Start Cond							*/	\
                (0x0<<USICNT0);												/* 8ビット（16カウント）受信したらオーバーフロー割り込み発生	*/	\
}

// ---- USIを初期状態へ（スタートコンディションの検出モードへ）
#define SET_USI_TO_TWI_START_CONDITION_MODE()																										\
{																																					\
	SET_SCL_HIGH();																																	\
	SET_SDA_HIGH();																																	\
	SET_SCL_OUT();																																	\
	SET_SDA_IN();																																	\
	USICR  =  (1<<USISIE)|(0<<USIOIE)|										/* Enable Start Condition Interrupt. Disable Overflow Interrupt.	*/	\
			  (1<<USIWM1)|(0<<USIWM0)|										/* Set USI in Two-wire mode. No USI Counter overflow hold.			*/	\
			  (1<<USICS1)|(0<<USICS0)|(0<<USICLK)|							/* Shift Register Clock Source = External, positive edge			*/	\
			  (0<<USITC);																															\
	USISR  =  (1<<USI_START_COND_INT)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|	/* Clear all flags, except Start Cond								*/	\
			  (0x0<<USICNT0);																														\
}

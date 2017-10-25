/*
============================================================================
 Name : main.cpp
 Author : P.Kutej
 Version :
 Copyright : Your copyright notice
 Description : Exe source file

Required:                   
    Connect Power through USB/External supply 12v
             ___________
            |   IDK     |
            |___________|
 
 ============================================================================
*/

#include "mbed.h"
#include "Shields.h"
// RSL10 Server vs. Client choice
#define RSL10_SERVER
// CS by RSL10 PCA9655E data expander
#define CS_SPI1_PCA9655E_RSL10
// SPI Slave to Master transfer by Interrupt
//#define SPI1_TRANSFER_BY_INTERRUPT
#define RSL10_DIO7	// Transfer request from slave by DIO7 vs. DIO6
#ifdef RSL10_SERVER
#define TITLE   "RSL10_IOCServer"
#else
#define TITLE   "RSL10_IOCClient"
#endif
// SPI1 Commands
#define	SPI1_DUMMY			0x00000005
#define SPI1_SYS_RESET		0x10000001
#define SPI1_SERVER_INIT	0x10000002
#define SPI1_CLIENT_INIT	0x10000003
//
NHD_C0216CZ lcd;
Serial uart2(USBTX, USBRX);
Serial uart1(p0, p1);
char u1;
char u2;
SPI spim(p5,p6,p4, NC);
static int resp=0;
#ifdef CS_SPI1_PCA9655E_RSL10
PCA9655E ioxp_S(p3,p2,0x4A,p17);
#else
PCA9655E ioxp_B(p3,p2,0x20,p17);
#endif
#ifdef CS_SPI1_PCA9655E_RSL10
#ifdef SPI1_TRANSFER_BY_INTERRUPT
void handler_int_pca9655e(void);
#endif
#endif

int main() {
	lcd.init();
	lcd.displayString(TITLE);
    lcd.setPosition(2,1);
	spim.format(32, 0);
	spim.frequency(100);
    uart2.format(8,SerialBase::None,1);
    uart1.format(8,SerialBase::None,1);

	//CS for SPI1
#ifdef CS_SPI1_PCA9655E_RSL10
//CS for SPI1 By PCA9655E RSL10 Shield
ioxp_S.setDirection(PORT0, 5, OUTPUT);
#ifdef RSL10_SERVER
ioxp_S.setValue(PORT0, 5, SET_HIGH);	//DIO8 Server
#else
ioxp_S.setValue(PORT0, 5, SET_LOW);		//DIO8 Client
#endif
ioxp_S.setDirection(PORT1, 0, OUTPUT);
ioxp_S.setValue(PORT1,0,SET_HIGH);
#ifdef RSL10_DIO7
ioxp_S.setDirection(PORT0, 4, INPUT);	//DIO7
#else
ioxp_S.setDirection(PORT0, 0, INPUT);	//DIO6
#endif
#ifdef SPI1_TRANSFER_BY_INTERRUPT
//Only falling edge supported !!!
ioxp_S.interruptInit(FALLING_EDGE, handler_int_pca9655e);
#endif
wait(0.5);
#else
//CS for SPI1 By PCA9655E Base Board NCS36510
ioxp_B.setDirection(PORT1, 5, OUTPUT);
ioxp_B.setValue(PORT1,5,SET_HIGH);
#endif


    while(1) {           // Idle Loop
#ifdef CS_SPI1_PCA9655E_RSL10
#ifdef SPI1_TRANSFER_BY_INTERRUPT
#else
    	// SPI1 transfer slave to master without interrupt
#ifdef RSL10_DIO7
    	while(0==ioxp_S.readPinVal(PORT0, 4))
#else
    	while(0==ioxp_S.readPinVal(PORT0, 0))
#endif
    		{
    		ioxp_S.setValue(PORT1,0,SET_LOW);

    		resp=spim.write((uint32_t) SPI1_DUMMY);
    		wait(0.02);	// Important for some baseboard!!!
    		ioxp_S.setValue(PORT1,0,SET_HIGH);
        		uart2.putc((int)resp);
        		//uart2.putc((int)'@');
        		//uart2.printf("%X", resp);
        		lcd.displayChar((char)resp);
        		//lcd.displayChar((char)'@');
    	}
#endif
#endif
    	// UART1 transfer
    	/*
    	if(uart1.readable()==1){
    		u1= uart1.getc();
    		//uart1.scanf("%c", (const char*) &u1);
    		lcd.displayChar((char)u1);
    		uart2.putc((int)u1);
    	}
    	*/

	if (uart2.readable()>0){
		u2=(char) uart2.getc();
		lcd.displayChar((char)u2);

#ifdef CS_SPI1_PCA9655E_RSL10
			ioxp_S.setValue(PORT1,0,SET_LOW);
#else
			ioxp_B.setValue(PORT1,5,SET_LOW);
#endif
			resp=spim.write((int) u2);
			//resp=0;

#ifdef CS_SPI1_PCA9655E_RSL10
			ioxp_S.setValue(PORT1,0,SET_HIGH);
#else
			ioxp_B.setValue(PORT1,5,SET_HIGH);
#endif
		}
    }
}

#ifdef CS_SPI1_PCA9655E_RSL10
#ifdef SPI1_TRANSFER_BY_INTERRUPT
// SPI1 transfer slave to master by interrupt
void handler_int_pca9655e(void)
{
#ifdef RSL10_DIO7
	//if(0==ioxp_S.readPinVal(PORT0, 4))
#else
	//if(0==ioxp_S.readPinVal(PORT0, 0))
#endif
	{
	ioxp_S.setValue(PORT1,0,SET_LOW);
	resp=spim.write((int) SPI1_DUMMY);
	ioxp_S.setValue(PORT1,0,SET_HIGH);
	uart2.putc((int)resp);
	//uart2.putc((int)'@');
	//uart2.printf("%X", resp);
	lcd.displayChar((char)resp);
	//lcd.displayChar((char)'@');
	/*Testing Reset */
	/*
	if((int)resp==(int)'A'){
		ioxp_S.setValue(PORT1,0,SET_LOW);
		resp=spim.write((uint32_t) SPI1_SYS_RESET);
		ioxp_S.setValue(PORT1,0,SET_HIGH);
	}
	*/
	//}
}
#endif
#endif

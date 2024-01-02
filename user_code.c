//Arduino clone for W801: 32-bit C_SKY chip from Winner Micro
//version history
// v0.1: 09/30/2022 - initial development, from STM32Duino
// v0.2: 10/01/2022 - ported over GPIO, ticks, and uart functions
// v0.3: 10/02/2022 - clock management ported over initially
// v0.4: 10/08/2022 - CDK workflow automated
// v0.5: 10/14/2022 - adc implemented
// v0.6: 10/15/2022 - fixed systick(): needed "volatile" on two variables -> likely a compiler bug
// v0.7: 10/15/2022 - RTC, PWM and ADC (temp/Vcc) are working.
// v0.7a:10/22/2022 - software i2c added
// v0.7b:10/26/2022 - true / pseudo random number generator implemented
// v0.7c:11/05/2022 - arand1/4/8/16/32(): using adc to generate random numbers
// v0.7d:11/29/2022 - added circular buffer
// v0.7e:12/23/2022 - implemented uart0 transmission over a circular buffer
//
//
//
//               |=======================|
//               |                       |
//     40M Xtal->|                  PB19 |>---------> UART0TX
//               |                  PB20 |<---------< UART0RX
//               |                       |
//               |                       |
//               |                  PB11 |>---------> LED0
//               |                  PB16 |>---------> LED1
//               |                  PB17 |>---------> LED2
//               |                  PB18 |>---------> LED3
//               |                  PB26 |>---------> LED4
//               |                  PB25 |>---------> LED5
//               |                  PB5  |>---------> LED6
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |                       |
//               |=======================|
//
//
//
//
#include "w801duino.h"					//we use w801
//#include "fractal.h"
#include "dhry.h"						//we use dhrystone benchmark

//hardware configuration
#define LED				PB5				//led on pb11/16/17/18/26/25/5
#define LED_PORT		GPIOB
#define LED_PIN			0xfffffffful
#define LED_DLY			(F_CPU / 2)		//some delay
//end hareware configuration

//global defines

//global variables
CB_TypeDef u0CB;		//uart0 buffer
CB_TypeDef u1CB;		//uart1 buffer
CB_TypeDef u5CB;		//uart5 buffer
char uBuf[400];
static uint32_t tmp0;						//temp variables

int32_t main_class_marks(void);

//flip led/pb11
void led_flp(void) {
	pinFlip(PB11);
}

//flip pb18
void pb18_flp(void) {
	pinFlip(PB18);
}

//using rtc to test for ticks on the main clock
//return: number of ticks measured over a period of time specified by sec
uint32_t rtcTks(uint8_t sec) {
	uint32_t tks;
	uint8_t tmp;
	
	tmp = RTCGetSec(); while (RTCGetSec() == tmp) continue;	//wait for the second to change
	//now a new second has arrived
	tks=ticks();
	while (sec--) {
		tmp = RTCGetSec(); while (RTCGetSec() == tmp) continue;	//wait for the second to change
	}
	return ticks() - tks;
}

void setup(void) {
	//configure SystemCoreClock via CPU_CLK in w801duino.h
	SystemCoreClock4MHZ();						//set systemcoreclock
	//SystemCoreClockUpdate();					//update systemcoreclock
	
	//initialize the led
	GIO_OUT(GPIOB, 0xfffffffful);				//all pins as output
	pinMode(LED, OUTPUT); digitalWrite(LED, LOW);
		
	//initialize the uart
	uart0Init(UART_BR115K2);					//initialize uart0
	//u0CBInit(&u0CB, UART_BR115K2);				//initialize uart0
	//u1CBInit(&u1CB, UART_BR9K6);				//initialize uart1
	//u5CBInit(&u5CB, UART_BR4K8);				//initialize uart5
	
	//initialize timer
	tmrSetPS(F_CPU / 1000000ul);
	tmr4Init(); tmr4SetPR(10000ul); tmr4AttachISR(led_flp);
	tmr5Init(); tmr5SetPR(10100ul); tmr5AttachISR(led_flp);
	
	//initialize the pmu
	//pmuInit(); pmuSetT0(100); pmuSetT1(5000);

	//set time
	//rtcSecSet(12); rtcMinSet(3); rtcHourSet(4); rtcDaySet(5); rtcMonSet(6); rtcYearSet(7);

	//wdg - interrupt every 3 seconds
	//wdgInit(); wdgSet(F_CPU * 3); wdgAttachISR(pb18_flp);

	//adc
	//adcInit();									//reset the adc
	//adcOffset(8);								//calibrate the adc

	//pwm
	//pwm3Init(); PWM3SetPS(50000ul); PWM3SetDC(10);					//output on PB25
	//pwm4Init(); PWM4SetPS(40000ul); PWM4SetDC(20);					//output on PB16
	//analogWrite(PB26, 10);						//write to pb26
	
	//touch
	//TSInit(); 
	//TSEnable(PA0); TSSetTHS(PA0, 40);
	//TSEnable(PA1); TSSetTHS(PA1, 60);
	
	//i2c
	//i2cInit();
	
	//spi
	//spiInit();
	
	//initialize prng
	//prngInit();
	
	//initialize pwm
	//pmuInit();
	
	//initialize rtc
	//RTCInit(); time2RTC(1234567890ul);			//initialize rtc
	
	//ei();										//enable interrupts - enabled by default
	
}

void loop(void) {
	static volatile uint32_t tick0=0;			//timing ticks
	uint32_t tmp;
	//time_t time_tmp;
	
	//one extra blinky
	//if (ticks() - tick1 > LED_DLY/2) {tick1 += LED_DLY/2; pinFlip(LED3);}
	
	//uart0 tranmit from CB
	//u0CBTX(&u0CB);								//transmit on uart0
	//u1CBTX(&u1CB);								//transmit on uart1
	//u5CBTX(&u5CB);								//transmit on uart5
	
	//1st blinky
	if (ticks() - tick0 > LED_DLY) {			//time is up
	//if (ticks() > tick0) {						//time is up
		tick0 += LED_DLY;						//to the next match point
		pinFlip(LED);							//flip led
		
		//benchmark
		tmp0=ticks();
		//do something
		dhrystone();							//dhrystone benchmark, 3.458M ticks
		//mandelbrot();							//33.680M ticks@double, 6.8M ticks@float
		//delayTks(1000);						//1056 ticks
		//for (tmp=0; tmp<1000/5; tmp++) {GIO_FLP(GPIOB, 1<<0); GIO_FLP(GPIOB, 1<<0); GIO_FLP(GPIOB, 1<<0); GIO_FLP(GPIOB, 1<<0); GIO_FLP(GPIOB, 1<<0); };	//8485 ticks
		//for (tmp=0; tmp<1000/5; tmp++) {pinFlip(PB0); pinFlip(PB0); pinFlip(PB0); pinFlip(PB0); pinFlip(PB0); };	//47.2K ticks
		//for (tmp=0; tmp<1000/5; tmp++) {HAL_GPIO_TogglePin(GPIOB, 1<<0); HAL_GPIO_TogglePin(GPIOB, 1<<0); HAL_GPIO_TogglePin(GPIOB, 1<<0); HAL_GPIO_TogglePin(GPIOB, 1<<0); HAL_GPIO_TogglePin(GPIOB, 1<<0); };	//51.1K ticks
		//tmp = adcReadN(ADC_CH0, 8);
		//tmp = adcReadTempCx100();
		//tmp = HAL_ADC_GET_INPUT_VOLTAGE(&hadc);
		//ticks();
		//adcReadmv1x(ADC_CH0);
		//sI2CWrite(0);
		//for (tick1=0; tick1<10; tick1++) spiWrite(0x00);	//32K ticks / 1K sends; 3.2k ticks / 100 sends; 358 ticks / 10 sends
		//spiWrite(0x00);spiWrite(0x00);spiWrite(0x00);spiWrite(0x00);spiWrite(0x00);spiWrite(0x00);spiWrite(0x00);spiWrite(0x00);spiWrite(0x00);spiWrite(0x00);	//268 ticks / 10 sends
		//u5CBTX(&u5CB);							//transmitting on uart5 / CB: 43-28 ticks
		//uart5 transmission
		//main_class_marks();						//4194 cycles
		//my_main_class_marks();					//2749 cycles
		tmp0=ticks()-tmp0;							//28 ticks overhead
		
		//display something on uart
		u0Print("F_CPU =                      ", F_CPU);
		//u0Print("F_PHB =                      ", F_PHB);
		//u0Print("ticks =                      ", ticks());
		//u0Print("tick0 =                      ", tick0);
		u0Print("tmp0  =                      ", tmp0);
		u0Print("u0bps =                      ", u0bps());
		//u0Print("rtcSec=                      ", rtcGetSec());
		//u0Print("rtcMin=                      ", rtcGetMin());
		//u0Print("rtcHur=                      ", rtcGetHour());
		//u0Print("rtcTks=                      ", rtcTicks(2));
		//u0Print("rtctim=                      ", rtc2time(&time_tmp)); uart0Puts(ctime(&time_tmp));			//print out current time
		u0Println();
	}
}

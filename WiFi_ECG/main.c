/**
 * Program skeleton for the course "Programming embedded systems"
 */

#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "setup.h"
#include "assert.h"

#include "firmware.h"
#include "graphics.h"
#include "adc.h"
#include "rtc.h"
//#include "uart.h"

#define ringBufferSize 200
#define packetSize 20
/*-----------------------------------------------------------*/
xSemaphoreHandle lcdLock;

typedef struct sample
{
	unsigned char value[9];						//eg: 1.075095
	unsigned char timestamp[2];				//hhmmss
}sample;

sample ringBuffer[ringBufferSize];
int hp=0,tp=0,ringBufferEntryCount=0;

int seq = 1;


long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void wifi_init(void);

	char data[20];
	char time_string[30];
	char uart[30];
	s16 adc_value = -1;
	uint32_t THH = 0, TMM = 0, TSS = 0;



static void lcdTask(void *params) {

	
	int i,k;
	s16 old_adc_value = 0;
	portTickType old; // variable that will store the current time (in ticks). It will then be updated by vTaskDelayUntil()
	
	
	
	old = xTaskGetTickCount(); // initial value
	
	i = 320;
	k = 0;
	GLCD_setTextColor(Red);
	
  for (;;) 
	{

		
    xSemaphoreTake(lcdLock, portMAX_DELAY);
				
				if(k>399)
				{
					k=0;
				}
				if(i<0)
				{
					i=320;
					GLCD_clear(White);
					//GLCD_setTextColor(Black);
					//GLCD_drawLine(120, 0, 320, Horizontal);
					GLCD_setTextColor(Red);
				}
				
				if(adc_value!= -1)
				{
					Draw_line(i,120-old_adc_value, i+1, 120-adc_value);
					//GLCD_putPixel(120-old_adc_value, i);
					old_adc_value = adc_value;
					adc_value = -1;
				//GLCD_displayStringLn(Line5, "PA0:");
					GLCD_displayStringLn(Line8,data);
					i--;
					GLCD_displayStringLn(Line7,time_string);
					
				}
			

   xSemaphoreGive(lcdLock);

    vTaskDelayUntil( &old, ( 15 / portTICK_RATE_MS ) ); // delay until 15 ms have passed
  }
}

/*-----------------------------------------------------------*/

/**
 * Blink the LEDs to show that we are alive
 */

static void ledTask(void *params) {
  const u8 led_val[8] = { 0x01,0x03,0x07,0x0F,0x0E,0x0C,0x08,0x00 };
  int cnt = 0;

  for (;;) {
    LED_out (led_val[cnt]);
    cnt = (cnt + 1) % sizeof(led_val);
    vTaskDelay(300 / portTICK_RATE_MS);
  }
}

/*-----------------------------------------------------------*/

static void ADCtask(void *params) {
	
	float adc_mV;
	portTickType old; // variable that will store the current time (in ticks). It will then be updated by vTaskDelayUntil()
	old = xTaskGetTickCount(); // initial value
	
  for (;;) {

			
			adc_value = readADC1(ADC_Channel_0);
			sprintf(uart,"%d\n",adc_value);
			adc_mV = ( 3.3 / 4095) * adc_value;
			adc_value = map(adc_value, 0, 4095, 0, 100);
			sprintf(data,"%f",adc_mV);
			//sprintf(uart,"%0.2d:%0.2d:%0.2d %f\n", THH, TMM, TSS,adc_mV);
			//sprintf(uart,"%f0\n",adc_mV);
		
			strcpy(ringBuffer[tp].value, data);
			strcpy(ringBuffer[tp].timestamp, "1");
			tp++;
		
			if(tp == ringBufferSize)
				tp = 0;
		
			if(ringBufferEntryCount < ringBufferSize)
				ringBufferEntryCount++;
			else
				hp = tp;
				
			//printf(uart);
			vTaskDelay(10 / portTICK_RATE_MS);
		
		
		//vTaskDelayUntil( &old, ( 1 / portTICK_RATE_MS ) ); // delay until 1 ms have passed
  }
}

static void consume(void *params) {
   sample b[packetSize];
	int i;
	wifi_init();
	for (;;) {
		
	if(ringBufferEntryCount >= packetSize)
	{
		printf("AT+CIPSTART=\"TCP\",\"inceptronix.com\",80\r\n");
		for(i=0; i<packetSize; i++)
		{
			strcpy(b[i].value,ringBuffer[(hp % ringBufferSize) + i].value);
			strcpy(b[i].timestamp,ringBuffer[(hp % ringBufferSize) + i].timestamp);
		}
		hp = hp + packetSize;
		if(hp >= ringBufferSize)
			hp = hp % ringBufferSize;
		ringBufferEntryCount = ringBufferEntryCount - packetSize;
		printf("AT+CIPSEND\r\n");
		vTaskDelay(1000 / portTICK_RATE_MS);
		printf("GET /index.php/bill-test/?seq=%d&ecg1=%s&ecg2=%s&ecg3=%s&ecg4=%s&ecg5=%s&ecg6=%s&ecg7=%s&ecg8=%s&ecg9=%s&ecg10=%s&ecg11=%s&ecg12=%s&ecg13=%s&ecg14=%s&ecg15=%s&ecg16=%s&ecg17=%s&ecg18=%s&ecg19=%s&ecg20=%s HTTP/1.1\r\nHost: inceptronix.com\r\n\r\n",seq,b[0].value,b[1].value,b[2].value,b[3].value,b[4].value,b[5].value,b[6].value,b[7].value,b[8].value,b[9].value,b[10].value,b[11].value,b[12].value,b[13].value,b[14].value,b[15].value,b[16].value,b[17].value,b[18].value,b[19].value);
		seq +=20;
		printf("AT+RST\r\n");
		vTaskDelay(30000 / portTICK_RATE_MS);
	}
	vTaskDelay(300 / portTICK_RATE_MS);
	}
	
  
}



/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/**
 * Get the RTC data
 */

static void RTCtask(void *params) {
	uint32_t TimeVar;
	portTickType old; // variable that will store the current time (in ticks). It will then be updated by vTaskDelayUntil()
	
	old = xTaskGetTickCount(); // initial value


	
  for (;;) {
		
	/* Store the RTC counter value */
	TimeVar = RTC_GetCounter();	
	/* Compute  hours */
  THH = (TimeVar / 3600) % 24;
  /* Compute minutes */
  TMM = (TimeVar % 3600) / 60;
  /* Compute seconds */
  TSS = (TimeVar % 3600) % 60; 	
		
	sprintf(time_string,"Time: %0.2d:%0.2d:%0.2d\r", THH, TMM, TSS);
		
	
	vTaskDelayUntil( &old, ( 500 / portTICK_RATE_MS ) ); // delay until 500 ms have passed
  }
}






/*
 * Entry point of program execution
 */
int main( void )
{
  prvSetupHardware();
  IOE_Config();
  initDisplay();
	
	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  {
    /* Backup data register value is not correct or not yet programmed (when
       the first time the program is executed) */
	
    /* RTC Configuration */
		RTC_Configuration();
		Time_Adjust();
		
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	}
	lcdLock = xSemaphoreCreateMutex();
	
	
	printf("Setup done\n");
	
	
  xTaskCreate(lcdTask, "lcd", 100, NULL, 2, NULL);
	xTaskCreate(consume, "consume", 500, NULL, 3, NULL);
  xTaskCreate(ledTask, "led", 100, NULL, 2, NULL);
	xTaskCreate(RTCtask, "rtc", 100, NULL, 2, NULL);
	xTaskCreate(ADCtask, "adc", 100, NULL, 3, NULL);

  vTaskStartScheduler();

  assert(0);
  return 0;                 // not reachable
}

/*-----------------------------------------------------------*/


void wifi_init(void)
{
	printf("AT+RST\r\n");
	vTaskDelay(3000 / portTICK_RATE_MS);
	printf("AT+CIPMODE=1\r\n");
	vTaskDelay(3000 / portTICK_RATE_MS);
	printf("AT+CWJAP=\"VLC\",\"visible2\"\r\n");
	printf("\r\n");
	vTaskDelay(5000 / portTICK_RATE_MS);
	printf("AT+CIPSTART=\"TCP\",\"inceptronix.com\",80\r\n");
	vTaskDelay(3000 / portTICK_RATE_MS);
}


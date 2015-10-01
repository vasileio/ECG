/**
 * UART related functions
 */
 
 
/** @addtogroup STM3210C_EVAL_COM
  * @{
  */
#define COMn                        1

/**
 * @brief Definition for COM port1, connected to USART2 (USART2 pins remapped on GPIOD)
 */ 
#define EVAL_COM1                   USART2
#define EVAL_COM1_GPIO              GPIOD
#define EVAL_COM1_CLK               RCC_APB1Periph_USART2
#define EVAL_COM1_GPIO_CLK          RCC_APB2Periph_GPIOD
#define EVAL_COM1_RxPin             GPIO_Pin_6
#define EVAL_COM1_TxPin             GPIO_Pin_5


typedef enum 
{
  COM1 = 0,
  COM2 = 1
} COM_TypeDef;

void USART_init(void);



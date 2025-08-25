#include "ht32.h"
#include "ht32_board.h"
#include "ht32_board_config.h"
#include <string.h>

u8* gURRx_Ptr;
u32 gURRx_Length;
u8* gURTx_Ptr;
u32 gURTx_Length;
bool gIsTxFinished;

void UART_Configuration(void)
{
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  USART_InitTypeDef USART_InitStructure;

  /* ?? UART0, UART1 ?? */
  CKCUClock.Bit.AFIO   = 1;
  CKCUClock.Bit.UART0  = 1;
  CKCUClock.Bit.UART1  = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);

	AFIO_GPxConfig(HTCFG_UART_TX_GPIO_ID, HTCFG_UART_TX_AFIO_PIN, AFIO_FUN_USART_UART);
  AFIO_GPxConfig(HTCFG_UART_RX_GPIO_ID, HTCFG_UART_RX_AFIO_PIN, AFIO_FUN_USART_UART);


  /* ?? UART0 */
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WORDLENGTH_8B;
  USART_InitStructure.USART_StopBits = USART_STOPBITS_1;
  USART_InitStructure.USART_Parity = USART_PARITY_NO;
  USART_InitStructure.USART_Mode = USART_MODE_NORMAL;
  USART_Init(HT_UART0, &USART_InitStructure);

  /* ?? UART1 */
  USART_Init(HT_UART1, &USART_InitStructure);

  /* ?? UART */
	USART_TxCmd(HTCFG_UART_PORT, ENABLE);
  USART_RxCmd(HTCFG_UART_PORT, ENABLE);
}


char UART_GetChar(HT_USART_TypeDef* USARTx)
{
  while (USART_GetFlagStatus(USARTx, USART_FLAG_RXDR) == RESET);
  return (char)USART_ReceiveData(USARTx);
}

void UART_PutChar(HT_USART_TypeDef* USARTx, char c)
{
  while (USART_GetFlagStatus(USARTx, USART_FLAG_TXDE) == RESET);
  USART_SendData(USARTx, c);
}


char incoming[64];
int in_idx = 0;

char replyFromPi[64];
int pi_idx = 0;

int main(void)
{
  UART_Configuration();

  // LED ???
  CKCU_PeripClockConfig_TypeDef CKCUClock = {{0}};
  CKCUClock.Bit.PA = 1;
  CKCU_PeripClockConfig(CKCUClock, ENABLE);
  GPIO_DirectionConfig(HT_GPIOA, GPIO_PIN_0, GPIO_DIR_OUT); // ?? LED ? PA0

  while (1)
  {
    /* ???? (UART1) */
    if (USART_GetFlagStatus(HT_UART1, USART_FLAG_RXDR))
    {
      char c = UART_GetChar(HT_UART1);
      if (c == '\n')
      {
        incoming[in_idx] = '\0';
        printf("??????:%s\n", incoming);
        in_idx = 0;
      }
      else
      {
        incoming[in_idx++] = c;
      }
    }

    /* ??????? (UART0) */
    if (USART_GetFlagStatus(HT_UART0, USART_FLAG_RXDR))
    {
      char c = UART_GetChar(HT_UART0);
      if (c == '\n')
      {
        replyFromPi[pi_idx] = '\0';
        printf("???????:%s\n", replyFromPi);

        if (strcmp(replyFromPi, "B") == 0)
        {
          GPIO_WriteOutBits(HT_GPIOA, GPIO_PIN_0, TRUE);
          for (volatile int i=0; i<500000; i++); // delay
          GPIO_WriteOutBits(HT_GPIOA, GPIO_PIN_0, FALSE);
        }
        pi_idx = 0;
      }
      else
      {
        replyFromPi[pi_idx++] = c;
      }
    }
  }
}


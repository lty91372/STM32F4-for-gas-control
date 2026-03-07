/*
 * uart_transmit_moudle.h
 *
 *  Created on: Oct 26, 2025
 *      Author: HP
 */

#ifndef USER_MODULES_INC_UART_TRANSMIT_MOUDLE_H_
#define USER_MODULES_INC_UART_TRANSMIT_MOUDLE_H_
#define TRANSMIT_BUFF 512

typedef struct{
	uint8_t buffer[TRANSMIT_BUFF];
	volatile uint16_t head;//写指针
	volatile uint16_t tail;//读指针
}RingBuffer;


void UART_STATE_MACHINE();
void UART_Transmit_Control();
void KeyBoard_Transmit();
void Receive_Init();

void Uart_Write_Buff(const uint8_t *data,uint16_t len);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

#endif /* USER_MODULES_INC_UART_TRANSMIT_MOUDLE_H_ */

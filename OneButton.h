/*
 * OneButton.h
 *
 *  Created on: May 23, 2021
 *      Author: arhimed32
 *      https://github.com/arhimed32/OneButton
 */

#ifndef INC_ONEBUTTON_H_
#define INC_ONEBUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define _debounce 60
#define _timeout 300
#define _click_timeout 300
#define _step_timeout 500

typedef enum {
	bf_press = ((uint16_t)0x01),
	bf_release = ((uint16_t)0x02),
	bf_single = ((uint16_t)0x04),
	bf_holded = ((uint16_t)0x08),
	bf_click = ((uint16_t)0x10),
	bf_step = ((uint16_t)0x20)
} ob_callback_flags;

typedef enum {
	OB_TICK_NORM=0,
	OB_TICK_AUTO=1
} ob_tick_mode;

#pragma pack(push,1)
typedef struct {		
	uint8_t btn_deb: 1;	
	uint8_t hold_flag: 1;
	uint8_t counter_flag: 1;
	uint8_t isHolded_f: 1;
	uint8_t isRelease_f: 1;
	uint8_t isPress_f: 1;
	uint8_t step_flag: 1;
	uint8_t oneClick_f: 1;
	uint8_t isOne_f: 1;
	uint8_t tickMode: 1;
	uint8_t no_callback: 1;
	uint8_t counter_reset: 1;			//Добавлен
} button_flags;
#pragma pack(pop)

typedef void (*callbackFunction)(ob_callback_flags flags);
typedef void (*callbackStepFunction)(uint8_t clicks);

typedef struct 
{
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;        // 

	GPIO_PinState norm_state; // свободное состояние пина кнопки

	button_flags flags;
	uint16_t debounce;
	uint16_t timeout;
	uint16_t click_timeout;
	uint16_t step_timeout;
	uint8_t counter, last_counter, last_hold_counter;
	uint32_t timer;	
	uint8_t state;
	uint8_t flag;

	callbackFunction callbackFunc;
	callbackStepFunction callbackStepFunc;

	
} OneButton_t;

void ob_init(OneButton_t *btn, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState NormState);    // инициализация структуры-кнопки
void ob_setTickMode(OneButton_t *btn, ob_tick_mode mode);                                           // (OB_TICK_NORM / OB_TICK_AUTO) ручной или автоматический опрос кнопки функцией ob_tick()
void ob_tick(OneButton_t *btn);                                                                     // опрос кнопки
void ob_resetStates(OneButton_t *btn);                                                              // сбрасывает все is-флаги и счётчики
void ob_clearFlags(OneButton_t *btn, ob_callback_flags flags);                                      // сбрасывает все is-флаги, должно вызываться внутри callbackEvent функции

void ob_attach_callbackEvent(OneButton_t *btn, callbackFunction func);                              // подключить callback метод обработчика событий вида void callbackEvent(ob_callback_flags flags)
void ob_attach_callbackStep(OneButton_t *btn, callbackStepFunction func);                           // подключить callback метод обработчика шагов вида void callbackStep(void)
void ob_detach_callbackEvent(OneButton_t *btn);                                                     // отключает ранее установленный callback метод обработчика событий
void ob_detach_callbackStep(OneButton_t *btn);                                                      // отключает ранее установленный callback метод обработчика шагов

uint8_t ob_isPress(OneButton_t *btn);                                                               // возвращает 1 при нажатии на кнопку. Сбрасывается после вызова
uint8_t ob_isRelease(OneButton_t *btn);                                                             // возвращает 1 при отпускании кнопки. Сбрасывается после вызова
uint8_t ob_isClick(OneButton_t *btn);                                                               // возвращает 1 при клике. Сбрасывается после вызова
uint8_t ob_isHolded(OneButton_t *btn);                                                              // возвращает 1 при удержании дольше timeout. Сбрасывается после вызова
uint8_t ob_isHold(OneButton_t *btn);                                                                // возвращает 1 при нажатой кнопке, не сбрасывается

uint8_t ob_isSingle(OneButton_t *btn);                                                              // возвращает 1 при одиночном клике. Сбрасывается после вызова
uint8_t ob_isDouble(OneButton_t *btn);                                                              // возвращает 1 при двойном клике. Сбрасывается после вызова
uint8_t ob_hasClicks(OneButton_t *btn);                                                             // проверка на наличие кликов. Сбрасывается после вызова
uint8_t ob_getClicks(OneButton_t *btn);                                                             // вернуть количество кликов
uint8_t ob_getHoldClicks(OneButton_t *btn);                                                         // вернуть количество кликов, предшествующее удерживанию

uint8_t ob_isStep(OneButton_t *btn, uint8_t clicks); //+

#ifdef __cplusplus
}
#endif

#endif /* INC_ONEBUTTON_H_ */

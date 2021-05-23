/*
 * OneButton.c
 *
 *  Created on: May 23, 2021
 *      Author: arhimed32
 *      https://github.com/arhimed32/OneButton
 */

#include "OneButton.h"

void ob_init(OneButton_t *btn, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState NormState){
    
    btn->GPIOx = GPIOx;
    btn->GPIO_Pin = GPIO_Pin;

    btn->debounce = _debounce;
    btn->timeout  = _timeout;
    btn->click_timeout = _click_timeout;
    btn->step_timeout = _step_timeout;

    btn->counter = 0;
    btn->last_counter = 0;
    btn->last_hold_counter = 0;
    btn->timer = 0;
    btn->state = 0;
    btn->flag = 0;

    btn->callbackFunc = NULL;
    btn->callbackStepFunc = NULL;

}

void ob_setTickMode(OneButton_t *btn, ob_tick_mode mode){
	btn->flags.tickMode = mode;
}

void ob_tick(OneButton_t *btn){

  // btn_state: 1-нажато, 0-нет
  btn->state = !btn->norm_state ^ ((btn->GPIOx->IDR & btn->GPIO_Pin) != GPIO_PIN_RESET);

  uint32_t thisMls = HAL_GetTick();

  // нажатие
  if (btn->state && !btn->flag){
  	if (!btn->flags.btn_deb){
  		btn->flags.btn_deb = 1;
  		btn->timer = thisMls;
  	} else {
  		if (thisMls - btn->timer >= btn->debounce){
  			btn->flag = 1;
  			btn->flags.isPress_f = 1;
  			btn->flags.oneClick_f = 1;

  			if (!btn->flags.no_callback && btn->callbackFunc) btn->callbackFunc(bf_press);
  		}
  	}
  } else {
      btn->flags.btn_deb = 0;
  }

  // отпускание
  if (!btn->state && btn->flag) {
  	btn->flag = 0;
  	if (!btn->flags.hold_flag) btn->counter++;
  	btn->flags.hold_flag = 0;
  	btn->flags.isRelease_f = 1;
  	btn->timer = thisMls;

  	if (btn->flags.step_flag){
  		btn->last_counter = 0;
  		btn->counter = 0;
  		btn->flags.step_flag = 0;
  	}

  	if (btn->flags.oneClick_f){
  		btn->flags.oneClick_f = 0;
  		btn->flags.isOne_f = 1;
  		if (!btn->flags.no_callback && btn->callbackFunc) btn->callbackFunc(bf_release|bf_single);
  	} else {
			if (!btn->flags.no_callback && btn->callbackFunc) btn->callbackFunc(bf_release);
  	}

	}

  // кнопка удерживается
  if (btn->flag && btn->state && (thisMls - btn->timer >= btn->timeout) && !btn->flags.hold_flag) {
  	btn->flags.hold_flag = 1;
  	btn->flags.isHolded_f = 1;
  	btn->flags.step_flag = 1;
  	btn->flags.oneClick_f = 0;
  	btn->last_hold_counter = btn->counter;
  	btn->timer = thisMls;
  	if (!btn->flags.no_callback && btn->callbackFunc) btn->callbackFunc(bf_holded);
	}

  // обработка накликивания
  if ((thisMls - btn->timer >= btn->click_timeout) && (btn->counter != 0) && !btn->state) {
  	btn->last_counter = btn->counter;
  	btn->counter = 0;
  	btn->flags.counter_flag = 1;

  	if (!btn->flags.no_callback && btn->callbackFunc) btn->callbackFunc(bf_click);

  }

 // обработка шагов и вызов callback функции
 if (btn->flags.step_flag && (thisMls - btn->timer >= btn->step_timeout)) {
	if (!btn->flags.no_callback && btn->callbackStepFunc) {
		btn->timer = thisMls;
		btn->callbackStepFunc(btn->counter);
	} else if (!btn->flags.no_callback && btn->callbackFunc) {
		btn->timer = thisMls;
		btn->callbackFunc(bf_step);
	}
 }

 // сброс накликивания
  if (btn->flags.counter_reset) {
  	btn->last_counter = 0;
  	btn->flags.counter_flag = 0;
  	btn->flags.counter_reset = 0;
  }

}

void ob_resetStates(OneButton_t *btn){
	btn->flags.isPress_f = 0;
	btn->flags.isRelease_f = 0;
	btn->flags.isOne_f = 0;
	btn->flags.isHolded_f = 0;
	btn->flags.step_flag = 0;
	btn->flags.counter_flag = 0;
	btn->last_hold_counter = 0;
	btn->last_counter = 0;
}

void ob_clearFlags(OneButton_t *btn, ob_callback_flags flags){

	if (flags & bf_press)   btn->flags.isPress_f = 0;
	if (flags & bf_release) btn->flags.isRelease_f = 0;
	if (flags & bf_single)  btn->flags.isOne_f = 0;
	if (flags & bf_holded)  btn->flags.isHolded_f = 0;

}

void ob_attach_callbackEvent(OneButton_t *btn, callbackFunction func){
	btn->callbackFunc = func;
}

void ob_attach_callbackStep(OneButton_t *btn, callbackStepFunction func){
	btn->callbackStepFunc = func;
}

void ob_detach_callbackEvent(OneButton_t *btn){
	btn->callbackFunc = NULL;
}

void ob_detach_callbackStep(OneButton_t *btn){
	btn->callbackStepFunc = NULL;
}

uint8_t ob_isPress(OneButton_t *btn){

	if (btn->flags.tickMode) {
			uint8_t f_nocallback = btn->flags.no_callback;
			ob_tick(btn);
			btn->flags.no_callback = f_nocallback;
		}

	if (btn->flags.isPress_f) {
		btn->flags.isPress_f = 0;
		return 1;
	} else return 0;

}

uint8_t ob_isRelease(OneButton_t *btn){

	if (btn->flags.tickMode) {
			uint8_t f_nocallback = btn->flags.no_callback;
			ob_tick(btn);
			btn->flags.no_callback = f_nocallback;
		}

	if (btn->flags.isRelease_f) {
		btn->flags.isRelease_f = 0;
		return 1;
	} else return 0;

}

uint8_t ob_isClick(OneButton_t *btn){

	if (btn->flags.tickMode) {
			uint8_t f_nocallback = btn->flags.no_callback;
			ob_tick(btn);
			btn->flags.no_callback = f_nocallback;
		}

	if (btn->flags.isOne_f) {
		btn->flags.isOne_f = 0;
		return 1;
	} else return 0;

}

uint8_t ob_isHolded(OneButton_t *btn){

	if (btn->flags.tickMode) {
			uint8_t f_nocallback = btn->flags.no_callback;
			ob_tick(btn);
			btn->flags.no_callback = f_nocallback;
		}

	if (btn->flags.isHolded_f) {
		btn->flags.isHolded_f = 0;
		return 1;
	} else return 0;

}

uint8_t ob_isHold(OneButton_t *btn){

	if (btn->flags.tickMode) {
		uint8_t f_nocallback = btn->flags.no_callback;
		ob_tick(btn);
		btn->flags.no_callback = f_nocallback;
	}

	return btn->flags.step_flag;

}


uint8_t ob_isSingle(OneButton_t *btn){

	if (btn->flags.tickMode) {
		uint8_t f_nocallback = btn->flags.no_callback;
		ob_tick(btn);
		btn->flags.no_callback = f_nocallback;
	}

	if (btn->flags.counter_flag && btn->last_counter == 1) {
		btn->flags.counter_reset = 1;
		return 1;
	} else return 0;
}

uint8_t ob_isDouble(OneButton_t *btn){

	if (btn->flags.tickMode) {
		uint8_t f_nocallback = btn->flags.no_callback;
		ob_tick(btn);
		btn->flags.no_callback = f_nocallback;
	}

	if (btn->flags.counter_flag && btn->last_counter == 2) {
		btn->flags.counter_reset = 1;
		return 1;
	} else return 0;
}

uint8_t ob_hasClicks(OneButton_t *btn){

	if (btn->flags.tickMode) {
		uint8_t f_nocallback = btn->flags.no_callback;
		ob_tick(btn);
		btn->flags.no_callback = f_nocallback;
	}

	if (btn->flags.counter_flag) {
		btn->flags.counter_reset = 1;
		return 1;
	} else return 0;
}

uint8_t ob_getClicks(OneButton_t *btn){
	btn->flags.counter_reset = 1;
	return btn->last_counter;
}

uint8_t ob_getHoldClicks(OneButton_t *btn){

	if (btn->flags.tickMode) {
			uint8_t f_nocallback = btn->flags.no_callback;
			ob_tick(btn);
			btn->flags.no_callback = f_nocallback;
		}

	return btn->flags.hold_flag ? btn->last_hold_counter : 0;

}

uint8_t ob_isStep(OneButton_t *btn, uint8_t clicks){

	if (btn->flags.tickMode) {
			uint8_t f_nocallback = btn->flags.no_callback;
			ob_tick(btn);
			btn->flags.no_callback = f_nocallback;
	}

	uint32_t thisMls = HAL_GetTick();
	if (btn->counter == clicks && btn->flags.step_flag && (thisMls - btn->timer >= btn->step_timeout)){
		btn->timer = thisMls;
		return 1;
	} else
		return 0;

}

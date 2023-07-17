//##############################################################################
//# ДРГБ-01 ЭКО-1 альтернативное программное обеспечение (версия 2.6)          #
//#                                                                            #
//# Загрузка данного программного обеспечения в память прибора запрещает       #
//# делать официальные заключения на основе результатов измерения прибором     #
//#                                                                            #
//#       ПРЕТЕНЗИИ НЕ ПРИНИМАЮТСЯ. ВЫ ДЕЛАЕТЕ ВСЁ НА СВОЙ СТРАХ И РИСК!       #
//#                                                                            #
//# Разработчик: SERJSOCHI                                                     #
//# http://forum.rhbz.org/profile.php?action=show&member=5153                  #
//# Тестирование и поддержка: suso                                             #
//# http://forum.rhbz.org/profile.php?action=show&member=4880                  #
//# Модификация: vlad072                                                       #
//# http://forum.rhbz.org/profile.php?action=show&member=6074                  #
//# Форум проекта:                                                             #
//# http://forum.rhbz.org/topic.php?forum=79&topic=53                          #
//##############################################################################

#include <AT89X52.H>  // AT89S52
#include <TIC55.H>  	// LCD
#include <MENU.H>  

//------------------------------------------------------------------------------
typedef char 							int8_t;
typedef unsigned char			uint8_t;
typedef int								int16;
typedef unsigned int			uint16_t;
typedef	long							int32_t;
typedef	unsigned long			uint32_t;



//------------------------------------------------------------------------------
// значения прибора по-умолчанию (при первом включении или сбросе)
//		Название				Значение	Диапазон	Описание
#define CALIB_BETA							   	  		 4.428f //	градуировочный коэффициент Kg=3.5 согласно инструкции к прибору БЕТА пересчитанный для Бета-5
#define CALIB_GAMMA								 			 	 344.8f //  20-999	калибровка гаммы, имп/с при 1 мкР/с
#define	CALIB_STUFF												1090.0f //	чувствительность Sp, приведённая из (Ки) в (Бк/Кг)(Бк/л) согласно интрукции прибора БЕТА = 3.7e+10/4.3e+7=860  пересчитанный для Бета-5
#define	SELF_CPS													 0.421f //  собственный фон датчика
//#define DEFAULT_LED_MODE 								   	  1 //  0-2 		режим работы светодиода, 0 - выкл, 1 - события с датчика, 2 - включен постоянно
//#define DEFAULT_SOUND_ENABLED 							  1	//  0-1			звуковые оповещения (все), 0 - запрещены, 1 - разрешены
//#define DEFAULT_CLICKER_LEN  									3 //  0-30		длительность оповещения о зарегистрированном событии, мс, 0 = без оповещения
//#define DEFAULT_ALERTS_ON  										1 //  0-1			оповещения о превышении фона и дозы, 0 - запрещены, 1 - разрешены
//#define DEFAULT_ALERT_LEVEL_1  			 		 		 30 //  20-49		первый уровень оповещения о превышении фона, мкР/ч
//#define DEFAULT_ALERT_LEVEL_2  			 		 		 60 //  50-99		второй уровень оповещения о превышении фона, мкР/ч
//#define DEFAULT_ALERT_LEVEL_3  							120	//  100-240	третий уровень оповещения о превышении фона, мкР/ч
//#define DEFAULT_ERRORS  						  				9 //  0-200		автоматический сброс статистики при резком изменении фона, мин. уровень, имп/с, 0 - сброс запрещен
//#define DEFAULT_NdoseMax  					 				 10	//  0-999		максимальный уровень накопленной дозы, мкЗв (для сработки оповещения), 0 - не считать накопленную дозу
//#define DEFAULT_STUFF_TIME								 1800 //  60-10800|(1min - 3hours) время счёта удельной активности


//------------------------------------------------------------------------------
// служебные макросы

// версия прошивки
#define FW_VER_MAJOR 				2
#define FW_VER_MINOR				6

// АКБ
#define BAT_CHECK_PIN 				P2_2
#define BAT_RESPONSE_PIN 			P2_0

#define BAT_STATE_FULL 				0x00
#define BAT_STATE_HALF 				0x60
#define BAT_STATE_DEAD 				0x01

// потребители (0 = подать питание; 1 = обесточить)
// 	7	|	6	|	5	|	4	|	3	|	2	|	1	|	0	|
//  BT  | 	HV_POWER_SUPPLY		|		|		|	   LED		|

#define LOAD_HV_POWER_SUPPLY 		0x8F 	// 0b10001111
#define LOAD_LED								0x03	// 0b00000011
//#define LOAD_BT								0x80	// 0b10000000
#define LOAD_ALL_OFF 						0xFF	// 0b11111111

//LCD
#define TIC55_LOAD_PIN  			P2_7
#define TIC55_DIN_PIN   			P2_6
#define TIC55_DCLK_PIN  			P2_5

// не используются:   *             *
// DIG				  			1 2 3 4 5 6 7 8     <------
// SEG				  			8 8 8 8 8 8 8 8
// POINT			   			, , , , , , , ,
// ARROW			  			V V V V V V V V

#define TIC55_POINT_CLEAR 		0
#define TIC55_POINT_1 				2
#define TIC55_POINT_2 				4
#define TIC55_POINT_3 				8
#define TIC55_POINT_4 				16
#define TIC55_POINT_5 				32
#define TIC55_POINT_6 				64
#define TIC55_POINT_ALL				126

// Основной таймер
// 65 536 -  0.001 * 1 000 000 Hz = 64 536 = 0xFC18 
#define MAIN_CLOCK_VAL_H 			0xFC
#define MAIN_CLOCK_VAL_L 			0x18 

// Кнопки
#define BUTTON_TOP_PIN 				P1_0
#define BUTTON_MIDDLE_PIN 		P1_1
#define BUTTON_BOTTOM_PIN 		P1_2

// Этапы включения и выключения
#define MODE_SLEEP_NOW 				0
#define MODE_SLEEP_READY 			1
#define MODE_SHOW_VER 				2
#define MODE_MENU_NAV 				3

// SIGNAL SPEAKER
#define SIGNAL_SPEAKER_PIN 		P2_3
#define SIGNAL_NO_REPEAT			0
#define SIGNAL_PP							1 
#define SIGNAL_P_P						2
#define SIGNAL_PP_P						3
#define SIGNAL_P_P_P					4
#define SIGNAL_PP_P_P					5
#define SIGNAL_P_P_P_P 				6
#define SIGNAL_PP_P_P_P 			7

//------------------------------------------------------------------------------
// обнуляемые переменные

// железо
uint16_t 	RESET_COUNTER;
uint8_t 	RESET_TIMER;
uint8_t 	BATTERY_STATE;
bit 			SETTINGS_VAL_STEP;

// LCD
uint8_t 		TIC55_digit_buffer[6]; // Буфер индикатора для подготовки к отправке
uint8_t 		TIC55_arrows_on; 			 // установить нужный бит чтобы зажечь стрелку
uint8_t 		TIC55_arrows_blink; 	 // установить нужный бит, чтобы стрелка мигала
uint8_t 		TIC55_points; 				 // установить нужный бит чтобы зажечь точку
bit 				GAMMA_UNITS  = 0; 		 // 0 - мкЗв; 1 - мкР
bit 				BETA_UNITS   = 0;  		 // 0 - частиц в мин; 1 - частиц в сек
bit					SEARCH_UNITS = 1; 		 // 0 - BPS, 1 - счётчик
bit					STUFF_UNITS  = 0;			 // 0 - Бк/кг; 1 - кБк/кг


// Таймеры
volatile uint16_t 				ONE_US_TIMER;   // Основной таймер, мкс
volatile bit 							ONE_MS_TIMER;   // мс
volatile bit 							ONE_SEC_TIMER;  // сек

// Кнопки
volatile bit ANY_BUTTON_PRESSING;
uint16_t 		 BUTTON_TOP_HOLD;
uint16_t 		 BUTTON_MIDDLE_HOLD;
uint16_t 		 BUTTON_BOTTOM_HOLD;

// Этапы включения и выключения
uint8_t 		DRGB_MODE;
uint16_t 		MENU_NAV_POS; // в соответствии с таблицей навигации

// Оповещение о событиях с датчика
bit 						clicker_enabled; // запрещает оповещения с датчика в момент системных уведомлений (не менять!)
volatile bit 		BETA5_REGISTRED; // устанавливается 1 при регистрации события

// Оповещения
uint8_t 		SIGNAL_PLAYNG;
uint8_t 		SIGNAL_REPEAT;
uint8_t 		SIGNAL_LENGTH;
uint8_t 		measure_error_alert;
uint16_t		led_timer;

// математика
volatile uint16_t 	CPS_MEASURED; 		// текущее измерение имп/с
uint32_t 						TOTAL_MEASURED; 	// всего измерено импульсов за время
uint16_t 						MEASURE_TIME; 		// время экспозиции
uint8_t 						error_val; 				// ошибка результата измерения

float								GAMMA_CPS = 0.0f;	// фон для расчёта чистой беты							

//------------------------------------------------------------------------------
// переменные, состояние которых требуется сохранить при цикле сна
bit       CONFIG_LED_ENABLED    =    1;           // свектодиод активен/нет
bit 			CONFIG_SOUND_ENABLED	= 	 1;		 	 			// разрешает звуковые оповещения (все)
uint8_t 	CONFIG_CLICKER_LEN	 	= 	 3; 			 		// длительность оповещения о зарегистрированном событии, мс; 0 = без оповещения
uint8_t 	CONFIG_ALERT_LEVEL_1	= 	30; 		 			// оповещение о превышении фона 1 диапазон
uint8_t 	CONFIG_ALERT_LEVEL_2	= 	60; 		 			// оповещение о превышении фона 2 диапазон
uint8_t 	CONFIG_ALERT_LEVEL_3	=  120;		 				// оповещение о превышении фона 3 диапазон max 10000
uint8_t		CONFIG_ERRORS 				= 	 5; 			 		// уровень погрешности для оповещения
uint16_t 	CONFIG_NdoseMax 			= 	10; 		 			// накопленная доза - максимум мкЗв
uint8_t		CONFIG_LED_DURATION		= 	 5;           // таймаут светодиода после нажатия (сек)


uint32_t 		NdoseVal = 0; // накопленная доза, мкР
uint32_t 		NdoseSec = 0; // время накопления дозы

//------------------------------------------------------------------------------
// железо
	
// аппаратный сброс
void HW_RESET(){
	
	WDTRST = WDTRES1_;
	WDTRST = WDTRES2_;
}


// оповещение о загистрированном событии
void HW_SENSOR(void) interrupt IE1_VECTOR{
	
	BETA5_REGISTRED = 1;
}

// оповещение о нажатии кнопок
void HW_KEYS(void) interrupt IE0_VECTOR{

	ANY_BUTTON_PRESSING = 1;	
}

// основной таймер
void HW_TIMER(void) interrupt TF2_VECTOR{
	TF2 = 0; // согласно ДШ данный флаг снимается программно

	ONE_MS_TIMER = 1;
	
//------------------------------
	ONE_US_TIMER++;

	if(ONE_US_TIMER >= 1000){
		ONE_US_TIMER=0;
		
		CPS_MEASURED = (TL0 | (TH0 << 8));

		TH0 = 0;	
		TL0 = 0;
		
		ONE_SEC_TIMER = 1;
	} // 1 sec
}

// led
void HW_led(bit state){
	if (state)
	{
		P0 &= ~LOAD_LED;
	}
	else
	{
		P0 |= LOAD_LED;
	}
}

// АКБ
uint8_t HW_battery_getState(){
	uint8_t result, check;
	result = BAT_STATE_FULL;

	if (!BAT_RESPONSE_PIN)
	{
		BAT_CHECK_PIN = 1;
		result = BAT_STATE_DEAD;
		for (check = 0; check < 255; check++){} 

		if (BAT_RESPONSE_PIN) 
		{
			result = BAT_STATE_HALF;
		}

	}
	BAT_CHECK_PIN = 0;
	return result;
}

// speaker
void HW_speaker(bit _mode){
	
	SIGNAL_SPEAKER_PIN = !(_mode && CONFIG_SOUND_ENABLED);
}

void HW_signal(uint8_t sigONE_US_TIMER, uint8_t sig_repeat){
	clicker_enabled = 0;
	SIGNAL_PLAYNG = sigONE_US_TIMER;
	SIGNAL_LENGTH = sigONE_US_TIMER;
	SIGNAL_REPEAT = sig_repeat;
	HW_speaker(1);
}



// LCD
void TIC55_send(){
uint8_t cur_dig, cur_seg;
	
	for(cur_dig=9;cur_dig>0;cur_dig--)
	{
		for(cur_seg=0; cur_seg<9; cur_seg++)
		{
			TIC55_DIN_PIN = 0;
			if (cur_dig >=2 && cur_dig <= 7)
			{
				switch (cur_seg)
				{
					case 0: 
					{
						TIC55_DIN_PIN = (1 & (TIC55_points >> cur_dig-1));
					} break;
					case 1:
					{
						TIC55_DIN_PIN = (1 & (TIC55_arrows_on >> cur_dig-1)) ;
					} break;
					default:
					{
						TIC55_DIN_PIN = (1 & (symbol[TIC55_digit_buffer[cur_dig-2]] >> cur_seg-2));
					}
				}
			}
			
			TIC55_DCLK_PIN = 1;
			TIC55_DCLK_PIN = 0;
		}
	}

	TIC55_LOAD_PIN = 1;
	TIC55_LOAD_PIN = 0;	
}

void TIC55_fullClear(){
	uint8_t i;
	for (i = 0; i <= 5; i++)
	{
		TIC55_digit_buffer[i] = ___;
	}
	TIC55_arrows_on = 0; 
	TIC55_arrows_blink = 0; 
	
	TIC55_points = TIC55_POINT_CLEAR;
	
//	TIC55_send();
}

void TIC55_fillBuffer(uint8_t d0,uint8_t d1,uint8_t d2,uint8_t d3,uint8_t d4,uint8_t d5){

	TIC55_digit_buffer[0] = d0;
	TIC55_digit_buffer[1] = d1;
	TIC55_digit_buffer[2] = d2;
	TIC55_digit_buffer[3] = d3;
	TIC55_digit_buffer[4] = d4;
	TIC55_digit_buffer[5] = d5;
}

void TIC55_printText(uint16_t TextNum){

	TIC55_points = TIC55_POINT_CLEAR;
	
	switch(TextNum)
	{
		case _VER_TEXT:
		{
			TIC55_fillBuffer(__A,__L,__t,___,FW_VER_MAJOR,FW_VER_MINOR); 
			TIC55_points = TIC55_POINT_5;
		} break;			

		case _EMPTY_TEXT:
		{
			TIC55_fillBuffer(___,___,___,___,___,___); 
		} break;
					
		case _SLEEP_TEXT:
		{
			TIC55_fillBuffer(__S,__L,__E,__E,__P,___);
		} break;

		case MENU_ITEM_BETA:
		{
			TIC55_fillBuffer(__b,__E,__t,__A,___,___);
		} break;	
		
		case MENU_ITEM_GAMMA:
		{
			TIC55_fillBuffer(__G,__minus,__F,__o,__n,___);
		} break;	
		
		case MENU_ITEM_SEARCH:
		{
			TIC55_fillBuffer(__C,__P,__S,___,___,___);
		} break;

		case MENU_ITEM_STUFF:
		{
			TIC55_fillBuffer(__S,__t,__u,__F,__F,___);
		} break;		
		
		case MENU_ITEM_CONFIG:
		{
			TIC55_fillBuffer(__C,__o,__n,__F,__I,__G);
		} break;	
		
		case _RESET_TEXT:
		{
			TIC55_fillBuffer(__r,__E,__S,__E,__t,RESET_TIMER);
		} break;
				
		case MENU_ITEM_ES:
		{
			TIC55_fillBuffer(__S,__E,__n,__S,__o,__r);
		} break;	
				
		case MENU_ITEM_AS1:
		case MENU_ITEM_AS2:		
		case MENU_ITEM_AS3:			
		{
			TIC55_fillBuffer(__A,__L,__r,__t,__minus,MENU_NAV_POS-MENU_ITEM_AS1+1);
		} break;	
		
		case MENU_ITEM_MAX_DOSE:
		{
			TIC55_fillBuffer(__A,__L,__r,__t,__minus,__d);
		} break;	
				
		case MENU_ITEM_DOSE:
		{
			TIC55_fillBuffer(__d,__O,__S,__E,___,___);
		} break;

		case MENU_ITEM_ERRORS:
		{
			TIC55_fillBuffer(__A,__L,__r,__t,__minus,__E);
		} break;
		
		case MENU_ITEM_LED:
		{
			TIC55_fillBuffer(__L,__E,__d,__minus,__t,___);
		} break;
				
	}
	TIC55_send();
}

void TIC55_setArrow(uint8_t pos, bit blink){

	TIC55_arrows_on |= (1 << pos);
	if (blink)
	{
		TIC55_arrows_blink |= (1 << pos);
	}
	else
	{
		TIC55_arrows_blink &= ~(1 << pos);
	}
}

void TIC55_clearArrow(uint8_t pos){

	TIC55_arrows_on &= ~(1 << pos);
	TIC55_arrows_blink &= ~(1 << pos);
}

void TIC55_clearArrows(){

	TIC55_arrows_on = 0;
	TIC55_arrows_blink = 0;
}

void TIC55_printNum(uint32_t val, bit flt){
	TIC55_printText(_EMPTY_TEXT);
	TIC55_points = flt ? TIC55_POINT_4 : TIC55_POINT_CLEAR;
	if (val > 999999) val = 999999;
	
	if (val >= 100000) TIC55_digit_buffer[0] = (val/100000)%10;
	if (val >= 10000)  TIC55_digit_buffer[1] = (val/10000)%10;
	if (val >= 1000) 	 TIC55_digit_buffer[2] = (val/1000)%10;
	if (val >= 100) 	 TIC55_digit_buffer[3] = (val/100)%10;
	if (val >= 10)		 TIC55_digit_buffer[4] = (val/10)%10;
										 TIC55_digit_buffer[5] =  val%10;
	
	if (flt) {															// вещественное число, не менее 3 знаков - 0.00
	 	TIC55_digit_buffer[3] = (val/100)%10;
		TIC55_digit_buffer[4] = (val/10)%10;
	}

	TIC55_send();
}

void TIC55_printTime(uint16_t val){ // 11.59.59
	uint8_t h, m, s;
	h =  val/3600;
	m = (val%3600)/60;
	s =	 val%60;
	TIC55_digit_buffer[0] = (h >= 10) ? (h/10)%10 : ___;
	TIC55_digit_buffer[1] =  h%10;
	TIC55_digit_buffer[2] = (m/10)%10;
	TIC55_digit_buffer[3] =  m%10;
	TIC55_digit_buffer[4] = (s/10)%10;
	TIC55_digit_buffer[5] =  s%10;

	TIC55_points = (TIC55_POINT_2 | TIC55_POINT_4);
	
	TIC55_send();
}

void TIC55_printVal(uint16_t val){

	TIC55_digit_buffer[0] = 1;
	TIC55_digit_buffer[1] = SETTINGS_VAL_STEP ? 0 : ___;
	TIC55_digit_buffer[2] = ___;
	
	if (val == 0)	{
		TIC55_digit_buffer[3] = __O;
		TIC55_digit_buffer[4] = __F;
		TIC55_digit_buffer[5] = __F;	
	}	else {			
		TIC55_digit_buffer[3] = (val >= 100) ? (val/100)%10 : ___;
		TIC55_digit_buffer[4] = (val >= 10)  ? (val/10)%10  : ___;
		TIC55_digit_buffer[5] = (val%10);				
	}
	TIC55_send();
}

	
void TIC55_printResult(float val, uint8_t err){
	uint16_t result_temp;
	if (val < 999) {
		if 				(val < 10)	{
			result_temp = (val*1000);
			TIC55_points = TIC55_POINT_1;
		}	else if (val < 100) {
			result_temp = (val*100);
			TIC55_points = TIC55_POINT_2;
		}	else {
			result_temp = (val*10);
			TIC55_points = (err < 10) ? TIC55_POINT_3 : TIC55_POINT_CLEAR;
		}	
		TIC55_digit_buffer[0] = (result_temp/1000)%10;
		TIC55_digit_buffer[1] = (result_temp/100)%10;
		TIC55_digit_buffer[2] = (result_temp/10)%10;
		if (err < 10) {
			TIC55_digit_buffer[3] = (result_temp%10);
			TIC55_digit_buffer[4] = (measure_error_alert > 0) ? __E : ___;
			TIC55_digit_buffer[5] = (err%10);
		}	else {
			TIC55_digit_buffer[3] = (measure_error_alert > 0) ? __E : ___;
			TIC55_digit_buffer[4] = (err/10)%10;	
			TIC55_digit_buffer[5] = (err%10);	
		}		
		TIC55_send();
	}	else {
		TIC55_points = TIC55_POINT_CLEAR;
		TIC55_printNum(val, 0);
	}
}

// математика

uint16_t math_isqrt(uint16_t x){
uint16_t m, y, b; 
   m = 0x4000;
   y = 0;
   while (m != 0){
      b = y | m;
      y = y >> 1;
      if (x >= b) {
        x = x - b;
        y = y | m;
      }
      m = m >> 2;
   }
   return y;
}	

void measure_alert() {
	if (measure_error_alert == 0) return;
	if (measure_error_alert == 1)	{
		measure_error_alert = 3;
		HW_signal(100, SIGNAL_PP_P_P);
	} else {
		measure_error_alert--;
	}
}

void math_calc_error(){	
	if 				(TOTAL_MEASURED == 0) {
		error_val = 99;
	} else if (TOTAL_MEASURED > 40000) {  					// < 1%
		error_val = 1;
	} else {
		error_val = 200 / math_isqrt(TOTAL_MEASURED); // +-2 sigma
		if (error_val > 99) error_val = 99;
	}
	
	if ((error_val <= CONFIG_ERRORS) && (measure_error_alert != 0)) measure_alert();

}


void STAT_DOSE_RESET(){
	NdoseVal = NdoseSec = 0;
}


void STAT_RESET(){
	TOTAL_MEASURED = 0;
	MEASURE_TIME = 0;
	CPS_MEASURED = 0;
	ONE_US_TIMER = 0;
	TH0 = 0;
	TL0 = 0;
}


void STAT_ADD(){
	
	MEASURE_TIME++;
	TOTAL_MEASURED += CPS_MEASURED;
	
	if ((MEASURE_TIME > 5) && (TOTAL_MEASURED == 0)) {// если не поступают сигналы с датчика 
		STAT_RESET();
		TIC55_arrows_on 	 = TIC55_POINT_ALL;
		TIC55_arrows_blink = TIC55_POINT_ALL;
		HW_signal(300, SIGNAL_NO_REPEAT);
	}
	
	if (TOTAL_MEASURED > 0xFFFF0000) STAT_RESET(); // если скоро будет переполнение

}

// подпрограммы

void PROG_BETA(){
	float _beta;

// ЧАСТ/(СЕК * СМ2) = ((число_импульсов / время_экспозиции) * эффективность) / 1000
// ЧАСТ/(МИН * СМ2) = ЧАСТ/(СЕК * СМ2) * 60

	STAT_ADD();

	if (TOTAL_MEASURED > 0)
	{
	
		_beta = (float)TOTAL_MEASURED / (float)MEASURE_TIME;			   // считаем CPS
		if (GAMMA_CPS == 0) {
			_beta = (_beta > SELF_CPS) ? (_beta - SELF_CPS) : 0;       // если нет статистики по гамме - вычитаем шум датчика
		} else {
			_beta = (_beta > GAMMA_CPS) ? (_beta - GAMMA_CPS) : 0;     // иначе вычитаем гамму
		}
		_beta *= CALIB_BETA;                                         // CPS -> 1/см.мин.

		if (BETA_UNITS) _beta /= 60.0f;														   // 1/см.мин. -> 1/см.сек.
		
		math_calc_error();
		TIC55_printResult(_beta, error_val);
	}	else 	{
		TIC55_printResult(0, 99);
	}

}

void GAMMA_ALERT(uint16_t level){
	if 				(level >= CONFIG_ALERT_LEVEL_3) {
		HW_signal(100, SIGNAL_P_P_P_P);
	}	else if (level >= CONFIG_ALERT_LEVEL_2) {
		HW_signal(100, SIGNAL_P_P_P);		
	}	else if (level >= CONFIG_ALERT_LEVEL_1)	{
		HW_signal(100, SIGNAL_P_P);
	}
}

void PROG_GAMMA(){
	float _gamma;
	static int32_t _dose_cash; // промежуточная накопленная доза (пР)
	#define mln 1000000
	
	if (CONFIG_NdoseMax != 0) {
		_dose_cash += (CPS_MEASURED*mln - SELF_CPS*mln) / CALIB_GAMMA;  //  пР/c
			
		if (_dose_cash >= mln) {
			NdoseVal += _dose_cash/mln;
			if (NdoseVal > 999999) NdoseVal = 999999; // 9999.99 - возможности дисплея
			_dose_cash %= mln;
		}
		if (NdoseSec < 356399) NdoseSec++;  				// 99:59:59 - возможности дисплея
		
		if ((NdoseVal >= (CONFIG_NdoseMax*100)) || (NdoseSec >= 86400)) {   // 86400 = 24:00:00 - суточное оповещение
			HW_signal(250, SIGNAL_P_P);
			TIC55_setArrow(5, NdoseSec >= 86400);
		}
	}

// f(x) = чувствительность (300) + (время экспозиции * 100 / число импульсов)
// мкР/с =  (число_импульсов / время_экспозиции) / чувствительность_f(x)
// мкР/ч =  мкР/с * 3600
// мкЗв/ч = мкР/ч / 100
	STAT_ADD();

	if (TOTAL_MEASURED > 0)
	{
		GAMMA_CPS  = (float)TOTAL_MEASURED/(float)MEASURE_TIME;     	 // CPS
		_gamma = (GAMMA_CPS > SELF_CPS) ? GAMMA_CPS - SELF_CPS : 0;    // вычитаем шум датчика
		_gamma /= CALIB_GAMMA; 				 																 // мкР/с
				
		_gamma *= 3600.0f;	// мкР/ч
		
		GAMMA_ALERT( (_gamma > 250) ? 250 : _gamma );
		
		if (!GAMMA_UNITS) _gamma /= 100.0f;
		math_calc_error();
		TIC55_printResult(_gamma, error_val);

	}	else {
		TIC55_printResult(0, 99);
	}
		
}

void PROG_SEARCH() {
	STAT_ADD(); 
	if (SEARCH_UNITS) {
//		HW_signal(1, SIGNAL_NO_REPEAT);
		TIC55_printNum(CPS_MEASURED, 0);
	} else {
		if (TOTAL_MEASURED > 0) {
			math_calc_error();
			TIC55_printResult( (float)TOTAL_MEASURED / (float)MEASURE_TIME, error_val );
		} else {
			TIC55_printResult(0, 99);
		}
	}
}

void PROG_STUFF() {
	float _stuff;

	STAT_ADD();

	if (TOTAL_MEASURED > 0) {
		_stuff = (float)TOTAL_MEASURED / (float)MEASURE_TIME;			   // считаем CPS
		if (GAMMA_CPS == 0) {
			_stuff = (_stuff > SELF_CPS) ? (_stuff - SELF_CPS) : 0;    // если нет статистики по гамме - вычитаем шум датчика
		} else {
			_stuff = (_stuff > GAMMA_CPS) ? (_stuff - GAMMA_CPS) : 0;  // иначе вычитаем гамму
		}
		_stuff *= CALIB_STUFF;                                       // CPS -> Бк/кг.
		if (STUFF_UNITS) _stuff /= 1000.0f;													 // Бк/кг -> кБк/кг
		
		math_calc_error();
		TIC55_printResult(_stuff, error_val);
	}	else 	{
		TIC55_printResult(0, 99);
	}

}
			

void PROG_ES(){
	TIC55_printVal(CONFIG_CLICKER_LEN);
}

void PROG_AS1(){
	TIC55_printVal(CONFIG_ALERT_LEVEL_1);
}

void PROG_AS2(){
	TIC55_printVal(CONFIG_ALERT_LEVEL_2);
}

void PROG_AS3(){
	TIC55_printVal(CONFIG_ALERT_LEVEL_3);
}

void PROG_DOSE(){
	if (!SETTINGS_VAL_STEP)	{
		TIC55_printNum(NdoseVal, 1);
	}	else {
		TIC55_printTime(NdoseSec);
	}
}

void PROG_MAXDOSE(){
	TIC55_setArrow(5, 0);
	TIC55_printVal(CONFIG_NdoseMax);
}

void PROG_ERRORS(){
	TIC55_printVal(CONFIG_ERRORS);
}

void PROG_LED(){
	TIC55_printVal(CONFIG_LED_DURATION);
}


// меню прибора

void menu_error_signal(){	
	HW_signal(50, SIGNAL_P_P);
}

void menu_navigation(uint8_t nav) {
	switch (nav) {
		
		case MENU_NAV_EXIT:											//	= E = X = I = T =
			TIC55_clearArrows();
			if ( MENU_NAV_POS >= MENU_PROG_SEARCH ) {
				MENU_NAV_POS /= 10;
			} else {
				MENU_NAV_POS = MENU_PROG_GAMMA;
				TIC55_setArrow(3, GAMMA_UNITS);
				STAT_RESET();
			}
		break;
		
		case MENU_NAV_ENTR:	{										//	= E = N = T = E = R =				
			switch (MENU_NAV_POS) {
				case MENU_ITEM_BETA:
					TIC55_setArrow(2, BETA_UNITS);
					if (GAMMA_CPS == 0) TIC55_setArrow(3, 0);
					STAT_RESET();
				break;
				case MENU_ITEM_GAMMA:
					TIC55_setArrow(3, GAMMA_UNITS);
					STAT_RESET();
				break;
				case MENU_ITEM_SEARCH:
					TIC55_setArrow(1, SEARCH_UNITS);
					STAT_RESET();
				break;
				case MENU_ITEM_STUFF:
					TIC55_setArrow(4, STUFF_UNITS);
					if (GAMMA_CPS == 0) TIC55_setArrow(3, 0);
					STAT_RESET();
				break;
				case MENU_ITEM_DOSE:
					TIC55_setArrow(5, 0);
					SETTINGS_VAL_STEP = 0;
				break;
				case MENU_ITEM_AS1:
        case MENU_ITEM_AS2:
        case MENU_ITEM_AS3:
					TIC55_setArrow(3, 1);
				break;
				case MENU_PROG_GAMMA:
					GAMMA_UNITS = !GAMMA_UNITS;
					TIC55_setArrow(3, GAMMA_UNITS);
				break;
				case MENU_PROG_BETA:
					BETA_UNITS = !BETA_UNITS;
					TIC55_setArrow(2, BETA_UNITS);
				break;
				case MENU_PROG_DOSE:
					SETTINGS_VAL_STEP =! SETTINGS_VAL_STEP;
					TIC55_setArrow(5, SETTINGS_VAL_STEP);
					TIC55_points = SETTINGS_VAL_STEP ? TIC55_POINT_CLEAR : TIC55_POINT_4;
				break;
				case MENU_PROG_SEARCH:
					SEARCH_UNITS = !SEARCH_UNITS;
					TIC55_setArrow(1, SEARCH_UNITS);					
				break;
				case MENU_PROG_STUFF:
					STUFF_UNITS = !STUFF_UNITS;
					TIC55_setArrow(4, STUFF_UNITS);
				break;
			}
			if ( (MENU_NAV_POS >= MENU_PROG_ES 		) &&  (MENU_NAV_POS <= MENU_PROG_LED) )	SETTINGS_VAL_STEP = !SETTINGS_VAL_STEP;
			if ( (MENU_NAV_POS <= MENU_ITEM_CONFIG)																			)	MENU_NAV_POS *= 10;
			else if ( (MENU_NAV_POS >= MENU_ITEM_ES) && (MENU_NAV_POS <= MENU_ITEM_LED) )	MENU_NAV_POS *= 10, SETTINGS_VAL_STEP = 0;
		} break;
				
		case MENU_NAV_UP:	{										// = L I S T   U P =

			switch (MENU_NAV_POS) {
				
				case MENU_PROG_STUFF:
				case MENU_PROG_BETA:
				case MENU_PROG_GAMMA:
				case MENU_PROG_SEARCH:
					measure_error_alert = (measure_error_alert == 0) ? 1 : 0;
				break;
			
				case MENU_PROG_ES:
					CONFIG_CLICKER_LEN += SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_CLICKER_LEN > 30) {
						menu_error_signal();
						CONFIG_CLICKER_LEN = 30;
					}
				break;

				case MENU_PROG_ERRORS:
					CONFIG_ERRORS += SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_ERRORS > 30) {
						menu_error_signal();
						CONFIG_ERRORS = 30;
					}
				break;
						
				case MENU_PROG_AS1:
					CONFIG_ALERT_LEVEL_1 += SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_ALERT_LEVEL_1 >= CONFIG_ALERT_LEVEL_2) {
						menu_error_signal();
						CONFIG_ALERT_LEVEL_1 = CONFIG_ALERT_LEVEL_2 - 1;
					}
				break;		
				
				case MENU_PROG_AS2:
					CONFIG_ALERT_LEVEL_2 += SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_ALERT_LEVEL_2 >= CONFIG_ALERT_LEVEL_3) {
						menu_error_signal();
						CONFIG_ALERT_LEVEL_2 = CONFIG_ALERT_LEVEL_3 - 1;
					}
				break;		
				
				case MENU_PROG_AS3:
					CONFIG_ALERT_LEVEL_3 += SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_ALERT_LEVEL_3 > 240) {
						menu_error_signal();
						CONFIG_ALERT_LEVEL_3 = 240;
					}
				break;
					
				case MENU_PROG_MAX_DOSE:
					CONFIG_NdoseMax += SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_NdoseMax > 999) {
						menu_error_signal();
						CONFIG_NdoseMax = 999;
					}
				break;
					
				case MENU_PROG_LED:
					CONFIG_LED_DURATION += SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_LED_DURATION > 60) {
						menu_error_signal();
						CONFIG_LED_DURATION = 60;
					}
					led_timer = CONFIG_LED_ENABLED ? CONFIG_LED_DURATION*1000 : 0;
					if (CONFIG_LED_ENABLED && (CONFIG_LED_DURATION != 0)) HW_led(1);
				break;
			}
					
			if 			( (MENU_NAV_POS >  MENU_ITEM_SEARCH) && (MENU_NAV_POS <= MENU_ITEM_CONFIG) ) MENU_NAV_POS--;
			else if ( (MENU_NAV_POS == MENU_ITEM_SEARCH) 																			 ) MENU_NAV_POS = MENU_ITEM_CONFIG;
			
			if 			( (MENU_NAV_POS >  MENU_ITEM_ES) && (MENU_NAV_POS <= MENU_ITEM_LED) ) MENU_NAV_POS--;
			else if ( (MENU_NAV_POS == MENU_ITEM_ES) 																		) MENU_NAV_POS = MENU_ITEM_LED;
			
	
		} break;
		
		case MENU_NAV_DOWN: {						//	= L I S T   D O W N =
			
			switch (MENU_NAV_POS) {
						
				case MENU_PROG_BETA: // кнопка "назад" в меню беты и гаммы сбрасывает статистику
				case MENU_PROG_GAMMA:
				case MENU_PROG_SEARCH:
				case MENU_PROG_STUFF:
					STAT_RESET();
				break;
				
				case MENU_PROG_DOSE:
					STAT_DOSE_RESET();
				break;
								
				case MENU_PROG_ES:
					CONFIG_CLICKER_LEN -= SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_CLICKER_LEN > 30) {
						menu_error_signal();
						CONFIG_CLICKER_LEN = 0;
					}
				break;	

				case MENU_PROG_ERRORS:
					CONFIG_ERRORS -= SETTINGS_VAL_STEP ? 10 : 1;
					if ( (CONFIG_ERRORS > 200) || (CONFIG_ERRORS < 1) ) {
						menu_error_signal();
						CONFIG_ERRORS = 1;
					}
				break;
				
				case MENU_PROG_AS1:
					CONFIG_ALERT_LEVEL_1 -= SETTINGS_VAL_STEP ? 10 : 1;
					if (CONFIG_ALERT_LEVEL_1 < 20) {
						menu_error_signal();
						CONFIG_ALERT_LEVEL_1 = 20;
					}
				break;		
				
				case MENU_PROG_AS2:
					CONFIG_ALERT_LEVEL_2 -= SETTINGS_VAL_STEP ? 10 : 1;	
					if (CONFIG_ALERT_LEVEL_2 <= CONFIG_ALERT_LEVEL_1) {
						menu_error_signal();
						CONFIG_ALERT_LEVEL_2 = CONFIG_ALERT_LEVEL_1 + 1;
					}
				break;		
				
				case MENU_PROG_AS3:
					CONFIG_ALERT_LEVEL_3 -= SETTINGS_VAL_STEP ? 10 : 1;
					if ( CONFIG_ALERT_LEVEL_3 <= CONFIG_ALERT_LEVEL_2 ) {
						menu_error_signal();
						CONFIG_ALERT_LEVEL_3 = CONFIG_ALERT_LEVEL_2 + 1;
					}
				break;	

				case MENU_PROG_MAX_DOSE:
					CONFIG_NdoseMax -= SETTINGS_VAL_STEP ?   10 : 1;
					if (CONFIG_NdoseMax > 999) {
						menu_error_signal();
						CONFIG_NdoseMax = 0;
					}
				break;
					
				case MENU_PROG_LED:
					CONFIG_LED_DURATION -= SETTINGS_VAL_STEP ?   10 : 1;
					if (CONFIG_LED_DURATION > 60) {
						menu_error_signal();
						CONFIG_LED_DURATION = 0;
					}
					led_timer = CONFIG_LED_ENABLED ? CONFIG_LED_DURATION*1000 : 0;
				break;
			}

			if 			( MENU_NAV_POS < 	MENU_ITEM_CONFIG )	MENU_NAV_POS++;
			else if ( MENU_NAV_POS == MENU_ITEM_CONFIG )	MENU_NAV_POS = MENU_ITEM_SEARCH;
				
			if 			( (MENU_NAV_POS >= MENU_ITEM_ES) && (MENU_NAV_POS <  MENU_ITEM_LED) ) MENU_NAV_POS++;
			else if ( 															 	 	(MENU_NAV_POS == MENU_ITEM_LED) ) MENU_NAV_POS = MENU_ITEM_ES;
		} break;		

		
		case MENU_PROG_CHECK: {
			switch (MENU_NAV_POS)	{
				case MENU_PROG_SEARCH: PROG_SEARCH();  break;		
				case MENU_PROG_BETA:   PROG_BETA();    break;
				case MENU_PROG_GAMMA:  PROG_GAMMA();   break;
				case MENU_PROG_STUFF:  PROG_STUFF(); 	 break;
			}
		} break;
	}

// -----------------------------------------------------------------------------
// отображение после изменения позиции в меню
	if ( (MENU_NAV_POS >= MENU_ITEM_SEARCH) && (MENU_NAV_POS <= MENU_ITEM_CONFIG) ) TIC55_printText(MENU_NAV_POS);
	if ( (MENU_NAV_POS >= MENU_ITEM_ES		) && (MENU_NAV_POS <= MENU_ITEM_LED)    ) TIC55_printText(MENU_NAV_POS);
	
	switch (MENU_NAV_POS) {		
		case MENU_PROG_DOSE:  		 PROG_DOSE(); 			break;	
		case MENU_PROG_ES:    		 PROG_ES(); 				break;
		case MENU_PROG_ERRORS: 		 PROG_ERRORS(); 		break;
		case MENU_PROG_AS1:   		 PROG_AS1(); 				break;
		case MENU_PROG_AS2:   		 PROG_AS2();     		break;
		case MENU_PROG_AS3:   		 PROG_AS3();     		break;		
		case MENU_PROG_MAX_DOSE:	 PROG_MAXDOSE();  	break;
		case MENU_PROG_LED:				 PROG_LED();				break;
	}
	
}


// инициализация
void setup(){
	
//------------------------------------------------------------------------------	
// ВВ преобразователь потребляет много энергии в момент запуска, поэтому МК и дисплей ведут себя неадекватно.
	P0 = LOAD_HV_POWER_SUPPLY; // Запуск ВВ преобразователя
	for(ONE_US_TIMER = 0; ONE_US_TIMER < 5000; ONE_US_TIMER ++){}; // ожидание выхода ВВ преобразователя в рабочий режим ms/10

//------------------------------------------------------------------------------	
// обнуление глобальных переменных после выхода из сна и при первом включении
	if (CONFIG_LED_ENABLED) HW_led(1); led_timer = CONFIG_LED_DURATION * 1000;
	DRGB_MODE = MODE_SHOW_VER;
		
	ANY_BUTTON_PRESSING = 0;
	BUTTON_TOP_HOLD = 0;
	BUTTON_MIDDLE_HOLD = 0;
	BUTTON_BOTTOM_HOLD = 0;
	MENU_NAV_POS = MENU_ITEM_GAMMA;	
		

	ONE_MS_TIMER = 0;
	ONE_SEC_TIMER = 0;
	
	SETTINGS_VAL_STEP = 0;
		
	clicker_enabled = 1;
	SIGNAL_PLAYNG 	= 0;
	SIGNAL_REPEAT 	= 0;
	SIGNAL_LENGTH 	= 0;

	STAT_RESET();
	GAMMA_CPS = 0;
	//GAMMA_UNITS = 0;
	//BETA_UNITS = 0;
	//SEARCH_MODE = 1;
	//STUFF_UNITS = 0;

//------------------------------------------------------------------------------	
// порт 2
	P2 = 0x7F;

	TIC55_LOAD_PIN = 0;
	TIC55_DIN_PIN  = 0;
	TIC55_DCLK_PIN = 0;
	
//------------------------------------------------------------------------------	
// прерывания
// кнопки
	IT0 = 1;   // Configure interrupt 0 for falling edge on /INT0 (P3.2)
	EX0 = 1;   // Enable EX0 Interrupt
	
// датчик
	IT1 = 1;   // Configure interrupt 1 for falling edge on /INT1 (P3.3)
	EX1 = 1;   // Enable EX1 Interrupt

//------------------------------------------------------------------------------
// таймер 0 (подсчет зарегистрированных импульсов)
	TMOD |= (T0_CT_ | T0_M0_); // Т0 в режиме счетчика на 16 бит
	
// таймер 2 - системный таймер

	TH2 = MAIN_CLOCK_VAL_H;
	TL2 = MAIN_CLOCK_VAL_L;
	
	RCAP2H = MAIN_CLOCK_VAL_H;
	RCAP2L = MAIN_CLOCK_VAL_L;	
	
	ET2 = 1;
	
//------------------------------------------------------------------------------
// настройка завершена
	EA = 1;   
	TR0 = 1;
	TR2 = 1;
}


// СОН
void sleep_prepare(){
	EX1 = 0;
	ONE_US_TIMER = 900;
	DRGB_MODE = MODE_SLEEP_READY;
	TIC55_printText(_SLEEP_TEXT);
	HW_speaker(1);
}

void sleep_now(){
	TIC55_printText(_EMPTY_TEXT);
	ET2 = 0;
	
	TR0 = 0;
	TR2 = 0;
	
	P2 = 0xFF;
	P0 = LOAD_ALL_OFF;

	DRGB_MODE = MODE_SLEEP_NOW;
}

void sleep_wakeup(){
	setup();
//	TIC55_arrows_on = 32;
	TIC55_printText(_VER_TEXT);
}


// основной цикл
void main(void) {

	STAT_DOSE_RESET();
	setup();
	
	TIC55_fullClear();
	TIC55_printText(_VER_TEXT);
	
	while(1)
	{
	
		if (ONE_MS_TIMER)
		{
			ONE_MS_TIMER = 0;
			 
			if (led_timer == 0) 			// гасим диод
				HW_led(0);
			else
				led_timer--;
		
//			if (led_mode < 2) HW_led(0);
		//------------------------------
		// длительность оповещений	
			if (SIGNAL_PLAYNG == 1) {
				
				if (SIGNAL_REPEAT != 0) {
					SIGNAL_REPEAT--;
					SIGNAL_PLAYNG = SIGNAL_LENGTH;
					HW_speaker(!(SIGNAL_REPEAT & 1)); // младший бит
				}	else {
					if ((CONFIG_CLICKER_LEN != 0) && !clicker_enabled) clicker_enabled = 1;
					SIGNAL_PLAYNG = 0;			
					HW_speaker(0);
				}
			}
			if (SIGNAL_PLAYNG != 0)	SIGNAL_PLAYNG--;
		//------------------------------
		// обработка кнопок
			if ((ANY_BUTTON_PRESSING) && (DRGB_MODE != MODE_SHOW_VER)) {
				if (CONFIG_LED_ENABLED && (CONFIG_LED_DURATION != 0)) HW_led(1), led_timer = CONFIG_LED_DURATION * 1000;
				
				// сброс если нажаты две нижние кнопки
				if ((BUTTON_MIDDLE_PIN == 0) && (BUTTON_BOTTOM_PIN == 0))
				{				
					RESET_TIMER = 6;
					while ((BUTTON_MIDDLE_PIN == 0) && (BUTTON_BOTTOM_PIN == 0))
					{
						if (RESET_TIMER != 0)
						{
							
							RESET_COUNTER = 50900;

							while (RESET_COUNTER > 0)
							{
								RESET_COUNTER--;
								if (RESET_COUNTER < 2500)
								{
									if (RESET_TIMER <= 3)
									{
										SIGNAL_SPEAKER_PIN = 0;
										HW_led(1);
									}
								}
							}
							RESET_TIMER--;
							TIC55_printText(_RESET_TEXT);
							SIGNAL_SPEAKER_PIN = 1;
							HW_led(0);
						}
						else
						{
							SIGNAL_SPEAKER_PIN = 0;
							HW_led(1);
							HW_RESET();
						}
					}
					
					// если не сброс, то управление подсветкой
					//led_mode = (led_mode < 2)	? (led_mode + 1) : 0;
					// если не сброс, то сброс статитстики по гамме
					HW_signal(100, SIGNAL_NO_REPEAT);
					GAMMA_CPS = 0;
					
				} // вверх + вниз
						
						
				
				if(BUTTON_MIDDLE_PIN == 0){
					BUTTON_MIDDLE_HOLD++;
					if (BUTTON_MIDDLE_HOLD > 0x767) // зажата средняя кнопка
					{
						HW_signal(100, SIGNAL_NO_REPEAT);
						ANY_BUTTON_PRESSING = BUTTON_MIDDLE_HOLD = 0;
						HW_led(CONFIG_LED_ENABLED = !CONFIG_LED_ENABLED);
						led_timer = CONFIG_LED_ENABLED ? CONFIG_LED_DURATION*1000 : 0;

					}
				}
				else if(BUTTON_BOTTOM_PIN == 0){ 
					BUTTON_BOTTOM_HOLD++;
					if (BUTTON_BOTTOM_HOLD > 0x255) // удержана нижняя кнопка
					{
						HW_signal(100, SIGNAL_NO_REPEAT);
						ANY_BUTTON_PRESSING = BUTTON_BOTTOM_HOLD = 0;
						menu_navigation(MENU_NAV_EXIT);
					}
				}
				else if (BUTTON_TOP_PIN == 0){
					BUTTON_TOP_HOLD++;
					if (BUTTON_TOP_HOLD > 767) // удержана верхняя кнопка
					{
						TIC55_clearArrows();
						sleep_prepare();
					}
				}
				else 
				{
					
					if (BUTTON_MIDDLE_HOLD > 255) {  // удержана средняя кнопка
						CONFIG_SOUND_ENABLED = !CONFIG_SOUND_ENABLED;
						HW_signal(100, SIGNAL_NO_REPEAT);						
					} else if (BUTTON_MIDDLE_HOLD > 31) { // нажата средняя кнопка
						HW_signal(100, SIGNAL_NO_REPEAT);
						menu_navigation(MENU_NAV_UP);						
					}
					
					if (BUTTON_BOTTOM_HOLD > 31) 
					{ // нажата нижняя кнопка
						HW_signal(10, SIGNAL_NO_REPEAT);
						//---
						menu_navigation(MENU_NAV_DOWN);
						
					}
					
					if (BUTTON_TOP_HOLD > 31)	{ // нажата верхняя кнопка
						if ((BUTTON_BOTTOM_HOLD == 0) && (BUTTON_MIDDLE_HOLD == 0))	{
							BUTTON_TOP_HOLD = 0;
							if (DRGB_MODE == MODE_MENU_NAV) {
								HW_signal(100, SIGNAL_NO_REPEAT);
								menu_navigation(MENU_NAV_ENTR);
							}
						}	
					}			
					
					ANY_BUTTON_PRESSING = 0;
					
					BUTTON_MIDDLE_HOLD = 0;
					BUTTON_BOTTOM_HOLD = 0;
				}
			} //(ANY_BUTTON_PRESSING) && (DRGB_MODE != MODE_SHOW_VER)
	
		} // ONE_MS_TIMER


//------------------------------------------------------------------------------
		
		if (ONE_SEC_TIMER)
		{
			ONE_SEC_TIMER = 0;
		//------------------------------
		//  
			switch(DRGB_MODE)
			{
				case MODE_SHOW_VER:{
					TIC55_fullClear();
					DRGB_MODE = MODE_MENU_NAV;
//					menu_navigation(MENU_NAV_EXIT);
				} break;
				
				
				case MODE_MENU_NAV:{
					
			//------------------------------
			// мигание стрелок 
					//if (TIC55_arrows_blink != 0) {
					TIC55_arrows_on ^= TIC55_arrows_blink;
					//	TIC55_send();
					//}
					
					//
					menu_navigation(MENU_PROG_CHECK);

					
				
					BATTERY_STATE = HW_battery_getState();
					switch (BATTERY_STATE)
					{
						case BAT_STATE_FULL:
						{
							TIC55_clearArrow(6);
						}break;
						case BAT_STATE_HALF:
						{
							TIC55_setArrow(6, 0);
							HW_signal(30, SIGNAL_P_P_P_P);
						}break;
						case BAT_STATE_DEAD:
						{
							TIC55_arrows_on = 64;
							sleep_prepare();
						}break;
					}
				} break;
				
				case MODE_SLEEP_READY:{
					sleep_now();
				} break;	
			}
			
		} // ONE_SEC_TIMER
		 

//		if ((ONE_US_TIMER & 0x200) == 0) {
//			if ((TIC55_arrows_blink & TIC55_arrows_on) == 0) {   // зажигаем мигалки
//				TIC55_arrows_on |= TIC55_arrows_blink;
//				TIC55_send();
//			}
//		} else {
//			if ((TIC55_arrows_blink & TIC55_arrows_on) != 0) {   // гасим мигалки
//				TIC55_arrows_on &= ~TIC55_arrows_blink;
//				TIC55_send();
//			}
//		}
		
//------------------------------------------------------------------------------
		
		// озвучивание событий
		if (BETA5_REGISTRED) {
			BETA5_REGISTRED = 0;
			
//			if (led_mode != 0) HW_led(1), led_timer = 15;
			if (CONFIG_LED_ENABLED && (led_timer < CONFIG_CLICKER_LEN*2)) HW_led(1), led_timer = CONFIG_CLICKER_LEN*2;

			if ((CONFIG_CLICKER_LEN != 0) && clicker_enabled && CONFIG_SOUND_ENABLED)	{
				if (SIGNAL_SPEAKER_PIN == 1) {
					SIGNAL_SPEAKER_PIN = 0;
					SIGNAL_PLAYNG = CONFIG_CLICKER_LEN;
				}	else {
					SIGNAL_SPEAKER_PIN = 1;
				}
			}
		}		
		
//------------------------------------------------------------------------------
			
		// сон
		if (DRGB_MODE != MODE_SLEEP_NOW) {			
			PCON |= IDL_; // пониженное энергопотребление на холостом ходу
		}	else {
			PCON |= STOP_; // сон до внешнего события
			sleep_wakeup(); // после внешнего события
		}
	} // while 1
} // main


/*
 * screen.c
 *
 *  Created on: 13 июн. 2026 г.
 *      Author: rubis
 */
#include <stdio.h>
#include "liquidcrystal_i2c.h"
#include "Keyboard.h"
#include "winding.h"
#include "eeprom.h"
#include "bcd_dihalt.h"
#include "ui.h"
#include "menu.h"



//-------------------
uint16_t NumForEdit;
char NumForEditText[11];
uint8_t NumEditPos;
uint8_t NumTypePos;
uint8_t NumCommaPos;
uint8_t NumLen;
uint8_t ColLCD;
uint8_t RowLCD;
uint8_t FieldType;
uint8_t FieldBytes;
uint16_t* AddrEditField;
ListItem* EditList;
char WindingShowSimbol;
//char* NumForEditText;
char* AddrNumForEdit;


extern uint16_t	ScrewStep;		// Шаг подаюего винта
extern uint16_t	QStepEng;		// Количество шагов двигателя укладчика за один оборот
extern uint16_t	QPulsVal;		// количество импульсов датчика вращения за один оборот подающего винта
extern uint16_t	StepEngPulseLen;// длина мепульса шагового двинателя
extern uint8_t	EngType;		// Тип шагового двигателя (0 - униполярный, 1 - биполятрный, 2 - внешний драйвер с интерфейсом "шаг, направление".

extern uint16_t	StepEng;		// перемещение укладчика за один шаг двигател = ScrewStep / QStepEng
extern uint16_t	StepVal;		// шаг укладки за один импульс валкодера = Step / QPulsVal

extern uint8_t		TurnDirEnkoder; //  направление вращения вала намотчика
extern int16_t		CurSum;		// Текущая разность позиуции укладчика и позиции намотки
extern int16_t		DWind;		// перемещение укладчика за один импульс енкодера
extern int16_t		DStEng;		// перемещение укладчика за один шаг двигателя
extern int16_t		PulseCount; // счетчик импульсов енкодера

extern uint16_t    StopTime;

extern uint8_t flMoveWindMotor=0;
extern uint8_t WinMotorSpeed = 0;
extern uint8_t MaxWindMotorSpeed = 255;
extern uint8_t CurMaxWindMotorSpeed;
extern uint8_t WindMotorFinishSpeed;

uint8_t NumEditPos;
uint8_t NumTypePos;



uint8_t indField = 0;  // номер поля, на котором установлен фокус (курсор)

enum WindDirEnum  {
	WDIR_RIGHT = 0,
	WDIR_LEFT = 1
};

enum Fields {
	Q_TURNS,
	Q_TURNS_IN_LAY,
	STEP,
	CUR_Q_TURNS,
	CUR_Q_TURNS_IN_LAY,
	CUR_LAY,
	WINDING_DIR,
	WINDING_MODE
};


const DialogFieldStuct DialogFields[]  =
{	//	Ind	Col	Row	ComaPos	NumLen	FieldType QBytes;
	{Q_TURNS,			0,	0,	0,	4, 0, 2}, //QTurns
	{Q_TURNS_IN_LAY,	5,	0,	0,	4, 0, 2}, //QTurnsInLay
	{STEP, 				10,	0,	1,	3, 0, 2}, //Step
	{CUR_Q_TURNS,		0,	1,	0,	4, 0, 2}, //CurQTurns
	{CUR_Q_TURNS_IN_LAY,5,	1,	0,	4, 0, 2}, //CurQTurnsInLay
	{CUR_LAY, 			10,	1,	0,	2, 0, 2}, //CurLay
	{WINDING_DIR, 		13,	1,	0,	0, 1, 2}, //WinDir
	{WINDING_MODE, 		15,	1,	0,	0, 1, 2}  //WinMode

} ;

const ListItem WindDirItems[]  =
{	//Num			Next		Prev
	{WDIR_RIGHT,	WDIR_LEFT,	WDIR_LEFT,	"-->\0", ">>\0"},
	{WDIR_LEFT,		WDIR_RIGHT, WDIR_RIGHT,	"<--\0", "<<\0"}
};

const ListItem WindModeItems[]  =
{	//Num				Next			Prev
	{ WMODE_BY_LAYS,	WMODE_ALL,		WMODE_ALL,		"S pauzoy\0", "P\0"},
	{ WMODE_ALL,		WMODE_BY_LAYS,	WMODE_BY_LAYS,	"Bez pauz\0", "N\0"}
};

const ListItem EngineTypeItems[]  =
{	//Num				Next			Prev
	{ 0,	1,	2,		"Unipolar\0", "\0"},
	{ 1,	2,	0,		"Bipolar\0", "\0"},
	{ 2,	0,	1,		"Ext. driver\0", "\0"}
};

const ListItem OffOnItems[]  =
{	//Num				Next			Prev
	{ OFF,	ON,		ON,		"Off\0",	"Off\0"},
	{ ON,	OFF,	OFF,	"On\0",		"On\0"}
};



const char ErorWireBreakDEscrition[] = "ERROR Wire Break\0";
const char StrMovingToA[] = "Moving to A\0";
const char StrMovingToB[] = "Moving to B\0";
const char StrPosAmemorized[] = "Pos. A memorized\0";
const char StrPosBmemorized[] = "Pos. B memorized\0";

extern Button_t myButton;


void lcd_clrscr(unsigned char data) {
	unsigned char i = 0;
	if (data == 1) {
		HD44780_SetCursor(0, 0);
		for (i = 0; i < 16; i++)
			HD44780_PrintStr(" ");
		HD44780_SetCursor(0, 0);
	}

	if (data == 2) {
		HD44780_SetCursor(0, 1);
		for (i = 0; i < 16; i++)
			HD44780_PrintStr(" ");
		HD44780_SetCursor(0, 1);
	}

	if (data == 3) {
		/*Clear display*/
		HD44780_Clear();
		//_delay_us(120);         //Delay after clearing display
	}
}

//===================================================
void PrintNumComma(char* NumForEdit, uint8_t NumCommaPos, uint8_t NumLen)
{
	uint8_t i;
	char* c;
	i=1;
	while  (c = (NumForEdit++))  {
		if (i== NumCommaPos)
		{
			HD44780_PrintStr(",");
		}
		HD44780_PrintStr(c);
		i++;
	}
}

//HD44780_SetCursor(0, 0);
//HD44780_PrintStr(buf);\

extern uint16_t	Step;			//  шаг укладки (диаметр провода + допуск)
extern uint16_t	WindMode;		//	режим намотки
extern uint16_t	QTurns;			//	количество витков
extern uint16_t	QTurnsInLay;	//	количество витков в слое
extern int16_t	CurQTurns;		//	текущее количество витков (уже намотано)
extern int16_t	CurQTurnsInLay;	//	текущее количество витков (уже намотано) в текущем слое
extern uint16_t	CurLay;
extern uint16_t	WindDir;		//	направление намотки (укладки)

extern uint16_t	ScrewStep;

//uint16_t	qLays;			//	количество слоёв
//uint8_t		StepperDirCorrection; // коррекция направления вращения мотора укладчика
//uint8_t		WireBreakSensorEnable; // коррекция направления вращения мотора укладчика
//uint8_t		WireBreakSensorNormVal; // коррекция направления вращения мотора укладчика
//
//uint8_t		TurnDirEnkoder; //  направление вращения вала намотчика
//int16_t		CurSum;		// Текущая разность позиуции укладчика и позиции намотки
//int16_t		DWind;		// перемещение укладчика за один импульс енкодера
//int16_t		DStEng;		// перемещение укладчика за один шаг двигателя
//int16_t		PulseCount; // счетчик импульсов енкодера

void GetFieldInfo(uint8_t NumField)
{
		ColLCD  = DialogFields[NumField].Col;
		RowLCD  = DialogFields[NumField].Row;
		NumLen  = DialogFields[NumField].NumLen;
		NumCommaPos  = DialogFields[NumField].ComaPos;
		FieldType = DialogFields[NumField].FieldType;
		FieldBytes = DialogFields[NumField].QBytes;

		//QTurns
		//QTurnsInLay
		//Step
		//CurQTurns
		//CurQTurnsInLay
		//CurLay

		switch (indField)
		{
			case Q_TURNS:
			//AddrNumForEdit = utoa_fast_div(QTurns, NumForEditText);
			AddrEditField = &QTurns;
			break;
			case Q_TURNS_IN_LAY:
			//AddrNumForEdit = utoa_fast_div(QTurnsInLay, NumForEditText);
			AddrEditField = &QTurnsInLay;
			break;
			case STEP:
			//AddrNumForEdit = utoa_fast_div(Step, NumForEditText);
			AddrEditField = &Step;
			break;
			case CUR_Q_TURNS:
			//AddrNumForEdit = utoa_fast_div(CurQTurns, NumForEditText);
			AddrEditField = &CurQTurns;
			break;
			case CUR_Q_TURNS_IN_LAY:
			//AddrNumForEdit = utoa_fast_div(CurQTurnsInLay, NumForEditText);
			AddrEditField = &CurQTurnsInLay;
			break;
			case CUR_LAY:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &CurLay;
			break;
			case WINDING_DIR:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &WindDir;
			EditList = (ListItem *) &WindDirItems;
			break;
			case WINDING_MODE:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &WindMode;
			EditList = (ListItem *) &WindModeItems;
			break;
		}
}


void ShowField()
{

	GetFieldInfo(indField);


	HD44780_SetCursor(ColLCD, RowLCD);
	if (FieldType==0)
	{
		if (FieldBytes==2){
			NumForEdit = *(uint16_t*)AddrEditField;
			AddrNumForEdit = utoa_fast_div_len(NumForEdit, NumForEditText, NumLen);
		}
		else{
			NumForEdit = (uint8_t)*AddrEditField;
			AddrNumForEdit = utoa_fast_div_len(NumForEdit, NumForEditText, NumLen);
		}

		PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
		HD44780_SetCursor(ColLCD, RowLCD);
	}
	else
	{
		HD44780_SetCursor(ColLCD, RowLCD);
		uint8_t ListFieldFoEdit = *AddrEditField;
		//WindDir = ChoiceListItem(EditList,  ListFieldFoEdit, 1, 1, 1);
		HD44780_PrintStr(&(*EditList[ListFieldFoEdit].Short));
		HD44780_SetCursor(ColLCD, RowLCD);
	}
}



//================================================
// Ввоод числа
// параметры из глобальных переменных :
//
// NumForEdit	- редактируемое число
// NumCommaPos  - позиция запятой в строчном представлении числа
// NumLen		- количество разрядов числа не учитывая запятую
// NumEditPos	- редактируемый разряд числа
// NumTypePos	- редактируемый разряд числа с учётом запятой
void EditNum(uint8_t NColLCD, uint8_t NStrLCD)
{
	//char NumForEditText[11];
	//char* AddrNumForEdit;
	char CurSymb;
	char InpDig;
	uint8_t flDigit;

//	KeyCode = 0;
//	EventCode = 0;
	NumEditPos = 1;

	AddrNumForEdit = utoa_fast_div_len(NumForEdit, NumForEditText, NumLen);

	// Дополним нулями
	//AddrNumForEdit = FormatNum((char*) &NumForEditText, NumLen);
	//utoa(NumForEdit, &NumForEditText,10);
	//AddrNumForEdit = &NumForEditText;
	HD44780_SetCursor(NColLCD, NStrLCD);
	//lcd_puts(AddrNumForEdit);
	PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
	NumTypePos = NumEditPos;
	if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
	{
		NumTypePos++;
	}
	HD44780_SetCursor(NColLCD + NumTypePos-1, NStrLCD);

//	Button_t myButton = { 0, BUTTON_RELEASED, 0 };
//	setTimer(&keyboard_timer);

	while (1)
	{
		flDigit = 0;
		// опрос клавиатуры
//		KeyCode = GetKey();
//		EventCode = KeyEvent(KeyCode);

//		if (!checkTimer(&keyboard_timer, 10)) {
//			continue;
//		}
//		setTimer(&keyboard_timer);

		uint8_t keyCode = Process_Button(&myButton);

		if (keyCode == KEYCODE_NOPRESSED) {
			continue;
		}

		//---debug----------
		//KeyCode = KEY_OK;
		//EventCode = 1;
		//-------------------
//		if (EventCode == 0)
//		{
//			continue;
//		}
//		EventCode = 0;

		switch (keyCode)
		{
			case KEY_0: flDigit = 1; InpDig = 48; break;
			case KEY_1: flDigit = 1; InpDig = 49; break;
			case KEY_2: flDigit = 1; InpDig = 50; break;
			case KEY_3: flDigit = 1; InpDig = 51; break;
			case KEY_4: flDigit = 1; InpDig = 52; break;
			case KEY_5: flDigit = 1; InpDig = 53; break;
			case KEY_6: flDigit = 1; InpDig = 54; break;
			case KEY_7: flDigit = 1; InpDig = 55; break;
			case KEY_8: flDigit = 1; InpDig = 56; break;
			case KEY_9: flDigit = 1; InpDig = 57; break;
			case KEY_RIGHT	:
				lvRight:
				if (NumEditPos >= NumLen)
				{
					NumEditPos = NumLen;
				}
				else
				{
					NumEditPos++;
				}

				NumTypePos = NumEditPos;
				if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
				{
					NumTypePos++;
				}
				HD44780_SetCursor(NColLCD + NumTypePos-1, NStrLCD);
				break;



			case KEY_LEFT	:

				if (NumEditPos > 1)
				{
					NumEditPos--;
				}
				else
				{
					//NumForEdit = BCDToHex(AddrNumForEdit);
					NumForEdit = atou_1((char *)AddrNumForEdit);
					return;
				}

				NumTypePos = NumEditPos;
				if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
				{
					NumTypePos++;
				}
				HD44780_SetCursor(NColLCD + NumTypePos-1, NStrLCD);
				break;
//			case KEY_RET	:
//
//				return;
//				break;
			case KEY_OK	:
				NumForEdit = atou_1((char *)AddrNumForEdit);
				return;
				break;


		}
		if (flDigit != 0)
		{
			flDigit = 0;
			(*(AddrNumForEdit+NumEditPos-1)) = InpDig;
			HD44780_SetCursor(NColLCD + NumTypePos-1, NStrLCD);
			HD44780_PrintStr((AddrNumForEdit+NumEditPos-1));
			goto lvRight;
		}
	}
}



//================================================
// Ввоод числа
// параметры из глобальных переменных :
//
// NumForEdit	- редактируемое число
// NumCommaPos  - позиция запятой в строчном представлении числа
// NumLen		- количество разрядов числа не учитывая запятую
// NumEditPos	- редактируемый разряд числа
// NumTypePos	- редактируемый разряд числа с учётом запятой
void EditNumStruct(uint8_t NColLCD, uint8_t NStrLCD)
{
	//char NumForEditText[11];
	//char* AddrNumForEdit;
	char CurSymb;
	char InpDig;
	uint8_t flDigit;

//	KeyCode = 0;
//	EventCode = 0;

	AddrNumForEdit = utoa_fast_div(NumForEdit, NumForEditText);

	// Дополним нулями
	//AddrNumForEdit = FormatNum((char*) &NumForEditText, NumLen);
	//utoa(NumForEdit, &NumForEditText,10);
	//AddrNumForEdit = &NumForEditText;
	HD44780_SetCursor(NColLCD, NStrLCD);
	//lcd_puts(AddrNumForEdit);
	PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
	NumTypePos = NumEditPos;
	if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
	{
		NumTypePos++;
	}
	HD44780_SetCursor(NColLCD + NumTypePos-1, NStrLCD);

	while (1)
	{
		flDigit = 0;

		// опрос клавиатуры
//		KeyCode = GetKey();
//		EventCode = KeyEvent(KeyCode);
//		if (!checkTimer(&keyboard_timer, 10)) {
//			continue;
//		}
//		setTimer(&keyboard_timer);

		uint8_t keyCode = Process_Button(&myButton);

		if (keyCode == KEYCODE_NOPRESSED) {
			continue;
		}

		//EventCode = 0;
		switch (keyCode)
		{
			case KEY_0: flDigit = 1; InpDig = 48; break;
			case KEY_1: flDigit = 1; InpDig = 49; break;
			case KEY_2: flDigit = 1; InpDig = 50; break;
			case KEY_3: flDigit = 1; InpDig = 51; break;
			case KEY_4: flDigit = 1; InpDig = 52; break;
			case KEY_5: flDigit = 1; InpDig = 53; break;
			case KEY_6: flDigit = 1; InpDig = 54; break;
			case KEY_7: flDigit = 1; InpDig = 55; break;
			case KEY_8: flDigit = 1; InpDig = 56; break;
			case KEY_9: flDigit = 1; InpDig = 57; break;
			case KEY_RIGHT	:
			lvRight:
			if (NumEditPos >= NumLen)
			{
				NumEditPos = NumLen;
			}
			else
			{
				NumEditPos++;
			}

			NumTypePos = NumEditPos;
			if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
			{
				NumTypePos++;
			}
			HD44780_SetCursor(NColLCD + NumTypePos-1, NStrLCD);
			break;



			case KEY_LEFT	:

			if (NumEditPos > 1)
			{
				NumEditPos--;
			}
			else
			{
				//NumForEdit = BCDToHex(AddrNumForEdit);
				NumForEdit = atou_1((char *)AddrNumForEdit);
				return;
			}

			NumTypePos = NumEditPos;
			if ((NumCommaPos !=0) & (NumCommaPos <= NumEditPos))
			{
				NumTypePos++;
			}
			HD44780_SetCursor(NColLCD + NumTypePos-1, NStrLCD);
			break;

		}
		if (flDigit != 0)
		{
			flDigit = 0;
			(*(AddrNumForEdit+NumEditPos-1)) = InpDig;
			HD44780_SetCursor(NColLCD + NumTypePos-1, NStrLCD);
			HD44780_PrintStr((AddrNumForEdit+NumEditPos-1));
			goto lvRight;
		}
	}
}


//void ShowMenu(uint8_t MenuState, uint8_t showmode)
//{
//	uint8_t ind;
//	uint8_t Parent;
//	//char *Buffer;
//	//char *CurrCodeStr;
//	uint8_t indStr;
//
//	ind = MenuState;
//	ind--;
//	if (ind> MAX_MENU_ITEM)
//	{
//		return;
//	}
//
//	for (indStr == 1; indStr<=LCDType; indStr++)
//	{
//		lcd_clrscr(indStr);
//	}
//
//	Parent  = pgm_read_byte(&(MenuItems[ind].Parent));
//	if ((Parent == 0) | (showmode == 1))
//	{
//		HD44780_SetCursor(0, 0);
//		HD44780_PrintStr(&(MenuItems[ind].Text));
//	}
//	else
//	{
//		HD44780_SetCursor(0, 0);
//		HD44780_PrintStr(&(MenuItems[Parent-1].Text));
//		HD44780_SetCursor(3, 1);
//		HD44780_PrintStr(&(MenuItems[ind].Text));
//	}
//	//lcd_putc("33");
//}



//**********************************************

// --------------------------------------------------------
void SetStep()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = Step;
	//NumForEditText =

	NumCommaPos = 2;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	draw_menu(1);
	EditNum(0, 1);
	Step = NumForEdit;
}

// --------------------------------------------------------
void SetQuant()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = QTurns;
	//NumForEditText =

	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	draw_menu(1);
	EditNum(0, 1);
	QTurns = NumForEdit;
	//PORTC = 17;
}

// --------------------------------------------------------
void SetQuantInLay()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = QTurnsInLay;
	//NumForEditText =

	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	draw_menu(1);
	EditNum(0, 1);
	QTurnsInLay = NumForEdit;
}

// --------------------------------------------------------
void SetScrewStep()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = ScrewStep ;

	NumCommaPos = 2;//ScrewStep.CommaPos;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 4 знака (без запятой) 0,00
	draw_menu(1);
	EditNum(0, 1);
	ScrewStep = NumForEdit;
	eeprom_write_word(&EEScrewStep, ScrewStep); // Записываем значение в ЕЕПРОМ
	EE_WriteVariable(uint16_t VirtAddress, ScrewStep);
}



// --------------------------------------------------------
void SetQStepEng()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = QStepEng;

	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 4 знака (без запятой) 0,00
	draw_menu(1);
	EditNum(0, 1);
	QStepEng = NumForEdit;
	eeprom_write_word(&EEQStepEng, QStepEng); // Записываем значение в ЕЕПРОМ

}

// --------------------------------------------------------
void SetQPulsVal()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = QPulsVal;

	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	draw_menu(1);
	EditNum(0, 1);
	QPulsVal = NumForEdit;
	eeprom_write_word(&EEQPulsVal, QPulsVal); // Записываем значение в ЕЕПРОМ
}

void SetWinMotorStopTime()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = StopTime;

	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	draw_menu(1);
	EditNum(0, 1);
	StopTime = NumForEdit;
	eeprom_write_word(&EEStopTime, StopTime); // Записываем значение в ЕЕПРОМ
}


// --------------------------------------------------------
uint8_t ChoiceListItem(ListItem* ListItems,  uint8_t  NumItem, uint8_t NColLCD, uint8_t NStrLCD, uint8_t flShort)
{
	uint8_t KeyCode;
	uint8_t EventCode;
	uint8_t NewNom;
	uint8_t CurNumItem;

	CurNumItem = NumItem;
	//CurNumItem;


	HD44780_SetCursor(NColLCD, NStrLCD);
	if (flShort == 0)
	{
		HD44780_PrintStr(&(*ListItems[CurNumItem].Text));
	}
	else
	{
		HD44780_PrintStr(&(*ListItems[CurNumItem].Short));
	}
	HD44780_SetCursor(NColLCD, NStrLCD);

	while (1)
	{
		// опрос клавиатуры
		KeyCode = GetKey();
		EventCode = KeyEvent(KeyCode);

		if (EventCode == 0) continue;


		switch (KeyCode)
		{
			//case KEY_OK		:
			case KEY_UP	:
			NewNom = pgm_read_byte(&(ListItems[CurNumItem].Previous));
			CurNumItem = NewNom;
			//lcd_clrscr(NStrLCD + 1);
			HD44780_SetCursor(NColLCD, NStrLCD);
			if (flShort == 0)
			{
			HD44780_PrintStr("               ");
			HD44780_SetCursor(NColLCD, NStrLCD);
			HD44780_PrintStr(&(*ListItems[CurNumItem].Text));
			}
			else
			{
			lcd_puts("  ");
			HD44780_SetCursor(NColLCD, NStrLCD);
			HD44780_PrintStr(&(*ListItems[CurNumItem].Short));
			}
			HD44780_SetCursor(NColLCD, NStrLCD);

			break;

			case KEY_DOWN	:
			NewNom = pgm_read_byte(&(ListItems[CurNumItem].Next));
			CurNumItem = NewNom;
			//lcd_clrscr(NStrLCD + 1);
			HD44780_SetCursor(NColLCD, NStrLCD);
			if (flShort == 0)
			{
				lcd_puts("               ");
				HD44780_SetCursor(NColLCD, NStrLCD);
				HD44780_PrintStr(&(*ListItems[CurNumItem].Text));
			}
			else
			{
				lcd_puts("  ");
				HD44780_SetCursor(NColLCD, NStrLCD);
				HD44780_PrintStr(&(*ListItems[CurNumItem].Short));
			}
			HD44780_SetCursor(NColLCD, NStrLCD);
			break;

			case KEY_LEFT	:
			case KEY_OK	:
			//if (flShort == 0)
			//{
			//lcd_clrscr(NStrLCD + 1);
			//}
			return CurNumItem;
			break;
			case KEY_RET:
			if (flShort == 0)
			{

				//HD44780_SetCursor(NColLCD, NStrLCD);
				HD44780_PrintStr(&(*ListItems[NumItem].Text));
			}
			else
			{
				//HD44780_SetCursor(NColLCD, NStrLCD);
				HD44780_PrintStr(&(*ListItems[NumItem].Short));
			}
			HD44780_SetCursor(NColLCD, NStrLCD);
			return NumItem;

		}
	}
}


// --------------------------------------------------------
void SetWindDir()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(1);

	WindDir = ChoiceListItem((ListItem *) &WindDirItems,  WindDir, 1, 1, 0);

	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(0);
}

// --------------------------------------------------------
void SetWindMode()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(1);

	WindMode = ChoiceListItem((ListItem *) &WindModeItems,  WindMode, 1, 1, 0);

	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(0);
}


//EngineTyes
// --------------------------------------------------------
void SetEngType()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(1);

	EngType = ChoiceListItem((ListItem *) &EngineTypeItems,  EngType, 1, 1, 0);

	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(0);
	eeprom_write_word(&EEEngType, EngType); // Записываем значение в ЕЕПРОМ
}

// --------------------------------------------------------
void SetStepperDirCorrection()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(1);

	StepperDirCorrection = ChoiceListItem((ListItem *) &OffOnItems,  StepperDirCorrection, 1, 1, 0);
	eeprom_write_word(&EEStepperDirCorrection, StepperDirCorrection); // Записываем значение в ЕЕПРОМ

	lcd_clrscr(1);
	lcd_clrscr(2);

	draw_menu(0);
}

// --------------------------------------------------------
void SetWireBreakSensorEnable()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(1);

	WireBreakSensorEnable = ChoiceListItem((ListItem *) &OffOnItems,  WireBreakSensorEnable, 1, 1, 0);
	eeprom_write_word(&EEWireBreakSensorEnable, WireBreakSensorEnable); // Записываем значение в ЕЕПРОМ

	lcd_clrscr(1);
	lcd_clrscr(2);
	draw_menu(0);
}

// --------------------------------------------------------
void SetWireBreakSensorNormVal ()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	drow_menu(1);

	WireBreakSensorNormVal = ChoiceListItem((ListItem *) &OffOnItems,  WireBreakSensorNormVal, 1, 1, 0);
	eeprom_write_word(&EEWireBreakSensorNormVal, WireBreakSensorNormVal); // Записываем значение в ЕЕПРОМ


	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 0);
}

// --------------------------------------------------------
void SetStepIngPulseLen()
{
	lcd_clrscr(1);
	lcd_clrscr(2);

	NumForEdit = StepEngPulseLen;

	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	EditNum(0, 1);
	StepEngPulseLen = NumForEdit;
	eeprom_write_word(&EEStepEngPulseLen, StepEngPulseLen); // Записываем значение в ЕЕПРОМ
}


void ShowByte(uint8_t Num)
{
    NumLen = 3;
    AddrNumForEdit = utoa_fast_div_len(Num, NumForEditText, NumLen);

	HD44780_SetCursor(1, 1);
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 3;			// число - 4 знака
	PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
}

void ShowByteXY(uint8_t Num, uint8_t X, uint8_t Y)
{
	NumLen = 3;
	AddrNumForEdit = utoa_fast_div_len(Num, NumForEditText, NumLen);

	HD44780_SetCursor(X, Y);
	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 3;			// число - 4 знака
	PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
}

// --------------------------------------------------------
void SetStepIngPulseLen2()
{
	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);
	ShowByte(StepEngPulseLen);

	NumForEdit = StepEngPulseLen;

	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);
	ShowByte(StepEngPulseLen);
	StepEngTimer = 0;
	while (1)
	{
		if (StepEngTimer == 0)
		{
			FinishStep();
		}

		// опрос клавиатуры
		KeyCode = GetKey();

		if ((KeyCode == KEY_FW) || (KeyCode == KEY_REW))
		{
			if (StepEngTimer == 0)
			{


				if (KeyCode == KEY_FW)
				{

					StartStepEng(0);
					//prevStateStepEng = ST_ENG_START_STEP;
				}
				else
				{
					StartStepEng(1);
					//prevStateStepEng = ST_ENG_START_STEP;
				}
			}
		}


		EventCode = KeyEvent(KeyCode);

		if (EventCode == 0)
		{
			continue;
		}
		switch (KeyCode)
		{
			case KEY_UP	:
			if (StepEngPulseLen < 255)
			{
			StepEngPulseLen++;
			ShowByte(StepEngPulseLen);

			}
			break;

			case KEY_DOWN	:
			{
				if (StepEngPulseLen > 1)
				{
					StepEngPulseLen--;
					ShowByte(StepEngPulseLen);
				}
			}
			break;

			case KEY_OK:
			eeprom_write_word(&EEStepEngPulseLen, StepEngPulseLen); // Записываем значение в ЕЕПРОМ
			return;
			break;

			case KEY_LEFT:
			return;
			break;
			default :

			break;
		}
	}
}



void SetMaxWinMotorSpeed(char fl_Finish)
{
	uint8_t CurrSpeed;

	lcd_clrscr(1);
	lcd_clrscr(2);
	ShowMenu(MenuState, 1);


	if (fl_Finish != 0)
	{
		CurrSpeed = WindMotorFinishSpeed;
		NumForEdit = WindMotorFinishSpeed;
		//ShowByte(WindMotorFinishSpeed);
	}
	else
	{
		CurrSpeed = MaxWindMotorSpeed;
		NumForEdit = MaxWindMotorSpeed;
		//ShowByte(MaxWindMotorSpeed);
	}


	NumCommaPos = 0;	// второй символ - запятая  0,...
	NumLen = 4;			// число - 3 знака (без запятой) 0,00
	ShowMenu(MenuState, 1);

	//if (fl_Finish != 0)
	//{
		//ShowByte(WindMotorFinishSpeed);
	//}
	//else
	//{
		//ShowByte(MaxWindMotorSpeed);
	//}
	ShowByte(CurrSpeed);


	//StepEngTimer = 0;
	while (1)
	{
		// опрос клавиатуры
		KeyCode = GetKey();


			if (KeyCode == KEY_ROTATE_F)
			{
				//if (fl_Finish != 0)
				//{
					//CurMaxWindMotorSpeed = WindMotorFinishSpeed;
				//}
				//else
				//{
					//CurMaxWindMotorSpeed = MaxWindMotorSpeed;
				//}
				CurMaxWindMotorSpeed = CurrSpeed;

				flMoveWindMotor = 1;
				MoveWindMotor();
			}
			else
			{
				flMoveWindMotor = 0;
				StopWindMotor();
			}



		EventCode = KeyEvent(KeyCode);

		if (EventCode == 0)
		{
			continue;
		}
		switch (KeyCode)
		{
			case KEY_UP	:
			if (CurrSpeed < 255)
			{
				CurrSpeed++;
				ShowByte(CurrSpeed);
				//ShowByteXY(PotentValue, 7, 1);
			}
			break;

			case KEY_DOWN	:
			{
				if (CurrSpeed > 1)
				{
					CurrSpeed--;
					ShowByte(CurrSpeed);
				}
			}
			break;

			case KEY_OK:
			if (fl_Finish != 0)
			{
				WindMotorFinishSpeed = CurrSpeed;
				eeprom_write_word(&EEWindMotorFinishSpeed, WindMotorFinishSpeed); // Записываем значение в ЕЕПРОМ
			}
			else
			{
				MaxWindMotorSpeed = CurrSpeed;
				eeprom_write_word(&EEMaxWindMotorSpeed, MaxWindMotorSpeed); // Записываем значение в ЕЕПРОМ
			}
			return;
			break;

			case KEY_RET:
			return;
			break;
			default :

			break;
		}
	}
}



void ShowField()
{

	GetFieldInfo(indField);


	HD44780_SetCursor(ColLCD, RowLCD);
	if (FieldType==0)
	{
		if (FieldBytes==2){
			NumForEdit = *(uint16_t*)AddrEditField;
			AddrNumForEdit = utoa_fast_div_len(NumForEdit, NumForEditText, NumLen);
		}
		else{
			NumForEdit = (uint8_t)*AddrEditField;
			AddrNumForEdit = utoa_fast_div_len(NumForEdit, NumForEditText, NumLen);
		}

		PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
		HD44780_SetCursor(ColLCD, RowLCD);
	}
	else
	{
		HD44780_SetCursor(ColLCD, RowLCD);
		uint8_t ListFieldFoEdit = *AddrEditField;
		//WindDir = ChoiceListItem(EditList,  ListFieldFoEdit, 1, 1, 1);
		HD44780_PrintStr(&(*EditList[ListFieldFoEdit].Short));
		HD44780_SetCursor(ColLCD, RowLCD);
	}
}

// ============================================================================
 void ShowWinding()
{
//char NumForEditText[11];
//char* AddrNumForEdit;


//lcd_clrscr(1);
//lcd_clrscr(0);
//AddrNumForEdit = utoa_fast_div(DWind , NumForEditText);
//HD44780_SetCursor(0, 0);
////lcd_puts(AddrNumForEdit);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);


//AddrNumForEdit = utoa_fast_div(QTurns, NumForEditText);
//HD44780_SetCursor(1, 0);
////utoa(QTurns, c, 10);
////lcd_puts(NumForEditText);
//
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(DStEng, NumForEditText);
//HD44780_SetCursor(0, 1);
//////lcd_puts(AddrNumForEdit);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(CurSum, NumForEditText);
//HD44780_SetCursor(5, 1);
//////lcd_puts(AddrNumForEdit);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 5;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(QTurnsInLay, NumForEditText);
//HD44780_SetCursor(6, 0);
////lcd_puts(AddrNumForEdit);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(CurQTurnsInLay, NumForEditText);
//HD44780_SetCursor(6, 1);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(Step, NumForEditText);
//HD44780_SetCursor(11, 0);
//NumCommaPos = 2;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
//
//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
//HD44780_SetCursor(11, 1);
//NumCommaPos = 0;	// второй символ - запятая  0,...
//NumLen = 4;			// число - 4 знака
//PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);

//ShowMainScreen();
indField = 3;
ShowField();
indField = 4;
ShowField();
indField = 5;
ShowField();
indField = 6;
ShowField();

HD44780_SetCursor(15, 0);
lcd_putc(WindingShowSimbol);
}


void GetFieldInfo(uint8_t NumField)
{



		ColLCD  = pgm_read_byte(&(DialogFields[NumField].Col));
		RowLCD  = pgm_read_byte(&(DialogFields[NumField].Row));
		NumLen  = pgm_read_byte(&(DialogFields[NumField].NumLen));
		NumCommaPos  = pgm_read_byte(&(DialogFields[NumField].ComaPos));
		FieldType = pgm_read_byte(&(DialogFields[NumField].FieldType));
		FieldBytes = pgm_read_byte(&(DialogFields[NumField].QBytes));


		//QTurns
		//QTurnsInLay
		//Step
		//CurQTurns
		//CurQTurnsInLay
		//CurLay
		switch (indField)
		{
			case 0:
			//AddrNumForEdit = utoa_fast_div(QTurns, NumForEditText);
			AddrEditField = &QTurns;
			break;
			case 1:
			//AddrNumForEdit = utoa_fast_div(QTurnsInLay, NumForEditText);
			AddrEditField = &QTurnsInLay;
			break;
			case 2:
			//AddrNumForEdit = utoa_fast_div(Step, NumForEditText);
			AddrEditField = &Step;
			break;
			case 3:
			//AddrNumForEdit = utoa_fast_div(CurQTurns, NumForEditText);
			AddrEditField = &CurQTurns;
			break;
			case 4:
			//AddrNumForEdit = utoa_fast_div(CurQTurnsInLay, NumForEditText);
			AddrEditField = &CurQTurnsInLay;
			break;
			case 5:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &CurLay;
			break;
			case 6:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &WindDir;
			EditList = (ListItem *) &WindDirItems;
			break;
			case 7:
			//AddrNumForEdit = utoa_fast_div(CurLay, NumForEditText);
			AddrEditField = &WindMode;
			EditList = (ListItem *) &WindModeItems;
			break;
		}
}




// ============================================================================
void TabField(uint8_t Right)
{

	//uint8_t FieldType;

	if (Right!=0){
		indField++;
		if (indField >= maxQFields)
		{
			indField = 0;
		}
	}
	else
	{
		if (indField==0 )
		{
			indField = maxQFields-1;
		}
		else
		{
			indField--;
		}
	}

	ShowField();
}

// ============================================================================
void SetToField(uint8_t NumField)
{

	//uint8_t FieldType;

	if (NumField < maxQFields)
		{
			indField = NumField;
			ShowField();
		}
}

//=============================================================================
// редактирование поля
void EditField(uint8_t NumField)
{
	GetFieldInfo(NumField);
	if (FieldType==0)
	{
		lcd_cursor(3);
		AddrNumForEdit = utoa_fast_div_len(*AddrEditField, NumForEditText, NumLen);
		HD44780_SetCursor(ColLCD, RowLCD);

		if (FieldBytes == 2){
			NumForEdit = *AddrEditField;
			EditNum(ColLCD, RowLCD);
			*AddrEditField = NumForEdit;

			GetFieldInfo(NumField);
			AddrNumForEdit = utoa_fast_div_len(*AddrEditField, NumForEditText, NumLen);
			HD44780_SetCursor(ColLCD, RowLCD);
			PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
			HD44780_SetCursor(ColLCD, RowLCD);
		}
		else if(FieldBytes==1){
			NumForEdit = (uint8_t)*AddrEditField;
			EditNum(ColLCD, RowLCD);
			*AddrEditField = (uint8_t)NumForEdit;

			GetFieldInfo(NumField);
			AddrNumForEdit = utoa_fast_div_len(*AddrEditField, NumForEditText, NumLen);
			HD44780_SetCursor(ColLCD, RowLCD);
			PrintNumComma(AddrNumForEdit, NumCommaPos, NumLen);
			HD44780_SetCursor(ColLCD, RowLCD);
		}

		lcd_cursor(2);

	}
	else
	{
		lcd_cursor(3);
		HD44780_SetCursor(ColLCD, RowLCD);
		uint8_t ListFieldFoEdit = *AddrEditField;
		*AddrEditField = ChoiceListItem(EditList,  ListFieldFoEdit, ColLCD, RowLCD, 1);
		lcd_cursor(2);
	}

}



// ============================================================================
void EditSetingsDialog()
{
	HD44780_SetCursor(15, 0);
	lcd_putc('E');

	//indField = 1;
	//TabField(0);
	SetToField(0);
	lcd_cursor(2);

	while (1)
	{
		KeyCode = 0;
		EventCode = 0;
		// опрос клавиатуры
		KeyCode = GetKey();
		EventCode = KeyEvent(KeyCode);

		////---debug
		//indField = 2;
		//KeyCode = KEY_OK;
		//EventCode = 1;
		////---------

		if (EventCode == 0)
		{
			continue;
		}


		EventCode = 0;	// перехватываем событие
		switch (KeyCode)
		{
		case KEY_RIGHT:
			TabField(1); // переход на следующее поле (вправо, по часовой)
			break;
		case KEY_LEFT:
			TabField(0); // переход на предыдущее поле (влево, против часовой)
			break;
		case KEY_UP:
			{
				switch (indField)
				{
				case  3:
					SetToField(0);
					break;
				case  4:
					SetToField(1);
					break;
				case  5:
				case  6:
				case  7:
					SetToField(2);
					break;
				}
			}
			break;
		case KEY_DOWN:
		{
			switch (indField)
			{
				case  0:
				SetToField(3);
				break;
				case  1:
				SetToField(4);
				break;
				case  2:
				SetToField(5);
				break;
			}
		}
		break;
		case KEY_OK:
			EditField(indField); // начинаем редактировать
			lcd_cursor(2);
			break;
		case KEY_RET: 	 // заканчиваем шариться по диалогу, выходим из режима редактирования установок
			return;
			break;
		}


	}

}


//// ============================================================================
//void ShowMessage(char* message)
//{
	//
//}


// ============================================================================
void ShowMessage(char* ErrorDesc, uint8_t Time)
{
	uint8_t flShow = 1;
	uint8_t counter = 0;

	lcd_clrscr(1);
	lcd_clrscr(2);

	lcd_cursor(1);

	while (1)
	{
		if (Time > 0  && counter >Time)
		{
			break;
		}

		KeyCode = 0;
		EventCode = 0;

		// опрос клавиатуры
		KeyCode = GetKey();


		if ((KeyCode == KEY_STOP) || (KeyCode == KEY_RET))
		{
			lcd_clrscr(1);
			lcd_clrscr(2);
			return;
		}

		if (BlinkTimer == 0)
		{
			counter++;

			HD44780_SetCursor(0, 0);

			BlinkTimer = 500;
			if (flShow == 0)
			{
				//lcd_putc(ErrorDesc);
				HD44780_PrintStr(ErrorDesc);
				flShow = 1;
			}
			else{
				lcd_clrscr(1);
				lcd_clrscr(2);
				flShow = 0;
			}
		}
	}
}





void ShowMainScreen()
{
	indField = MAX_Q_FIELDS;
	for(uint16_t LoopInd=0; LoopInd < MAX_Q_FIELDS; LoopInd++)
	{
		TabField(1);
	}
	indField = MAX_Q_FIELDS;
	TabField(1);

}


void MoveToPosStaker(int16_t  StakerPoint, char* StrMoving)
{
	uint8_t flShow = 1;

	lcd_clrscr(3);
	//lcd_clrscr(2);
	lcd_cursor(1);

	uint32_t keyboard_timer = 0;
	setTimer(&keyboard_timer);

	while(1)
	{
		KeyCode = 0;
		EventCode = 0;
		// Индикация режим "Намотка" - мигает буква "W"
		if (BlinkTimer == 0)
		{
			HD44780_SetCursor(0, 0);

			BlinkTimer = 1000;
			if (flShow == 0)
			{
				//lcd_putc(ErrorDesc);
				HD44780_PrintStr(StrMoving);
				flShow = 1;

			}
			else{
				lcd_clrscr(3);
				//lcd_clrscr(0);
				flShow = 0;
			}

		}

		// опрос клавиатуры
		//KeyCode = GetKey();
		if (checkTimer(&keyboard_timer, 10)) {
			uint8_t keyCode = Process_Button(&myButton);
			setTimer(&keyboard_timer);
		}



		if (keyCode == KEYCODE_NOPRESSED)
			continue;


		if (KeyCode == KEY_STOP)
		{
			break;
		}

		if (StepCouner == StakerPoint)
		{

			FinishStep();
			break;
		}



			if (StepCouner < StakerPoint)
			{
				if (StepEngTimer == 0)
				{
				StartStepEng(0);
				}
			}
			else
			{
				if (StepEngTimer == 0)
				{
				StartStepEng(1);
				}
			}

			if (StepEngTimer == 0)
			{
			FinishStep();
			}

	}

	//if (StepEngTimer == 0)
	//{
		FinishStep();
	//}

}


void  HandlSetStep()
{
	//case HE_SET_STEP : // Установка шага намотки
	//ProgState = PS_SET_STEP;
	ShowMenu(1);
	NumEditPos = 1;
	NumTypePos = 1;
	SetStep();
	//ProgState = PS_MENU;
	ShowMenu(0);
}

void hendl_menu_engine_steps() {   	// Установка количества шагов двигателя укладчика за один оборот
	//ProgState = PS_SET_QSTEP_ENG;
	draw_menu(1);
	NumEditPos = 1;
	NumTypePos = 0;
	SetQStepEng();
	//ProgState = PS_MENU;
	draw_menu(0);
	}




//HD44780_Clear();
//HD44780_SetCursor(0, 0);
//HD44780_PrintStr(parent_item->name);
//HD44780_SetCursor(3, 1);
//HD44780_PrintStr(current_item->name);


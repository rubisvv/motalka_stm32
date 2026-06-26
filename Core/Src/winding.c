/*
 * winding.c
 *
 *  Created on: 13 июн. 2026 г.
 *      Author: rubis
 */
#include "eeprom.h"

int16_t StepCouner = 0;
int16_t StepCounerA = 0;
int16_t StepCounerB = 0;

// -- Параметры аппаратуры  -------

uint16_t	ScrewStep;		// Шаг подаюего винта
uint16_t	QStepEng;		// Количество шагов двигателя укладчика за один оборот
uint16_t	QPulsVal;		// количество импульсов датчика вращения за один оборот подающего винта
uint16_t	StepEngPulseLen;// длина мепульса шагового двинателя
uint8_t		EngType;		// Тип шагового двигателя (0 - униполярный, 1 - биполятрный, 2 - внешний драйвер с интерфейсом "шаг, направление".

uint16_t	StepEng;		// перемещение укладчика за один шаг двигател = ScrewStep / QStepEng
uint16_t	StepVal;		// шаг укладки за один импульс валкодера = Step / QPulsVal

// -- Параметры намотки  ------------

uint16_t	Step;			//  шаг укладки (диаметр провода + допуск)
uint8_t	    WindMode;		//	режим намотки
uint16_t	qLays;			//	количество слоёв
uint16_t	QTurns;			//	количество витков
uint16_t	QTurnsInLay;	//	количество витков в слое
int16_t		CurQTurns;		//	текущее количество витков (уже намотано)
int16_t		CurQTurnsInLay;	//	текущее количество витков (уже намотано) в текущем слое
uint16_t	CurLay;
uint8_t		WindDir;		//	направление намотки (укладки)

uint8_t		StepperDirCorrection; // коррекция направления вращения мотора укладчика
uint8_t		WireBreakSensorEnable; // коррекция направления вращения мотора укладчика
uint8_t		WireBreakSensorNormVal; // коррекция направления вращения мотора укладчика

uint8_t		TurnDirEnkoder; //  направление вращения вала намотчика
int16_t		CurSum;		// Текущая разность позиуции укладчика и позиции намотки
int16_t		DWind;		// перемещение укладчика за один импульс енкодера
int16_t		DStEng;		// перемещение укладчика за один шаг двигателя
int16_t		PulseCount; // счетчик импульсов енкодера

uint16_t    StopTime;

uint8_t flMoveWindMotor=0;
uint8_t WinMotorSpeed = 0;
uint8_t MaxWindMotorSpeed = 255;
uint8_t WindMotorFinishSpeed;

uint8_t CurMaxWindMotorSpeed;


struct ee_hard_setings {
uint16_t EEScrewStep;				// Шаг подаюего винта
uint16_t EEQStepEng;				// Количество шагов двигателя укладчика за один оборот
uint16_t EEQPulsVal;				// количество импульсов датчика вращения за один оборот подающего винта
uint16_t EEStepEngPulseLen;			// длина импульса шагового двинателя
uint16_t EEEngType;					// Тип шагового двигателя (0 - униполярный, 1 - биполятрный, 2 - внешний драйвер с интерфейсом "шаг, направление".

uint16_t EEStepperDirCorrection; 	// Коррекция направления шагового двигателя
uint16_t EEWireBreakSensorEnable; 	// Датчик обрыва провода включен
uint16_t EEWireBreakSensorNormVal;	// нормальное значение датчика обрыва провода
uint16_t EEMaxWindMotorSpeed; 		// Максимальная скорость
uint16_t EEWindMotorFinishSpeed;	// Скорость
uint16_t EEStopTime;

int16_t SavedStep;					//  шаг укладки (диаметр провода + допуск)
uint16_t SavedWindMode;				//	режим намотки (непрерывный или перерыв между слоями)
uint16_t SavedqLays;				//	количество слоёв
uint16_t SavedQTurns;				//	количество витков
uint16_t SavedQTurnsInLay;			//	количество витков в слое
int16_t	 SavedCurQTurns;			//	текущее количество витков (уже намотано)
int16_t	 SavedCurQTurnsInLay;		//	текущее количество витков (уже намотано) в текущем слое
uint16_t SavedCurLay;				//	текуий слой
uint16_t SavedWindDir;				//	направление намотки (укладки)

};





// =======================================================================
void StartStepEng(uint8_t StepDir)
{

	uint8_t CurStepDir;

	// корректируем направление движения шаговика
	if (StepperDirCorrection == 0)
	{
		CurStepDir = StepDir;
	}
	else
	{
		CurStepDir = StepperDirCorrection - StepDir;
	}

	if (StepDir == 0)
	{
		StepCouner ++;
	}
	else
	{
		StepCouner --;
	}


	switch (EngType){
		case 0:// Униполярный шаговик
		{

			if (phaseStepEng == 0)
			{
				phaseStepEng = 1;
			}

			if (CurStepDir ==0) // Если крутим туда
			{
				switch (phaseStepEng){
					case 0b0001:
					phaseStepEng = 0b0100;
					break;
					case 0b0100:
					phaseStepEng = 0b0010;
					break;
					case 0b0010:
					phaseStepEng = 0b1000;
					break;
					case 0b1000:
					phaseStepEng = 0b0001;
					break;
				}

			}
			else         // Если крутим сюда
			{
				switch (phaseStepEng){
					case 0b0001:
					phaseStepEng = 0b1000;
					break;
					case 0b1000:
					phaseStepEng = 0b0010;
					break;
					case 0b0010:
					phaseStepEng = 0b0100;
					break;
					case 0b0100:
					phaseStepEng = 0b0001;
					break;
				}
			}
			//ENG_PORT = phaseStepEng | ENG_PORT & 0xF0;
			ENG_PORT = (phaseStepEng<<4) | ENG_PORT & 0x0F;
		}
		break;

		case 1:// Биполярный шаговик
		{

			if (CurStepDir ==0) // Если крутим туда
			{
				if ((phaseStepEng >= 8) || (phaseStepEng==0))
				{
					phaseStepEng = 1;
				}
				else
				{
					phaseStepEng = phaseStepEng << 1;
				}
			}
			else         // Если крутим сюда
			{
				if ((phaseStepEng == 1) || (phaseStepEng==0) || (phaseStepEng>8))
				{
					phaseStepEng = 8;
				}
				else
				{
					phaseStepEng = phaseStepEng >> 1;
				}
			}
			//ENG_PORT = phaseStepEng | ENG_PORT & 0xF0;
			ENG_PORT = (phaseStepEng<<4) | ENG_PORT & 0x0F;
			//---------------------------------

		}
		break;

		case 2:  // внешний драйвер.
		if (CurStepDir ==0) // Если крутим туда
		{
			ENG_PORT = 0b00100000 | (ENG_PORT & 0b11001111);	// на первый бит подаём сигнал шаг, на второй направление "туда"
		}
		else
		{
			ENG_PORT = 0b00110000 | (ENG_PORT & 0b11001111);	// на первый бит подаём сигнал шаг, на второй направление "сюда"
		}
		break;
	}
	StateStepEng = ST_ENG_START_STEP;
	StepEngTimer = StepEngPulseLen;


}

// ============================================================================
void FinishStep()
{
	ENG_PORT = ENG_PORT & 0x0F;
	StateStepEng = ST_ENG_FINISH_STEP;

}

// ============================================================================
void StartWindMotor()
{
	flMoveWindMotor = 1;

 //WENG_PORT = WENG_PORT | (1<<WIND_MOTOR_PIN0);
}

// ============================================================================
void MoveWindMotor()
{
uint8_t Temp = 0;

	if (flMoveWindMotor == 1)
	{
		if (WindEngTimer ==0)
		{
			if (WinMotorSpeed < CurMaxWindMotorSpeed)
			{
				WindEngTimer = 125;
				Temp = WinMotorSpeed>>3;
				if (Temp == 0)
				{
					Temp++;
				}
				if ((WinMotorSpeed +  Temp)> CurMaxWindMotorSpeed)
				{
					WinMotorSpeed = CurMaxWindMotorSpeed;
				}
				else
				{
					WinMotorSpeed = WinMotorSpeed +  Temp;
				}
				if (WinMotorSpeed<10)
				{
				WinMotorSpeed = 10;
				}
				OCR1BH = 0;
				OCR1BL = WinMotorSpeed;
			}
		}
		if (WinMotorSpeed > CurMaxWindMotorSpeed)
		{
			WinMotorSpeed = CurMaxWindMotorSpeed;

		OCR1BH = 0;
		OCR1BL = WinMotorSpeed;
		}
	}
	//OCR1BH = 0;
	//OCR1BL = 125;
	//WENG_PORT = WENG_PORT | (1<<WIND_MOTOR_PIN0);
}
// ============================================================================
void StopWindMotor()
{
	//WENG_PORT = ~(1<<WIND_MOTOR_PIN0) & WENG_PORT;
	OCR1BH = 0;
	OCR1BL = 0;
	flMoveWindMotor = 0;
	WinMotorSpeed = 0;
}



// ============================================================================
// Намотка
//
char Winding()
{
	uint16_t PrevCurSum;
	char CurSymb;
	char flStop = 0;
	uint16_t PrevQTurns;
	uint8_t PotentWindMotorSpeed = 0;
	uint8_t PrevPotentValue = 0;
	char WireBreakSensor = 0;

	ProgState = PS_WIND;
	TurnDirEnkoder = 0;

	DStEng = ScrewStep *10 / QStepEng;  // перемещение укладчик аза один шаг двигателя
	DWind =  Step*10 / QPulsVal;	   // изменение ширины намотки за один импульс валкодера
	PrevCurSum = 9999;
	CurSum = 0;
	StateStepEng = 0;
	PrevQTurns = 0;
	StateStepEng = ST_ENG_FINISH_STEP;
	prevStateStepEng = ST_ENG_FINISH_STEP;
	phaseStepEng = 0;

	//char[17] ErrorDescription = "";
	uint8_t flError = 0;


	PrevQTurns = CurQTurns;

	ShowWinding();

	while (1)
	{

		KeyCode = 0;
		EventCode = 0;
		// Индикация режим "Намотка" - мигает буква "W"
		if (BlinkTimer == 0)
		{
			BlinkTimer = 500;
			if (WindingShowSimbol == 'W')
			{
				WindingShowSimbol = ' ';
			}
			else{
				WindingShowSimbol = 'W';
			}
			lcd_gotoxy(15, 0);
			lcd_putc(WindingShowSimbol);
		}

		// опрос клавиатуры
		KeyCode = GetKey();


		if (KeyCode == KEY_STOP)
		{
			if (flStop==0)
			{
				StopTimer = StopTime; // некоторое время, необходимое для остановки намотчик после команды остановки, продолжаем счиать витки и двигать укладчик
			}
			flStop = 1;
		}

		WireBreakSensor = DDRC & (1<<WIRE_BRAKE_SENSOR_PIN) == 0 ? 0: 1;

		if ((WireBreakSensor != WireBreakSensorNormVal) && (WireBreakSensorEnable==1))
		{

			//ShowMessage(char* ErrorDesc);
			flError = 1;
			char *ErrorDescription = &ErorWireBreakDEscrition;
			flStop = 1;
			if (flStop==0)
			{
				StopTimer = StopTime; // некоторое время, необходимое для остановки намотчик после команды остановки, продолжаем счиать витки и двигать укладчик
			}
		}

		if (flStop != 0)
		{
			flMoveWindMotor = 0;
			StopWindMotor();
			if (StopTimer == 0)
			{
				FinishStep();
				ShowWinding();


				if (flError==1)
				{
					ProgState = PS_ERROR;
					//lcd_puts_p(&ErorWireBreakDEscrition);
					ShowMessage(ErorWireBreakDEscrition, 0);
				}
				ProgState = 0;

				return 1;
			}
		}

		else
		{
			if (PrevPotentValue != PotentValue)
			{
				PotentWindMotorSpeed = MaxWindMotorSpeed * PotentValue /255;
			}
			PrevPotentValue  = PotentValue;


			if ((CurQTurnsInLay > QTurnsInLay-5) && (WindMode == WMODE_BY_LAYS) ||(CurQTurns > QTurns-5))
			{
				if (PotentWindMotorSpeed < WindMotorFinishSpeed)
				{
					CurMaxWindMotorSpeed =  PotentWindMotorSpeed;
				}
				else
				{
					CurMaxWindMotorSpeed = WindMotorFinishSpeed;
				}
			}
			else
			{
				CurMaxWindMotorSpeed =  PotentWindMotorSpeed;
				//if (PotentValue < MaxWindMotorSpeed)
				//{
					//CurMaxWindMotorSpeed =  PotentValue;
				//}
				//
				//else
				//{
					//CurMaxWindMotorSpeed = MaxWindMotorSpeed;
				//}
			}

			flMoveWindMotor = 1;
			MoveWindMotor();


		}



		// считаем, отображаем что мы ту намотали
		if (CurQTurns < QTurns)
		{
			//-------------------------------

			if (CurQTurns != PrevQTurns)
			{
				//ShowWinding();

				//continue;
				// --------------------------------------

				if (CurQTurnsInLay<0)
				{
					CurLay--;
					CurQTurnsInLay = CurQTurnsInLay + QTurnsInLay;
				}
				else
				{
					if (PrevPotentValue != PotentValue)
					{
						PotentWindMotorSpeed = MaxWindMotorSpeed * PotentValue /255;
					}
					PrevPotentValue = PotentValue;

					if ((CurQTurnsInLay > QTurnsInLay-5) && (WindMode == WMODE_BY_LAYS) ||(CurQTurns > QTurns-5))
					{
						if (PotentWindMotorSpeed < WindMotorFinishSpeed)
						{
							 CurMaxWindMotorSpeed =  PotentWindMotorSpeed;
						}
						else
						{
							CurMaxWindMotorSpeed = WindMotorFinishSpeed;
						}
					}
					else
					{
						CurMaxWindMotorSpeed =  PotentWindMotorSpeed;
						//if (PotentValue < MaxWindMotorSpeed)
						//{
							//CurMaxWindMotorSpeed =  PotentValue;
						//}
						//
						//else
						//{
							//CurMaxWindMotorSpeed = MaxWindMotorSpeed;
						//}
					}

					if (CurQTurnsInLay >= QTurnsInLay) // Если витков больше или  равно  максимального количества в слое
					{
						if ((WindMode == WMODE_BY_LAYS))// если режим с ожиданием между слоями
						{
							if (flStop==0)
							{
								StopTimer = StopTime;
							}
							flStop = 1;								// останавливаем двигатель намоточный
						}

						CurLay++;	// счетчик слоёв увеличиваем
						CurQTurnsInLay = CurQTurnsInLay - QTurnsInLay; // счетчик количества в слое уменьшаем на макс. количество в слое

						// тут нужно  поменять направление укладчика
						if(WindDir == WDIR_RIGHT)
						{
							WindDir = WDIR_LEFT;
						}
						else
						{
							WindDir = WDIR_RIGHT;
						}
					}
					//else     // если счетчик витков в слое не переполнен
					//{
						//if (CurQTurnsInLay == QTurnsInLay) //последний виток слоя намотан
						//{
							//if ((WindMode == WMODE_BY_LAYS))// если режим с ожиданием между слоями
							//{
								//if (flStop==0)
								//{
									//StopTimer = StopTime;
								//}
								//flStop = 1;								// останавливаем двигатель намоточный
							//}
						//}
					//}
				}
				PrevQTurns = CurQTurns;
				ShowWinding();
			}

		}
		else
		{
			if (flStop==0)
			{
				StopTimer = StopTime;
			}
			flStop = 1;
			ShowWinding();
		}



		// смотрим что намотали, как двигать укладчик
		// ================================================================
		if (StepEngTimer == 0)
		{
			FinishStep();
			if (TurnDirEnkoder==0) // если вал намотчика вращается в правильном направлении
			{
				//if (CurSum != PrevCurSum) // jотладка
				//{
				//ShowWinding();
				//}
				PrevCurSum = CurSum;


				if (CurSum < DWind)
				{

					StartStepEng(WindDir);
					//PrevCurSum = CurSum;
					CurSum = CurSum + DStEng;
					prevStateStepEng = ST_ENG_START_STEP;
				}

				//}
			}
			else  // если вал намотчика вращается в обратном направлении
			{

				PrevCurSum = CurSum;

				if (CurSum > DStEng)
				{
					StartStepEng(1-WindDir); // крутим в обратную сторону
					PrevCurSum = CurSum;
					CurSum = CurSum - DStEng;
					prevStateStepEng = ST_ENG_START_STEP;
				}


			}
		}


		//======================================================================
		//}

	}

	//while (StepEngTimer>0)
	//{
	//}
	FinishStep();
	//ProgState = PS_ERROR;
	//if (flError==1)
	//{
		////lcd_puts_p(&ErorWireBreakDEscrition);
		//ShowMessage(ErorWireBreakDEscrition, 0);
	//}
	ProgState = 0;
	return 1;
}


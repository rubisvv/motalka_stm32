/*
 * flashmem.h
 *
 *  Created on: 13 июн. 2026 г.
 *      Author: rubis
 */

#ifndef INC_FLASHMEM_H_
#define INC_FLASHMEM_H_

enum eprom_adress{
	EEAD_EESCREW_STER,					// Шаг подаюего винта
	EEAD_EEQ_STEP_ENG,					// Количество шагов двигателя укладчика за один оборот
	EEAD_EEQ_PULS_VAL,					// количество импульсов датчика вращения за один оборот подающего винта
	EEAD_EE_STEP_ENG_PULSE_LEN,		// длина импульса шагового двинателя
	EEAD_EE_ENG_TYPE,					// Тип шагового двигателя (0 - униполярный, 1 - биполятрный, 2 - внешний драйвер с интерфейсом "шаг, направление".

	EEAD_EE_STEPPER_DIR_CORRECTION,		// Коррекция направления шагового двигателя
	EEAD_EE_WIRE_BREAK_SENSOR_ENABLE,	// Датчик обрыва провода включен
	EEAD_EE_WIRE_BREAK_SENSOR_NORM_VAL, // нормальное значение датчика обрыва провода
	EEAD_EE_MAX_WIND_MOTOR_SPEED,		// Максимальная скорость
	EEAD_EE_WIND_MOTOR_FINISH_SPEED,	// Скорость
	EEAD_EE_STOP_TIME,

	EEAD_SAVED_STEP,					// шаг укладки (диаметр провода + допуск)
	EEAD_SAVED_WIND_MODE,				// режим намотки (непрерывный или перерыв между слоями)
	EEAD_SAVEDQ_LAYS,					// количество слоёв
	EEAD_SAVED_QTURNS,					// количество витков
	EEAD_SAVED_QTURNS_IN_LAY,			// количество витков в слое
	EEAD_SAVED_CUR_QTURNS,				// текущее количество витков (уже намотано)
	EEAD_SAVED_CUR_QTURNS_IN_LAY,		// текущее количество витков (уже намотано) в текущем слое
	EEAD_SAVED_CUR_LAY,					// текуий слой
	EEAD_SAVED_WIND_DIR,				// направление намотки (укладки)
};

// ========================================================
// Чтение настроек аппаратуры из EEPROM
//------------------------------------------------------
void readHardSetings()
{
	EE_ReadVariable(EEAD_EESCREW_STEP, ScrewStep);		// Шаг подаюего винта
	if (ScrewStep > 9999) ScrewStep = 0;

	EE_ReadVariable(EEAD_EEQ_STEP_ENG, QStepEng);		// Количество шагов двигателя укладчика за один оборот
	if (QStepEng > 999) QStepEng = 0;

	EE_ReadVariable(EEAD_EEQ_PULS_VAL, QPulsVal);		// Количество импульсов датчика вращения за один оборот подающего винта
	if (QPulsVal > 999) QPulsVal = 0;

	EE_ReadVariable(EEAD_EE_STEP_ENG_PULSE_LEN, StepEngPulseLen);	// Длина импульса шагового двинателя
	if (StepEngPulseLen > 999) StepEngPulseLen = 0;

	EE_ReadVariable(EEAD_EE_ENG_TYPE, EngType);		// Тип двигателя укладчика
	if (EngType > 3) EngType = 1;

	EE_ReadVariable(EEAD_EE_STEPPER_DIR_CORRECTION, StepperDirCorrection);	//
	if (StepperDirCorrection > 1) StepperDirCorrection = 0;

	EE_ReadVariable(EEAD_EE_WIRE_BREAK_SENSOR_ENABLE, WireBreakSensorEnable);	//
	if (WireBreakSensorEnable > 1) WireBreakSensorEnable = 0;

	EE_ReadVariable(EEAD_EE_WIRE_BREAK_SENSOR_NORM_VAL, WireBreakSensorNormVal);	//
	if (WireBreakSensorNormVal > 1) WireBreakSensorNormVal = 0;

	EE_ReadVariable(EEAD_EE_MAX_WIND_MOTOR_SPEED, MaxWindMotorSpeed);	//
	if (MaxWindMotorSpeed > 255) MaxWindMotorSpeed = 255;

	EE_ReadVariable(EEAD_EE_WIND_MOTOR_FINISH_SPEED, WindMotorFinishSpeed);	//
	if (WindMotorFinishSpeed > 255) WindMotorFinishSpeed = 255;

	EE_ReadVariable(EEAD_EE_STOP_TIME, StopTime);	//
	if (StopTime > 9999) StopTime = 9999;

}


// ========================================================
// Запись настроек аппаратуры в EEPROM
//------------------------------------------------------
void writeHardSetings()
{




	  EE_WriteVariable(VirtAddVarTab[0], MyVariables[0]);

	eeprom_write_word(&EEScrewStep, ScrewStep);		// Шаг подаюего винта
	eeprom_write_word(&EEQStepEng, QStepEng);		// Количество шагов двигателя укладчика за один оборот
	eeprom_write_word(&EEQPulsVal, QPulsVal);		// количество импульсов датчика вращения за один оборот подающего винта
	eeprom_write_word(&EEStepEngPulseLen, StepEngPulseLen);		// длина импульса шагового двинателя
	eeprom_write_word(&EEStepperDirCorrection, StepperDirCorrection);		// длина импульса шагового двинателя
	eeprom_write_word(&EEWireBreakSensorEnable, WireBreakSensorEnable);		// длина импульса шагового двинателя
	eeprom_write_word(&EEWireBreakSensorNormVal, WireBreakSensorNormVal);		// длина импульса шагового двинателя
	eeprom_write_word(&EEMaxWindMotorSpeed, MaxWindMotorSpeed);
	eeprom_write_word(&EEWindMotorFinishSpeed, WindMotorFinishSpeed);
	eeprom_write_word(&EEStopTime, StopTime);
}


#endif /* INC_FLASHMEM_H_ */

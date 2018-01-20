/*
 * File: Rte_S401_TBox.h
 *
 * Code generated for Simulink model 'S401_TBox'.
 *
 * Model version                  : 1.114
 * Simulink Coder version         : 8.6 (R2014a) 27-Dec-2013
 * C/C++ source code generated on : Wed Oct 19 09:08:52 2016
 *
 * Target selection: autosar.tlc
 * Embedded hardware selection: Generic->32-bit Embedded Processor
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Rte_S401_TBox_h_
#define RTW_HEADER_Rte_S401_TBox_h_

/*
 *
 * This file contains dummy implementations of the AUTOSAR functions
 * used by this model for testing the code generated from Simulink.
 * Simulink invokes the generated code through its S-Function API.
 * Note that this file is replaced with the real implementation
 * when deploying the generated code outside of Simulink.
 *
 */
#include "rtwtypes.h"

//不需要打印log时，注释下面宏定义
#define CHANGAN_DEBUG			
#ifdef CHANGAN_DEBUG 
#define DEBUG_MCUAPP_CA(...)  PRINTF(## __VA_ARGS__)
#else
#define DEBUG_MCUAPP_CA(...)
#endif

typedef void (*RTC_AlarmCBack_t)(void);

typedef enum {
		RTC_CANCEL                     	= 0,
		RTC_SETUP                          = 1
} RTC_SetMode_t;

typedef struct 
{
	uint8_T ConfigTime[6];
	RTC_SetMode_t SetMode;
	RTC_AlarmCBack_t callBackFunc;
} RTC_AlarmCfg_t;


/*  正式版接口 */
/* can read */
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_32A_Buff_CANRX_GW_32A_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_326_Buff_CANRX_GW_326_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_320_Buff_CANRX_GW_320_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_2B0_Buff_CANRX_GW_2B0_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_580_Buff_CANRX_HU_580_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_2F8_Buff_CANRX_HU_2F8_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_PEPS_512_Buff_CANRX_PEPS_512_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_PEPS_266_Buff_CANRX_PEPS_266_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_380_Buff_CANRX_IP_380_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_280_Buff_CANRX_IP_280_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_384_Buff_CANRX_BCM_384_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_347_Buff_CANRX_BCM_347_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_288_Buff_CANRX_BCM_288_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_260_Buff_CANRX_IP_260_Buff(void);

/* can send */
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_296_Buff_CANTX_TBox_296_Buff(uint8_T* u);

/* RTC */
void RTC_SetAlarm(RTC_AlarmCfg_t *config);

/* DID read */
uint8_T* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F210_Buff_EEPROMRX_F210_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F6_Buff_EEPROMRX_F1F6_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F5_Buff_EEPROMRX_F1F5_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F190_Buff_EEPROMRX_F190_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F1_Buff_EEPROMRX_F1F1_Buff(void);
uint8_T Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F0_Buff_EEPROMRX_F1F0_Buff(void);

/* DID write */
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F1F5_Buff_EEPROMTX_F1F5_Buff(uint8_T* u);
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F1F6_Buff_EEPROMTX_F1F6_Buff(uint8_T* u);
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F22A_Buff_EEPROMTX_F22A_Buff(uint8_T  u);
void Rte_IWrite_Fun_TboxLogic_Call_RAMTX_F1FD_Buff_RAMTX_F1FD_Buff(uint8_T u);

/* uart */
uint8_T* Rte_IRead_Fun_TboxLogic_Call_UARTRX_Buff_UARTRX_Buff(void);
void Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(uint8_T* u);

/* other */
uint8_T* Rte_IRead_Fun_TboxLogic_Call_LocalTime_LocalTime(void);

uint32_T Rte_IRead_Fun_TboxLogic_Call_SystemMs_SystemMs(void);
uint8_T Rte_IRead_Fun_TboxLogic_Call_BatteryValue_BatteryValue(void);
uint8_T Rte_IRead_Fun_TboxLogic_Call_CrashSignal_Crashsignal(void);
uint8_T Rte_IRead_Fun_TboxLogic_Call_parkmode_parkmode(void);
uint8_T Rte_IRead_Fun_TboxLogic_Call_WakeUp_Source_WakeUp_Source(void);

void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_SRS_CrashOutputStatus_CANSignal_SRS_CrashOutputStatus(uint8_T u);
void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_SRS_CrashOutputStatusCheckSum_CANSignal_SRS_CrashOutputStatusCheckSum(uint8_T u);
void Rte_IWrite_Fun_TboxLogic_Call_gAppAllowSleepFlag_gAppAllowSleepFlag(uint8_T  u);
void Rte_IWrite_Fun_TboxLogic_Call_Led_ReqControl_Led_ReqControl(uint8_T u);
void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_HU_BcallReq_CANSignal_HU_BcallReq(uint8_T u);

uint8_T* Rte_IRead_Fun_TboxLogic_Call_AppSave_ReadBuff_AppSave_ReadBuff(void);
void Rte_IWrite_Fun_TboxLogic_Call_AppSave_WriteBuff_AppSave_WriteBuff(uint8_T* u);



/*   测试新功能接口  */
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_601_Buff_CANTX_TBox_601_Buff(uint8_T* u);
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_600_Buff_CANTX_TBox_600_Buff(uint8_T* u);
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_6C8_Buff_CANTX_TBox_6C8_Buff(uint8_T* u);
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_6C9_Buff_CANTX_TBox_6C9_Buff(uint8_T* u);
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_Test_6B8_Buff_CANTX_Test_6B8_Buff(uint8_T* u);

uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_AD_6C6_Buff_CANRX_AD_6C6_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_AD_6C7_Buff_CANRX_AD_6C7_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_Test_6B7_Buff_CANRX_Test_6B7_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_Test_6B6_Buff_CANRX_Test_6B6_Buff(void);
uint8_T* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_5FF_Buff_CANRX_BCM_5FF_Buff(void);



#endif                                 /* RTW_HEADER_Rte_S401_TBox_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */

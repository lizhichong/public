#ifndef CAN_APP_MIDDLE_H
#define CAN_APP_MIDDLE_H

#include "Can_config.h"

#define TIMER_INT_CYC           ((int32_t)10u)
#ifndef NULL_PTR
 #define NULL_PTR  ((void *)0)
#endif
typedef enum
{
    CANTX_IDLE = 0u,
    CANTX_WAIT,
    CANTX_ING ,
    CANTX_SUCCEED ,
    CANTX_FAILED,
} CANTxMsg_ETypeDef;

typedef enum
{
	CAN_OK  = 0,
	CAN_BUSY,
	CAN_WAKEUP
} Can_ReturnType;

void CAN_AppInit(void);
void CAN_AppMsgPeriodSend(void);
void CAN_AppMsgTxStateConf(u8 ucPara);
u8 CAN_CheckAppMsgID(u32 id);
void CAN_RxAppMsgPro(can_frame *rxframe);
void CAN_RxCallTypeHandle(Message_t *pMsg);
void CAN_COMRx4GmsgProcess(Message_t *pMsg);

u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_260_Buff_CANRX_IP_260_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_PEPS_266_Buff_CANRX_PEPS_266_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_276_Buff_CANRX_GW_276_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_280_Buff_CANRX_IP_280_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_288_Buff_CANRX_BCM_288_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_2B0_Buff_CANRX_GW_2B0_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_2D8_Buff_CANRX_GW_2D8_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_2E0_Buff_CANRX_GW_2E0_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_2F8_Buff_CANRX_HU_2F8_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_320_Buff_CANRX_GW_320_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_326_Buff_CANRX_GW_326_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_32A_Buff_CANRX_GW_32A_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_AC_330_Buff_CANRX_AC_330_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_AC_340_Buff_CANRX_AC_340_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_347_Buff_CANRX_BCM_347_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_373_Buff_CANRX_GW_373_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_376_Buff_CANRX_GW_376_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_IP_380_Buff_CANRX_IP_380_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_BCM_384_Buff_CANRX_BCM_384_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_3D0_Buff_CANRX_GW_3D0_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_3E0_Buff_CANRX_GW_3E0_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_500_Buff_CANRX_HU_500_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_HU_501_Buff_CANRX_HU_501_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_510_Buff_CANRX_GW_510_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_PEPS_512_Buff_CANRX_PEPS_512_Buff();
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_580_Buff_CANRX_GW_580_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_600_Buff_CANRX_GW_600_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_6B6_Buff_CANRX_GW_6B6_Buff(void);
u8* Rte_IRead_Fun_TboxLogic_Call_CANRX_GW_6B7_Buff_CANRX_GW_6B7_Buff(void);

void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_296_Buff_CANTX_TBox_296_Buff(u8* u);
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_TBox_555_Buff_CANTX_TBox_555_Buff(u8* u);
void Rte_IWrite_Fun_TboxLogic_Call_gTbox_555_SendFlag_gTbox_555_SendFlag(u8 u);
void Rte_IWrite_Fun_TboxLogic_Call_CANTX_Test_6B8_Buff_CANTX_Test_6B8_Buff(u8* u);

u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F1_Buff_EEPROMRX_F1F1_Buff(void);	
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F190_Buff_EEPROMRX_F190_Buff(void);	//VIN
void Rte_IWrite_Fun_TboxLogic_Call_gAppAllowSleepFlag_gAppAllowSleepFlag(u8 u);
void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_SRS_CrashOutputStatus_CANSignal_SRS_CrashOutputStatus(u8 u);
void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_SRS_CrashOutputStatusCheckSum_CANSignal_SRS_CrashOutputStatusCheckSum(u8 u);
u32 Rte_IRead_Fun_TboxLogic_Call_SystemMs_SystemMs(void);
u8* Rte_IRead_Fun_TboxLogic_Call_AppSave_ReadBuff_AppSave_ReadBuff(void);
void Rte_IWrite_Fun_TboxLogic_Call_AppSave_WriteBuff_AppSave_WriteBuff(u8* u);
u8 Rte_IRead_Fun_TboxLogic_Call_BatteryValue_BatteryValue(void);
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F5_Buff_EEPROMRX_F1F5_Buf(void);
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F6_Buff_EEPROMRX_F1F6_Buf(void);
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F1F5_Buff_EEPROMTX_F1F5_Buff(u8* u);
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F1F6_Buff_EEPROMTX_F1F6_Buff(u8* u);
u8* Rte_IRead_Fun_TboxLogic_Call_LocalTime_LocalTime(void);
u8 Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F1F0_Buff_EEPROMRX_F1F0_Buff(void);
u8 Rte_IRead_Fun_TboxLogic_Call_CrashSignal_Crashsignal(void);
u8* Rte_IRead_Fun_TboxLogic_Call_EEPROMRX_F210_Buff_EEPROMRX_F210_Buff(void);
void Rte_IWrite_Fun_TboxLogic_Call_EEPROMTX_F22A_Buff_EEPROMTX_F22A_Buff(u8 u);
void Rte_IWrite_Fun_TboxLogic_Call_RAMTX_F1FD_Buff_RAMTX_F1FD_Buff(u8 u);
void Rte_IWrite_Fun_TboxLogic_Call_Led_ReqControl_Led_ReqControl(u8 u);
u8 Rte_IRead_Fun_TboxLogic_Call_parkmode_parkmode(void);
void Rte_IWrite_Fun_TboxLogic_Call_CANSignal_HU_BcallReq_CANSignal_HU_BcallReq(u8 u);
u8 Rte_IRead_Fun_TboxLogic_Call_WakeUp_Source_WakeUp_Source(void);

u8 *Rte_IRead_Fun_TboxLogic_Call_UARTRX_Buff_UARTRX_Buff(void);
void Rte_IWrite_Fun_TboxLogic_Call_UARTTX_Buff_UARTTX_Buff(u8* u);
#endif

/** File: S401_TBox.h** Code generated for Simulink model 'S401_TBox'.** Model version                  : 1.95* Simulink Coder version         : 8.6 (R2014a) 27-Dec-2013* C/C++ source code generated on : Tue Sep 06 10:38:29 2016** Target selection: autosar.tlc* Embedded hardware selection: Generic->32-bit Embedded Processor* Code generation objectives: Unspecified* Validation result: Not run*/

#ifndef RTW_HEADER_S401_TBox_h_
#define RTW_HEADER_S401_TBox_h_
#ifndef S401_TBox_COMMON_INCLUDES_
# define S401_TBox_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "Rte_S401_TBox.h"
#endif                                 /* S401_TBox_COMMON_INCLUDES_ */




/* Exported entry point functions */
extern void Fun_TboxLogic_Call(void);

/* Model init function */
extern void Runnable_Init(void);

extern void Fun_TboxLogic_WakeUpCall(void);

extern void Fun_TboxLogic_SleepCall(void);


#endif                                 /* RTW_HEADER_S401_TBox_h_ */

/** File trailer for generated code.** [EOF]*/

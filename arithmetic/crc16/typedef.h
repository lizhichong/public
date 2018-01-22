#ifndef     __TBOX_TYPEDEF_H__
#define     __TBOX_TYPEDEF_H__


typedef unsigned char       	u8;
typedef unsigned short      	u16;
typedef unsigned int      	 	u32;
typedef unsigned long long      u64;
typedef signed char         	s8;
typedef signed short       	 	s16;
typedef signed int         	s32;
typedef signed long long        s64;
typedef signed int                 boolean;

typedef struct{
	u16 length;
	u8 *data;
}common_structure;

enum SERVER_TASK_CODE{
    CAN_SERVICE = 1ul<<0,
    MCU_SERVICE = 1ul<<1,
    LOCATION_SERVICE = 1ul<<2,
    TELEPHONY_SERVICE = 1ul<<3,
    VERBOSE_SERVICE = 1ul<<4,
    TCP_SOCKET_SERVICE = 1ul<<5,
    SHORT_MESSAGE_SERVICE = 1ul<<6
};
void release_service_sem(enum SERVER_TASK_CODE);

//#define     TRUE            ((boolean)1)
#define     true                1 //TRUE
//#define     FALSE           ((boolean)0)
#define     false               0 //FALSE
#define     NULL            ((void *)0)


#endif

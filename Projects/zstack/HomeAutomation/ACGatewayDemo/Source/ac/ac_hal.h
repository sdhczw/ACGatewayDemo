/**
******************************************************************************
* @file     ac_hal.h
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief    
******************************************************************************
*/

#ifndef  __AC_HAL_H__ 
#define  __AC_HAL_H__

#include <ac_common.h>
#include <ac_protocol_interface.h>




#define PRINT_MSG_PREFIX		"\1\2\3\4"
#define RCTRL_MSG_PREFIX		"\2\3\4\5"
#define ZIGBEE_INTER_MSG_FLAG   "ZIGB"
#define CLIENT_SERVER_OK  (102)   
#define MSG_SERVER_CLIENT_SET_LED_ONOFF_REQ  (68)

#define AC_Malloc osal_mem_alloc
#define AC_Free osal_mem_free

typedef struct tag_STRU_QUERY_MSG_RSP
{
    u8  MachineType;        //���ӻ�״̬0���ӻ���1������
    u8 sn[8];             //sn
}STRU_QUERY_MSG_RSP;    //��Ϣͷ����

typedef struct
{
    u8 DomainId[AC_DOMAIN_LEN]; //�û�ID������ZC_HS_DEVICE_ID_LEN��8�ֽڣ������豸������Ϣ����������
    u8 DeviceId[AC_HS_DEVICE_ID_LEN];//�û�ID������ZC_HS_DEVICE_ID_LEN��16�ֽڣ������豸id����������
}ZC_SubDeviceInfo;


typedef struct
{
    u8 u8ClientNum;	//���豸��Ŀ
    u8 u8Pad[3];
    ZC_SubDeviceInfo StruSubDeviceInfo[0];
}ZC_SubDeviceList;

typedef struct
{
    u8 u8DeviceOnline;	//Online ?
    u8 u8Pad[3];
}ZC_DeviceOnline;

typedef struct tag_STRU_LED_ONOFF
{		
    u8	     u8LedOnOff ; // 0:�أ�1������2����ȡ��ǰ����״̬
    u8	     u8Pad[3];		 
}STRU_LED_ONOFF;

void AC_DispatchMessage(u8 *pu8Msg, u16 u16DataLen);
void AC_SendMessage(u8 *pu8Msg, u16 u16DataLen);
void AC_TranportMessageToZigbee(u8 *pu8Msg, u16 u16DataLen, AC_OptList *pstruOptList);
void AC_PrintString(u8 *str);
void AC_PrintValue(char *title, u16 value, u8 format);
void AC_DealGateWayMessage(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload);
void AC_DealNotifyMessage(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload);
void AC_DealEvent(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload);
void AC_DealOtaMessage(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload);
#endif

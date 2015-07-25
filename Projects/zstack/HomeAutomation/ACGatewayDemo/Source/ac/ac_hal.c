#include <ac_common.h>
#include <ac_protocol_interface.h>
#include <ac_api.h>
#include <ac_hal.h>
#include "OSAL.h"
#include "OnBoard.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "coordinator.h"
#include "hal_uart.h"
#include "ac_cfg.h"
#include "hal_led.h"
extern devStates_t GenericApp_NwkState;
extern endPointDesc_t GenericApp_epDesc;
extern byte GenericApp_TransID;  // 数据发送序列
extern byte GenericApp_TaskID;   //任务优先级

const u8 g_u8Domain[AC_DOMAIN_LEN]  = {0,0,((u32)MAJOR_DOMAIN_ID>>24)&0xff,((u32)MAJOR_DOMAIN_ID>>16)&0xff,((u32)MAJOR_DOMAIN_ID>>8)&0xff,MAJOR_DOMAIN_ID&0xff,(SUB_DOMAIN_ID>>8)&0xff,SUB_DOMAIN_ID&0xff};

/*************************************************
* Function: AC_RevHexToString
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/

void AC_RevHexToString(u8 *StringBuf,u8* HexBuf,u8 len)
{
  u8 i;
  u8 *xad;

  // Display the extended address.
  xad = HexBuf + len - 1;

  for (i = 0; i < len*2; xad--)
  {
    u8 ch;
    ch = (*xad >> 4) & 0x0F;
    StringBuf[i++] = ch + (( ch < 10 ) ? '0' : '7');
    ch = *xad & 0x0F;
    StringBuf[i++] = ch + (( ch < 10 ) ? '0' : '7');
  }
 // buf[len*2] = '\0';
}
/*************************************************
* Function: AC_SendDeviceRegsiter
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendDeviceRegsiter()
{
    u8 *pu8Msg = osal_mem_alloc( sizeof( AC_RegisterReq ) + sizeof(AC_MessageHead));
    AC_RegisterReq struReg={{0},DEFAULT_IOT_PRIVATE_KEY,{0},{0}};
    u16 u16DataLen;
    AC_RevHexToString(struReg.u8DeviceId,NLME_GetExtAddr(),8);

    memcpy(struReg.u8Domain, g_u8Domain, AC_DOMAIN_LEN); 


    AC_BuildMessage(AC_CODE_REGSITER, 0, 
        (u8*)&struReg, sizeof(AC_RegisterReq),   /*payload+payload len*/
        NULL,
        pu8Msg, &u16DataLen);
    
    AC_SendMessage(pu8Msg, u16DataLen);
    
    osal_mem_free(pu8Msg);
}

/*************************************************
* Function: AC_DealLed
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealLed(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    u8 *pu8Msg = osal_mem_alloc(100);
    u16 u16DataLen;
    u8 test[] = "hello";
    
    if(((STRU_LED_ONOFF *)pu8Playload)->u8LedOnOff<2)
    {
        //SetFanOnOff(((STRU_LED_ONOFF *)pu8Playload)->u8FanOnOff);
        
        HalLedSet(HAL_LED_4,((STRU_LED_ONOFF *)pu8Playload)->u8LedOnOff);
       AC_BuildMessage(CLIENT_SERVER_OK,pstruMsg->MsgId,
                        (u8*)test, 5,
                        pstruOptList, 
                        pu8Msg, &u16DataLen);
        //AC_SendMessage(pu8Msg, u16DataLen);
       AC_TranportMessageToZigbee(pu8Msg, u16DataLen,pstruOptList);
    }
     osal_mem_free(pu8Msg);
}

/*************************************************
* Function: AC_DealEvent
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealEvent(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{   
    
    if (GenericApp_NwkState == DEV_END_DEVICE||GenericApp_NwkState ==DEV_ROUTER)
    {
        //处理设备自定义控制消息
        switch(pstruMsg->MsgCode)
        {
            case MSG_SERVER_CLIENT_SET_LED_ONOFF_REQ:
            {
                AC_DealLed(pstruMsg, pstruOptList, pu8Playload);
                break;
                
            }
        }
    }
    else
    {     
         switch(pstruMsg->MsgCode)
        {
            case MSG_SERVER_CLIENT_SET_LED_ONOFF_REQ:
            {
                 AC_TranportMessageToZigbee((u8 *)pstruMsg, AC_HTONS(pstruMsg->Payloadlen)+sizeof(AC_MessageHead),  pstruOptList);
                 break;
                
            }
            case CLIENT_SERVER_OK:
            {
                 AC_SendMessage((u8 *)pstruMsg, AC_HTONS(pstruMsg->Payloadlen)+sizeof(AC_MessageHead)); 
                 break;
            }
        }
         //透传消息
  
    }
    
}


/*************************************************
* Function: AC_DealGateWayCtrl
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_MgmtPermitJoin(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    zAddrType_t dstAddr; 
    u8 duration = 0;
    dstAddr.addr.shortAddr = 0xfffc;        // all routers (for PermitJoin) devices
    dstAddr.addrMode = AddrBroadcast;
    if(AC_NtoHl(*(u32 *)pu8Playload)>255)
    {
        duration = 0xff;
    }
    else
    {
        duration = AC_NtoHl(*(u32 *)pu8Playload);
    }
    NLME_PermitJoiningRequest(duration);
    ZDP_MgmtPermitJoinReq( &dstAddr, duration, TRUE, FALSE);
    AC_SendAckMsg(pstruOptList,pstruMsg->MsgId);
}

/*************************************************
* Function: AC_LeaveDevice
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_LeaveDevice(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    NLME_LeaveReq_t req;
    ZStatus_t ret;
    req.rejoin = true;
    req.silent = false;
    req.removeChildren= false;
    osal_revmemcpy(req.extAddr,((ZC_SubDeviceInfo *)pu8Playload)->DeviceId+8,Z_EXTADDR_LEN);
    ret = NLME_LeaveReq(&req );
    if(ZSuccess==ret)
    {
        AC_SendAckMsg(pstruOptList,pstruMsg->MsgId);
    }
    else
    {
        AC_SendErrMsg(pstruOptList,pstruMsg->MsgId);
    }
}

/*************************************************
* Function: AC_ListSubDevice
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_ListSubDevices(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    u8 *pu8Msg = osal_mem_alloc(sizeof(AC_MessageHead) + sizeof(ZC_SubDeviceList) + g_struDeviceStatus.num*(sizeof(ZC_SubDeviceInfo)));
    ZC_SubDeviceList *pu8DeviceListInfo = (ZC_SubDeviceList *)(pu8Msg + sizeof(AC_MessageHead));
    ZC_SubDeviceInfo *pu8SubDeviceInfo = (ZC_SubDeviceInfo *)(pu8Msg + sizeof(AC_MessageHead) + sizeof(ZC_SubDeviceList));
    u16 u16DataLen;
    u16 u16PayloadLen;
    u8 i = 0;
    memset(pu8DeviceListInfo,0,sizeof(ZC_SubDeviceList));
    for(i=0;i<g_struDeviceStatus.num;i++)
    {
        if(g_struDeviceStatus.struSubDeviceInfo[i].u8IsOnline)
        {
            osal_memset(pu8SubDeviceInfo,0,sizeof(ZC_SubDeviceInfo));
            osal_memcpy(pu8SubDeviceInfo->DomainId,g_u8Domain,AC_DOMAIN_LEN);
            osal_revmemcpy(pu8SubDeviceInfo->DeviceId + 8,g_struDeviceStatus.struSubDeviceInfo[i].ExtAddr,Z_EXTADDR_LEN);
            pu8SubDeviceInfo++;
            pu8DeviceListInfo->u8ClientNum++;
        }
    }
    u16PayloadLen = sizeof(ZC_SubDeviceList) + pu8DeviceListInfo->u8ClientNum*sizeof(ZC_SubDeviceInfo);
    AC_BuildMessage(AC_CODE_LIST_SUBDEVICES_RSP,pstruMsg->MsgId,
                    (u8*)pu8DeviceListInfo, u16PayloadLen,
                    NULL, 
                    pu8Msg, &u16DataLen);
    AC_SendMessage(pu8Msg, u16DataLen);
    osal_mem_free(pu8Msg);
  //  AC_SendAckMsg(pstruOptList,pstruMsg->MsgId);
    
}

/*************************************************
* Function: AC_GetDeviceStatuss
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_GetDeviceStatus(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    u8 *pu8Msg = osal_mem_alloc(sizeof(AC_MessageHead) + sizeof(ZC_DeviceOnline));
    ZC_DeviceOnline DeviceStatus = {0};
    u16 u16DataLen;
    u8 i = 0;
    for(i=0;i<g_struDeviceStatus.num;i++)
    {
        if(osal_revmemcmp(((ZC_SubDeviceInfo*) pu8Playload)->DeviceId+8,g_struDeviceStatus.struSubDeviceInfo[i].ExtAddr,Z_EXTADDR_LEN))
        {
             if(g_struDeviceStatus.struSubDeviceInfo[i].u8IsOnline)
             {
                DeviceStatus.u8DeviceOnline = 1;
                break;
             }
        }
    }

    AC_BuildMessage(AC_CODE_IS_DEVICEONLINE_RSP,pstruMsg->MsgId,
                    (u8*)&DeviceStatus, sizeof(DeviceStatus),
                    NULL, 
                    pu8Msg, &u16DataLen);
    AC_SendMessage(pu8Msg, u16DataLen);
    osal_mem_free(pu8Msg);


}

/*************************************************
* Function: AC_ConfigWifi
* Description: 
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_ConfigWifi()
{
    AC_Configuration struConfig;
    u16 u16DateLen;
    u8 u8CloudKey[AC_CLOUD_KEY_MAX_LEN] = DEFAULT_IOT_CLOUD_KEY;
    u8 *pu8Msg = osal_mem_alloc(sizeof(AC_MessageHead) + sizeof(AC_Configuration));
    struConfig.u32TraceSwitch = AC_HTONL(0);     //Trace data switch, 1:open, 0:close,default 0
    struConfig.u32SecSwitch = AC_HTONL(0);       //Sec data switch, 1:open, 0:close, 2:close RSA, default 1
    struConfig.u32WifiConfig =  AC_HTONL(0);      //Use Config SSID,password,1:open, 0:close, default 0
    struConfig.u32TestAddrConfig = AC_HTONL(0);      //connect with test url,1:open, 0:close, default 0
    memcpy(struConfig.u8Ssid, "HW_test", 7);
    memcpy(struConfig.u8Password, "87654321", 8);
    struConfig.u32IpAddr = AC_HTONL(0xc0a8c772);  //local test ip
    struConfig.u16Port = AC_HTONS(9100); 
    
    memcpy(struConfig.u8CloudAddr, "test.ablecloud.cn", AC_CLOUD_ADDR_MAX_LEN);
    memcpy(struConfig.u8CloudKey, u8CloudKey, AC_CLOUD_KEY_MAX_LEN);
    
    AC_BuildMessage(AC_CODE_CONFIG, 0, 
                    (u8*)&struConfig, sizeof(AC_Configuration),        /*payload+payload len*/
                    NULL,
                    pu8Msg, &u16DateLen);
    
    AC_SendMessage(pu8Msg, u16DateLen);
    osal_mem_free(pu8Msg);
}
/*************************************************
* Function: AC_DealNotifyMessage
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealNotifyMessage(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    
    //处理wifi模块的通知类消息
    switch(pstruMsg->MsgCode)
    {
        case AC_CODE_EQ_BEGIN://wifi模块启动通知
       // AC_SendDeviceRegsiter();
        //AC_StoreStatus(WIFIPOWERSTATUS , WIFIPOWERON);
        AC_ConfigWifi();
       // UARTprintf("Wifi Power On!\n");
        break;
        case AC_CODE_WIFI_CONNECTED://wifi连接成功通知
        AC_SendDeviceRegsiter();
        //AC_ConfigWifi();
       // AC_SendDeviceRegsiter(NULL, g_u8EqVersion,g_u8ModuleKey,g_u64Domain,g_u8DeviceId);
        break;
        case AC_CODE_CLOUD_CONNECTED://云端连接通知
       // AC_StoreStatus(CLOUDSTATUS,CLOUDCONNECT);
        break;
        case AC_CODE_CLOUD_DISCONNECTED://云端断链通知
        //AC_StoreStatus(CLOUDSTATUS,CLOUDDISCONNECT);
        break;
    }
}

/*************************************************
* Function: AC_DealOtaMessage
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void  AC_DealOtaMessage(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    AC_SendAckMsg(pstruOptList,pstruMsg->MsgId);
}
/*************************************************
* Function: AC_DealGateWayMessage
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DealGateWayMessage(AC_MessageHead *pstruMsg, AC_OptList *pstruOptList, u8 *pu8Playload)
{
    
    //处理wifi模块的通知类消息
    switch(pstruMsg->MsgCode)
    {
        case AC_CODE_GATEWAY_CTRL://wifi模块启动通知
        AC_MgmtPermitJoin(pstruMsg, pstruOptList, pu8Playload);
        //AC_StoreStatus(WIFIPOWERSTATUS , WIFIPOWERON);
       // AC_ConfigWifi();
       // UARTprintf("Wifi Power On!\n");
        break;
        case AC_CODE_LIST_SUBDEVICES_REQ://wifi连接成功通知
        AC_ListSubDevices(pstruMsg, pstruOptList, pu8Playload);
       // AC_SendDeviceRegsiter(NULL, g_u8EqVersion,g_u8ModuleKey,g_u64Domain,g_u8DeviceId);
        break;
        case AC_CODE_IS_DEVICEONLINE_REQ://云端连接通知
        AC_GetDeviceStatus(pstruMsg, pstruOptList, pu8Playload);
        break;
        case AC_CODE_LEAVE_DEVICE://云端断链通知
        AC_LeaveDevice(pstruMsg, pstruOptList, pu8Playload);
        break;
    }
}

/*************************************************
* Function: AC_DispatchMessage
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_DispatchMessage(u8 *pu8Msg, u16 u16DataLen)
{
    if(osal_memcmp(RCTRL_MSG_PREFIX,pu8Msg,4))
    {
        AC_RecvMessage((AC_MessageHead *)(pu8Msg+4));
    }       
}

/*************************************************
* Function: AC_TranportMessage
* Description: 发送云端或模块的AC消息到串口
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_SendMessage(u8 *pu8Msg, u16 u16DataLen)
{   
    HalUARTWrite(0,RCTRL_MSG_PREFIX,4); 
    HalUARTWrite(0,pu8Msg,u16DataLen);
}

/*************************************************
* Function: AC_SendMessage
* Description: 传输AC消息到zigbee
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_TranportMessageToZigbee(u8 *pu8Msg, u16 u16DataLen, AC_OptList *pstruOptList)
{
    afAddrType_t my_DstAddr;//储存发送方的信息
    
    if (GenericApp_NwkState == DEV_END_DEVICE||GenericApp_NwkState ==DEV_ROUTER)
    {
        my_DstAddr.addrMode= (afAddrMode_t)Addr16Bit;//以单播方式发送
        my_DstAddr.endPoint=GENERICAPP_ENDPOINT;//自己端口号   
        my_DstAddr.addr.shortAddr =0x0000;      //要发送的网络地址协调器固定为0x0000
        AF_DataRequest(&my_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       u16DataLen,
                       pu8Msg,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS);
    }
    else
    {
        my_DstAddr.addrMode = (afAddrMode_t)Addr64Bit;//以单播方式发送
        my_DstAddr.endPoint = GENERICAPP_ENDPOINT;//自己端口号   
        osal_revmemcpy(my_DstAddr.addr.extAddr,pstruOptList->pstruTransportInfo->DeviceId+8,Z_EXTADDR_LEN);      //要发送的网络地址协调器固定为0x0000
        AF_DataRequest(&my_DstAddr, &GenericApp_epDesc,
                       GENERICAPP_CLUSTERID,
                       u16DataLen,
                       pu8Msg,
                       &GenericApp_TransID,
                       AF_DISCV_ROUTE,
                       AF_DEFAULT_RADIUS);
    }
    
}
/*************************************************
* Function: AC_CheckCrc
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_PrintString(uint8 *str)
{
//    u8 u8MagicFlag[4] = PRINT_MSG_PREFIX;
//
//    HalUARTWrite(HAL_UART_PORT_1,u8MagicFlag,4); 
    HalUARTWrite(HAL_UART_PORT_1,str,osal_strlen((char*)str));
}

/*************************************************
* Function: AC_PrintValue
* Description: 
* Author: zw 
* Returns: 
* Parameter: 
* History:
*************************************************/
void AC_PrintValue(char *title, uint16 value, uint8 format)
{
    uint8 tmpLen;
    uint8 buf[128];
    u8 u8MagicFlag[4] = PRINT_MSG_PREFIX;
    uint32 err;
    
    tmpLen = (uint8)osal_strlen( (char*)title );
    osal_memcpy( buf, title, tmpLen );
    buf[tmpLen] = ' ';
    err = (uint32)(value);
    _ltoa( err, &buf[tmpLen+1], format );	

//    HalUARTWrite(HAL_UART_PORT_1,u8MagicFlag,4); 
    HalUARTWrite(HAL_UART_PORT_1,buf,osal_strlen((char*)buf));
    HalUARTWrite(HAL_UART_PORT_1,"\r\n",osal_strlen("\r\n"));
}

#include "OSAL.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "SampleAppHw.h"
#include "coordinator.h"
#include "DebugTrace.h"
#include <ac_protocol_interface.h>
#if !defined( WIN32 )
  #include "OnBoard.h"
#endif
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_ezmode.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "mt_uart.h"
#include "OSAL_Nv.h"
#include "ac_hal.h"
#include "ac_api.h"

#define AC_PAYLOADLENOFFSET 9

#define SEND_TO_ALL_EVENT 0x01

#define UART_TO_ALL_EVENT 0x02

#define SUBDEVICE_DETECT_EVENT 0x04

// This list should be filled with Application specific Cluster IDs.
/*���������ʲô���*/

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] =
{
  GENERICAPP_CLUSTERID,
  GENERICHEART_CLUSTERID,
  GENERICACDATA_CLUSTERID
};

aps_Group_t GenericApp_Group;

/*����Ǹ�ʲô�ģ�����һ���豸���������������豸����Ϣ*/
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
  GENERICAPP_ENDPOINT,              //  int Endpoint;
  GENERICAPP_PROFID,                //  uint16 AppProfId[2];
  GENERICAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  GENERICAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                 //  int   AppFlags:4;
  GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList,  //  byte *pAppInClusterList;
  GENERICAPP_MAX_CLUSTERS,//GENERICAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)GenericApp_ClusterList//GenericApp_ClusterList   //  byte *pAppInClusterList;
};

u8 *UARTRxBuf;


const char PureDataPrefix[] = RCTRL_MSG_PREFIX;//ARM DATA

ZIGBEE_HEART_STATUS HeartFlag;
/*������3������*/
endPointDesc_t GenericApp_epDesc;//�ڵ����������á�-t����β�Ķ����¶����Э��ջ����
byte GenericApp_TransID;  // ���ݷ�������
byte GenericApp_TaskID;   //�������ȼ�
devStates_t GenericApp_NwkState;//Э����û�б���ڵ�״̬�ı���

DeviceStatus g_struDeviceStatus;




/*��Ϣ�������������ߵ���յ����ݺ�����������д���*/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
/*��Ϣ���ͺ��������������ߵ緢������ʱ���������*/
void GenericApp_SendTheMessage( void );

void UART_RX(uint8 port, uint8 enent);

void GenericApp_HeartDetect(void);
void AC_DispatchMessage(u8 *pu8Msg, u16 u16DataLen);

void GenericApp_PrintDevInfo(u8* ExtAddr);

void GenericApp_SubDevieDetect();


/*************************************************
* Function: GenericApp_Init
* Description: ��ʼ������
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void GenericApp_Init( byte task_id )
{
  halUARTCfg_t uartConfig;
  
  HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
  HalLedSet(HAL_LED_2,HAL_LED_MODE_OFF);
  HalLedSet(HAL_LED_3,HAL_LED_MODE_OFF);
  
 // MT_UartInit();
 //MT_UartRegisterTaskID(task_id);
 // HalUARTWrite(0,"Hello World\n",12);
  
  GenericApp_TaskID = task_id;//��ʼ���������ȼ����������ȼ���Э��ջ�Ĳ���ϵͳOSAL���䣬�����Լ��ƶ�ô������
  GenericApp_TransID = 0;//���������ݰ������г�ʼ��Ϊ0����Э��ջ��ÿ��һ�����ݰ��������Զ���1���ն˿��Բ鿴���кż��㶪���ʣ���ô����
  GenericApp_NwkState = DEV_INIT;  
  
#if defined ( BUILD_ALL_DEVICES )
  // The "Demo" target is setup to have BUILD_ALL_DEVICES and HOLD_AUTO_START
  // We are looking at a jumper (defined in SampleAppHw.c) to be jumpered
  // together - if they are - we will start up a coordinator. Otherwise,
  // the device will start as a router.
  if ( readCoordinatorJumper() )
  {
   //   zgConfigPANID= 0x7681;
      zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;

  }
  else
  {
      zgDeviceLogicalType = ZG_DEVICETYPE_ENDDEVICE;
  }
#endif // BUILD_ALL_DEVICES
  
#if defined ( HOLD_AUTO_START )
  // HOLD_AUTO_START is a compile option that will surpress ZDApp
  //  from starting the device and wait for the application to
  //  start the device.
  ZDOInitDevice(0);
#endif
  
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc
            = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;//����ȫ���Խڵ���г�ʼ��

  afRegister( &GenericApp_epDesc );//ע��ڵ���������ֻ��ע�����Ժ����ʹ��OSAL�ṩ��ϵͳ����

    // Register for all key events - This app will handle all key events
  RegisterForKeys( GenericApp_TaskID );

  // By default, all devices start out in Group 1
  GenericApp_Group.ID = 0x8888;
  osal_memcpy( GenericApp_Group.name, "AIRBURG", 7  );
  aps_AddGroup( GENERICAPP_ENDPOINT, &GenericApp_Group );
  
  uartConfig.configured  =TRUE;
  uartConfig.baudRate = HAL_UART_BR_115200;
  uartConfig.flowControl = FALSE;
  uartConfig.callBackFunc  = UART_RX;//���ڽ��յ����ݵĻص����������ǽӵ�������һ�ν������
  HalUARTOpen(HAL_UART_PORT_0, &uartConfig);//�򿪴���
  
   uartConfig.tx.maxBufSize        = MT_UART_TX_BUFF_MAX;//uart���ͻ�������С
   uartConfig.baudRate             = HAL_UART_BR_115200;//������ 
 //  uartConfig.callBackFunc         = NpiSerialCallback;//uart���ջص��������ڸú����ж�ȡ����uart����

  // start UART1
  // Note: Assumes no issue opening UART port.
  (void)HalUARTOpen( HAL_UART_PORT_1, &uartConfig );
  // ���������

//  // ���ַ������
  //���Э�����Ƿ����
  HeartFlag = COORD_IS_NOT_EXIT;
  osal_memset(&g_struDeviceStatus,0,sizeof(g_struDeviceStatus));

  {
      extern  uint8 macRadioSetTxPower(uint8 txPower);
      macRadioSetTxPower(3);
  }
//    HalLedSet(HAL_LED_4,1);
  AC_PrintString("Device reset: ");

  // Set the NV startup option to force a "new" join
//  zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
}

/*************************************************
* Function: UART_Receive
* Description: ���ڽ���
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void UART_Receive(u8 ch)
{   
    static u8  UartRxTempBuffer[20];
    static u8  PDMatchNum = 0;
    static u16 ZABodyLen =0;
    static u16 UARTRxIndex = 0;
    static PKT_TYPE cur_type;
    static u8 *msgPtr = UartRxTempBuffer; 


    switch (cur_type)
    {
        
        case PKT_UNKNOWN:
        {  
            
            if (PureDataPrefix[PDMatchNum] == ch)
            {         
                PDMatchNum++;
            }
            else
            {         
                PDMatchNum = 0;
            } 
            if ((PDMatchNum == sizeof(PureDataPrefix)-1))   
            {             
                cur_type = PKT_PUREDATA;           //match case 2:iwpriv ra0  
                osal_memcpy(UartRxTempBuffer,PureDataPrefix,sizeof(PureDataPrefix)-1);
                UARTRxIndex = UARTRxIndex + sizeof(PureDataPrefix)-1;                  
                PDMatchNum = 0;
            }           
            
        }
        break;
        case PKT_PUREDATA:
        {   
            msgPtr[UARTRxIndex++]= ch;
            if(UARTRxIndex==AC_PAYLOADLENOFFSET)
            {
                ZABodyLen = ch;
            }
            else if(UARTRxIndex==(AC_PAYLOADLENOFFSET +1))
            {
                ZABodyLen = ((ZABodyLen<<8) + ch);
                msgPtr = osal_msg_allocate(ZABodyLen + sizeof(RCTRL_STRU_MSGHEAD) +sizeof(osal_event_hdr_t));
                if(msgPtr)
                {
                    osal_memcpy(msgPtr + sizeof(osal_event_hdr_t), UartRxTempBuffer, UARTRxIndex);
                    UARTRxIndex = UARTRxIndex + sizeof(osal_event_hdr_t);
                }
                else
                {
                    ZABodyLen =0;
                    UARTRxIndex = 0;
                    cur_type = PKT_UNKNOWN;
                    msgPtr = UartRxTempBuffer;
                }
            } 
            
            if(UARTRxIndex==ZABodyLen+sizeof(RCTRL_STRU_MSGHEAD) + sizeof(osal_event_hdr_t))
            {                
                ZABodyLen =0;
                UARTRxIndex = 0;  
                ((UartIncomingMSGPacket_t *)msgPtr)->hdr.event = UART_INCOMING_ZAPP_DATA;
                ((UartIncomingMSGPacket_t *)msgPtr)->hdr.status = ZABodyLen +sizeof(RCTRL_STRU_MSGHEAD);
                cur_type = PKT_UNKNOWN;
                osal_msg_send(GenericApp_TaskID, msgPtr);
                msgPtr = UartRxTempBuffer;
            }           
        }
        break;
        default:
        break;
    }
}
/*************************************************
* Function: UART_RX
* Description: ���ڽ���
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
 
static void UART_RX(uint8 port, uint8 event)/*���ַ������ã�����һֵ�ڽ������ݣ�����*/
{
    u8 buffer[MT_UART_RX_BUFF_MAX];
    u8 i= 0;
    uint32 ret =0;
    u8 reallength = 0;
    reallength= HalUARTRead(port,buffer,MT_UART_RX_BUFF_MAX);
    for(i=0;i<reallength;i++)
    {
        UART_Receive(buffer[i]);
          
    }
    if(0!=reallength)
    {
      AC_PrintValue("recv = ",reallength,10);
    }
    return;
}

/*************************************************
* Function: GenericApp_ProcessEvent
* Description: ��Ϣ�����������豸�յ����ݺ�ͻ����������������ǾͿ��Ը��ݽ��յ������ݵ�����Ӧ�Ĵ�������ʽ�ǹ̶��Ĳ��øı�
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
  afIncomingMSGPacket_t *MSGpkt;//������һ����Ϣ�ṹ��ָ��
  uint16 timeout_value = 0;
  zAddrType_t dstAddr;
  
  if ( events & SYS_EVENT_MSG )//�յ���һ����û�д������Ϣ
  {
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );//������Ϣ���൱�ڰ����ݴ�����
      while ( MSGpkt )
      {
          switch ( MSGpkt->hdr.event )//�ж����ݵ�����
          {
              
              case AF_INCOMING_MSG_CMD:
              GenericApp_MessageMSGCB( MSGpkt ); 
              if (GenericApp_NwkState == DEV_ZB_COORD)
              {
                  GenericApp_SendTheMessage();
              }
              else
              {
                  HeartFlag=COORD_IS_EXIT;
              }
              break;//ִ������
              
              case ZDO_STATE_CHANGE://��鵽����ı仯
              GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
              if (GenericApp_NwkState == DEV_ZB_COORD)
              {
                  dstAddr.addr.shortAddr = 0xfffc;        // all routers (for PermitJoin) devices
                  dstAddr.addrMode = AddrBroadcast;
                  u8 time = 0xff;
                  NLME_PermitJoiningRequest(time);
                  ZDP_MgmtPermitJoinReq( &dstAddr, time, TRUE, FALSE);
                  GenericApp_SendTheMessage();
                  timeout_value = 3000;
                  osal_start_timerEx(GenericApp_TaskID,SEND_TO_ALL_EVENT,timeout_value);
                  HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);

                  timeout_value = HEART_DETECT_TIME;
                  osal_start_timerEx(GenericApp_TaskID,SUBDEVICE_DETECT_EVENT,timeout_value);
              }
              else if((GenericApp_NwkState == DEV_ROUTER)||(GenericApp_NwkState ==DEV_END_DEVICE))
              {
                  GenericApp_SendTheMessage();
                  HeartFlag=COORD_IS_EXIT;
                  timeout_value = 3000;
                  osal_start_timerEx(GenericApp_TaskID,SEND_TO_ALL_EVENT,timeout_value);
                  HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
              }
              else
              {
                  HalLedSet(HAL_LED_2,HAL_LED_MODE_OFF);
              }
//GenericApp_SendTheMessage();//��ʼ��������
              break;
              case UART_INCOMING_ZAPP_DATA:
              {
                  AC_DispatchMessage(((UartIncomingMSGPacket_t *)MSGpkt)->Message,((UartIncomingMSGPacket_t *)MSGpkt)->hdr.status);
              }
              break;//ִ������             
              default :
              break;
          }
          
          osal_msg_deallocate( (uint8 *)MSGpkt );//��Ϣ�Ƿ��ڶ��ϵģ��ͷŶ��ڴ棬��ֹ�ڴ�й¶
          
          MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( GenericApp_TaskID );//��Ϣ�������ˣ��ڽ�����Ϣ��ֱ��������Ϣ�������
      }
      
      return (events ^ SYS_EVENT_MSG);//����û�д��������
  }  
  if(events &SEND_TO_ALL_EVENT)//�����������ᴥ����ʱ������ʱ���ڴ�������¼�
  {
      if(GenericApp_NwkState ==DEV_ZB_COORD)
      {
          GenericApp_SendTheMessage();
          timeout_value =3000;
      }
      else
      {
//          timeout_value = 1000;
          GenericApp_HeartDetect();
          timeout_value = 6000;
          GenericApp_SendTheMessage();
      }
      osal_start_timerEx(GenericApp_TaskID,SEND_TO_ALL_EVENT,timeout_value);
      return (events ^ SEND_TO_ALL_EVENT);//����û�д��������
      
  }
    if(events &SUBDEVICE_DETECT_EVENT)//�����������ᴥ����ʱ������ʱ���ڴ�������¼�
  {
       GenericApp_SubDevieDetect();
      timeout_value = HEART_DETECT_TIME;
      osal_start_timerEx(GenericApp_TaskID,SUBDEVICE_DETECT_EVENT,timeout_value);
      return (events ^ SUBDEVICE_DETECT_EVENT);//����û�д��������
      
  }
  return 0;
}

/*************************************************
* Function: GenericApp_MessageMSGCB
* Description: ���ߵ���յ����ݴ�����
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    //afStatus_t  ret= 0;
    unsigned char buffer[4]="";
    unsigned char Rssibuf[10]="\nrss=-";
    uint8 rss = 0;
    rss = 255 - pkt->rssi;
    switch ( pkt->clusterId )
    {
        case GENERICAPP_CLUSTERID:
        {
            AC_RecvMessage((AC_MessageHead *)(pkt->cmd.Data));
        }  
        break;
        case GENERICHEART_CLUSTERID:
        {        
            switch(((ZIGB_SUB_NODE_INFO*)pkt->cmd.Data)->Opcode)
            {
                case ZIGB_REPORT_SUBNODE_ADDR_REQ:
                {
                    u8 i = 0;
                    for(i=0;i<g_struDeviceStatus.num;i++)
                    {
                        if(osal_memcmp(((ZIGB_SUB_NODE_INFO*)pkt->cmd.Data)->ExtAddr,g_struDeviceStatus.struSubDeviceInfo[i].ExtAddr,Z_EXTADDR_LEN))
                        {
                            break;
                        }
                    }
                    if((i==g_struDeviceStatus.num)&&(g_struDeviceStatus.num<NWK_MAX_DEVICES))
                    {
                        osal_memcpy(g_struDeviceStatus.struSubDeviceInfo[i].ExtAddr,((ZIGB_SUB_NODE_INFO*)pkt->cmd.Data)->ExtAddr,Z_EXTADDR_LEN);
                        g_struDeviceStatus.num++;
                        AC_PrintString("Device Join: ");
                        GenericApp_PrintDevInfo(g_struDeviceStatus.struSubDeviceInfo[i].ExtAddr);
                    }
                    g_struDeviceStatus.struSubDeviceInfo[i].u8IsOnline = 1;
                    g_struDeviceStatus.struSubDeviceInfo[i].u8Flag = 1;
                    g_struDeviceStatus.struSubDeviceInfo[i].u16nwk = ((ZIGB_SUB_NODE_INFO*)pkt->cmd.Data)->nwk;
                }
                break;
                case ZIGB_HEART_STATUS_REQ:
                {
                    
                    HeartFlag=COORD_IS_EXIT;
                    HalLedBlink(HAL_LED_1,1,50,100);
                }
                break;
            }
            AC_PrintValue("rss=-",rss,10);
            
        }  
        break;
        case GENERICACDATA_CLUSTERID:
        {
            AC_DispatchMessage(pkt->cmd.Data,pkt->cmd.DataLength);
        }
        break;
    }
    
}

/*************************************************
* Function: GenericApp_SendTheMessage
* Description: ��������,��������Ǳ��ĵĹؼ�֮��
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void GenericApp_SendTheMessage(void)
{
    afStatus_t ret= afStatus_SUCCESS;
    unsigned char data=ZIGB_HEART_STATUS_REQ;//ӦΪЭ����Ҫ����"led"
    ZIGB_SUB_NODE_INFO sub_node_info;
    afAddrType_t my_DstAddr;//���淢�ͷ�����Ϣ
    
    if (GenericApp_NwkState == DEV_END_DEVICE||GenericApp_NwkState ==DEV_ROUTER)
    {
        my_DstAddr.addrMode= (afAddrMode_t)Addr16Bit;//�Ե�����ʽ����
        my_DstAddr.endPoint=GENERICAPP_ENDPOINT;//�Լ��˿ں�
        sub_node_info.Msgflag=0x76;
        sub_node_info.Opcode=0x00;
        sub_node_info.nwk=NLME_GetShortAddr();
        osal_memcpy(sub_node_info.ExtAddr,NLME_GetExtAddr(),8);
        my_DstAddr.addr.shortAddr =0x0000;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        ret=AF_DataRequest(&my_DstAddr, &GenericApp_epDesc,
                           GENERICHEART_CLUSTERID,
                           sizeof(sub_node_info),
                           (unsigned char*)&sub_node_info,
                           &GenericApp_TransID,
                           AF_DISCV_ROUTE,
                           AF_DEFAULT_RADIUS);
        HeartFlag=COORD_IS_NOT_EXIT;
    }
    else
    {
//        static u16 i= 0;
//        data = i++;
        my_DstAddr.addrMode= (afAddrMode_t)AddrBroadcast;//�Ե�����ʽ����
        my_DstAddr.addr.shortAddr =0xffff;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        my_DstAddr.endPoint=GENERICAPP_ENDPOINT;//�Լ��˿ں�
        ret=AF_DataRequest(&my_DstAddr, &GenericApp_epDesc,
                           GENERICHEART_CLUSTERID,
                           1,
                           &data,
                           &GenericApp_TransID,
                           AF_DISCV_ROUTE,
                           AF_DEFAULT_RADIUS);
        //        my_DstAddr.addrMode= (afAddrMode_t)Addr64Bit;//�Ե�����ʽ����
        //        my_DstAddr.addr.extAddr[7] =0x00;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        //        my_DstAddr.addr.extAddr[6] =0x12;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        //        my_DstAddr.addr.extAddr[5] =0x4b;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        //        my_DstAddr.addr.extAddr[4] =0x00;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        //        my_DstAddr.addr.extAddr[3] =0x04;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        //        my_DstAddr.addr.extAddr[2] =0xef;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        //        my_DstAddr.addr.extAddr[1] =0x78;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        //        my_DstAddr.addr.extAddr[0] =0x67;      //Ҫ���͵������ַЭ�����̶�Ϊ0x0000
        //        my_DstAddr.endPoint=GENERICAPP_ENDPOINT;//�Լ��˿ں�
        //        ret=AF_DataRequest(&my_DstAddr, &GenericApp_epDesc,
        //                           GENERICHEART_CLUSTERID,
        //                           1,
        //                           &data,
        //                           &GenericApp_TransID,
        //                           AF_DISCV_ROUTE,
        //                           AF_DEFAULT_RADIUS);    
        
    }
    
    HalLedBlink(HAL_LED_2,1,50,100);
    
}

/*************************************************
* Function: GenericApp_HeartDetect
* Description: �������
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void GenericApp_HeartDetect()
{
    if((HeartFlag==COORD_IS_NOT_EXIT))
    {
//        AC_PrintString("Coordinator is not exit,reset!");
        Onboard_soft_reset();
    }
    else
    {
        HeartFlag=COORD_IS_NOT_EXIT;
    }
}

/*************************************************
* Function: GenericApp_SubDevieDetect
* Description: ���豸���
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
void GenericApp_SubDevieDetect()
{
    u8 i = 0;
    for(i=0;i<g_struDeviceStatus.num;i++)
    {
        if(1==g_struDeviceStatus.struSubDeviceInfo[i].u8Flag)
        {
            g_struDeviceStatus.struSubDeviceInfo[i].u8Flag = 0;
            g_struDeviceStatus.struSubDeviceInfo[i].u8IsOnline = 1;
        }
        else
        {
            g_struDeviceStatus.struSubDeviceInfo[i].u8IsOnline = 0; 
            AC_PrintString("Device Offline: ");
            GenericApp_PrintDevInfo(g_struDeviceStatus.struSubDeviceInfo[i].ExtAddr);
        }
    }

}

/*************************************************
* Function: GenericApp_SubDevieDetect
* Description: ��ӡ���豸��Ϣ
* Author: cxy 
* Returns: 
* Parameter: 
* History:
*************************************************/
static void GenericApp_PrintDevInfo(u8* ExtAddr)
{
  uint8 i;
  uint8 *xad;
  uint8 buf[Z_EXTADDR_LEN*2+1];

  // Display the extended address.
  xad = ExtAddr + Z_EXTADDR_LEN - 1;

  for (i = 0; i < Z_EXTADDR_LEN*2; xad--)
  {
    uint8 ch;
    ch = (*xad >> 4) & 0x0F;
    buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
    ch = *xad & 0x0F;
    buf[i++] = ch + (( ch < 10 ) ? '0' : '7');
  }
  buf[Z_EXTADDR_LEN*2] = '\0';
  AC_PrintString("SUB Device IEEE: ");
  AC_PrintString(buf);
  AC_PrintString("\r\n");
}



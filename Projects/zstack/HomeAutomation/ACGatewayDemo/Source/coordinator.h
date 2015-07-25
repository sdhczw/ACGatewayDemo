#ifndef _COORDINATOR_H_
#define _COOEDINATOR_H_
#include <ac_common.h>
#include "ZComDef.h"
//#include "bmd.h"

#define ZNP_SPI_RX_AREQ_EVENT     0x4000
#define ZNP_SPI_RX_SREQ_EVENT     0x2000
#define ZNP_UART_TX_READY_EVENT   0x1000
#define ZNP_PWRMGR_CONSERVE_EVENT 0x0080

#define GENERICAPP_ENDPOINT           10

#define GENERICAPP_PROFID             0x0F04
#define GENERICAPP_DEVICEID           0x0001
#define GENERICAPP_DEVICE_VERSION     0
#define GENERICAPP_FLAGS              0

#define GENERICAPP_MAX_CLUSTERS       3
#define GENERICAPP_CLUSTERID          1
#define GENERICHEART_CLUSTERID        2
#define GENERICACDATA_CLUSTERID       3


#define RCTRL_MSG_FLAG		         0x76814350

//how many packets can be buffered in rxring ,(each packet size need < UARTRX_RING_LEN - 1)
#define    NUM_DESCS           30

//buffer length for uart rx buffer whose data is moved from uart UART HW RX FIFO
#define    UART1RX_RING_LEN    512 

#define     PREFIXLEN 4

#define    HEART_DETECT_TIME ((u32)12*1000)  /*heart detect time 1min */     


#define SEND_TO_ALL_EVENT 0x01

#define UART_TO_ALL_EVENT 0x02

typedef enum {
    PKT_UNKNOWN,
    PKT_ATCMD,
    PKT_PUREDATA,
    PKT_ZADATA,
    PKT_PRINTCMD,
    PKT_HR01DATA
} PKT_TYPE;

typedef enum
{
    MSG_CLIENT_WIFI_EQ_BEGIN = 0x0,
    MSG_WIFI_CLIENT_EQ_DONE= 0x1,
    MSG_WIFI_CONNECT = 0x2,
    MSG_WIFI_DISCONNECT = 0x3,
    MSG_CLOUD_CONNECT = 0x4,
    MSG_CLOUD_DISCONNECT = 0x5,
    MSG_LOCAL_HANDSHAKE= 0x6,
    MSG_CLIENT_SERVER_REGISTER_REQ = 0x7,
    MSG_SERVER_CLIENT_REGISTER_RSP = 0x8,
    MSG_CLIENT_SERVER_ACK = 0xf,
    MSG_CLIENT_SERVER_ERR = 0x10,
    MSG_CLIENT_SERVER_ACCESS_REQ = 0x1c,
    MSG_SERVER_CLIENT_ACCESS_RSP = 0x1d,
    MSG_SERVER_CLIENT_ADD_SUBNODE_REQ,  
    MSG_SERVER_CLIENT_BASE = 0x40,
    MSG_SERVER_CLIENT_SET_NET_ACCESS_REQ = 0x41,
    MSG_SERVER_CLIENT_SET_FAN_SPEED_REQ = 0x43,
    MSG_SERVER_CLIENT_GET_DEV_STATUS_REQ = 0x45,
    MSG_CLIENT_SERVER_GET_DEV_STATUS_RSP = 0x1045,
    MSG_SERVER_CLIENT_SET_REPORT_PERIOD_REQ = 0x46,
    MSG_SERVER_CLIENT_SET_REPORT_TH_REQ   = 0x47,
    MSG_SERVER_CLIENT_GET_VER_STATUS_REQ =0x48,
    MSG_SERVER_CLIENT_GET_VER_STATUS_RSP = 0x1048,
}RCTRL_MSG_TYPE;

typedef enum
{
    ZIGB_REPORT_SUBNODE_ADDR_REQ = 0,
    ZIGB_HEART_STATUS_REQ = 0xff
}ZIGBEE_MSG_TYPE;

typedef enum
{
	COORD_IS_NOT_EXIT = 0,
	COORD_IS_EXIT = 1,
}ZIGBEE_HEART_STATUS;

typedef struct tag_ARM_ZIGB_MSG_HEAD
{
    uint16 MsgType;        //消息类型
    uint16 MsgLen;
}ARM_ZIGB_MSG_HEAD;    //消息头定义


typedef struct tag_SET_NET_ACCESS_REQ
{
    s8 MsgFlag[4];         //消息标示 byte0:0X76, byte1:0x81, byte2:0x43, byte3:0x50 0x76814350
    ARM_ZIGB_MSG_HEAD MsgHead;
    u8 u8AccessTime; //单位：s 0，关闭，ff打开，中间值是时间
}ARM_ZIGB_SET_ACCESS_TIMER_REQ;    //消息头定义

typedef struct ZIGB_SUB_NODE_INFO
{
    uint8 Opcode;       //消息标示 byte0:0X76, byte1:0x00
    uint8 Msgflag;        //消息标示 byte0:0X76 
    uint16 nwk;         //消息标示 byte0:0X76, byte1:0x81, byte2:0x43, byte3:0x50 0x76814350
    uint8 ExtAddr[8];             //sn
}ZIGB_SUB_NODE_INFO;    //消息定义

typedef struct ZIGB_MAIN_NODE_INFO
{
    uint8 Msgflag;        //消息标示 byte0:0X76 
    uint8 Opcode;       //消息标示  byte1:0x01
    uint16 nwk;         //消息标示 byte0:0X76, byte1:0x81, byte2:0x43, byte3:0x50 0x76814350
    uint8 ExtAddr[Z_EXTADDR_LEN];             //sn
}ZIGB_MAIN_NODE_INFO;    //消息定义

typedef struct
{
    u8  Version;
    u8  MsgId;        //消息ID  
    u8  MsgCode;		//消息类型
    u8  OptNum;
    
    u16 Payloadlen;   //msg payload len + opt len + opt head len
    u8  TotalMsgCrc[2];
    
}ZC_MessageHead;


typedef struct tag_RCTRL_STRU_MSGHEAD
{
    u32 MsgFlag;         //消息标示 byte0:0X76, byte1:0x81, byte2:0x43, byte3:0x50 0x76814350
    ZC_MessageHead  struMessageHead;
}RCTRL_STRU_MSGHEAD;    //消息头定义

typedef struct
{
    u8 u8ClientNum;			//用户数目
    u8 u8Pad[3];
    u8 DeviceId[8];             //用户ID，定长ZC_HS_DEVICE_ID_LEN（12字节），依次下排
}ZC_ClientAccessInfo;


typedef struct tag_STRU_CLIENT_SERVER_REPORT_CLIENT_INFO
{
    RCTRL_STRU_MSGHEAD	struMsgHeader;	
//    ZC_TransportInfo    struTansportInfo;	
    ZC_ClientAccessInfo  struClientAccessInfo;		//  
}STRU_CLIENT_SERVER_REPORT_CLIENT_INFO;

typedef struct
{
  osal_event_hdr_t hdr; 
  u8 Message[0];  
} UartIncomingMSGPacket_t;

typedef struct
{
    u16 u16nwk;         //消息标示 byte0:0X76, byte1:0x81, byte2:0x43, byte3:0x50 0x76814350
    u8 ExtAddr[Z_EXTADDR_LEN];             //sn
    u8 u8IsOnline;
    u8 u8Flag;
}SubDeviceInfo;

 typedef struct
{
    SubDeviceInfo struSubDeviceInfo[NWK_MAX_DEVICES/2];
    u8 num;
}DeviceStatus;
    
extern void GenericApp_Init( byte task_id );

extern UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events );

extern byte GenericApp_TaskID;   //任务优先级

extern DeviceStatus g_struDeviceStatus;
#endif
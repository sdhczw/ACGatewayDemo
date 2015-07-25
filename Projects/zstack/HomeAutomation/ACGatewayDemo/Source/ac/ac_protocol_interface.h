/*******************************************************************************
* @file     ac_protocol_interface.h
* @authors  cxy
* @version  V1.0.0
* @date     10-Sep-2014
* @brief    Security module
******************************************************************************
*/
#ifndef  __AC_PROTOCOL_INTERFACE_H__ 
#define  __AC_PROTOCOL_INTERFACE_H__
#include <ac_common.h>

#define AC_HS_MSG_LEN                       (32)
#define AC_HS_DEVICE_ID_LEN                 (16)
#define AC_HS_SESSION_KEY_LEN               (16)
#define AC_HS_SESSION_IV_LEN                (16)
#define AC_MODULE_KEY_LEN                   (112)
#define AC_DOMAIN_LEN                       (8)
#define AC_EQVERSION_LEN                    (4)


#define AC_SEC_ALG_NONE                     (0) 
#define AC_SEC_ALG_RSA                      (1) 
#define AC_SEC_ALG_AES                      (2) 


#define AC_OTA_MAX_CHUNK_LEN                (384)
#define AC_SSID_MAX_LEN                     (32)

#define AC_PASSWORD_MAX_LEN                 (64)

#define AC_CLOUD_ADDR_MAX_LEN               (20)

#define AC_CLOUD_KEY_MAX_LEN                (36)
/****************************************************************************************
*message format: 
*|AC_MessageHead||AC_MessageOptHead||AC_MessageOption|.......|AC_MessagePayload|
*****************************************************************************************/

/*ACloud Message*/
typedef struct
{
    u8  Version;
    u8  MsgId;
    u8  MsgCode;
    u8  OptNum;        
    
    u16 Payloadlen;     //msg payload len + opt len + opt head len
    u8  TotalMsgCrc[2];
    
}AC_MessageHead;

/*ACloud Option Head*/
typedef struct
{
    u16 OptCode;
    u16 OptLen;
}AC_MessageOptHead;

/*******************************Message and Option opcode***********************************/

/*ACloud Message code*/
typedef enum 
{
    AC_CODE_EQ_BEGIN = 0,
    AC_CODE_EQ_DONE,
    AC_CODE_WIFI_CONNECTED,
    AC_CODE_WIFI_DISCONNECTED,
    AC_CODE_CLOUD_CONNECTED,
    AC_CODE_CLOUD_DISCONNECTED,
    AC_CODE_CONFIG,
    AC_CODE_REGSITER,                 
    AC_CODE_REST,                

    AC_CODE_ACK = 15,         /*user define payload*/
    AC_CODE_ERR,         /*use AC_ErrorMsg*/

    /*OTA Code*/
    AC_CODE_OTA_BEGIN,
    AC_CODE_OTA_FILE_BEGIN,      /*file name, len, version*/
    AC_CODE_OTA_FILE_CHUNK,
    AC_CODE_OTA_FILE_END,
    AC_CODE_OTA_END,

    /*GATEWAY Code*/
    AC_CODE_GATEWAY_CTRL      = 45,
    AC_CODE_LIST_SUBDEVICES_REQ   = 46,
    AC_CODE_LIST_SUBDEVICES_RSP   = 47,
    AC_CODE_IS_DEVICEONLINE_REQ   = 48,
    AC_CODE_IS_DEVICEONLINE_RSP   = 49,
    AC_CODE_LEAVE_DEVICE      = 50,
    /*Custom Code*/   
    AC_CODE_EVENT_BASE = 64,
}AC_MsgCode;

typedef enum 
{
    AC_OPT_TRANSPORT = 0,
    AC_OPT_SSESSION  = 1,
}AC_OptCode;

/*******************************Message definition***********************************/

/*Error Msg*/
typedef struct{
    u8 ErrorCode;
    //u8 ErrorMsg[0];
}AC_ErrorMsg;



/*msg code: AC_CODE_DESCRIBE*/
typedef struct 
{
    u8  u8EqVersion[AC_EQVERSION_LEN];      //eq vision
    u8  u8ModuleKey[AC_MODULE_KEY_LEN];
    u8  u8Domain[AC_DOMAIN_LEN];
    u8 u8DeviceId[AC_HS_DEVICE_ID_LEN];
}AC_RegisterReq;

typedef struct
{
    u32 u32TraceSwitch;     //Trace data switch, 1:open, 0:close,default 0
    u32 u32SecSwitch;       //Sec data switch, 1:open, 0:close, 2:close RSA, default 1
    u32 u32WifiConfig;      //Use Config SSID,password,1:open, 0:close, default 0
    u32 u32TestAddrConfig;  //connect with test url,0:open, 1:"test.ablecloud.cn", 2:use u32IpAddr, default 0
    u8  u8Ssid[AC_SSID_MAX_LEN];
    u8  u8Password[AC_PASSWORD_MAX_LEN];
    u32 u32IpAddr;
    u8  u8CloudAddr[AC_CLOUD_ADDR_MAX_LEN];
    u8  u8CloudKey[AC_CLOUD_KEY_MAX_LEN];
    u16 u16Port;
    u16 u16Pad;
}AC_Configuration;

/*msg code: AC_CODE_OTA_BEGIN*/
typedef struct
{
    u8 u8FileNum;
    u8 u8Pad[3];
    //u8 u8FileType[0];
}AC_OtaBeginReq;

/*msg code: AC_CODE_OTA_FILE_BEGIN*/
typedef struct
{
    u8 u8FileType;
    u8 u8FileVersion;
    u8  u8TotalFileCrc[2];
    
    u32 u32FileTotalLen;
}AC_OtaFileBeginReq;

/*msg code: AC_CODE_OTA_FILE_CHUNK, max data len is AC_OTA_MAX_CHUNK_LEN*/
typedef struct
{
    u32 u32Offset;
}AC_OtaFileChunkReq;

/******************************* Option definition***********************************/
/*AC_OPT_TRANSPORT*/
typedef struct
{
    u8 Domain[AC_DOMAIN_LEN];
    u8 DeviceId[AC_HS_DEVICE_ID_LEN];
}AC_TransportInfo;

/*AC_OPT_SSESSION*/
typedef struct{
    u32 u32SsessionId;
}AC_SsessionInfo;

typedef struct 
{
    AC_TransportInfo *pstruTransportInfo;
    AC_SsessionInfo *pstruSsession;
    
}AC_OptList;

#endif
/******************************* FILE END ***********************************/


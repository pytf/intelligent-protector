/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

  Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : CFTExcetpionHandle.h
  Version       : Initial Draft
  Author        : 
  Created       : 2014/11/19
  Last Modified :
  Description   : �Խⶳ/��������쳣���д���
  History       :
  1.Date        : 2015/07/27
    Author      : 
    Modification: ��д�򻯶��������ر������̣�����һ�����в��Ҳ���״̬����ʽ�Լ�ض�����д���
******************************************************************************/

#ifndef _FT_EXCEPTION_HANDLE_
#define _FT_EXCEPTION_HANDLE_

#include <vector>
#include "rest/MessageProcess.h"
#include "common/Thread.h"
#include "agent/Communication.h"

#define MAX_MONITOR_TIME            3600 * 6        //���������ȴ�6Сʱ,��λ��
#define SLEEP_TIME                  3 * 1000        //�߳���ѯ����ʱ�䣬��λ����
#define THAW_WAIT_TIME              60              //�ⶳ�ȴ�ʱ��
#define MAX_GET_TIME                9000            //1����
#define MAX_PRIVATE_KEY_NUM         2
#define DELAY_UNFREEZE_TIME         600             //�����¼ӵ�����б��еĶ��������ӳ�10���ӽⶳ��oracleģ�鶳�����Ŀǰ��5���ӳ�ʱ
                                                    //(IO��æʱ��������ִ��ʱ��ܳ�����ʽͳ�ƽⶳʱ����Ҫ�Ӷ���������Ӧ��ʼ����)

const mp_string INST_NAME           = "instName";   //���ݿ�ʵ������
const mp_string INST_NUM            = "instNum";    //HANA���ݿ��ʵ�����Ǳ��
const mp_string APP_TYPE            = "appType";    //Ӧ�����ͣ��ο�APP_TYPE_E
const mp_string FLUSH_TYPE          = "frushType";  //�������ͣ��ο�FLUSH_TYPE_E
const mp_string VSS_INSTANCE_NAME   = "VSSinstName";
const mp_string DBNAME              = "dbName";
const mp_string VSS_DB_NAME         = "VSSdbName";
const mp_string FILESYTEM_DB_NAME   = "FiledbName";
const mp_string DISKNAMES           = "diskNames";
const mp_string LOOP_TIME           = "loop_time";
const mp_string ERROR_CODE          = "error";
const mp_string DATA                = "data";
const mp_string DB_FREEZE_STATUS    = "state";
const mp_string ORACLE              = "oracle";
const mp_string DB2                 = "db2";
const mp_string SQL                 = "sql";
const mp_string EXCHANGE            = "exchange";
const mp_string SYBASE              = "sybase";
const mp_string HANA                = "hana";
const mp_string FREEZE              = "freeze";
const mp_string UNFREEZE            = "unfreeze";
const mp_string FILESYSTEM          = "filesystems";
const mp_string THIRDPARTY          = "thirdparty";
const mp_string APP                 = "app";
const mp_string APPDBNAME           = "AppdbName";
const mp_string APPINSTNAME         = "AppInstName";

enum FLUSH_TYPE_E
{
    FLUSH_TYPE_FREEZE = 0,
    FLUSH_TYPE_THAW
};

enum APP_TYPE_E
{
    TYPE_APP_DB2 = 0,
    TYPE_APP_ORACLE,
    TYPE_APP_SQL,
    TYPE_APP_EXCHANGE,
    TYPE_APP_FILESYSTEM,
    TYPE_APP_THIRDPARTY,
    TYPE_APP_APP,
    TYPE_APP_SYBASE,
    TYPE_APP_HANA,
    TYPE_APP_UNKNOWN
};

enum MONITOR_OBJ_STATUS_E
{
    MONITOR_STATUS_FREEZED = 0,      //����ɹ�������ʧ��(�������ն���ӿڷ���ʧ��ʵ�ʺ����ɹ�)��δ֪״̬ͳһ�ɸ�״̬��ʾ
    MONITOR_STATUS_UNFREEZING,       //���·��ⶳ����
    MONITOR_STATUS_GETSTATUSING      //���·���ѯ״̬����
};

typedef struct stMONITOR_OBJ
{
    CRequestMsg* pReqMsg;      //������Ϣ
    mp_uint32 uiStatus;        //��ض���״̬
    mp_uint64 ulBeginTime;     //��ؿ�ʼʱ��
    mp_string strInstanceName;
    mp_string strDBName;
    mp_int32 iAppType;
    mp_uint32 uiLoopTime;
}MONITOR_OBJ;

class CFTExceptionHandle
{
private:
    static CFTExceptionHandle m_Instance;
    vector<MONITOR_OBJ> m_vecMonitors;         //��ض������
    thread_lock_t m_tMonitorsLock;             //��ض�����
    thread_id_t m_hHandleThread;               //�����߳�id
    mp_uint32 m_iCurrIndex;                    //��ǰ���ڴ���ļ�ض�������
    volatile mp_int32 m_iThreadStatus;         //����߳�״̬
    volatile mp_bool m_bNeedExit;              //�߳��˳���ʶ
    
public:
    static CFTExceptionHandle& GetInstance()
    {
        return m_Instance;
    }

    mp_int32 Init();
    mp_void WaitForExit();
    mp_void MonitorFreezeOper(CRequestMsg* pReqMsg);
    mp_void UpdateFreezeOper(CRequestMsg* pReqMsg, CResponseMsg* pRspMsg);
    ~CFTExceptionHandle()
    {
        CMpThread::DestroyLock(&m_tMonitorsLock);
    }
    
private:
    CFTExceptionHandle()
    {
        (mp_void)memset_s(&m_hHandleThread, sizeof(m_hHandleThread), 0, sizeof(m_hHandleThread));
        //Coverity&Fortify��:UNINIT_CTOR
        //Coveirty&Fortify����ʶ��˾��ȫ����memset_s����ʾm_dispatchTid.os_idδ��ʼ��
        CMpThread::InitLock(&m_tMonitorsLock);
        m_iThreadStatus = THREAD_STATUS_IDLE;
        m_bNeedExit = MP_FALSE;
        m_iCurrIndex = 0;
    }

    //����̻߳ص�����
#ifdef WIN32
    static DWORD WINAPI HandleMonitorObjsProc(LPVOID param);
#else
    static mp_void* HandleMonitorObjsProc(mp_void* param);
#endif
    mp_void SetThreadStatus(mp_int32 iThreadStatus);
    mp_int32 GetThreadStatus();
    thread_lock_t& GetThreadLock();
    mp_bool NeedExit();
    //�����������еĶ���
    mp_int32 ProcessInternalRsps();
    mp_int32 ProccessUnFreezeRsp(CRequestMsg* pReqMsg, CResponseMsg* pRspMsg);
    mp_int32 ProcessQueryStatusRsp(CRequestMsg* pReqMsg, CResponseMsg* pRspMsg);
    mp_void HandleMonitorObjs();
    mp_int32 HandleUnFreezingMonitorObj(MONITOR_OBJ* pMonitorObj, mp_bool bIsUnFreezeSucc);
    mp_int32 HandleQueryStatusMonitorObj(MONITOR_OBJ* pMonitorObj, mp_int32 iQueryStatus);
    mp_int32 HandleFreezedMonitorObj(MONITOR_OBJ* pMonitorObj);
    mp_int32 PushUnFreezeReq(MONITOR_OBJ* pMonitorObj);
    mp_int32 PushQueryStatusReq(MONITOR_OBJ* pMonitorObj);
    mp_void AddMonitorObjs(vector<MONITOR_OBJ>& vecMonitorObjs);  
    mp_int32 AddMonitorObj(MONITOR_OBJ& monitorObj);
    MONITOR_OBJ* GetHandleMonitorObj();
    MONITOR_OBJ* GetMonitorObj(mp_int32 iAppType, mp_string& strInstanceName, mp_string& strDbName);
    MONITOR_OBJ* GetMonitorObj(MONITOR_OBJ& monitorObj);
    mp_bool IsExistInList(MONITOR_OBJ& monitorObj);
    mp_void DelMonitorObj(MONITOR_OBJ* pMonitorObj);
    mp_int32 CreateMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateVSSMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateDBMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateFSMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateThirdPartyMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj);
    mp_int32 CreateAppMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj);
    mp_char** DuplicateHead(mp_string strDbUser, mp_string strDbPp);
    mp_int32 CreateReqMsg(MONITOR_OBJ& monitorObj, mp_string& strDbUser, mp_string& strDbPp,
        const Json::Value& jvJsonData);
    mp_int32 CreateReqMsg(MONITOR_OBJ& monitorObj, mp_string& strEncryptDbUser, mp_string& strEncryptDbPp,
        mp_string& strJsonData);
    mp_void FreeReqMsg(CRequestMsg* pReqMsg);
    mp_void FreeMonitorObj(MONITOR_OBJ& monitorObj);
    mp_int32 InitMonitorObj(   MONITOR_OBJ& newMonitorObj, CRequestMsg* pReqMsg, mp_string LoopTimeKey = mp_string(LOOP_TIME));
    mp_int32 SaveToDB(MONITOR_OBJ& monitorObj);
    mp_bool IsExistInDB(MONITOR_OBJ& monitorObj);
    mp_int32 RemoveFromDB(MONITOR_OBJ* pMonitorObj);
    mp_int32 LoadFromDB(vector<MONITOR_OBJ>& vecObj);
    mp_bool IsSame(MONITOR_OBJ& monitorObj1, MONITOR_OBJ& monitorObj2);
    mp_void SendHandleFailedAlarm(MONITOR_OBJ* pMonitorObj);
    mp_int32 GetRequestAppType(CRequestMsg* pReqMsg);
    mp_int32 GetRequestInstanceName(CRequestMsg* pReqMsg, mp_string& strInstanceName);
    mp_int32 GetRequestDbName(CRequestMsg* pReqMsg, mp_string& strDbName);
    mp_string GetQueryStatusUrl(mp_int32 iAppType);
    mp_string GetUnFreezeUrl(mp_int32 iAppType);
    mp_bool IsQueryStatusRequest(CRequestMsg* pReqMsg);
    mp_bool IsFreezeRequest(CRequestMsg* pReqMsg);
    mp_bool IsUnFreezeRequest(CRequestMsg* pReqMsg);
    mp_bool IsVSSRequest(CRequestMsg* pReqMsg);
    mp_bool IsDB2OrOracleRequest(CRequestMsg* pReqMsg);
    mp_bool IsFSRequest(CRequestMsg* pReqMsg);
    mp_bool IsThirdPartyRequest(CRequestMsg* pReqMsg);
    mp_bool IsAppRequest(CRequestMsg* pReqMsg);

    mp_void MonitorSingleFreezeOper(CRequestMsg* pReqMsg);
    mp_void UpdateSingleFreezeOper(CRequestMsg* pReqMsg, CResponseMsg* pRspMsg);
    mp_string GetDBNameFromObj(MONITOR_OBJ* pMonitorObj);
};

#endif


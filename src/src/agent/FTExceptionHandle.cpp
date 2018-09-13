/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/*==============================================================================
Copyright(C) : 2009 - 2019, Huawei Tech. Co. Ltd. All rights reserved
File Name    : CFTExcetptionHandle.cpp
Author       : 
Version      : 1.0
DateTime     : 2014.11.21
Others       :
Description  : ����/�ⶳ���ݿ��쳣����������۶�������ɹ�����ʧ�ܶ�����м�أ�
               �ڵ���һ��������ʱ���нⶳ������
Function List:
Method List  :
History      :
<author>              <time>       <version >   <desc>
�ع��򻯶��������ر������̣�����һ�����в��Ҳ���״̬����ʽ�Լ�ض������
��������߼�ԭ��ʼ�ռ�����һ�ζ��������
===============================================================================*/
#include "agent/FTExceptionHandle.h"
#include "common/Utils.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/CryptAlg.h"
#include "common/ErrorCode.h"
#include "alarm/Trap.h"
#include "alarm/Db.h"
#include "rest/Interfaces.h"
#include <sstream>

CFTExceptionHandle CFTExceptionHandle::m_Instance;
/*---------------------------------------------------------------------------
Function Name: HandleMonitorObjsProc
Description  : �̴߳����������̴߳����ض����б��е�����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
#ifdef WIN32
DWORD WINAPI CFTExceptionHandle::HandleMonitorObjsProc(LPVOID param)
#else
mp_void* CFTExceptionHandle::HandleMonitorObjsProc(mp_void* param)
#endif
{
    CFTExceptionHandle* pFtHandle = (CFTExceptionHandle*)param;
    vector<MONITOR_OBJ>::iterator iter;
    MONITOR_OBJ monitorObj;

    pFtHandle->SetThreadStatus(THREAD_STATUS_RUNNING);
    while (!pFtHandle->NeedExit())
    {
        pFtHandle->HandleMonitorObjs();
        DoSleep(SLEEP_TIME);
    }

    pFtHandle->SetThreadStatus(THREAD_STATUS_EXITED);
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

mp_void CFTExceptionHandle::SetThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}

mp_int32 CFTExceptionHandle::GetThreadStatus()
{
    return m_iThreadStatus;
}

thread_lock_t& CFTExceptionHandle::GetThreadLock()
{
    return m_tMonitorsLock;
}

mp_bool CFTExceptionHandle::NeedExit()
{
    return m_bNeedExit;
}

mp_int32 CFTExceptionHandle::ProcessInternalRsps()
{
    mp_int32 iRet = MP_SUCCESS;
    message_pair_t stMsgPair;

    for (;;)
    {
        iRet = CCommunication::GetInstance().PopRspInternalMsgQueue(stMsgPair);
        if (iRet != 0)
        {
            break;
        }

        //��Ҫ�������������ж���Ӧ����
        if (IsUnFreezeRequest(stMsgPair.pReqMsg))
        {
            ProccessUnFreezeRsp(stMsgPair.pReqMsg, stMsgPair.pRspMsg);
        }
        else if (IsQueryStatusRequest(stMsgPair.pReqMsg))
        {
            ProcessQueryStatusRsp(stMsgPair.pReqMsg, stMsgPair.pRspMsg);
        }

        //�ڲ������ͷ�ReqMsg�ڲ���FcgRequest��env�ڴ棬��ɾ����ض���ʱ������ڴ�
        //��ʱ�������ڴ�����Ѿ�����ض���ʱ���ⲿ�ⶳ����ִ�гɹ�ɾ����ض�������ͷ�
        if (NULL != stMsgPair.pReqMsg)
        {
            delete stMsgPair.pReqMsg;
            stMsgPair.pReqMsg = NULL;
        }

        if (NULL != stMsgPair.pRspMsg)
        {
            delete stMsgPair.pRspMsg;
            stMsgPair.pRspMsg = NULL;
        }
    }

    return MP_SUCCESS;
}

mp_int32 CFTExceptionHandle::ProccessUnFreezeRsp(CRequestMsg* pReqMsg, CResponseMsg* pRspMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iAppType = 0;
    mp_string strInstanceName;
    mp_string strDbName;
    MONITOR_OBJ* pMonitorObj = NULL;
    mp_bool bIsUnFreezeSucc = MP_FALSE;
    mp_int32 iUnFreezeRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin process unfreeze response.");
    iAppType = GetRequestAppType(pReqMsg);
    iRet = GetRequestInstanceName(pReqMsg, strInstanceName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get instance name failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = GetRequestDbName(pReqMsg, strDbName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get db name failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get request type info, app type %d, instance name %s, db name %s.",
        iAppType, strInstanceName.c_str(), strDbName.c_str());
    pMonitorObj = GetMonitorObj(iAppType, strInstanceName, strDbName);
    if (NULL == pMonitorObj)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Monitor obj dosen't exist.");
        return MP_SUCCESS;
    }

    iUnFreezeRet = (mp_int32)pRspMsg->GetRetCode();
    if (MP_SUCCESS == iUnFreezeRet)
    {
        bIsUnFreezeSucc = MP_TRUE;
    }
    iRet = HandleUnFreezingMonitorObj(pMonitorObj, bIsUnFreezeSucc);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Handle unfreeze monitor obj failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Process unfreeze response succ.");
    return MP_SUCCESS;
}

mp_int32 CFTExceptionHandle::ProcessQueryStatusRsp(CRequestMsg* pReqMsg, CResponseMsg* pRspMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iAppType = 0;
    mp_string strInstanceName;
    mp_string strDbName;
    MONITOR_OBJ* pMonitorObj = NULL;
    mp_int32 iQueryStatus = DB_UNKNOWN;
    mp_int32 iTmpStatus = DB_UNKNOWN;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin query status response.");
    iAppType = GetRequestAppType(pReqMsg);
    iRet = GetRequestInstanceName(pReqMsg, strInstanceName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get instance name failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = GetRequestDbName(pReqMsg, strDbName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get db name failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get request type info, app type %d, instance name %s, db name %s.",
        iAppType, strInstanceName.c_str(), strDbName.c_str());

    pMonitorObj = GetMonitorObj(iAppType, strInstanceName, strDbName);
    if (NULL == pMonitorObj)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Monitor obj dosen't exist.");
        return MP_SUCCESS;
    }

    if (NULL != pRspMsg && MP_SUCCESS == pRspMsg->GetRetCode())
    {
        iRet = CJsonUtils::GetJsonInt32(pRspMsg->GetJsonValueRef(), DB_FREEZE_STATUS, iTmpStatus);
        iQueryStatus = (iRet == MP_SUCCESS ? iTmpStatus : DB_UNKNOWN);
    }
    
    iRet = HandleQueryStatusMonitorObj(pMonitorObj, iQueryStatus);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Handle query status monitor obj failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Process query status response succ.");
    return MP_SUCCESS;
}

mp_void CFTExceptionHandle::HandleMonitorObjs()
{
    MONITOR_OBJ* pMonitorObj = NULL;
    CThreadAutoLock tmpLock(&m_tMonitorsLock);
    //��Ӧ������Ҫͳһ�����ڿ��ܴ��ڶ����Ӧ�ڶ�����ʱ��Ŀǰ�޷���ȡָ���������Ӧ
    //���մ����͵��ڲ��ⶳ����ѯ״̬����
    ProcessInternalRsps();

    //�����������б�
    for(;;)
    {
        pMonitorObj = GetHandleMonitorObj();
        if (NULL == pMonitorObj)
        {
            break;
        }

        if (MONITOR_STATUS_FREEZED == pMonitorObj->uiStatus)
        {
            HandleFreezedMonitorObj(pMonitorObj);
        }
    }
}

//����߼�ԭ��ʼ�ռ�����һ�ζ������
mp_int32 CFTExceptionHandle::HandleUnFreezingMonitorObj(MONITOR_OBJ* pMonitorObj, mp_bool bIsUnFreezeSucc)
{
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin handle unfreezing monitor obj.");
    //���·���ѯ���󵽽��ղ�ѯ��Ӧ�ڼ������µĶ��������·������Ը���Ӧ
    if (MONITOR_STATUS_FREEZED == pMonitorObj->uiStatus)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The monitor obj was freezed again.");
        return MP_SUCCESS;
    }

    //�ⶳʧ���ϱ��澯�����޸�״̬ΪMONITOR_STATUS_FREEZED�������
    if (!bIsUnFreezeSucc)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unfreeze monitor obj failed, send alarm.");
        SendHandleFailedAlarm(pMonitorObj);
        //�ⶳʧ���ӳ��´μ�ؽⶳʱ��
        if (MAX_MONITOR_TIME >= pMonitorObj->uiLoopTime)
        {
            pMonitorObj->uiLoopTime = pMonitorObj->uiLoopTime * 2;
        }
        pMonitorObj->uiStatus = MONITOR_STATUS_FREEZED;
        return MP_FAILED;
    }

    RemoveFromDB(pMonitorObj);
    DelMonitorObj(pMonitorObj);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Handle unfreezing monitor obj succ.");
    return MP_SUCCESS;
}

mp_int32 CFTExceptionHandle::HandleQueryStatusMonitorObj(MONITOR_OBJ* pMonitorObj, mp_int32 iQueryStatus)
{
    mp_int32 iRet = MP_SUCCESS;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin handle query status monitor obj.");
    //���·���ѯ���󵽽��ղ�ѯ��Ӧ�ڼ������µĶ��������·������Ը���Ӧ
    if (MONITOR_STATUS_FREEZED == pMonitorObj->uiStatus)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The monitor obj was freezed again.");
        return MP_SUCCESS;
    }

    //�ⶳ�ɹ�ɾ����ض���
    if (DB_UNFREEZE == iQueryStatus)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The monitor obj has been unfreezed, delete it.");
        RemoveFromDB(pMonitorObj);
        DelMonitorObj(pMonitorObj);
    }
    //������Ƕ���״̬���ͽⶳ����Ҹ���״̬ΪMONITOR_STATUS_UNFREEZING
    else if (DB_FREEZE == iQueryStatus)
    {
        iRet = PushUnFreezeReq(pMonitorObj);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Push unfreeze request failed, iRet = %d.", iRet);
            pMonitorObj->uiStatus = MONITOR_STATUS_FREEZED;
            return iRet;
        }
        else
        {
            pMonitorObj->uiStatus = MONITOR_STATUS_UNFREEZING;
        }
    }
    //�����ѯ״̬ʧ�����޸�״̬ΪMONITOR_STATUS_FREEZED�������
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get query status failed, monitor this obj again, iRet = %d.", iRet);
        pMonitorObj->uiStatus = MONITOR_STATUS_FREEZED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Handle query status monitor obj succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: ProccessFreezedMonitorObj
Description  : ���������ļ�ض���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::HandleFreezedMonitorObj(MONITOR_OBJ* pMonitorObj)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_uint64 ulCurrentTime = CMpTime::GetTimeSec();

    //�������û����Ӧ��Ŀǰ�������ӳٽⶳ�ڼ䣬�����д���
    if (ulCurrentTime <= pMonitorObj->ulBeginTime)
    {
        return MP_SUCCESS;
    }

    //����6Сʱ������
    if (ulCurrentTime - pMonitorObj->ulBeginTime >= MAX_MONITOR_TIME)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Time of monitoring is over, db type is %d, dbInstname is %s, , dbName is %s",
            pMonitorObj->iAppType, pMonitorObj->strInstanceName.c_str(), pMonitorObj->strDBName.c_str());
        RemoveFromDB(pMonitorObj);
        DelMonitorObj(pMonitorObj);
        return MP_SUCCESS;
    }

    //��δ���ⶳʱ���򲻽��д���
    if (ulCurrentTime - pMonitorObj->ulBeginTime < (mp_uint64)pMonitorObj->uiLoopTime)
    {
        return MP_SUCCESS;
    }

    //ִ�в�ѯ״̬�ű�ʱ������ű������ڣ�ֱ����PushQueryStatusReq��ɾ����ض���
    iRet = PushQueryStatusReq(pMonitorObj);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Push query status request failed, iRet %d.", iRet);
        return iRet;
    }

    pMonitorObj->uiStatus = MONITOR_STATUS_GETSTATUSING;
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: Init
Description  : ��ʼ�����������������߳�.
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::Init()
{
    mp_int32 iRet = MP_SUCCESS;
    vector<MONITOR_OBJ> vecMonitorObj;

    CThreadAutoLock tmpLock(&m_tMonitorsLock);
    //���־û��ļ�ض�����ص��ڴ���
    iRet = LoadFromDB(m_vecMonitors);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Load monitor obj from database failed, iRet %d.", iRet);
        return iRet;
    }
    
    //���������ش����̺߳Ͷ�ʱ�߳�
    iRet = CMpThread::Create(&m_hHandleThread, HandleMonitorObjsProc, this);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create ftexception handle thread failed, iRet %d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: WaitForExit
Description  : �ȴ��߳��˳�
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CFTExceptionHandle::WaitForExit()
{
    m_bNeedExit = MP_TRUE;
    while(THREAD_STATUS_EXITED != m_iThreadStatus)
    {
        DoSleep(100);
    }
}

/*---------------------------------------------------------------------------
Function Name: MonitorFreezeOper
Description  : ����������
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CFTExceptionHandle::MonitorFreezeOper(CRequestMsg* pReqMsg)
{
    //�ļ�ϵͳ����������������Ҫ���⴦��
    if (IsFSRequest(pReqMsg))
    {
        //�ֲ�json����
        vector<mp_string> vecDisks;
        mp_int32 iRet = CJsonUtils::GetJsonArrayString(pReqMsg->GetMsgBody().GetJsonValueRef(), DISKNAMES, vecDisks);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json array value failed, key %s.", mp_string(DISKNAMES).c_str());
            return;
        }
        Json::Reader r;
        Json::Value jsRequest = pReqMsg->GetMsgBody().GetJsonValue();
   
        mp_string strJson;
        for (vector<mp_string>::iterator it = vecDisks.begin(); it != vecDisks.end(); it++)
        {
            strJson = "{\"diskNames\":[\"" + *it + "\"]}";
            Json::Value jsValue;
            mp_bool bRet = r.parse(strJson.c_str(), jsValue);
            if (!bRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Parse json failed, strJson is %s.", strJson.c_str());
                return;
            }
            pReqMsg->SetJsonData(jsValue);
            MonitorSingleFreezeOper(pReqMsg);
        }

        //��ԭjson����
        pReqMsg->SetJsonData(jsRequest);
    }
    else
    {
        return MonitorSingleFreezeOper(pReqMsg);
    }
}

/*---------------------------------------------------------------------------
Function Name: MonitorSingleFreezeOper
Description  : ��������������
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CFTExceptionHandle::MonitorSingleFreezeOper(CRequestMsg* pReqMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    MONITOR_OBJ monitorObj;

    if (!IsFreezeRequest(pReqMsg) && !IsUnFreezeRequest(pReqMsg))
    {
        return;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin monitor freeze oper.");
    if (IsFreezeRequest(pReqMsg))
    {
        iRet = CreateMonitorObj(pReqMsg, monitorObj);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create monitor obj failed, iRet = %d", iRet);
            return;
        }

        iRet = SaveToDB(monitorObj);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Save monitor obj to database failed, iRet = %d", iRet);
        }

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin add monitor obj to list.");
        CThreadAutoLock tmpLock(&m_tMonitorsLock);
        iRet = AddMonitorObj(monitorObj);
        //����Ǽ���б��Ѿ��еĶ��󣬸��������Ϣ�Ժ���Ҫ�ͷ�pReqMsg�ȶ�̬�ڴ�
        if (MP_SUCCESS != iRet)
        {
            FreeMonitorObj(monitorObj);
        }
        
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Monitor freeze operation succ, app type %d, isntance name %s, db name %s, "
            "status %d, begin time %d.", monitorObj.iAppType, monitorObj.strInstanceName.c_str(),
            monitorObj.strDBName.c_str(), monitorObj.uiStatus, monitorObj.ulBeginTime);
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Add monitor obj succ.");
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Monitor freeze oper succ.");
}

/*---------------------------------------------------------------------------
Function Name: CreateMonitorObj
Description  : ���ݼ�ض������ʹ�����Ӧ�ļ�ض���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::CreateMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj)
{
    mp_int32 iRet = MP_SUCCESS;

    if (IsDB2OrOracleRequest(pReqMsg))
    {
        iRet = CreateDBMonitorObj(pReqMsg, monitorObj);
    }
    else if (IsVSSRequest(pReqMsg))
    {
        iRet = CreateVSSMonitorObj(pReqMsg, monitorObj);
    }
    else if (IsFSRequest(pReqMsg))
    {
        iRet = CreateFSMonitorObj(pReqMsg, monitorObj);
    }
    else if (IsThirdPartyRequest(pReqMsg))
    {
        iRet = CreateThirdPartyMonitorObj(pReqMsg, monitorObj);
    }
    else if (IsAppRequest(pReqMsg))
    {
        iRet = CreateAppMonitorObj(pReqMsg, monitorObj);
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupported monitor obj, url %s.", 
            pReqMsg->GetURL().GetProcURL().c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create monitor obj succ, app type %d, instance name %s, db name %s, "
        "status %d, begin time %d.", monitorObj.iAppType, monitorObj.strInstanceName.c_str(),
        monitorObj.strDBName.c_str(), monitorObj.uiStatus, monitorObj.ulBeginTime);

    return iRet;
}

/*---------------------------------------------------------------------------
Function Name: CreateVSSMonitorObj
Description  : ����VSS��ض���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::CreateVSSMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj)
{
    mp_int32 iRet = MP_SUCCESS;
    //VSSͳһ�������ﲻ����ʵ���������ݿ���
    monitorObj.strInstanceName = VSS_INSTANCE_NAME;
    monitorObj.strDBName = VSS_DB_NAME;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create vss monitor obj.");
    iRet = InitMonitorObj(monitorObj, pReqMsg);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init vss monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create vss monitor obj succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CreateDBMonitorObj
Description  : �������ݿ��ض���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::CreateDBMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetGetNum = MP_SUCCESS;
    mp_string strInstanceNameForOtherDB = "";
    mp_string strInstanceNumForSAPHana = "";
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create db monitor obj.");
    iRet = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), INST_NAME, strInstanceNameForOtherDB);
    iRetGetNum = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), INST_NUM, strInstanceNumForSAPHana);
    if (MP_SUCCESS != iRet && MP_SUCCESS != iRetGetNum)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json string value failed, key1(%s) for sap hana, key2(%s) for other database.", 
            mp_string(INST_NUM).c_str(), mp_string(INST_NAME).c_str());
        return iRet;
    }
    monitorObj.strInstanceName = (MP_SUCCESS == iRet ? strInstanceNameForOtherDB : strInstanceNumForSAPHana);
    iRet = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), DBNAME, monitorObj.strDBName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json string value failed, key %s.", mp_string(DBNAME).c_str());
        return iRet;
    }

    iRet = InitMonitorObj(monitorObj, pReqMsg);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init db2 or oracle monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create db monitor obj succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CreateFSMonitorObj
Description  : �����ļ�ϵͳ��ض���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::CreateFSMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecDisks;
    mp_string strInstanceName;
    vector<mp_string>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create file system monitor obj.");
    iRet = CJsonUtils::GetJsonArrayString(pReqMsg->GetMsgBody().GetJsonValueRef(), DISKNAMES, vecDisks);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json array value failed, key %s.", mp_string(DISKNAMES).c_str());
        return iRet;
    }
    
    for (iter = vecDisks.begin(); iter != vecDisks.end(); iter++)
    {
        strInstanceName = strInstanceName + *iter + ":";
    }
    //�ļ�ϵͳʵ�������ݿ���ͳһʹ�ù��ص�����������Ϣ
    monitorObj.strInstanceName = strInstanceName;
    monitorObj.strDBName = FILESYTEM_DB_NAME;

    iRet = InitMonitorObj(monitorObj, pReqMsg);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init file system monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create file system monitor obj succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CreateThirdPartyMonitorObj
Description  : �����������ű���ض���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::CreateThirdPartyMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strFileName, strParam;
    iRet = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME, strFileName);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json string value failed, key %s.", REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME);
        return iRet;
    }
    if (pReqMsg->GetMsgBody().GetJsonValueRef().isMember(REST_PARAM_HOST_FREEZE_SCRIPT_PARAM))
    {
        iRet = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), REST_PARAM_HOST_FREEZE_SCRIPT_PARAM, strParam);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json string value failed, key %s.", REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME);
            return iRet;
        }
    }

    //���ڵ���������ű���InstanceNameʹ�ýű��ļ�������䣬DBNameʹ�ýű��������
    monitorObj.strInstanceName = strFileName;
    monitorObj.strDBName = strParam;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create third-party monitor obj.");
    iRet = InitMonitorObj(monitorObj, pReqMsg, mp_string(REST_PARAM_HOST_FREEZE_TIMEOUT));
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init thirdparty monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create thirdparty monitor obj succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: CreateAppMonitorObj
Description  : �������ݲ�����Ӧ�ýӿڼ�ض���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::CreateAppMonitorObj(CRequestMsg* pReqMsg, MONITOR_OBJ& monitorObj)
{
    //���ڵ���������ű���InstanceNameʹ�ýű��ļ�������䣬DBNameʹ�ýű��������
    monitorObj.strInstanceName = APPINSTNAME;
    monitorObj.strDBName = APPDBNAME;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create app monitor obj.");
    mp_int32 iRet = InitMonitorObj(monitorObj, pReqMsg);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init app monitor obj failed, iRet = %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create app monitor obj succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: UpdateFreezeOper
Description  : ���¼�ض���״̬
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CFTExceptionHandle::UpdateFreezeOper(CRequestMsg* pReqMsg, CResponseMsg* pRspMsg)
{
    //�ļ�ϵͳ�������������Ҫ���⴦��
    if (IsFSRequest(pReqMsg))
    {
        //�ֲ�json����
        vector<mp_string> vecDisks;
        mp_int32 iRet = CJsonUtils::GetJsonArrayString(pReqMsg->GetMsgBody().GetJsonValueRef(), DISKNAMES, vecDisks);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json array value failed, key %s.", mp_string(DISKNAMES).c_str());
            return;
        }
        Json::Reader r;
        Json::Value jsRequest = pReqMsg->GetMsgBody().GetJsonValue();
        mp_string strJson;
        for (vector<mp_string>::iterator it = vecDisks.begin(); it != vecDisks.end(); it++)
        {
            strJson = "{\"diskNames\":[\"" + *it + "\"]}";
            Json::Value jsValue;
            mp_bool bRet = r.parse(strJson.c_str(), jsValue);
            if (!bRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Parse json failed, strJson is %s.", strJson.c_str());
                return;
            }
            pReqMsg->SetJsonData(jsValue);
            UpdateSingleFreezeOper(pReqMsg, pRspMsg);
        }

        //��ԭjson����
        pReqMsg->SetJsonData(jsRequest);
    }
    else
    {
        return UpdateSingleFreezeOper(pReqMsg, pRspMsg);
    }
}

/*---------------------------------------------------------------------------
Function Name: UpdateSingleFreezeOper
Description  : ���µ�����ض���״̬
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CFTExceptionHandle::UpdateSingleFreezeOper(CRequestMsg* pReqMsg, CResponseMsg* pRspMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    MONITOR_OBJ objCond;
    MONITOR_OBJ* pMonitorObj = NULL;
    
    if (!IsFreezeRequest(pReqMsg) && !IsUnFreezeRequest(pReqMsg))
    {
        return;
    }

    iRet = CreateMonitorObj(pReqMsg, objCond);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create montior obj failed, iRet = %d.", iRet);
        return;
    }

    CThreadAutoLock tmpLock(&m_tMonitorsLock);
    pMonitorObj = GetMonitorObj(objCond);
    if (NULL == pMonitorObj)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Monitor obj dosen't exist.");
        FreeMonitorObj(objCond);
        return;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin update freeze operation monitor obj.");
    if (IsFreezeRequest(pReqMsg))
    {
        if (ERROR_COMMON_DB_USERPWD_WRONG == pRspMsg->GetRetCode())
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "User or password wrong, begin free monitor obj resource.");
            RemoveFromDB(pMonitorObj);
            DelMonitorObj(pMonitorObj);
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Release monitor obj succ, app type %d, isntance name %s, "
                "db name %s.", objCond.iAppType, objCond.strInstanceName.c_str(), objCond.strDBName.c_str());
        }
        else
        {
            //���յ����������Ӧ��Ҫ���¿�ʼ���ʱ�䣬��ض����ؿ�ʼʱ��ӽ��յ�����������Ӧ��ʼ
            pMonitorObj->ulBeginTime = CMpTime::GetTimeSec();
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Update monitor obj freeze begin time to %llu.", pMonitorObj->ulBeginTime);
        }
    }
    else
    {
        if (MP_SUCCESS == pRspMsg->GetRetCode())
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin free monitor obj resource.");
            RemoveFromDB(pMonitorObj);
            DelMonitorObj(pMonitorObj);
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Release monitor obj succ, app type %d, isntance name %s, "
                "db name %s.", objCond.iAppType, objCond.strInstanceName.c_str(), objCond.strDBName.c_str());
        }
    }
    
    FreeMonitorObj(objCond);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Update freeze operation monitor obj succ.");
}

/*---------------------------------------------------------------------------
Function Name:PushUnFreezeReq
Description  :�ⶳ��ض��󣬹����ڲ��������ͨ���������
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::PushUnFreezeReq(MONITOR_OBJ* pMonitorObj)
{
    CRequestMsg* pReqMsg = NULL;
    CResponseMsg* pRspMsg = NULL;
    mp_string strUrl;

    strUrl = GetUnFreezeUrl(pMonitorObj->iAppType);
    if ("" == strUrl)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get unfreeze url failed.");
        return MP_FAILED;
    }
    //CodeDex�󱨣�Memory Leak
    NEW_CATCH_RETURN_FAILED(pReqMsg, CRequestMsg);
    NEW_CATCH(pRspMsg, CResponseMsg);
    if (!pRspMsg)
    {
        delete pReqMsg;
        pReqMsg = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New response msg failed.");
        return MP_FAILED;
    }
    
    *pReqMsg = *pMonitorObj->pReqMsg;
    message_pair_t stReqMsg(pReqMsg, pRspMsg);
    pReqMsg->GetURL().SetProcURL(strUrl);
    pReqMsg->GetHttpReq().SetMethod(REST_URL_METHOD_PUT);
    //pReqMsg��pRspMsg���ڲ���Ӧ������֮���ͷ�
    CCommunication::GetInstance().PushReqMsgQueue(stReqMsg);
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Push unfreeze request succ, url %s.", pReqMsg->GetURL().GetProcURL().c_str());
    return MP_SUCCESS; //lint !e429
}

mp_int32 CFTExceptionHandle::PushQueryStatusReq(MONITOR_OBJ* pMonitorObj)
{
	//CodeDex�󱨣�UNUSED_VALUE
    mp_string strQueryUrl;
    mp_string strQueryParam;
    CRequestMsg* pReqMsg = NULL;
    CResponseMsg* pRspMsg = NULL;

    strQueryUrl = GetQueryStatusUrl(pMonitorObj->iAppType);
    if ("" == strQueryUrl)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get query status url failed.");
        return MP_FAILED;
    }
    
    if (TYPE_APP_FILESYSTEM == pMonitorObj->iAppType)
    {
        strQueryParam = DISKNAMES + "=" + pMonitorObj->strInstanceName;
    }
    else if (TYPE_APP_THIRDPARTY == pMonitorObj->iAppType)
    {
        //��json��Ϣ���л�ȡ�ⶳ�ű����ƺͲ���
        Json::Value jv = pMonitorObj->pReqMsg->GetMsgBody().GetJsonValueRef();
        mp_string strStatusScript, strParm, strScriptName;
        GET_JSON_STRING(jv, REST_PARAM_HOST_QUERY_SCRIPT_FILENAME , strStatusScript);
        if (jv.isMember(REST_PARAM_HOST_QUERY_SCRIPT_PARAM))
        {
            GET_JSON_STRING(jv, REST_PARAM_HOST_QUERY_SCRIPT_PARAM, strParm);
        }

        strScriptName = CPath::GetInstance().GetBinPath() + mp_string(PATH_SEPARATOR) + 
            mp_string(AGENT_THIRDPARTY_DIR) + mp_string(PATH_SEPARATOR) + strStatusScript;
        if (!CMpFile::FileExist(strScriptName.c_str()))
        {
            //�ű�������ɾ����ض��󣬲���Ҫ�ϱ��澯��
            //��ִ�ж���ʱ�ű�������ֱ�ӻ�ʧ��
            //ɾ��ʱ�����ڶ��߳����⣬HandleMonitorObjs��������ǰ������monitor�б�
            RemoveFromDB(pMonitorObj);
            DelMonitorObj(pMonitorObj);
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "QueryStatus file %s is not exists, delete monitor obj.", 
                strScriptName.c_str());
            return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
        }        
        strQueryParam = mp_string(REST_PARAM_HOST_QUERY_SCRIPT_FILENAME) + "=" + strStatusScript + "&"
              + mp_string(REST_PARAM_HOST_QUERY_SCRIPT_PARAM) + "=" + strParm;
    }
    else if (TYPE_APP_HANA == pMonitorObj->iAppType)
    {
        strQueryParam = INST_NUM + "=" + pMonitorObj->strInstanceName + "&" + DBNAME + "=" + pMonitorObj->strDBName;
    }
    else 
    {
        strQueryParam = INST_NAME + "=" + pMonitorObj->strInstanceName + "&" + DBNAME + "=" + pMonitorObj->strDBName;
    }
	//CodeDex�󱨣�Memory Leak
    NEW_CATCH(pReqMsg, CRequestMsg);
    if (!pReqMsg)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New CRequestMsg failed");
        return MP_FAILED;
    }
    
    NEW_CATCH(pRspMsg, CResponseMsg);
    if (!pRspMsg)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New CResponseMsg failed");
        delete pReqMsg;
        pReqMsg = NULL;
        return MP_FAILED;
    }
    
    *pReqMsg = *pMonitorObj->pReqMsg;
    message_pair_t stReqMsg(pReqMsg, pRspMsg);
    pReqMsg->GetURL().SetProcURL(strQueryUrl);
    if (TYPE_APP_APP != pMonitorObj->iAppType)
    {
        pReqMsg->GetURL().SetQueryParam(strQueryParam, MP_FALSE);
    }
    pReqMsg->GetHttpReq().SetMethod(REST_URL_METHOD_GET);
    //pReqMsg��pRspMsg���ڲ���Ӧ������֮���ͷ�
    CCommunication::GetInstance().PushReqMsgQueue(stReqMsg);
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Push query status request succ, url %s.", pReqMsg->GetURL().GetProcURL().c_str());
    return MP_SUCCESS; //lint !e429
}

mp_bool CFTExceptionHandle::IsExistInList(MONITOR_OBJ& monitorObj)
{
    vector<MONITOR_OBJ>::iterator iter;
    
    for (iter = m_vecMonitors.begin(); iter != m_vecMonitors.end(); iter++)
    {
        if(IsSame(monitorObj, *iter))
        {
            return MP_TRUE;
        }
    }

    return MP_FALSE;
}

/*---------------------------------------------------------------------------
Function Name: GetHandleMonitorObj
Description  : �Ӽ���б��л�ȡ������ļ�ض�������Ѿ�������һ����ӵ�һ��
               Ԫ�ؿ�ʼ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
MONITOR_OBJ* CFTExceptionHandle::GetHandleMonitorObj()
{
    vector<MONITOR_OBJ>::iterator iter;
    MONITOR_OBJ* pMonitorObj = NULL;
    
    if (0 < m_vecMonitors.size() && m_iCurrIndex < m_vecMonitors.size())
    {
        pMonitorObj = &m_vecMonitors[m_iCurrIndex];
        m_iCurrIndex++;
        return pMonitorObj;
    }

    m_iCurrIndex = 0;
    return NULL;
}

MONITOR_OBJ* CFTExceptionHandle::GetMonitorObj(mp_int32 iAppType, mp_string& strInstanceName, mp_string& strDbName)
{
    MONITOR_OBJ* pMonitorObj = NULL;

    for (mp_uint32 uiIndex = 0; uiIndex < m_vecMonitors.size(); uiIndex++)
    {
        pMonitorObj = &m_vecMonitors[uiIndex];
        if ((iAppType == pMonitorObj->iAppType)
            && (0 == strcmp(strInstanceName.c_str(), pMonitorObj->strInstanceName.c_str()))
            && (0 == strcmp(strDbName.c_str(), pMonitorObj->strDBName.c_str())))
        {
            return pMonitorObj;
        }
    }

    return NULL;
}

MONITOR_OBJ* CFTExceptionHandle::GetMonitorObj(MONITOR_OBJ& monitorObj)
{
    return GetMonitorObj(monitorObj.iAppType, monitorObj.strInstanceName, monitorObj.strDBName);
}

mp_int32 CFTExceptionHandle::AddMonitorObj(MONITOR_OBJ& monitorObj)
{    
    //���������Ķ����ض���BeginTime��Ҫ�ӳ٣��������м�ؽⶳ�Ĳ�����Ҫ�Ӷ��������Ӧ���ؿ�ʼ��ʱ
    //���浽���ݿ��е�BeginTime�������ӳ�ʱ�䣬ֻ���ڴ��еļ�ض�������
    monitorObj.ulBeginTime += DELAY_UNFREEZE_TIME;
    
    MONITOR_OBJ* pMonitorObj = NULL;
    if (IsExistInList(monitorObj))
    {
        pMonitorObj = GetMonitorObj(monitorObj);
        if (NULL != pMonitorObj)
        {
            pMonitorObj->uiStatus = monitorObj.uiStatus;
            pMonitorObj->ulBeginTime = monitorObj.ulBeginTime;
            pMonitorObj->uiLoopTime = monitorObj.uiLoopTime;
        }
        
        return MP_FAILED;
    }

    m_vecMonitors.push_back(monitorObj);
    return MP_SUCCESS;
}

mp_void CFTExceptionHandle::AddMonitorObjs(vector<MONITOR_OBJ>& vecMonitorObjs)
{
    vector<MONITOR_OBJ>::iterator iter;
    
    for (iter = vecMonitorObjs.begin(); iter != vecMonitorObjs.end(); iter++)
    {
        m_vecMonitors.push_back(*iter);
    }
}

mp_void CFTExceptionHandle::DelMonitorObj(MONITOR_OBJ* pMonitorObj)
{
    vector<MONITOR_OBJ>::iterator iter;

    if (NULL == pMonitorObj)
    {
        return;
    }
    
    for (iter = m_vecMonitors.begin(); iter != m_vecMonitors.end();)
    {
        if (IsSame(*pMonitorObj, *iter))
        {
            FreeMonitorObj(*iter);
            iter = m_vecMonitors.erase(iter);
            return;
        }
        else
        {
            iter++;
        }
    }
}

/*---------------------------------------------------------------------------
Function Name: DuplicateHead
Description  : ������Ϣͷ���������
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_char** CFTExceptionHandle::DuplicateHead(mp_string strDbUser, mp_string strDbPp)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_char** env = NULL;
    mp_string strHeaderDbUser;
    mp_string strHeaderDbPp;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin duplicate head.");
    strHeaderDbUser = mp_string(HTTPPARAM_DBUSERNAME) + "=" + strDbUser;
    //������һ������յ�ַ������FCGX_GetParam���������־
    //CodeDex�󱨣�ZERO_LENGTH_ALLOCATIONS
    //CodeDex�󱨣�Memory Leak
    NEW_ARRAY_CATCH(env, mp_char*, MAX_PRIVATE_KEY_NUM + 1);
    if (!env)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New env failed");
        return NULL;
    }

    NEW_ARRAY_CATCH(env[0], mp_char, strHeaderDbUser.length() + 1);
    if (!env[0])
    {
        delete[] env;
        env = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New env[0] failed");
        return NULL;
    }
    
    iRet = strcpy_s(env[0], strHeaderDbUser.length() + 1, strHeaderDbUser.c_str());
    if (EOK != iRet)
    {
        delete[] env[0];
        env[0] = NULL;
        delete[] env;
        env = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call strcpy_s failed, iRet = %d", iRet);
        return NULL;
    }

    strHeaderDbPp = mp_string(HTTPPARAM_DBPASSWORD) + "=" + strDbPp;
    NEW_ARRAY_CATCH(env[1], mp_char, strHeaderDbPp.length() + 1);
    if (!env[1])
    {
        delete[] env[0];
        env[0] = NULL;
        delete[] env;
        env = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New env[1] failed");
        strHeaderDbPp.replace(0, strHeaderDbPp.length(), "");
        return NULL;
    }
    env[2] = NULL;

    iRet = strcpy_s(env[1], strHeaderDbPp.length() + 1, strHeaderDbPp.c_str());
    if (EOK != iRet)
    {
        delete[] env[0];
        env[0] = NULL;
        delete[] env[1];
        env[1] = NULL;
        delete[] env;
        env = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call strcpy_s failed, iRet = %d", iRet);
        strHeaderDbPp.replace(0, strHeaderDbPp.length(), "");
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Duplicate head succ.");
    strHeaderDbPp.replace(0, strHeaderDbPp.length(), "");
    return env;
}

mp_int32 CFTExceptionHandle::CreateReqMsg(MONITOR_OBJ& monitorObj, mp_string& strDbUser, mp_string& strDbPp,
    const Json::Value& jvJsonData)
{
    mp_char** env = NULL;
    FCGX_Request* pFcgxReq = NULL;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin create request msg.");
    //CodeDex�󱨣�Memory Leak
    NEW_CATCH(monitorObj.pReqMsg, CRequestMsg);
    if (NULL == monitorObj.pReqMsg)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create request msg for monitor obj failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    try
    {
        monitorObj.pReqMsg->SetJsonData(jvJsonData);
    }
    //SetJsonData�����л�����Json::Value�ĸ��Ʋ��������ܻ����JSON_ASSERT_MESSAGE���׳�std::runtime_error�쳣
    //Coverity&Fortify����Ҫ��׽std::runtime_error�쳣
    catch (...)
    {
        delete monitorObj.pReqMsg;
        monitorObj.pReqMsg = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "JsonData is invalid, jsonValue %s.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    env = DuplicateHead(strDbUser, strDbPp);
    if (!env)
    {
        delete monitorObj.pReqMsg;
        monitorObj.pReqMsg = NULL;
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    NEW_CATCH(pFcgxReq, FCGX_Request);
    if (!pFcgxReq)
    {
        delete[] env[0];
        env[0] = NULL;
        delete[] env[1];
        env[1] = NULL;
        delete[] env;
        env = NULL;
        delete monitorObj.pReqMsg;
        monitorObj.pReqMsg = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New fcgx request failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    pFcgxReq->envp = env;
    monitorObj.pReqMsg->GetHttpReq().SetFcgxReq(pFcgxReq);
    //��monitor�����ͷ�ʱ�ͷ�
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Create request msg succ.");
    return MP_SUCCESS; //lint !e429
}

/*---------------------------------------------------------------------------
Function Name: CreateReqMsg
Description  : ����������Ϣ,�������ݿ��ж�ȡ��������������Ϣʱʹ��
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::CreateReqMsg(MONITOR_OBJ& monitorObj, mp_string& strEncryptDbUser,
    mp_string& strEncryptDbPp, mp_string& strJsonData)
{
	//CodeDex�󱨣�UNUSED_VALUE
    mp_int32 iRet = MP_SUCCESS;
    Json::Reader jsonReader;
    Json::Value jsonValue;
    mp_string strDbUser;
    mp_string strDbPp;
    
    jsonReader = Json::Reader();
    try
    {
        if (!jsonReader.parse(strJsonData, jsonValue))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "JsonData is invalid, jsonValue %s.", strJsonData.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    }
    //jsonReader.parse�����е���ջ�п��ܻ����JSON_ASSERT_MESSAGE���׳�std::runtime_error�쳣
    //Coverity&Fortify����Ҫ��׽std::runtime_error�쳣
    catch (...)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "JsonData is invalid, jsonValue %s.", strJsonData.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    DecryptStr(strEncryptDbUser, strDbUser);
    DecryptStr(strEncryptDbPp, strDbPp);

    iRet = CreateReqMsg(monitorObj, strDbUser, strDbPp, jsonValue);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create request message failed, iRet %d.", iRet);
    }

    strDbPp.replace(0, strDbPp.length(), "");
    return iRet;
}

mp_void CFTExceptionHandle::FreeReqMsg(CRequestMsg* pReqMsg)
{
    //CodeDex�󱨣�UNUSED_VALUE
    if (!pReqMsg)
    {
        return;
    }

    //�ͷ��ڴ�ǰ���������
    (void)memset_s(pReqMsg->GetHttpReq().GetAllHead()[0], 
              strlen(pReqMsg->GetHttpReq().GetAllHead()[0]), 0, strlen(pReqMsg->GetHttpReq().GetAllHead()[0]));
    (void)memset_s(pReqMsg->GetHttpReq().GetAllHead()[1], 
              strlen(pReqMsg->GetHttpReq().GetAllHead()[1]), 0, strlen(pReqMsg->GetHttpReq().GetAllHead()[1]));

    delete[] pReqMsg->GetHttpReq().GetAllHead()[0];
    delete[] pReqMsg->GetHttpReq().GetAllHead()[1];
    delete[] pReqMsg->GetHttpReq().GetAllHead();
    delete pReqMsg->GetHttpReq().GetFcgxReq();
    delete pReqMsg;
    pReqMsg = NULL;
}

mp_void CFTExceptionHandle::FreeMonitorObj(MONITOR_OBJ& monitorObj)
{
    FreeReqMsg(monitorObj.pReqMsg);
    monitorObj.pReqMsg = NULL;
}

mp_int32 CFTExceptionHandle::InitMonitorObj(MONITOR_OBJ& newMonitorObj, CRequestMsg* pReqMsg, mp_string LoopTimeKey)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strDbUser;
    mp_string strDbPp;
    mp_int32 iLoopTime = 0;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin init monitor obj.");
    newMonitorObj.iAppType = GetRequestAppType(pReqMsg);
    newMonitorObj.ulBeginTime = CMpTime::GetTimeSec();
    newMonitorObj.uiStatus = MONITOR_STATUS_FREEZED;
    iRet = CJsonUtils::GetJsonInt32(pReqMsg->GetMsgBody().GetJsonValueRef(), LoopTimeKey, iLoopTime);
    if (MP_SUCCESS == iRet)
    {
        newMonitorObj.uiLoopTime = iLoopTime;
    }
    else
    {
        newMonitorObj.uiLoopTime = THAW_WAIT_TIME;
    }

    strDbUser = pReqMsg->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME);
    strDbPp = pReqMsg->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD);
    iRet = CreateReqMsg(newMonitorObj, strDbUser, strDbPp, pReqMsg->GetMsgBody().GetJsonValueRef());
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create request msg for new monitor obj failed.");
    }
    else
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Init monitor obj succ.");
    }
    strDbPp.replace(0, strDbPp.length(), "");
    return iRet;
}

/*---------------------------------------------------------------------------
Function Name: SaveToDB
Description  : ��������ݳ־û������ݿ�
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CFTExceptionHandle::IsExistInDB(MONITOR_OBJ& monitorObj)
{
    mp_int32 iRet = MP_SUCCESS;
    ostringstream buff;
    mp_string strSql;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    CDB &db = CDB::GetInstance();
    
    buff << "select " << InstanceName << "," << DBName << " from " << FreezeObjTable 
          << " where " << InstanceName << " == ? and " << DBName << " == ?";
        
    strSql = buff.str();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "buff is = %s.", strSql.c_str());

    DbParamStream dps;
    DbParam dp = monitorObj.strInstanceName;
    dps <<dp;
    dp = monitorObj.strDBName;
    dps <<dp;
    
    iRet = db.QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query monitor object failed, iRet = %d.",iRet);
        return MP_FALSE;
    }

    if (0 < iRowCount)
    {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/*---------------------------------------------------------------------------
Function Name: SaveToDB
Description  : ��������ݳ־û������ݿ�
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::SaveToDB(MONITOR_OBJ& monitorObj)
{
    mp_int32 iRet = MP_SUCCESS;
    Json::FastWriter jfWriter;
    mp_string strDbUser;
    mp_string strDbPp;
    mp_string strEncrpytDBUser;
    mp_string strEncrpytDBPwd;
    mp_string strJsonData;
    std::ostringstream buff;
    CDB &db = CDB::GetInstance();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin save monitor obj to database.");
    if (IsExistInDB(monitorObj))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Monitor obj is aleady in db, instanceName is %s, dbName is %s",
            monitorObj.strInstanceName.c_str(), monitorObj.strDBName.c_str());
        return MP_SUCCESS;
    }
    
    strDbUser = monitorObj.pReqMsg->GetHttpReq().GetHead(HTTPPARAM_DBUSERNAME);
    strDbPp = monitorObj.pReqMsg->GetHttpReq().GetHead(HTTPPARAM_DBPASSWORD);
    EncryptStr(strDbUser, strEncrpytDBUser);
    EncryptStr(strDbPp, strEncrpytDBPwd);
   
    strJsonData = jfWriter.write(monitorObj.pReqMsg->GetMsgBody().GetJsonValueRef());
    buff << "insert into " << FreezeObjTable << "(" << InstanceName << "," << DBName << "," 
          << BeginStatus <<"," << LoopTime<< "," << User <<"," << MP << ","<< JsonData << "," 
          << AppType << "," << BeginTime << ") values(?, ?, ?, ?, ?, ?, ?, ?, ?);";
    mp_string sql = buff.str();
    
    DbParamStream dps;
    DbParam dp = monitorObj.strInstanceName;
    dps << dp;
    dp = monitorObj.strDBName;
    dps << dp;
    dp = monitorObj.uiStatus;
    dps << dp;
    dp = monitorObj.uiLoopTime;
    dps << dp;
    dp = strEncrpytDBUser;
    dps << dp;
    dp = strEncrpytDBPwd;
    dps << dp;
    dp = strJsonData;
    dps << dp;
    dp = monitorObj.iAppType;
    dps << dp;
    dp = monitorObj.ulBeginTime;
    dps << dp;
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "buff is = %s.", sql.c_str());
    
    iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS!= iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "Execute sql failed,iRet = %d.",iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Save monitor obj succ, app type %d, isntance name %s, db name %s, "
            "status %d, begin time %d.", monitorObj.iAppType, monitorObj.strInstanceName.c_str(),
            monitorObj.strDBName.c_str(), monitorObj.uiStatus, monitorObj.ulBeginTime);

        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Save monitor obj to database succ.");
    }
    strDbPp.replace(0, strDbPp.length(), "");
    return iRet;
}

/*---------------------------------------------------------------------------
Function Name: RemoveFromDB
Description  : ��������ݴ����ݿ����Ƴ�
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::RemoveFromDB(MONITOR_OBJ* pMonitorObj)
{
    mp_int32 iRet = MP_SUCCESS;
    CDB &db = CDB::GetInstance();
    std::ostringstream buff;

    if (NULL == pMonitorObj)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "Input param is null.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin remove monitor obj from database.");

    buff<<"delete from " << FreezeObjTable <<" where " << InstanceName << "== ? and " << DBName <<" == ?";
    mp_string sql = buff.str();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "buff is = %s.", sql.c_str());

    DbParamStream dps;
    DbParam dp = pMonitorObj->strInstanceName;
    dps <<dp;
    dp = pMonitorObj->strDBName;
    dps << dp;
    
    iRet = db.ExecSql(sql, dps);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR,LOG_COMMON_ERROR, "db.ExecSql failed,iRet = %d.",iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Remove monitor obj succ, app type %d, isntance name %s, db name %s, "
        "status %d, begin time %d.", pMonitorObj->iAppType, pMonitorObj->strInstanceName.c_str(),
        pMonitorObj->strDBName.c_str(), pMonitorObj->uiStatus, pMonitorObj->ulBeginTime);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Remove monitor obj from database succ.");
    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name: LoadFromDB
Description  : ��db�ж���������ݣ����ڽ�������ʱ����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::LoadFromDB(vector<MONITOR_OBJ>& vecObj)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;
    ostringstream buff;
    CDB &db = CDB::GetInstance();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin load monitor objs from database.");
    buff << "select " << InstanceName << "," << DBName << "," << BeginStatus << "," << LoopTime "," \
         << User << "," << MP << "," << JsonData << "," << AppType << "," << BeginTime << " from " << FreezeObjTable;

    DbParamStream dps;
    iRet = db.QueryTable(buff.str(), dps, readBuff, iRowCount, iColCount);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Query freeze monitor obj table failed, iRet = %d.",iRet);
        return iRet;
    }

    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow)
    {
        MONITOR_OBJ monitorObj;
        mp_string strInstanceName, strDBName, strStatus, strLoopTime;
        mp_string strUser, strPp, strJsonData, strAppType, strBeginTime;
        readBuff >> strInstanceName;
        readBuff >> strDBName;
        readBuff >> strStatus;
        readBuff >> strLoopTime;
        readBuff >> strUser;
        readBuff >> strPp;
        readBuff >> strJsonData;
        readBuff >> strAppType;
        readBuff >> strBeginTime;
        
        monitorObj.strInstanceName = strInstanceName;
        monitorObj.strDBName = strDBName;
        monitorObj.uiStatus = atoi(strStatus.c_str());
        monitorObj.uiLoopTime = atoi(strLoopTime.c_str());
        monitorObj.iAppType = atoi(strAppType.c_str());
        monitorObj.ulBeginTime = atol(strBeginTime.c_str());
        iRet = CreateReqMsg(monitorObj, strUser, strPp, strJsonData);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Generate request msg failed, iRet = %d", iRet);
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Load monitor obj succ, app type %d, isntance name %s, db name %s, "
                "status %d, begin time %d.", monitorObj.iAppType, monitorObj.strInstanceName.c_str(),
                monitorObj.strDBName.c_str(), monitorObj.uiStatus, monitorObj.ulBeginTime);
            vecObj.push_back(monitorObj);
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load monitor objs from database succ.");
    return MP_SUCCESS;
}

mp_string CFTExceptionHandle::GetDBNameFromObj(MONITOR_OBJ* pMonitorObj)
{
    mp_string strDBName = pMonitorObj->strDBName;
    if (pMonitorObj->pReqMsg && (TYPE_APP_SQL == pMonitorObj->iAppType) || (TYPE_APP_EXCHANGE == pMonitorObj->iAppType))
    {
        mp_int32 iRet = MP_FAILED;
        vector<Json::Value> vecJson;
        try
        {
            iRet = CJsonUtils::GetArrayJson(pMonitorObj->pReqMsg->GetMsgBody().GetJsonValue(), vecJson);
        }
        catch(...)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "pMonitorObj is NULL.");
        }
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetArrayJson failed, iRet = %d.", iRet);
            return strDBName;
        }
        strDBName.clear();
        for (vector<Json::Value>::iterator it = vecJson.begin(); it != vecJson.end(); it++)
        {
            mp_string strSingleDBName = "";
            iRet = CJsonUtils::GetJsonString(*it, DBNAME, strSingleDBName);
            if (iRet != MP_SUCCESS)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetJsonString failed, iRet = %d.", iRet);
                return strDBName;
            }
            strDBName = strDBName + strSingleDBName + ":";
        }      
    }
    else if(TYPE_APP_FILESYSTEM == pMonitorObj->iAppType)
    {
        strDBName = pMonitorObj->strInstanceName;
    }

    return strDBName;
}


/*---------------------------------------------------------------------------
Function Name: SendHandleFailedAlarm
Description  : ���ͽⶳʧ�ܸ澯.
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CFTExceptionHandle::SendHandleFailedAlarm(MONITOR_OBJ* pMonitorObj)
{
    std::ostringstream buff;
    alarm_param_st alarmParam;

    //����sqlserver��exchange,��Ҫ�������ݿ�����
    if (!pMonitorObj)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "pMonitorObj is NULL.");
        return;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin send handle failed alarm.");
    buff<< pMonitorObj->iAppType << "," << GetDBNameFromObj(pMonitorObj);
    alarmParam.strAlarmParam = buff.str();
    alarmParam.iAlarmID = ALARM_ID_THAWFAILED;
    CTrapSender::SendAlarm(alarmParam);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End send handle failed alarm.");
}

/*---------------------------------------------------------------------------
Function Name:IsSame
Description  :�ж�������ض����Ƿ���ͬ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CFTExceptionHandle::IsSame(MONITOR_OBJ& monitorObj1, MONITOR_OBJ& monitorObj2)
{
    if ((monitorObj1.iAppType == monitorObj2.iAppType) 
        && (0 == strcmp(monitorObj1.strInstanceName.c_str(), monitorObj2.strInstanceName.c_str()))
        && (0 == strcmp(monitorObj1.strDBName.c_str(), monitorObj2.strDBName.c_str())))
    {
        return MP_TRUE;
    }
    
    return MP_FALSE;
}

mp_bool CFTExceptionHandle::IsFreezeRequest(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    
    if ((REST_ORACLE_FREEZE == strUrl) || (REST_DB2_FREEZE == strUrl) || (REST_SQLSERVER_FREEZE_DB == strUrl) 
        || (REST_EXCHANGE_FREEZE_DB == strUrl) || (REST_DEVICE_FILESYS_FREEZE == strUrl) 
        || (REST_HOST_FREEZE_SCRIPT == strUrl) || (REST_APP_FREEZE == strUrl)
        || (REST_SYBASE_FREEZE == strUrl) || (REST_HANA_FREEZE == strUrl))
    {
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_bool CFTExceptionHandle::IsUnFreezeRequest(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    
    if ((REST_ORACLE_UNFREEZE == strUrl) ||  (REST_DB2_UNFREEZE == strUrl) ||(REST_SQLSERVER_UNFREEZE_DB == strUrl)
        || (REST_EXCHANGE_UNFREEZE_DB == strUrl) || (REST_DEVICE_FILESYS_UNFREEZE == strUrl)
        || (REST_HOST_UNFREEZE_SCRIPT == strUrl) || (REST_APP_UNFREEZE == strUrl) || (REST_APP_UNFREEZEEX == strUrl)
        || (REST_SYBASE_THAW == strUrl) || (REST_HANA_THAW == strUrl))
    {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/*---------------------------------------------------------------------------
Function Name:IsFTRequest
Description  :����url�ж��Ƿ�Ϊvss����ⶳ����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CFTExceptionHandle::IsVSSRequest(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    
    if ((REST_SQLSERVER_FREEZE_DB == strUrl) || (REST_SQLSERVER_UNFREEZE_DB == strUrl)
        || (REST_EXCHANGE_FREEZE_DB == strUrl) || (REST_EXCHANGE_UNFREEZE_DB == strUrl)
        || (REST_SQLSERVER_GET_FREEZE_STAT == strUrl) || (REST_EXCHANGE_GET_FREEZE_STAT == strUrl))
    {
        return MP_TRUE;
    }
    
    return MP_FALSE;
}

/*---------------------------------------------------------------------------
Function Name:IsFTDB2OrOracleRequest
Description  :����url�ж��Ƿ�Ϊdb2��oracle����ⶳ����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CFTExceptionHandle::IsDB2OrOracleRequest(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    
    if ((REST_ORACLE_FREEZE == strUrl) || (REST_ORACLE_UNFREEZE == strUrl)
        || (REST_DB2_FREEZE == strUrl) || (REST_DB2_UNFREEZE == strUrl)
        || (REST_DB2_FREEZESTATE == strUrl) || (REST_ORACLE_FREEZESTATE == strUrl)
        || (REST_SYBASE_FREEZE == strUrl) || (REST_SYBASE_THAW == strUrl) 
        || (REST_SYBASE_GET_FREEZE_STATE == strUrl) || (REST_HANA_FREEZE == strUrl)
        || (REST_HANA_THAW == strUrl) || (REST_HANA_GET_FREEZE_STATE == strUrl))
    {
        return MP_TRUE;
    }
    
    return MP_FALSE;
}

/*---------------------------------------------------------------------------
Function Name:IsFTFSRequest
Description  :����url�ж��Ƿ�Ϊ�ļ�ϵͳ����ⶳ����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CFTExceptionHandle::IsFSRequest(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    
    if ((REST_DEVICE_FILESYS_FREEZE == strUrl) || (REST_DEVICE_FILESYS_UNFREEZE == strUrl)
        || (REST_DEVICE_FILESYS_FREEZESTATUS == strUrl))
    {
        return MP_TRUE;
    }
    
    return MP_FALSE;
}

/*---------------------------------------------------------------------------
Function Name:IsThirdPartyRequest
Description  :����url�ж��Ƿ�Ϊ�����ű�����ⶳ����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CFTExceptionHandle::IsThirdPartyRequest(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    
    if ((REST_HOST_FREEZE_SCRIPT == strUrl) || (REST_HOST_UNFREEZE_SCRIPT == strUrl)
        || (REST_HOST_QUERY_STATUS_SCRIPT == strUrl))
    {
        return MP_TRUE;
    }
    
    return MP_FALSE;
}

/*---------------------------------------------------------------------------
Function Name:IsThirdPartyRequest
Description  :����url�ж��Ƿ�Ϊ���ݵĲ���֪Ӧ�ö�������
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CFTExceptionHandle::IsAppRequest(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    
    if ((REST_APP_FREEZE == strUrl) || (REST_APP_UNFREEZE == strUrl) 
        || (REST_APP_QUERY_DB_FREEZESTATE == strUrl) || (REST_APP_UNFREEZEEX == strUrl))
    {
        return MP_TRUE;
    }
    
    return MP_FALSE;
}


/*---------------------------------------------------------------------------
Function Name:GetRequestAppType
Description  :����url�ж����ݿ�����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::GetRequestAppType(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    mp_int32 iAppType = TYPE_APP_UNKNOWN;
    
    if (mp_string::npos != strUrl.find(ORACLE))
    {
        iAppType = TYPE_APP_ORACLE;
    }
    else if  (mp_string::npos != strUrl.find(DB2))
    {
        iAppType = TYPE_APP_DB2;
    }
    else if  (mp_string::npos != strUrl.find(SQL))
    {
        iAppType = TYPE_APP_SQL;
    }
    else if  (mp_string::npos != strUrl.find(EXCHANGE))
    {
        iAppType = TYPE_APP_EXCHANGE;
    }
    else if (mp_string::npos != strUrl.find(FILESYSTEM))
    {
        iAppType = TYPE_APP_FILESYSTEM;
    }  
    else if (mp_string::npos != strUrl.find(THIRDPARTY))
    {
        iAppType = TYPE_APP_THIRDPARTY;
    }
    else if (mp_string::npos != strUrl.find(APP))
    {
        iAppType = TYPE_APP_APP;
    }
    else if (mp_string::npos != strUrl.find(SYBASE))
    {
        iAppType = TYPE_APP_SYBASE;
    }
    else if (mp_string::npos != strUrl.find(HANA))
    {
        iAppType = TYPE_APP_HANA;
    }
    else
    {
        iAppType = TYPE_APP_UNKNOWN;
    }

    return iAppType;
}

/*---------------------------------------------------------------------------
Function Name:GetRequestInstanceName
Description  :�����������ݻ�ȡʵ������
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CFTExceptionHandle::GetRequestInstanceName(CRequestMsg* pReqMsg, mp_string& strInstanceName)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetGetNum = MP_SUCCESS;
    vector<mp_string> vecDisks;
    mp_string strInstanceTmp;
    mp_string strInstanceNumForSAPHana = "";
    mp_string strInstanceNameForOtherDB = "";
    vector<mp_string>::iterator iter;

    if (IsDB2OrOracleRequest(pReqMsg))
    {
        iRet = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), INST_NAME, strInstanceNameForOtherDB);
        iRetGetNum = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), INST_NUM, strInstanceNumForSAPHana);
        if (MP_SUCCESS != iRet && MP_SUCCESS != iRetGetNum)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json string value failed, key1(%s) for hana, key2(%s) for other db.", 
                mp_string(INST_NUM).c_str(), mp_string(INST_NAME).c_str());
            return iRet;
        }
        strInstanceName = (MP_SUCCESS == iRet ? strInstanceNameForOtherDB : strInstanceNumForSAPHana);
    }
    else if (IsVSSRequest(pReqMsg))
    {
        strInstanceName = VSS_INSTANCE_NAME;
    }
    else if (IsFSRequest(pReqMsg))
    {
        iRet = CJsonUtils::GetJsonArrayString(pReqMsg->GetMsgBody().GetJsonValueRef(), DISKNAMES, vecDisks);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json array value failed, key %s.", mp_string(DISKNAMES).c_str());
            return iRet;
        }
        
        for (iter = vecDisks.begin(); iter != vecDisks.end(); iter++)
        {
            strInstanceTmp = strInstanceTmp + *iter + ":";
        }
        strInstanceName = strInstanceTmp;
    }
    else if (IsThirdPartyRequest(pReqMsg))
    {
        iRet = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME, strInstanceName);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json string value failed, key %s.", REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME);
            return iRet;
        }
    }
    else if (IsAppRequest(pReqMsg))
    {
        strInstanceName = APPINSTNAME;
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupported monitor obj, url %s.", 
            pReqMsg->GetURL().GetProcURL().c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 CFTExceptionHandle::GetRequestDbName(CRequestMsg* pReqMsg, mp_string& strDbName)
{
    mp_int32 iRet = MP_SUCCESS;

    if (IsDB2OrOracleRequest(pReqMsg))
    {
        iRet = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), DBNAME, strDbName);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json string value failed, key %s.", mp_string(DBNAME).c_str());
            return iRet;
        }
    }
    else if (IsVSSRequest(pReqMsg))
    {
        strDbName = VSS_DB_NAME;
    }
    else if (IsFSRequest(pReqMsg))
    {
        strDbName = FILESYTEM_DB_NAME;
    }
    else if (IsThirdPartyRequest(pReqMsg))
    {
        if (pReqMsg->GetMsgBody().GetJsonValueRef().isMember(REST_PARAM_HOST_FREEZE_SCRIPT_PARAM))
        {
            iRet = CJsonUtils::GetJsonString(pReqMsg->GetMsgBody().GetJsonValueRef(), REST_PARAM_HOST_FREEZE_SCRIPT_PARAM, strDbName);
            if (MP_SUCCESS != iRet)
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get json string value failed, key %s.", REST_PARAM_HOST_FREEZE_SCRIPT_PARAM);
                return iRet;
            }
        }
        else
        {
            strDbName = "";
        }
    }
    else if (IsAppRequest(pReqMsg))
    {
        strDbName = APPDBNAME;
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Unsupported monitor obj, url %s.", 
            pReqMsg->GetURL().GetProcURL().c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*---------------------------------------------------------------------------
Function Name:ParseFTUrl
Description  :����Ӧ���������ɻ�ȡdb״̬url
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_string CFTExceptionHandle::GetQueryStatusUrl(mp_int32 iAppType)
{
    switch (iAppType)
    {
    case TYPE_APP_DB2:
        return REST_DB2_FREEZESTATE;
    case TYPE_APP_ORACLE:
        return REST_ORACLE_FREEZESTATE;
    case TYPE_APP_SQL:
        return REST_SQLSERVER_GET_FREEZE_STAT;
    case TYPE_APP_EXCHANGE:
        return REST_EXCHANGE_GET_FREEZE_STAT;
    case TYPE_APP_FILESYSTEM:
        return REST_DEVICE_FILESYS_FREEZESTATUS;
    case TYPE_APP_THIRDPARTY:
        return REST_HOST_QUERY_STATUS_SCRIPT;
    case TYPE_APP_APP:
        return REST_APP_QUERY_DB_FREEZESTATE;
    case TYPE_APP_SYBASE:
        return REST_SYBASE_GET_FREEZE_STATE;
    case TYPE_APP_HANA:
        return REST_HANA_GET_FREEZE_STATE;
    default:
        return "";
    }
}

mp_string CFTExceptionHandle::GetUnFreezeUrl(mp_int32 iAppType)
{
    switch (iAppType)
    {
    case TYPE_APP_DB2:
        return REST_DB2_UNFREEZE;
    case TYPE_APP_ORACLE:
        return REST_ORACLE_UNFREEZE;
    case TYPE_APP_SQL:
        return REST_SQLSERVER_UNFREEZE_DB;
    case TYPE_APP_EXCHANGE:
        return REST_EXCHANGE_UNFREEZE_DB;
    case TYPE_APP_FILESYSTEM:
        return REST_DEVICE_FILESYS_UNFREEZE;
    case TYPE_APP_THIRDPARTY:
        return REST_HOST_UNFREEZE_SCRIPT;
    case TYPE_APP_APP:
        return REST_APP_UNFREEZEEX;
    case TYPE_APP_SYBASE:
        return REST_SYBASE_THAW;
    case TYPE_APP_HANA:
        return REST_HANA_THAW;
    default:
        return "";
    }
}

mp_bool CFTExceptionHandle::IsQueryStatusRequest(CRequestMsg* pReqMsg)
{
    mp_string strUrl = pReqMsg->GetURL().GetProcURL();
    
    if ((REST_ORACLE_FREEZESTATE == strUrl) ||  (REST_DB2_FREEZESTATE == strUrl) 
        ||(REST_SQLSERVER_GET_FREEZE_STAT == strUrl) || (REST_EXCHANGE_GET_FREEZE_STAT == strUrl) 
        || (REST_DEVICE_FILESYS_FREEZESTATUS == strUrl) || (REST_HOST_QUERY_STATUS_SCRIPT == strUrl)
        || (REST_APP_QUERY_DB_FREEZESTATE == strUrl) || (REST_SYBASE_GET_FREEZE_STATE == strUrl)
        || (REST_HANA_GET_FREEZE_STATE == strUrl))
    {
        return MP_TRUE;
    }

    return MP_FALSE;
}


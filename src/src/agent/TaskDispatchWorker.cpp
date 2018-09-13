/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "agent/TaskDispatchWorker.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"

CTaskDispatchWorker::CTaskDispatchWorker()
{
    m_pWorkers = NULL;
    m_iWorkerCount = 0;
    m_lastPushedWorker = 0;
    (mp_void)memset_s(&m_dispatchTid, sizeof(m_dispatchTid), 0, sizeof(m_dispatchTid));
    //Coverity&Fortify��:UNINIT_CTOR
    //Coveirty&Fortify����ʶ��˾��ȫ����memset_s����ʾm_dispatchTid.os_idδ��ʼ��
    m_bNeedExit = MP_FALSE;
    m_iThreadStatus = THREAD_STATUS_IDLE;
}

CTaskDispatchWorker::~CTaskDispatchWorker()
{
    m_pWorkers = NULL;
}

mp_bool CTaskDispatchWorker::NeedExit()
{
    return m_bNeedExit;
}

mp_void CTaskDispatchWorker::SetRecvThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}

/*------------------------------------------------------------ 
Description  : ��ʼ���ַ��߳�
Input        : pTaskWorkers -- task worker�߳������ָ��
               iCount -- task worker�߳�����ĸ���
               pVssWorker -- vss worker�߳�ָ��
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
#ifdef WIN32
mp_int32 CTaskDispatchWorker::Init(CTaskWorker** pTaskWorkers, mp_int32 iCount, CTaskWorker* pVssWorker)
#else
mp_int32 CTaskDispatchWorker::Init(CTaskWorker** pTaskWorkers, mp_int32 iCount)
#endif
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin init task dispatch worker.");
    m_pWorkers = pTaskWorkers;
#ifdef WIN32
    m_pVssWorker = pVssWorker;
#endif
    m_iWorkerCount = iCount;
    iRet = CMpThread::Create(&m_dispatchTid, DispacthProc, this);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init task worker failed, ret %d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Init task dispatch worker succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : �˳��ַ��߳�
Input        : 
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskDispatchWorker::Exit()
{
    CMpThread::WaitForEnd(&m_dispatchTid, NULL);
}

/*------------------------------------------------------------
Description  : �ж��Ƿ���VSS����
Input        : msg -- ����
Output       : 
Return       : MP_TRUE -- ��VSS����
               MP_FALSE -- ����VSS����
Create By    :
Modification : 
-------------------------------------------------------------*/
#ifdef WIN32
mp_bool CTaskDispatchWorker::IsVSSRequst(message_pair_t& msg)
{
    mp_string strUrl = msg.pReqMsg->GetURL().GetProcURL();
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Request url is %s.", strUrl.c_str());

    if (REST_SQLSERVER_FREEZE_DB == strUrl || REST_SQLSERVER_UNFREEZE_DB == strUrl
        || REST_EXCHANGE_FREEZE_DB == strUrl || REST_EXCHANGE_UNFREEZE_DB == strUrl
        || REST_DEVICE_FILESYS_FREEZE == strUrl || REST_DEVICE_FILESYS_UNFREEZE == strUrl
        || REST_APP_FREEZE == strUrl || REST_APP_UNFREEZE == strUrl || REST_APP_ENDBACKUP == strUrl)
    {
        return MP_TRUE;
    }

    return MP_FALSE;
}
#endif

/*------------------------------------------------------------
Description  : ������Ϣ��worker�߳�
Input        : msg -- ��Ϣ
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskDispatchWorker::PushMsgToWorker(message_pair_t& msg)
{
    mp_int32 iWorkIndex = 0;
    CTaskWorker* pCurrWorker = NULL;
    mp_int32 m_firstPushedWorker = m_lastPushedWorker;
#ifdef WIN32
    if(IsVSSRequst(msg))
    {
        m_pVssWorker->PushRequest(msg);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Push vss request to vss worker succ.");
        return;
    }
#endif

    if (m_iWorkerCount <= 0 || NULL == m_pWorkers)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Worker count is zero or worker pointer is null, can't dispatch request.");
        return;
    }

    do
    {
        iWorkIndex = m_lastPushedWorker % m_iWorkerCount;
        m_lastPushedWorker++;
        if (m_lastPushedWorker > MAX_PUSHED_WORKERS_CNT)
        {
            m_lastPushedWorker = 0;
        }
        pCurrWorker = *(m_pWorkers + iWorkIndex);
        if (((m_lastPushedWorker - m_firstPushedWorker) - 1) == m_iWorkerCount)
        {
            msg.pRspMsg->SetRetCode((mp_int64)ERROR_COMMON_PROC_REQUEST_BUSY);
            CCommunication::GetInstance().PushRspMsgQueue(msg);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "All task workers are busy.");
            return;
        }

        if (pCurrWorker->GetThreadProcReqStatus())
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Task worker[%d] is busy.", iWorkIndex + 1);
        }
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Task worker[%d] is idle.", iWorkIndex + 1);
        }
    }while(pCurrWorker->GetThreadProcReqStatus());
    pCurrWorker->PushRequest(msg);
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Push request to task worker[%d] succ.", iWorkIndex + 1);
}

/*------------------------------------------------------------
Description  : �ַ��̵߳��̻߳ص�����
Input        : pThis -- �̻߳ص���������
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/
#ifdef WIN32
DWORD WINAPI CTaskDispatchWorker::DispacthProc(void* pThis)
#else
void* CTaskDispatchWorker::DispacthProc(void* pThis)
#endif
{
    mp_int32 iRet = MP_SUCCESS;
    message_pair_t msg;
    CTaskDispatchWorker* pDispathWorker = (CTaskDispatchWorker*)pThis;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin dispatch request.");
    pDispathWorker->SetRecvThreadStatus(THREAD_STATUS_RUNNING);
    while (!pDispathWorker->NeedExit())
    {
        iRet = CCommunication::GetInstance().PopReqMsgQueue(msg);
        if (MP_SUCCESS != iRet)
        {
            DoSleep(100);
            continue;
        }

        pDispathWorker->PushMsgToWorker(msg);
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End dispatch request.");
    pDispathWorker->SetRecvThreadStatus(THREAD_STATUS_EXITED);
#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}


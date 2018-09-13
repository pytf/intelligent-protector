/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "agent/TaskWorker.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"

CTaskWorker::CTaskWorker()
{
    m_plgCfgParse = NULL;
    m_bClosed = MP_FALSE;
    m_SCN = 0;
    m_plgName = 0;
    m_plgVersion = 0;
    m_plgCfgParse = 0;
    m_plgMgr = 0;
    (mp_void)memset_s(&m_threadId, sizeof(m_threadId), 0, sizeof(m_threadId));
    //Coverity&Fortify��:UNINIT_CTOR
    //Coveirty&Fortify����ʶ��˾��ȫ����memset_s����ʾm_threadId.os_idδ��ʼ��
    CMpThread::InitLock(&m_tPlgLock);
    CMpThread::InitLock(&m_tReqLock);
    m_iThreadStatus = THREAD_STATUS_IDLE;
    m_bNeedExit = MP_FALSE;
    m_bProcReq = MP_FALSE;
}

CTaskWorker::~CTaskWorker()
{
    CMpThread::DestroyLock(&m_tPlgLock);
    CMpThread::DestroyLock(&m_tReqLock);
}

/*------------------------------------------------------------
Description  : ��ʼ��task worker�߳�
Input        : pPlgCfgParse -- ��������ļ���������ָ��
               pPlgMgr -- ����������ָ��
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskWorker::Init(CPluginCfgParse* pPlgCfgParse, CPluginManager* pPlgMgr)
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin init task worker.");

    if (NULL == pPlgCfgParse || NULL == pPlgMgr)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is null.");
        return MP_FAILED;
    }

    m_plgCfgParse = pPlgCfgParse;
    m_plgMgr =pPlgMgr;
    m_SCN = m_plgMgr->GetSCN();

    iRet = CMpThread::Create(&m_threadId, WorkProc, this);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init task worker failed, ret %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Init task worker succ.");

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : �˳�task worker�߳�
Input        :
Output       : 
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskWorker::Exit()
{
    //��ʱ�����̷߳���ֵ
    CMpThread::WaitForEnd(&m_threadId, NULL);
}

/*------------------------------------------------------------
Description  : ����Ϣ�����л�ȡ��Ϣ
Input        :
Output       : msg -- ��ȡ����Ϣ
Return       : MP_SUCCESS -- �ɹ�
               MP_FAILED -- ���������û������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskWorker::PopRequest(message_pair_t& msg)
{
    vector<message_pair_t>::iterator iter;

    CThreadAutoLock tlock(&m_tPlgLock);
    if (m_vecReqQueue.empty())
    {
        return MP_FAILED;
    }

    iter = m_vecReqQueue.begin();
    msg = *iter;
    m_vecReqQueue.erase(iter);

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : ����Ϣ���浽��Ϣ����
Input        : msg -- Ҫ���浽���е���Ϣ
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskWorker::PushRequest(message_pair_t& msg)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    m_vecReqQueue.push_back(msg);
}

mp_bool CTaskWorker::NeedExit()
{
    return m_bNeedExit;
}

mp_bool CTaskWorker::GetThreadProcReqStatus()
{
    return m_bProcReq;
}

mp_void CTaskWorker::SetThreadStatus(mp_int32 iThreadStatus)
{
    m_iThreadStatus = iThreadStatus;
}

/*------------------------------------------------------------
Description  : ������Ϣ����task worker�̻߳ص��������øú�����������
Input        :
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CTaskWorker::ReqProc()
{
    CServicePlugin* pPlugin = NULL;
    mp_string strService;
    message_pair_t msg;
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin process request.");

    SetThreadStatus(THREAD_STATUS_RUNNING);
    while (!NeedExit())
    {
        m_SCN = m_plgMgr->GetSCN(); //lint !e613
        iRet = PopRequest(msg);
        if (MP_SUCCESS != iRet)
        {
            m_bProcReq = MP_FALSE;
            DoSleep(100);
            continue;
        }
        m_bProcReq = MP_TRUE;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get request succ.");

        strService = msg.pReqMsg->GetURL().GetServiceName();
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get service %s.", strService.c_str());
        
        pPlugin = GetPlugin((mp_char*)strService.c_str());
        if (NULL == pPlugin)
        {
            msg.pRspMsg->SetRetCode((mp_int64)ERROR_COMMON_PLUGIN_LOAD_FAILED);
            CCommunication::GetInstance().PushRspMsgQueue(msg);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get plugin failed, type %s.", strService.c_str());
            DoSleep(100);
            continue;
        }

        //��ȡ��ǰ�Ĳ�����Ƽ�scn
        SetPlugin(pPlugin);
        iRet = pPlugin->Invoke(msg.pReqMsg, msg.pRspMsg);
        if (MP_SUCCESS != iRet)
        {
            //������ĳЩ��֧û�����÷����룬����ͳһ����
            if (msg.pRspMsg->GetRetCode() == MP_SUCCESS)
            {
                msg.pRspMsg->SetRetCode((mp_int64)iRet);
            }
            CCommunication::GetInstance().PushRspMsgQueue(msg);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invoke service plugin failed, iRet %d.", iRet);
            continue;
        }

        CCommunication::GetInstance().PushRspMsgQueue(msg);
    }
    
    SetThreadStatus(THREAD_STATUS_EXITED);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Process request succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : �жϲ���Ƿ����ж��(�����ܶ�̬����Ԥ��)
Input        : newSCN -- scn��
               plgName -- �����
Output       : 
Return       : MP_TRUE -- ����ж��
               MP_FALSE -- ������ж��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CTaskWorker::CanUnloadPlugin(mp_uint64 newSCN, const char* plgName)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    //workû�й������ߵ�ǰʹ�õĲ��������Ҫɾ���Ĳ�������ɾ��
    if (NULL == m_plgName || 0 != strcmp(plgName, m_plgName))
    {
        return MP_TRUE;
    }

    //work��ǰʹ�õĲ������Ҫɾ����һ�£�scnһ��˵��work�Ѿ���ʹ���µĲ�������ɾ��
    if (newSCN == m_SCN)
    {
        return MP_TRUE;
    }

    //��ǰwork����ʹ�þɵĲ����������ɾ��
    return MP_FALSE;
}

/*------------------------------------------------------------
Description  : ���ݷ������ƻ�ȡ���
Input        : pszService -- ������
Output       : 
Return       : �ɹ����ػ�ȡ�Ĳ��ָ�룬ʧ�ܷ���NULL
Create By    :
Modification : 
-------------------------------------------------------------*/
CServicePlugin* CTaskWorker::GetPlugin(mp_char* pszService)
{
    mp_int32 iRet = MP_SUCCESS;
    plugin_def_t plgDef;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin get plugin.");
    if (NULL == pszService)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Input param is null.");
        return NULL;
    }
    //CodeDex�󱨣�Dead Code
    iRet = m_plgCfgParse->GetPluginByService(pszService, plgDef);  //lint !e613
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get plugin failed.");
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get plugin %s.", plgDef.name.c_str());
    IPlugin* pPlg = m_plgMgr->GetPlugin(plgDef.name.c_str());  //lint !e613
    if (NULL == pPlg)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Load new plugin %s.", plgDef.name.c_str());
        pPlg = m_plgMgr->LoadPlugin(plgDef.name.c_str());  //lint !e613
        if (NULL == pPlg)
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Load plugin failed, name %s.", plgDef.name.c_str());
            return NULL;
        }
    }

    if (pPlg->GetTypeId() != CServicePlugin::APP_PUGIN_ID)
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Plugin's type is wrong. expect = %d, actual = %d.",
            CServicePlugin::APP_PUGIN_ID, pPlg->GetTypeId());
        return NULL;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get plugin succ.");
    return (CServicePlugin*)pPlg;
}

/*------------------------------------------------------------
Description  : �������ȹ���Ϣ(�����ܶ�̬����Ԥ��)
Input        : pPlug -- ���ָ��
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CTaskWorker::SetPlugin(CServicePlugin* pPlug)
{
    CThreadAutoLock tlock(&m_tPlgLock);
    //m_SCN = CPluginManager::GetImpl()->GetSCN();
    m_plgVersion = pPlug->GetVersion();
    m_plgName = pPlug->GetName();
    //m_workState = workStat_work;
    //COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Set plugin info, scn %d, version %s, name %s.", m_SCN,
    //    m_plgVersion, m_plgName);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Set plugin info, scn %d.", m_SCN);
}

/*------------------------------------------------------------
Description  : Task worker�̻߳ص�����
Input        : pThis -- �̻߳ص���������
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
#ifdef WIN32
DWORD WINAPI CTaskWorker::WorkProc(void* pThis)
#else
void* CTaskWorker::WorkProc(void* pThis)
#endif
{
    CTaskWorker* pTaskWorker = (CTaskWorker*)pThis;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin request process.");

    (void)pTaskWorker->ReqProc();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "End request process.");

#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}


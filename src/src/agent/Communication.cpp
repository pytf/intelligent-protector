/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#include "agent/Communication.h"
#include "agent/Authentication.h"
#include "agent/FTExceptionHandle.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "fcgi/include/fcgios.h"

CCommunication CCommunication::m_instance;

/*------------------------------------------------------------
Function Name: ~CCommunication
Description  : ��������������Դ�����ͷ�
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/

CCommunication::~CCommunication()
{
    DeleteRequest();
    //�̻߳���
    //�ͷŻ�����
    CMpThread::DestroyLock(&m_reqTableMutex);
    CMpThread::DestroyLock(&m_reqMsgQueueMutex);
    CMpThread::DestroyLock(&m_rspMsgQueueMutex);
}

/*------------------------------------------------------------
Function Name: init
Description  : ��ʼ�������������˿ڣ������߳�
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CCommunication::Init()
{
    LOGGUARD("");
    //��ʼ��FCGI������Ϣ
    mp_int32 iRet = FCGX_Init();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "FCGX_Init error! iRet = %d\n.", iRet);
        return ERROR_COMMON_OPER_FAILED;
    }

    //����FCGI�����˿�
    mp_string strPort;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_PORT, strPort);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get port number from xml config failed.");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    mp_string strAddress;
#ifdef WIN32
    strAddress = "localhost:" + strPort;
#else
    strAddress = "127.0.0.1:" + strPort;
#endif
    mp_int32 handler = FCGX_OpenSocket(strAddress.c_str(), 100);
    if (handler < 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "FCGX_OpenSocket failed! handler = %d\n.", handler);
        return ERROR_COMMON_OPER_FAILED;
    }

    //�����ļ���������FD_CLOEXEC��־����֤fork�������ӽ��̲��̳и����̵���Դ
#ifndef WIN32
    mp_int32 iFlag = fcntl(handler, F_GETFD);
    if (iFlag == MP_FAILED)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "fcntl failed! handler = %d\n.", handler);
        OS_Close(handler, MP_TRUE);
        return ERROR_COMMON_OPER_FAILED;
    }
    iFlag = (mp_uint32)iFlag | FD_CLOEXEC;
    iRet = fcntl(handler, F_SETFD, iFlag);
    if (iRet == MP_FAILED)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "fcntl failed! handler = %d\n.", handler);
        OS_Close(handler, MP_TRUE);
        return ERROR_COMMON_OPER_FAILED;
    }
#endif

    iRet = InitRequest(handler);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init request failed! iRet = %d\n.", iRet);
        OS_Close(handler, MP_TRUE);
        return iRet;
    }
	//CodeDex�󱨣�KLOCWORK.RH.LEAK
	//CodeDex�󱨣�Unreleased Resource
    iRet = CMpThread::Create(&m_hReceiveThread, ReceiveThreadFunc, this);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create receive thread failed! iRet = %d\n.", iRet);
        OS_Close(handler, MP_TRUE);
        return iRet;
    }

    iRet = CMpThread::Create(&m_hSendThread, SendThreadFunc, this);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Create send thread failed! iRet = %d\n.", iRet);
        OS_Close(handler, MP_TRUE);
        return iRet;
    }

    //��ʼ����Ȩģ��
    iRet = CAuthentication::GetInstance().Init();
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Init authentication failed! iRet = %d\n.", iRet);
        OS_Close(handler, MP_TRUE);
        return iRet;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: InitRequest
Description  : Ԥ����FCGX_Request����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CCommunication::InitRequest(mp_int32 handler)
{
    LOGGUARD("");
    CThreadAutoLock lock(&m_reqTableMutex);
    for (mp_uint32 i = 0; i < MAX_REQUEST_HANDLER; i++)
    {
        request_handler_info_t rh;
        rh.pFcgxReq = NULL;
        rh.isUsed = false;
        m_ReqHandlerTable.push_back(rh);
    }

    mp_int32 iRet = MP_SUCCESS;
    //��ǰ����100��req���󣬽��fastcgiֻ�ܷ���128��request��������
    for (vector<request_handler_info_t>::iterator it = m_ReqHandlerTable.begin(); it != m_ReqHandlerTable.end(); it++)
    {
        FCGX_Request* pfcgxReq;
        //CodeDex�󱨣�Memory Leak
        NEW_CATCH(pfcgxReq, FCGX_Request);
        if (NULL == pfcgxReq)
        {
            //��¼��־
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "new FCGX_Request failed!");
            iRet = ERROR_COMMON_OPER_FAILED;
            break;
        }
        FCGX_InitRequest(pfcgxReq, handler, 0);
        it->pFcgxReq = pfcgxReq;
    }

    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Allocate request handler failed!");
        DeleteRequest();
    }

    return iRet;
}

/*------------------------------------------------------------
Function Name: GetFcgxReq
Description  : ��ȡ���е�FCGX_Request����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
FCGX_Request* CCommunication::GetFcgxReq()
{
    LOGGUARD("");
    CThreadAutoLock lock(&m_reqTableMutex);
    for (vector<request_handler_info_t>::iterator it = m_ReqHandlerTable.begin(); it != m_ReqHandlerTable.end(); it++)
    {
        if (!it->isUsed)
        {
            it->isUsed = MP_TRUE;
            return it->pFcgxReq;
        }
    }

    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "All Request handlers have been used");
    return NULL;
}

/*------------------------------------------------------------
Function Name: DeleteRequest
Description  : ɾ������FCGX_Request��������ʱ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommunication::DeleteRequest()
{
    CThreadAutoLock lock(&m_reqTableMutex);
    for(vector<request_handler_info_t>::iterator it = m_ReqHandlerTable.begin(); it != m_ReqHandlerTable.end(); it++)
    {
        if (it->pFcgxReq != NULL)
        {
            delete it->pFcgxReq;
            it->pFcgxReq = NULL;
        }
    }
}

mp_bool CCommunication::NeedExit()
{
    return m_bNeedExit;
}

mp_void CCommunication::SetRecvThreadStatus(mp_int32 iThreadStatus)
{
    m_iRecvThreadStatus = iThreadStatus;
}

mp_void CCommunication::SetSendThreadStatus(mp_int32 iThreadStatus)
{
    m_iSendThreadStatus = iThreadStatus;
}

/*------------------------------------------------------------
Function Name: ReleaseRequest
Description  : �ͷ�FCGX_Request����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommunication::ReleaseRequest(FCGX_Request* pReq)
{
    LOGGUARD("");
    CThreadAutoLock lock(&m_reqTableMutex);
    for(vector<request_handler_info_t>::iterator it = m_ReqHandlerTable.begin(); it != m_ReqHandlerTable.end(); it++)
    {
        if (it->pFcgxReq == pReq)
        {
            it->isUsed = false;
            break;
        }
    }
}

/*---------------------------------------------------------------------
Function Name: SendUnAuthedMsg
Description  : ����ʧ����Ϣ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommunication::SendFailedMsg(CCommunication *pInstance, FCGX_Request *pFcgiReq, mp_int32 iHttpStatus, mp_int32 iRetCode)
{
    if (!pFcgiReq || !pInstance)
    {
        return;
    }
    CResponseMsg rsp(pFcgiReq);
    rsp.SetRetCode((mp_int64)iRetCode);
    rsp.SetHttpStatus(iHttpStatus);  //��server�˷��ؾ���״̬
    rsp.Send();
    pInstance->ReleaseRequest(pFcgiReq);
}

/*---------------------------------------------------------------------
Function Name: HandleReceiveMsg
Description  : ����ͨ��fastcgi���ܵ�����Ϣ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
void CCommunication::HandleReceiveMsg(CCommunication *pInstance, FCGX_Request *pFcgiReq)
{
    //CodeDex�󱨣�UNUSED_VALUE
    if (!pInstance || !pFcgiReq)
    {
        return;
    }

    CRequestMsg *pReq;
    CResponseMsg *pRsp;
    try
    {
        //CodeDex�󱨣�Memory Leak
        pReq = new CRequestMsg(pFcgiReq);
    }
    catch(...)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New CRequestMsg failed");
        pReq = NULL;
    }
    if (!pReq)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New CRequestMsg failed.");
        SendFailedMsg(pInstance, pFcgiReq, SC_NOT_ACCEPTABLE, MP_FAILED);
        return;
    }

    try
    {
        pRsp= new CResponseMsg(pFcgiReq);
    }
    catch(...)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New CResponseMsg failed");
        pRsp = NULL;
    }
    if (!pRsp)
    {
        delete pReq;
        pReq = NULL;
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "New CResponseMsg failed.");
        SendFailedMsg(pInstance, pFcgiReq, SC_NOT_ACCEPTABLE, MP_FAILED);
        return;
    }

    //������Ϣ�е�����
    mp_int32 iRet = pReq->Parse();
    if (iRet != 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Parse request failed! iRet = %d.", iRet);
        SendFailedMsg(pInstance, pFcgiReq, SC_NOT_ACCEPTABLE, iRet);
        delete pReq;
        pReq = NULL;
        delete pRsp;
        pRsp = NULL;
        return;
    }

    //��ǰ�Զ���������д�����ֹ���������agent�쳣�˳����½ⶳ����û�м�¼
    CFTExceptionHandle::GetInstance().MonitorFreezeOper(pReq);

    message_pair_t stReqMsg(pReq, pRsp);
    //����Ϣ���������
    pInstance->PushReqMsgQueue(stReqMsg);

} //lint !e429

/*---------------------------------------------------------------------
Function Name: ReceiveThreadFunc
Description  : �����̴߳�����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
#ifdef WIN32
DWORD WINAPI CCommunication::ReceiveThreadFunc(mp_void* pThis)
#else
mp_void * CCommunication::ReceiveThreadFunc(mp_void* pThis)
#endif
{
    LOGGUARD("");
    CCommunication* pInstance = (CCommunication*)pThis;
    mp_int32 iRet = 0;
    mp_bool bRet;
    
    pInstance->SetRecvThreadStatus(THREAD_STATUS_RUNNING);
    while (!pInstance->NeedExit())
    {
        FCGX_Request *pFcgiReq = pInstance->GetFcgxReq();
        if (NULL == pFcgiReq)
        {
            //��¼��־
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Call GetFcgxReq failed!");
            //����2����;
            DoSleep(RCV_SLEEP_TIME * 100);
            continue;
        }

        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin accept fcgx");
        iRet = FCGX_Accept_r(pFcgiReq);
        if (iRet < 0)
        {
            //��¼��־
            pInstance->ReleaseRequest(pFcgiReq);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "FCGX accept failed! iRet = %d.", iRet);
            DoSleep(RCV_SLEEP_TIME);
            continue;
        }
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End accept fcgx");
        CRequestMsg req(pFcgiReq);
        mp_char **allHead = req.GetHttpReq().GetAllHead();
        if (!allHead)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Http head is null!");
            SendFailedMsg(pInstance, pFcgiReq, SC_UNAUTHORIZED, MP_FAILED);
            continue;
        }
        for (mp_uint32 i = 0; allHead[i] != 0; i++)
        {
            //����ӡ������Ϣ
            bRet = (!allHead[i] || NULL != strstr(allHead[i], PW) || NULL != strstr(allHead[i], HTTPPARAM_DBPASSWORD)
                       || NULL != strstr(allHead[i], HTTPPARAM_ASMPASSWORD) 
                       || NULL != strstr(allHead[i], HTTPPARAM_SNMPAUTHPW)
                       || NULL != strstr(allHead[i], HTTPPARAM_SNMPENCRYPW));
            if (bRet)
            {
                continue;
            }
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "[Http Head Recieved]: \"%s\"", allHead[i]);
        }

        mp_string strUser = req.GetHttpReq().GetHead(UNAME);
        mp_string strPW = req.GetHttpReq().GetHead(PW);
        mp_string strClientIP = req.GetHttpReq().GetRemoteIP();
        iRet = CAuthentication::GetInstance().Auth(strClientIP, strUser, strPW);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Authenticate failed! IP = \"%s\", User name = \"%s\".",
                      strClientIP.c_str(), strUser.c_str());
            SendFailedMsg(pInstance, pFcgiReq, SC_UNAUTHORIZED, iRet);
            continue;
        }
        else
        {
            COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "User \"%s\" from \"%s\" has been authenticated successfully!",
                    strUser.c_str(), strClientIP.c_str());
        }

        HandleReceiveMsg(pInstance, pFcgiReq);
        strPW.replace(0, strPW.length(), "");
    }//lint !e429

    pInstance->SetRecvThreadStatus(THREAD_STATUS_EXITED);
#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}

/*------------------------------------------------------------
Function Name: SendThreadFunc
Description  : �����̴߳�����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
#ifdef WIN32
DWORD WINAPI CCommunication::SendThreadFunc(mp_void* pThis)
#else
mp_void *CCommunication::SendThreadFunc(mp_void* pThis)
#endif
{
	//CodeDex�󱨣�UNUSED_VALUE
    LOGGUARD("");
    CCommunication* pInstance = (CCommunication*)pThis;

    pInstance->SetSendThreadStatus(THREAD_STATUS_RUNNING);
    while (!pInstance->NeedExit())
    {
        message_pair_t rspMsg;
        mp_int32 iRet = CCommunication::GetInstance().PopRspMsgQueue(rspMsg);
        if (iRet != 0)
        {
            DoSleep(RCV_SLEEP_TIME);
            continue;
        }

        if (NULL == rspMsg.pReqMsg || NULL == rspMsg.pRspMsg)
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "NULL pointer, rspMsg.pReqMsg = %d, rspMsg.pRspMsg = %d", 
                   rspMsg.pReqMsg ,rspMsg.pRspMsg);
            continue;
        }

        //�Զ���ⶳ������й���
        CFTExceptionHandle::GetInstance().UpdateFreezeOper(rspMsg.pReqMsg, rspMsg.pRspMsg);

        //���ͷ�����Ϣ
        rspMsg.pRspMsg->Send();

        //�ͷ�request
        CCommunication::GetInstance().ReleaseRequest(rspMsg.pReqMsg->GetHttpReq().GetFcgxReq());

        //�ͷ��ڴ�
        if (rspMsg.pReqMsg)
        {
            delete rspMsg.pReqMsg;
            rspMsg.pReqMsg = NULL;
        }
        if (rspMsg.pRspMsg)
        {
            delete rspMsg.pRspMsg;
            rspMsg.pRspMsg = NULL;
        }
    }

    pInstance->SetSendThreadStatus(THREAD_STATUS_EXITED);
#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

/*------------------------------------------------------------
Function Name: PopReqMsgQueue
Description  : ����������л�ȡ������ǰ�����Ϣ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CCommunication::PopReqMsgQueue(message_pair_t &msgPair)
{
    //��ȡ������
    CThreadAutoLock lock(&m_reqMsgQueueMutex);
    if (!m_reqMsgQueue.empty())
    {
        LOGGUARD("");
        vector<message_pair_t>::iterator it = m_reqMsgQueue.begin();
        msgPair = *it;
        m_reqMsgQueue.erase(it);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
           "pop message from request queue success, req=0x%x, rsp=0x%x!", msgPair.pReqMsg, msgPair.pRspMsg);
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

/*------------------------------------------------------------
Function Name: PushReqMsgQueue
Description  : ����Ϣ���뵽������Ϣ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommunication::PushReqMsgQueue(message_pair_t msgPair)
{
    LOGGUARD("");
    //��ȡ������
    CThreadAutoLock lock(&m_reqMsgQueueMutex);
    m_reqMsgQueue.push_back(msgPair);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
        "push message to request queue success, req=0x%x, rsp=0x%x!", msgPair.pReqMsg, msgPair.pRspMsg);
    //�ͷŻ�����
}

/*------------------------------------------------------------
Function Name: PopRspMsgQueue
Description  : ����Ϣ��Ӧ������ȡ��������ǰ�����Ӧ��Ϣ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CCommunication::PopRspMsgQueue(message_pair_t &msgPair)
{
    //��ȡ������
    CThreadAutoLock lock(&m_rspMsgQueueMutex);
    if (!m_rspMsgQueue.empty())
    {
        vector<message_pair_t>::iterator it = m_rspMsgQueue.begin();
        while(it != m_rspMsgQueue.end() && it->pRspMsg->IsInternalMsg())
        {
            it++;
        }

        if (it == m_rspMsgQueue.end())
        {
            return MP_FAILED;
        }
        msgPair = *it;
        m_rspMsgQueue.erase(it);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
          "pop message from response queue success, req=0x%x, rsp=0x%x!", msgPair.pReqMsg, msgPair.pRspMsg);
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }

}

/*------------------------------------------------------------
Function Name: PopRspInternalMsgQueue
Description  : ����Ϣ��Ӧ������ȡ��������ǰ����ڲ���Ӧ��Ϣ����ν�ڲ���Ϣ��ָagent�ڲ���������Ϣ����
               �������ͨ��fcgi���ⷵ��
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CCommunication::PopRspInternalMsgQueue(message_pair_t &msgPair)
{
    //��ȡ������
    CThreadAutoLock lock(&m_rspMsgQueueMutex);
    if (!m_rspMsgQueue.empty())
    {
        LOGGUARD("");
        vector<message_pair_t>::iterator it = m_rspMsgQueue.begin();
        while(it != m_rspMsgQueue.end() && !it->pRspMsg->IsInternalMsg())
        {
            it++;
        }

        if (it == m_rspMsgQueue.end())
        {
            return MP_FAILED;
        }
        msgPair = *it;
        m_rspMsgQueue.erase(it);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
          "pop internal message from response queue success, req=0x%x, rsp=0x%x!", msgPair.pReqMsg, msgPair.pRspMsg);
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }

}

/*------------------------------------------------------------
Function Name: PushRspMsgQueue
Description  : ����Ӧ��Ϣ���뵽��Ϣ��Ӧ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommunication::PushRspMsgQueue(message_pair_t msgPair)
{
    LOGGUARD("");
    //��ȡ������
    CThreadAutoLock lock(&m_rspMsgQueueMutex);
    m_rspMsgQueue.push_back(msgPair);
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG,
          "push message to response queue success, req=0x%x, rsp=0x%x!", msgPair.pReqMsg, msgPair.pRspMsg);
}


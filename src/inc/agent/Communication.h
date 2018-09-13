/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

******************************************************************************
File Name     : Communication.h
Version       : Initial Draft
Author        : 
Created       : 2015/01/19
Last Modified :
Description   : ͨ���ඨ��
History       :
1.Date        :
Author      :
Modification:
******************************************************************************/
#ifndef _AGENT_COMMUNICATION_H_
#define _AGENT_COMMUNICATION_H_

#include "common/Types.h"
#include "rest/HttpCGI.h"
#include "rest/MessageProcess.h"
#include "common/Thread.h"

#define MAX_REQUEST_HANDLER 100
#define RCV_SLEEP_TIME 20

//����Ԥ�����FCGX_Request�����ʹ��״̬
typedef struct tag_request_handler_info
{
    FCGX_Request* pFcgxReq;
    mp_bool isUsed;
}request_handler_info_t;

//��Ϣ�ԣ���Ϣ���д洢����������
typedef struct tag_message_pair_t
{
    CRequestMsg *pReqMsg;
    CResponseMsg *pRspMsg;
    tag_message_pair_t()
    {
        pReqMsg = NULL;
        pRspMsg = NULL;
    }
    tag_message_pair_t(CRequestMsg * pReq,  CResponseMsg * pRsp)
    {
        pReqMsg = pReq;
        pRspMsg = pRsp;
    }
}message_pair_t;

class CCommunication
{
public:
    static CCommunication &GetInstance()
    {
        return m_instance;
    }

    ~CCommunication();
    mp_int32 Init();
    mp_bool IsQueueEmpty()
    {
        return m_reqMsgQueue.empty() ? true : false;
    }

    mp_void ReleaseRequest(FCGX_Request* pReq);
    mp_int32 PopReqMsgQueue(message_pair_t &msgPair);
    mp_void PushReqMsgQueue(message_pair_t msgPair);
    mp_int32 PopRspMsgQueue(message_pair_t &msgPair);
    mp_void PushRspMsgQueue(message_pair_t msgPair);
    mp_int32 PopRspInternalMsgQueue(message_pair_t &msgPair);

private:
    CCommunication()
    {     
        (mp_void)memset_s(&m_hReceiveThread, sizeof(m_hReceiveThread), 0, sizeof(m_hReceiveThread));
        (mp_void)memset_s(&m_hSendThread, sizeof(m_hSendThread), 0, sizeof(m_hSendThread));
        //Coverity&Fortify��:UNINIT_CTOR
        //Coveirty&Fortify����ʶ��˾��ȫ����memset_s����ʾm_dispatchTid.os_idδ��ʼ��
        CMpThread::InitLock(&m_reqTableMutex);
        CMpThread::InitLock(&m_reqMsgQueueMutex);
        CMpThread::InitLock(&m_rspMsgQueueMutex);
        m_iRecvThreadStatus = THREAD_STATUS_IDLE;
        m_iSendThreadStatus = THREAD_STATUS_IDLE;
        m_bNeedExit = MP_FALSE;
    }
    mp_int32 InitRequest(mp_int32 handler);
    mp_void DeleteRequest();
    mp_bool NeedExit();
    mp_void SetRecvThreadStatus(mp_int32 iThreadStatus);
    mp_void SetSendThreadStatus(mp_int32 iThreadStatus);
    FCGX_Request* GetFcgxReq();
#ifdef WIN32
    static DWORD WINAPI ReceiveThreadFunc(mp_void* pThis);
    static DWORD WINAPI SendThreadFunc(mp_void* pThis);
#else
    static mp_void* ReceiveThreadFunc(mp_void* pThis);
    static mp_void* SendThreadFunc(mp_void* pThis);
#endif
    static mp_void SendFailedMsg(CCommunication *pInstance, FCGX_Request *pFcgiReq, mp_int32 iHttpStatus, mp_int32 iRetCode);
    static mp_void HandleReceiveMsg(CCommunication *pInstance, FCGX_Request *pFcgiReq);

private:
    static CCommunication m_instance;                  //��������
    vector<request_handler_info_t> m_ReqHandlerTable;  //�洢Ԥ�����FCGX_Request����
    thread_lock_t m_reqTableMutex;                     //m_ReqHandlerTable���ʻ�����
    vector<message_pair_t> m_reqMsgQueue;              //������Ϣ����
    thread_lock_t m_reqMsgQueueMutex;                  //m_reqMsgQueue���ʻ�����
    vector<message_pair_t> m_rspMsgQueue;              //������Ϣ����
    thread_lock_t m_rspMsgQueueMutex;                  //m_reqMsgQueue���ʻ�����
    thread_id_t m_hReceiveThread;                      //�����Ľ����߳̾��
    thread_id_t m_hSendThread;                         //�����ķ����߳̾��
    volatile mp_bool m_bNeedExit;                      //�߳��˳���ʶ
    volatile mp_int32 m_iRecvThreadStatus;             //�����߳�״̬
    volatile mp_int32 m_iSendThreadStatus;             //�����߳�״̬
};


#endif //_AGENT_COMMUNICATION_H_


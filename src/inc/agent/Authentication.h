/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

******************************************************************************
File Name     : Authentication.h
Version       : Initial Draft
Author        : 
Created       : 2015/01/19
Last Modified :
Description   : ��Ȩ�ඨ��
History       :
1.Date        :
Author      :
Modification:
******************************************************************************/
#ifndef _AGENT_AUTHENTICATION_H_
#define _AGENT_AUTHENTICATION_H_

#include "common/Types.h"
#include "common/Thread.h"
#include <vector>

#define MAX_TRY_TIME 3  //����ʧ�ܴ��� 3��
#define LOCKED_TIME 15 * 60   //��15����������ʧ��3�Σ�����15����
#define CONTINUOUS_FAILURE_TIME   15 * 60   //����ʧ�ܵ�¼��¼�������30���ӣ��򲻼���������¼ʧ�ܴ���

typedef struct st_locked_client_info_st
{
    mp_bool isLocked;        //�Ƿ�������true ������false δ����
    mp_int64 failedTimes;    //������¼ʧ�ܴ���
    mp_uint64 lastFailedTime;//��һ�μ�Ȩʧ��ʱ��
    mp_uint64 lockedTime;      //����ʱ�䣬����������£���ֵ��0
    mp_string strClientIP;   //�ͻ���ip��ַ
}locked_client_info;

class CAuthentication
{
public:
    static CAuthentication& GetInstance()
    {
        return m_instance;
    }
    mp_int32 Init();
    mp_int32 Auth(mp_string& strClientIP, mp_string& strUsr, mp_string& strPw);
    ~CAuthentication()
    {
        CMpThread::DestroyLock(&m_lockedIPListMutex);
    }
private:
    CAuthentication()
    {
        CMpThread::InitLock(&m_lockedIPListMutex);
    }
    mp_bool IsLocked(mp_string& strClientIP);
    mp_bool Check(mp_string& strUsr, mp_string& strPw);
    mp_void Lock(mp_string& strClientIP);
    mp_void Unlock(mp_string& strClientIP);

private:
    mp_string m_strUsr;  //������û�����sha256
    mp_string m_strPwd;  //�����pw��sha256
    static CAuthentication m_instance;   //��������
    vector<locked_client_info> m_lockedIPList;    //����http�ͻ���ip��ַ�б�
    thread_lock_t m_lockedIPListMutex;  //m_lockedIPList���ʻ�����
};
#endif

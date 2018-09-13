/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Thread.h"
#include "common/Time.h"
#include "common/Utils.h"

/*------------------------------------------------------------ 
Description  : �����߳�
Input        : id -- ���߳�id
               proc -- �̻߳ص�����
               arg -- �̲߳���
               uiStackSize -- �߳�ջ��С
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::Create(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
#ifdef WIN32
    id->handle = CreateThread(NULL, uiStackSize, proc, arg, 0, &id->os_id);
    if (NULL == id->handle)
    {
        return MP_FAILED;
    }
#else
    mp_int32 iRet = MP_SUCCESS;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    iRet = pthread_attr_setstacksize(&attr, (mp_int32)uiStackSize);
    if (0 != iRet)
    {
        (mp_void)pthread_attr_destroy(&attr);
        return MP_FAILED;
    }

    iRet = pthread_create(&id->os_id, &attr, proc, arg);
    if (0 != iRet)
    {
        (mp_void)pthread_attr_destroy(&attr);
        return MP_FAILED;
    }

    (mp_void)pthread_attr_destroy(&attr);
#endif
    
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : �ȴ��߳�
Input        : id -- ���߳�id
               retValue -- 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::WaitForEnd(thread_id_t* id, mp_void** retValue)
{
#ifdef WIN32
    if(WaitForSingleObject(id->handle, INFINITE) == WAIT_FAILED)
    {
        return MP_FAILED;
    }
    
    if(NULL != retValue)
    {
        (mp_void)GetExitCodeThread(id->handle, (LPDWORD)retValue);
    }
    (mp_void)CloseHandle(id->handle);
    id->handle = NULL;
#else
    mp_int32 iRet;

    iRet = pthread_join(id->os_id, retValue);
    if (iRet != 0)
    {
        return MP_FAILED;
    }
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ��ʼ���߳���
Input        : plock -- �߳���
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::InitLock(thread_lock_t* plock)
{
    if (NULL == plock)
    {
        return MP_FAILED;
    }

#ifdef WIN32
    InitializeCriticalSection(plock);
#else
    if (0 != pthread_mutex_init(plock, NULL))
    {
        return MP_FAILED;
    }
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ������
Input        : plock -- �߳���
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::DestroyLock(thread_lock_t* plock)
{
    if (NULL == plock)
    {
        return MP_FAILED;
    }
    
#ifdef WIN32
    DeleteCriticalSection(plock);
#else
    if (0 != pthread_mutex_destroy(plock))
    {
        return MP_FAILED;
    }
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ��ʼ����������
Input        : pcond -- ��������
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::InitCond(thread_cond_t* pcond)
{
    if (NULL == pcond)
    {
        return MP_FAILED;
    }
    
#ifdef WIN32
    pcond->sem = CreateSemaphore(NULL, 0, 2048, NULL);
    pcond->count = 0;
#else
    (mp_void)pthread_mutex_init(&pcond->lock, NULL);
    (mp_void)pthread_cond_init(&pcond->cond, NULL);
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ������������
Input        : pcond -- ��������
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::DestroyCond(thread_cond_t* pcond)
{
    if (NULL == pcond)
    {
        return MP_FAILED;
    }
    
#ifdef WIN32
    (mp_void)CloseHandle(pcond->sem);
    pcond->sem = NULL;
    pcond->count = 0;
#else
    (mp_void)pthread_mutex_destroy(&pcond->lock);
    (mp_void)pthread_cond_destroy(&pcond->cond);
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ��ȡ�߳�������ȡ�����߳���ʱ������
Input        : plock -- �߳���
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::Lock(thread_lock_t* plock)
{
    if (NULL == plock)
    {
        return MP_FAILED;
    }

#ifdef WIN32
    EnterCriticalSection(plock);
#else
    if (0 != pthread_mutex_lock(plock))
    {
        return MP_FAILED;   //lint !e454
    }
#endif

    return MP_SUCCESS;  //lint !e454
}

/*------------------------------------------------------------ 
Description  : ���Ի�ȡ�߳�������ȡ�����߳����������˳�
Input        : plock -- �߳���
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
               (THREAD_LOCK_BUSY -- �߳����������߳�ռ��)
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::TryLock(thread_lock_t* plock)
{
#ifdef WIN32
    if (0 == TryEnterCriticalSection(plock))
    {
        return THREAD_LOCK_BUSY;
    }

    return MP_SUCCESS;
#else
    mp_int32 iRet = 0;

    iRet = pthread_mutex_trylock(plock);
    if (0 == iRet)
    {
        return MP_SUCCESS;
    }
    else if (EBUSY == iRet)
    {
        return THREAD_LOCK_BUSY;
    }
    else
    {
        return MP_FAILED;
    }
#endif
}

/*------------------------------------------------------------ 
Description  : �������Լ��ʱ��
Input        : uiTimeoutInMillisecs -- ��ʱʱ��
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_uint32 CMpThread::CalcRetryInterval(mp_uint32 uiTimeoutInMillisecs)
{
    mp_uint32 result = uiTimeoutInMillisecs / 100;
    return result < 1 ? 1 : result;
}

/*------------------------------------------------------------ 
Description  : �ж��Ƿ�ʱ
Input        : ulStartTime -- ��ʱ����ʼʱ��
               uiTimeoutInMillisecs -- ��ʱʱ��(��λ:����)
Output       : 
Return       : ��ǰ�̵߳��߳�id
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CMpThread::IsTimeout(mp_uint64 ulStartTime, mp_uint32 uiTimeoutInMillisecs)
{
    if ((CMpTime::GetTimeUsec() - ulStartTime) >= (mp_uint64)uiTimeoutInMillisecs * 1000)
    {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/*------------------------------------------------------------ 
Description  : ��ָ����ʱʱ���ڻ�ȡ�߳���
Input        : plock -- �߳���
               uiTimeoutInMillisecs -- ��ʱʱ��(��λ:����)
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
               (THREAD_LOCK_BUSY -- �߳����������߳�ռ��)
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::TimedLock(thread_lock_t* plock, mp_uint32 uiTimeoutInMillisecs)
{
    mp_uint32 uiRetryInterval = CalcRetryInterval(uiTimeoutInMillisecs);
    mp_uint64 start_time = CMpTime::GetTimeUsec();

    mp_int32 result = TryLock(plock);
    while((MP_SUCCESS != result) && !IsTimeout(start_time, uiTimeoutInMillisecs))
    {
        DoSleep(uiRetryInterval);
        result = TryLock(plock);
    }

    return result;
}

/*------------------------------------------------------------ 
Description  : �ͷ��߳���
Input        : plock -- �߳���
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CMpThread::Unlock(thread_lock_t* plock)
{
    if (NULL == plock)
    {
        return MP_FAILED;
    }

#ifdef WIN32
    LeaveCriticalSection(plock);
#else
    if (0 != pthread_mutex_unlock(plock))  //lint !e455
    {
        return MP_FAILED;
    }
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ��ȡ��ǰ�̵߳��߳�id
Input        : 
Output       : 
Return       : ��ǰ�̵߳��߳�id
Create By    :
Modification : 
-------------------------------------------------------------*/
thread_os_id_t CMpThread::GetThreadId()
{
    thread_os_id_t tid;

#ifdef WIN32
    tid = GetCurrentThreadId();
#else
    tid = pthread_self();
#endif

    return tid;
}

/*------------------------------------------------------------ 
Description  : �߳��Զ����������������ڽ���ʱ�����������Զ��ͷ�
               �����е��߳���
Input        : pLock -- �߳���
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
CThreadAutoLock::CThreadAutoLock(thread_lock_t* pLock)
{
    m_plock = pLock;
    if(m_plock)
    {
        CMpThread::Lock(m_plock);
    }
}

CThreadAutoLock::~CThreadAutoLock()
{
    if(m_plock)
    {
        CMpThread::Unlock(m_plock);
        m_plock = NULL;
    }
}


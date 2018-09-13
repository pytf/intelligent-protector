/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "agent/Agent.h"
#include "agent/TaskPool.h"
#include "agent/Communication.h"
#include "agent/Authentication.h"
#include "agent/FTExceptionHandle.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/UniqueId.h"
#include "common/String.h"
#include "common/ConfigXmlParse.h"
#include "common/SystemExec.h"
#include "common/CryptAlg.h"
#include "securec.h"
//#include <Windows.h>

#ifdef WIN32
#include "common/ServiceHandler.h"
#endif

mp_bool g_bExitFlag = MP_FALSE;
/*------------------------------------------------------------ 
Description  : �����ֲ�
Input        : 
Output       : 
Return       :  
               
Create By    :
Modification : 
-------------------------------------------------------------*/   

mp_void ShowOnlineManual()
{
    printf("rdagent -v : show version.\n");
    printf("rdagent -h : show online manual.\n");
}

#ifndef WIN32
/*------------------------------------------------------------ 
Description  : ������̨���н���
Input        : 
Output       : 
Return       :  
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
//AIXƽ̨�»������ں�ִ̨�У���Ҫ��λԭ��
mp_void MakeIndependent()
{
    printf("make indepentdent\n");
    pid_t pid;
    if (0 == (pid = fork()))
    {
        if (-1 == setpgid(getpid(), 0))
        {
            printf("Set process group id failed, errno[%d]: %s.",
                errno, strerror(errno));

            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set process group id failed, errno[%d]: %s.",
                errno, strerror(errno));

            return;
        }

        printf("go on, pid %d\n", getpid());
        return;
    }
    else if (pid < 0)
    {
        printf("Fork failed, errno[%d]: %s.", errno, strerror(errno));
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Fork failed, errno[%d]: %s.", errno, strerror(errno));

        return;
    }
    else
    {
        printf("exit 0, pid %d\n", getpid());
        exit(0);
    }
}
#else  //WIN32
SERVICE_STATUS_HANDLE g_hServiceStatus;
/*------------------------------------------------------------ 
Description  : agent������
Input        : request---������������
Output       : 
Return       :  MP_SUCCESS---�ɹ�
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
DWORD WINAPI AgentServiceHandler(DWORD request, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    LOGGUARD("");
    switch (request)
    {
    case SERVICE_CONTROL_STOP:
        CWinServiceHanlder::UpdateServiceStatus(g_hServiceStatus, SERVICE_STOPPED, START_SERVICE_TIMEOUT);
        g_bExitFlag = MP_TRUE;
        break;
    default:
        break;
    }
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : agent���� 
Input        : 
Output       : 
Return       :  
               
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void WINAPI AgentServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    LOGGUARD("");
    g_hServiceStatus = RegisterServiceCtrlHandlerExW((LPWSTR)AGENT_SERVICE_NAME, AgentServiceHandler, NULL);
    if (!g_hServiceStatus)
    {
        COMMLOG(OS_LOG_CRI, LOG_COMMON_ERROR, "Registe agent service failed.");
        return;
    }

    CWinServiceHanlder::UpdateServiceStatus(g_hServiceStatus, SERVICE_RUNNING, START_SERVICE_TIMEOUT);
}
/*------------------------------------------------------------ 
Description  : ����agent����
Input        : 
Output       : 
Return       :  MP_SUCCESS---�ɹ�
               MP_FAILED---ʧ�ܣ��������Ƶ��ȷ���ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 RunAgentService()
{
    LOGGUARD("");
    SERVICE_TABLE_ENTRY st[] =
    {
        {(LPSTR)AGENT_SERVICE_NAME, AgentServiceMain},
        {NULL, NULL}
    };

    mp_bool bRet = StartServiceCtrlDispatcher(st);
    if (!bRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "StartServiceCtrlDispatcher failed, errorno[%d].", GetLastError());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
#endif //WIN32
/*------------------------------------------------------------ 
Description  : agent��ʼ��
Input        : 
Output       : pszFullBinPath---ȫ·����taskPool---�����
Return       :  MP_SUCCESS---��ʼ���ɹ�
               iRet---��Ӧ�Ĵ�����
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 AgentInit(mp_char* pszFullBinPath, CTaskPool& taskPool)
{
    mp_int32 iRet = MP_SUCCESS;

    iRet = InitCommonModules(pszFullBinPath);
    if (MP_SUCCESS != iRet)
    {
        return iRet;
    }
    
    //��ʼ��ͨ��ģ��
    iRet = CCommunication::GetInstance().Init();
    if (MP_SUCCESS != iRet)
    {
        printf("Init Communication failed.\n");
        return iRet;
    }

    //��ʼ��task
    iRet = taskPool.Init();
    if (MP_SUCCESS != iRet)
    {
        printf("Init task pool failed.\n");
        return iRet;
    }

    
    //��ʼ��KMC
    iRet = InitCrypt();
    if (iRet != MP_SUCCESS)
    {
        printf("Init crypt failed.\n");
        return iRet;
    }

    //��ʼ������ⶳ����
    iRet = CFTExceptionHandle::GetInstance().Init();
    if (MP_SUCCESS != iRet)
    {
        printf("Init ftexception handle failed.\n");
        return iRet;
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : agent ������
Input        : 
Output       : 
Return       :  MP_SUCCESS---�ɹ�
               iRet---ʧ�ܣ����ض�Ӧ������
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_int32 iRet = MP_SUCCESS;
    CTaskPool taskPool;
    mp_bool bService = MP_FALSE;

    if (argc > 1)
    {
        //��ǰֻ֧��-v��-s
        mp_char* pszOp = argv[1];
        if (0 == strcmp(pszOp, "-v"))
        {
            AgentVersion();
            return MP_SUCCESS;
        }
        else if (0 == strcmp(pszOp, "-s"))
        {
            bService = MP_TRUE;
        }
        else
        {
            ShowOnlineManual();
            return MP_SUCCESS;
        }
    }

    iRet = AgentInit(argv[0], taskPool);
    if (MP_SUCCESS != iRet)
    {
        return iRet;
    }

#ifdef WIN32
    if (bService)
    {
        iRet = RunAgentService();
        if (MP_SUCCESS != iRet)
        {
            printf("Run agent windows service failed.\n");
            return iRet;
        }
    }
#endif
    
    for (;;)
    {
        if (g_bExitFlag)
        {
            exit(0);
        }
        DoSleep(1000);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Init agent succ.");

    return iRet;
}//lint !e550



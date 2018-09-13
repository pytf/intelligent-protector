/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "tools/agentcli/ShowStatus.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/SystemExec.h"
#include "common/AppVersion.h"
#include "common/Log.h"
/*------------------------------------------------------------ 
Description  : �����̵�����״̬
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_int32 CShowStatus::Handle()
{
#ifndef WIN32
    //��ӡ����ʱ��
    printf("Compile at  : %s\n", COMPILE_TIME);
#endif
    //��ӡ�汾����Ϣ
    printf("Version     : %s\n", AGENT_VERSION);
    printf("Build Number: %s\n", AGENT_BUILD_NUM); 
    //ShowSvn();
    
    //��ӡrdagent����״̬
    mp_bool bStartted = IsStartted(PROCESS_RDAGENT);
    if (bStartted)
    {
        printf("rdagent     : Running\n");
    }
    else
    {
        printf("rdagent     : Stopped\n");
    }

    //��ӡnginx����״̬
    bStartted = IsStartted(PROCESS_NGINX);
    if (bStartted)
    {
        printf("nginx       : Running\n");
    }
    else
    {
        printf("nginx       : Stopped\n");
    }

    //��ӡmonitor����״̬
    bStartted = IsStartted(PROCESS_MONITOR);
    if (bStartted)
    {
        printf("monitor     : Running\n");
    }
    else
    {
        printf("monitor     : Stopped\n");
    }
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : rdagent����״̬
Input        : 
Output       : 
Return       : MP_TRUE -- �ɹ� 
               ��MP_TRUE -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_bool CShowStatus::IsStartted(PROCESS_TYPE eType)
{
#ifdef WIN32
    mp_string strServiceName;
    switch (eType)
    {
    case PROCESS_RDAGENT:
        strServiceName = AGENT_SERVICE_NAME;
        break;
    case PROCESS_NGINX:
        strServiceName = NGINX_SERVICE_NAME;
        break;
    case PROCESS_MONITOR:
        strServiceName = MONITOR_SERVICE_NAME;
        break;
    default:
        return MP_FALSE;
    }

    mp_string strCmd = "sc query " + strServiceName;
    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (MP_SUCCESS != iRet || vecRlt.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "sc query %s failed.", strServiceName.c_str());
        return MP_FALSE;
    }

    if (vecRlt.front().find(RUNNING_TAG) != mp_string::npos)
    {
        return MP_TRUE;
    }
    else
    {
        return MP_FALSE;
    }
#else
    mp_string strPidFilePath;
    mp_string strProcessName;
    switch (eType)
    {
    case PROCESS_RDAGENT:
        strPidFilePath = CPath::GetInstance().GetLogFilePath(AGENT_PID);
        strProcessName = AGENT_EXEC_NAME;
        break;
    case PROCESS_NGINX:
        strPidFilePath = CPath::GetInstance().GetNginxLogsFilePath(NGINX_PID);
        strProcessName = NGINX_EXEC_NAME;
        break;
    case PROCESS_MONITOR:
        strPidFilePath = CPath::GetInstance().GetLogFilePath(MONITOR_PID);
        strProcessName = MONITOR_EXEC_NAME;
        break;
    default:
        return MP_FALSE;
    }

    //��pid�ļ���ȡ���̺�
    vector<mp_string> vecRlt;
    mp_string strPid = " ";
    mp_int32 iRet = CMpFile::ReadFile(strPidFilePath, vecRlt);
    if (MP_SUCCESS == iRet && !vecRlt.empty())
    {
        strPid = vecRlt.front();
    }

    mp_string strCmd = "ps -aef | grep '" + strPid + "' | grep " + strProcessName + " | grep -v grep | grep -v gdb | grep -v vi | grep -v tail";
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        return MP_FALSE;
    }
    else
    {
        return MP_TRUE;
    }

#endif
}

mp_void CShowStatus::ShowSvn()
{
    mp_string svnPath = CPath::GetInstance().GetConfFilePath(SVN_CONF);
    vector<mp_string> vecs;
    
    mp_int32 iRet = CMpFile::ReadFile(svnPath,  vecs);
    if(iRet != MP_SUCCESS || vecs.size() != 1 || vecs.begin()->size() == 0)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "read %s failed.", SVN_CONF);
        printf("SVN         : Unknown\n");
    }
    else
    {
        printf("SVN         : %s\n", vecs.begin()->c_str());
    }
}


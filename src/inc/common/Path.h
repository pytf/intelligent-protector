/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef __AGENT_PATH_H__
#define __AGENT_PATH_H__

#include "common/Types.h"
#include "common/Defines.h"

//·�����
class AGENT_API CPath
{
public:
    static CPath& GetInstance()
    {
        return m_instance;
    }

    ~CPath()
    {
    }

    //pszBinFilePath�ĸ�ʽΪ/home/rdadmin/Agent/bin/rdagent
    mp_int32 Init(mp_char* pszBinFilePath);
    //SetRootPath�ṩͨ��AgentRoot·��֧�ֳ�ʼ���ķ�ʽ
    mp_void SetRootPath(mp_string& strRootPath)
    {
        m_strAgentRootPath = strRootPath;
    }
    //��ȡagent·�����簲װ·��Ϊ/home/rdagent/agent���򷵻�/home/rdagent/agent
    mp_string GetRootPath()
    {
        return m_strAgentRootPath;
    }
    //��ȡagent��װ·���µ�bin�ļ���·��
    mp_string GetBinPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR;
    }
    //��ȡ���·��
    mp_string GetPluginsPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR + PATH_SEPARATOR + AGENT_PLUGIN_DIR;
    }
    //��ȡagent��װ·���µ�conf�ļ���·��
    mp_string GetConfPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_CONF_DIR;
    }
    //��ȡagent��װ·���µ�log�ļ���·��
    mp_string GetLogPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_LOG_DIR;
    }
    //��ȡagent��װ·���µ�tmp�ļ���·��
    mp_string GetTmpPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_TMP_DIR;
    }
    //��ȡagent��װ·���µ�thirdparty�ļ���·��
    mp_string GetThirdPartyPath()
    {
        return GetBinPath() + PATH_SEPARATOR + AGENT_THIRDPARTY_DIR;
    }
    //��ȡagent��װ·���µ�DB�ļ���·��
    mp_string GetDbPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_DB;
    }
    //��ȡagent��װ·����bin�ļ�����nginx���ļ���·��
    mp_string GetNginxPath()
    {
        return m_strAgentRootPath + PATH_SEPARATOR + AGENT_BIN_DIR + PATH_SEPARATOR + AGENT_NGINX;
    }
    //��ȡagent��װ·���µ�bin�ļ�����ĳ���ļ�·��
    mp_string GetBinFilePath(mp_string strFileName)
    {
        return GetBinPath() + PATH_SEPARATOR + strFileName;
    }
    //��ȡagent��װ·���µ�conf�ļ�����ĳ���ļ�·��
    mp_string GetConfFilePath(mp_string strFileName)
    {
        return GetConfPath() + PATH_SEPARATOR + strFileName;
    }
    //��ȡagent��װ·���µ�log�ļ�����ĳ���ļ�·��
    mp_string GetLogFilePath(mp_string strFileName)
    {
        return GetLogPath() + PATH_SEPARATOR + strFileName;
    }
    //��ȡagent��װ·���µ�tmp�ļ�����ĳ���ļ�·��
    mp_string GetTmpFilePath(mp_string strFileName)
    {
        return GetTmpPath() + PATH_SEPARATOR + strFileName;
    }
    //��ȡagent��װ·���µ�DB�ļ�����ĳ���ļ�·��
    mp_string GetDbFilePath(mp_string strFileName)
    {
        return GetDbPath() + PATH_SEPARATOR + strFileName;
    }
    //��ȡagent��װ·���µ�thirdparty�ļ�����ĳ���ļ�·��
    mp_string GetThirdPartyFilePath(mp_string strFileName)
    {
        return GetThirdPartyPath() + PATH_SEPARATOR + strFileName;
    }
    //��ȡnginxĿ¼��ĳ���ļ���·��
    mp_string GetNginxFilePath(mp_string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + strFileName;
    }
    //��ȡgninx logsĿ¼��ĳ���ļ���·��
    mp_string GetNginxLogsFilePath(mp_string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + AGENT_NGINX_LOGS + PATH_SEPARATOR + strFileName;
    }
    //��ȡgninx confĿ¼��ĳ���ļ���·��
    mp_string GetNginxConfFilePath(mp_string strFileName)
    {
        return GetNginxPath() + PATH_SEPARATOR + AGENT_NGINX_CONF + PATH_SEPARATOR + strFileName;
    }

private:
    CPath()
    {
    }

private:
    static CPath m_instance;   //��������
    mp_string m_strAgentRootPath;
};

#endif //__AGENT_PATH_H__


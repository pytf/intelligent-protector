/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "openssl/sha.h"
#include "agent/Authentication.h"
#include "common/Log.h"
#include "common/Types.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"

CAuthentication CAuthentication::m_instance;

/*------------------------------------------------------------
Function Name: init
Description  : ��ʼ��ʵ������ȡ��������ļ�
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAuthentication::Init()
{
    //�������ļ��ж�ȡ�û���/������Ϣ
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, m_strUsr);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "parse xml config failed ,key is name");
        return MP_FAILED;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, m_strPwd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "parse xml config failed, key is hash");
        return MP_FAILED;
    }
   return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: Auth
Description  : �Ե�¼�ͻ��˽��м�Ȩ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CAuthentication::Auth(mp_string& strClientIP, mp_string& strUsr, mp_string& strPw)
{
    LOGGUARD("");
     //���´������ļ��ж�ȡ�������û���������
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, m_strUsr);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "parse xml config failed ,key is name");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, m_strPwd);
    if (MP_SUCCESS != iRet)
    {
         COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "parse xml config failed, key is hash");
         return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    //�жϿͻ����Ƿ�����
    if (IsLocked(strClientIP))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Client IP address %s is locked.", strClientIP.c_str());
        //���ض�Ӧ������
        return ERROR_COMMON_CLIENT_IS_LOCKED;
    }

    //���û����������У��
    mp_bool bRet = Check(strUsr, strPw);
    //ʧ�ܣ���ip��ַ��������
    if (MP_FALSE == bRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check user %s failed.", strUsr.c_str());
        //����������������ip��ַ��������
        Lock(strClientIP);
        //���ؼ�Ȩʧ�ܴ�����
        return ERROR_COMMON_USER_OR_PASSWD_IS_WRONG;
    }
    //�ɹ����������
    else
    {
        //������������ip��ַ���н���
        Unlock(strClientIP);
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Client IP address %s is unlocked.", strClientIP.c_str());
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: IsLocked
Description  : �жϵ�¼�ͻ����Ƿ�����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CAuthentication::IsLocked(mp_string& strClientIP)
{
    CThreadAutoLock lock(&m_lockedIPListMutex);
    for(vector<locked_client_info>::iterator it = m_lockedIPList.begin(); it != m_lockedIPList.end(); it++)
    {
        if (0 == strcmp(it->strClientIP.c_str(), strClientIP.c_str()))
        {
            if (it->isLocked)
            {
                //��ȡϵͳ��ǰʱ��
                mp_uint64 ullCurrentTime = CMpTime::GetTimeSec();
                //�������ʱ���Ѿ���ʱ,�������������¼
                if (ullCurrentTime - it->lockedTime >= LOCKED_TIME)
                {
                    it = m_lockedIPList.erase(it);
                    return MP_FALSE;
                }
            }
            return it->isLocked;
        }
    }

    return MP_FALSE;
}


/*------------------------------------------------------------
Function Name: Check
Description  : ���û������������У��
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CAuthentication::Check(mp_string& strUsr, mp_string& strPwd)
{
    //��server�˴���������������룬����ֵ����ɢ��
    mp_string strSalt;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if(MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get salt from xml failed.");
        return MP_FALSE;
    }

    //֧�ִ��ϰ汾����
    mp_char outHashHex[SHA256_BLOCK_SIZE + 1] = {0};
    mp_string strInput = strPwd + strSalt;
    iRet = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, sizeof(outHashHex));
    strInput.replace(0, strInput.length(), "");
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "GetSha256Hash failed, iRet = %d.", iRet);
        return MP_FALSE;
    }

    //�°汾������PBKDF2����ɢ��
    mp_string strOut;
    iRet = PBKDF2Hash(strPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
        return MP_FALSE;
    }

    if ((strUsr == m_strUsr) && (mp_string(outHashHex) == m_strPwd || strOut == m_strPwd))
    {
        return MP_TRUE;
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check failed, auth user: %s.", strUsr.c_str());
        return MP_FALSE;
    }
}

/*------------------------------------------------------------
Function Name: Lock
Description  : �Կͻ���ip��ַ����������ǰ���ǵ�ǰ�û�������У���Ѿ�ʧ��
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CAuthentication::Lock(mp_string& strClientIP)
{
    CThreadAutoLock lock(&m_lockedIPListMutex);
    for(vector<locked_client_info>::iterator it = m_lockedIPList.begin(); it != m_lockedIPList.end(); it++)
    {
        //֮ǰ�Ѵ��ڼ�¼
        if (0 == strcmp(it->strClientIP.c_str(), strClientIP.c_str()))
        {
            mp_uint64 ullCurrentTime = CMpTime::GetTimeSec();
            //����Ѿ�������ֱ�ӷ���
            if (it->isLocked)
            {
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "current client %s is locked.", it->strClientIP.c_str());
                it->lastFailedTime = ullCurrentTime;
                return;
            }

            //�����ǰ��¼ʱ�������һ��ʧ���Ѿ�����һ��ʱ��
            if (ullCurrentTime - it->lastFailedTime >= CONTINUOUS_FAILURE_TIME)
            {
                //����״̬���¸���
                it->failedTimes = 0;
                it->isLocked = MP_FALSE;
            }

            //����ʧ�ܴ�������һ��
            it->failedTimes++;
            if (it->failedTimes >= MAX_TRY_TIME)
            {
                //����MAX_TRY_TIME������
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Failed trying times is over %d, %s is locked.",
                                      MAX_TRY_TIME, it->strClientIP.c_str());
                it->isLocked = MP_TRUE;
                it->lockedTime = ullCurrentTime;
            }
            it->lastFailedTime = ullCurrentTime;
            return;
        }
    }

    //����vector��û���ҵ�ָ��ip��������¼�����´���һ��
    locked_client_info newLockedInfo;
    newLockedInfo.failedTimes = 1;
    newLockedInfo.isLocked = MP_FALSE;
    newLockedInfo.lastFailedTime = CMpTime::GetTimeSec();
    newLockedInfo.lockedTime = 0;
    newLockedInfo.strClientIP = strClientIP;
    m_lockedIPList.push_back(newLockedInfo);
}

/*------------------------------------------------------------
Function Name: Unlock
Description  : �Կͻ���ip��ַ�������
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CAuthentication::Unlock(mp_string& strClientIP)
{
    CThreadAutoLock lock(&m_lockedIPListMutex);
    for(vector<locked_client_info>::iterator it = m_lockedIPList.begin(); it != m_lockedIPList.end(); it++)
    {
        //�ҵ���¼
        if (0 == strcmp(it->strClientIP.c_str(), strClientIP.c_str()))
        {
            //���������¼
            it = m_lockedIPList.erase(it);
            return;
        }
    }
}


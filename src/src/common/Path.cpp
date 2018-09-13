/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef WIN32
#include <libgen.h>
#endif
#include "common/Path.h"
#include "securec.h"
#include "common/ErrorCode.h"

CPath CPath::m_instance;

/*------------------------------------------------------------
Function Name: Init
Description  : ��ʼ����������ڴ����ã����ڻ�ȡagent����·��
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CPath::Init(mp_char* pszBinFilePath)
{
    mp_char* pszTmp = NULL;
    mp_char acAgentRoot[MAX_FULL_PATH_LEN] = {0};
    mp_uint32 iLastIndex = 0;

    if (NULL == pszBinFilePath)
    {
        return ERROR_COMMON_OPER_FAILED;
    }

#ifdef WIN32
    mp_int32 iRet = 0;
    iRet = GetFullPathName(pszBinFilePath, MAX_FULL_PATH_LEN, acAgentRoot, &pszTmp);
    if (0 == iRet)
    {
        return ERROR_COMMON_OPER_FAILED;
    }
    *pszTmp = 0;

    iLastIndex = strlen(acAgentRoot) - 1;
    acAgentRoot[iLastIndex] = 0;
    pszTmp = strrchr(acAgentRoot, '\\');
    iLastIndex = pszTmp - acAgentRoot;
    acAgentRoot[iLastIndex] = 0;
#else
    mp_char* pszPath = NULL;
    pszPath = dirname(pszBinFilePath);
    if (NULL == pszPath)
    {
        return ERROR_COMMON_OPER_FAILED;
    }
    //CodeDex�󱨣�Buffer Overflow��·�����Ȳ��ᳬ��300
    mp_int32 iRet = strncpy_s(acAgentRoot, sizeof(acAgentRoot), pszPath, strlen(pszPath));
    if (EOK != iRet)
    {
        return ERROR_COMMON_OPER_FAILED;
    }

    pszTmp = strrchr(acAgentRoot, '/');
    //���ֱ��ִ��rdagentִ�г�����dirname��ȡ��·��Ϊ"."������strrchr����NULL
    if (NULL == pszTmp)
    {
        return ERROR_COMMON_OPER_FAILED;
    }
    iLastIndex = pszTmp - acAgentRoot;
    acAgentRoot[iLastIndex] = 0;
#endif

    m_strAgentRootPath = acAgentRoot;

    return MP_SUCCESS;
}


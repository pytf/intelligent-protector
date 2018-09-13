/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/RootCaller.h"
#include "common/Utils.h"
#include "common/UniqueId.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/SystemExec.h"
#include "securec.h"

/*------------------------------------------------------------
Function Name: Exec
Description  : rootȨ��ִ�к�����������ģ�龲̬����
               iCommandID: ����ID,����μ�ROOT_COMMAND
               strParam: rootȨ��ִ�в����������ã����޲�����ֱ������""
               pvecResult: ����ִ�н����vector������������ֱ������NULL
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRootCaller::Exec(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;

    //��ȡ��ǰִ��Ψһid
    mp_string strUniqueID = CUniqueID::GetInstance().GetString();

    if (!strParam.empty())
    {
        //������д��ipc�ļ�
        iRet = CIPCFile::WriteInput(strUniqueID, strParam);
        if (iRet != MP_SUCCESS)
        {
            //��ӡ��־
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "WriteInput failed, ret %d.", iRet);
            return iRet;
        }
    }

    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    mp_string strBinFilePath = CPath::GetInstance().GetBinFilePath(ROOT_EXEC_NAME);
    strBinFilePath = CMpString::BlankComma(strBinFilePath);
    CHECK_FAIL(SNPRINTF_S(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "%s -c %d -u %s",
         strBinFilePath.c_str(), iCommandID, strUniqueID.c_str()));

    mp_string strCmd = acCmd;
    mp_string strLogCmd;
    RemoveFullPathForLog(strCmd, strLogCmd);
    //����Ƿ�����Ƿ��ַ�
    if (CheckCmdDelimiter(strCmd) == MP_FALSE)
    {
        //��ӡ��־
        return ERROR_COMMON_INVALID_PARAM;
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strLogCmd.c_str());
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    //���ڵ������ű������۳ɹ�ʧ�ܣ�����Ҫ��ȡ����ļ�
    if (iRet != MP_SUCCESS)
    {
        //������ת�����ű�ִ�з���ת����Ĵ����룬�ǽű�ִ��ͳһ����-1
        if (iCommandID >= ROOT_COMMAND_SCRIPT_BEGIN && iCommandID <= ROOT_COMMAND_SCRIPT_END)
        {
            CErrorCodeMap errorCode;
            mp_int32 iNewRet = errorCode.GetErrorCode(iRet);
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                 "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
            if(ROOT_COMMAND_THIRDPARTY != iCommandID)
            {
                return iNewRet;
            }
            iRet = iNewRet;
        }
        else
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                 "Exec system failed, initial return code %d", iRet);
            return MP_FAILED;
        }
    }

    //��ȡ����ļ�
    mp_int32 iRet1 = ReadResultFile(iCommandID, strUniqueID, pvecResult);
    

    return (iRet == MP_SUCCESS) ? iRet1 : iRet;
}


/*------------------------------------------------------------
Function Name: ReadResultFile
Description  : Exec����ִ����������Ҫ��ȡ����ļ����������������
               iCommandID: ����ID,����μ�ROOT_COMMAND
               strUniqueID: Exec ����ִ�е�Ψһid
               pvecResult: ����ִ�н����vector������������ֱ������NULL
Return       :
Call         :
Called by    :
Modification :
Others       :
--------------------------------------------------------*/
mp_int32 CRootCaller::ReadResultFile(mp_int32 iCommandID, mp_string& strUniqueID, vector<mp_string>* pvecResult)
{
    mp_int32 iRet = MP_SUCCESS;
    
    if (0 != pvecResult)
    {
        if ( ROOT_COMMAND_THIRDPARTY != iCommandID )
        {
            iRet = CIPCFile::ReadResult(strUniqueID, *pvecResult);
        }
        else
        {
            iRet = CIPCFile::ReadOldResult(strUniqueID, *pvecResult);
        }
        
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read result file failed.");
        }
    }

    return iRet;
}


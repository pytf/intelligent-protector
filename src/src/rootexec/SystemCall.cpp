/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "rootexec/SystemCall.h"
#include "common/RootCaller.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/Defines.h"
#include "common/SystemExec.h"
#include "common/ErrorCode.h"
#include "common/UniqueId.h"
#include "common/Sign.h"
#include "array/Array.h"
#include "securec.h"
#include <algorithm>

#ifdef LINUX
#include <linux/raw.h>
//#include <linux/major.h>
#include <sys/ioctl.h>
#include <sstream>
#endif

#ifdef LIN_FRE_SUPP
#include <linux/fs.h>
#endif

/*------------------------------------------------------------
Function Name: GetParamFormTmpFile
Description  : ����id��ȡ��ʱ�ļ�����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::GetParamFromTmpFile(mp_string& strUniqueID, mp_string& strParam)
{
    //������ʱ�����ļ�·��
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);
    if (CMpFile::FileExist(strFilePath.c_str()))
    {
        mp_int32 iRet = CIPCFile::ReadInput(strUniqueID, strParam);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ReadInput failed, ret %d.", iRet);   
            return iRet;
        }
    }
    //��ʱ�ļ������ڰ�����Ϊ�մ���
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ExecSysCmd
Description  : ִ��ϵͳ����Ӽ��ܵ�������ʱ�ļ��ж��������н��ܣ�ִ�У������д������ʱ�ļ�
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::ExecSysCmd(mp_string& strUniqueID, mp_int32 iCommandID)
{
    LOGGUARD("");
    //����iCommandID��ȡ����������
    CCommandMap commandMap;
    mp_int32 iRet = MP_FAILED;
    mp_string strCommand = commandMap.GetCommandString(iCommandID);

    //������ʱ�����ļ�·��
    mp_string strParam = "";
    iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    //��װ����
    mp_string strExecCmd = strCommand + " " + strParam;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command to be executed is \"%s\".", strExecCmd.c_str());
    mp_bool bNeedEcho = commandMap.NeedEcho(iCommandID);
    vector<mp_string> vecResult;
    if (bNeedEcho)
    {
        //�������������Ҫ�ض���������������
        iRet = CSystemExec::ExecSystemWithEcho(strExecCmd, vecResult);
    }
    else
    {
        //echo������д����ض���
        if (iCommandID == ROOT_COMMAND_ECHO)
        {
            iRet = CSystemExec::ExecSystemWithoutEcho(strExecCmd, MP_FALSE);
        }
        else
        {
            iRet = CSystemExec::ExecSystemWithoutEcho(strExecCmd);
        }
    }

    if (MP_SUCCESS != iRet)
    {
        //��¼��־
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call ExecSystemWithoutEcho failed, ret %d.", iRet);
        return iRet;
    }

    //��ִ�н��д�����ļ�
    if (bNeedEcho)
    {
        return CIPCFile::WriteResult(strUniqueID, vecResult);
    }
    else
    {
        return MP_SUCCESS;
    }
}

/*------------------------------------------------------------
Function Name: ExecScript
Description  : ִ�нű����������������ɽű�����,�����д����ʱ����ļ���
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::ExecScript(mp_string& strUniqueID, mp_int32 iCommandID)
{
    LOGGUARD("");
    //����iCommandID��ȡ����ű�����
    CCommandMap commandMap;
    mp_string strScriptName = commandMap.GetCommandString(iCommandID);
    mp_string strAgentPath = CPath::GetInstance().GetRootPath();
    //������ʱ�����ļ�·��

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Script name is \"%s\".", strScriptName.c_str());

    mp_int32 iRet = MP_FAILED;
    mp_int32 iChowRet = MP_FAILED;

    //�жϽű��Ƿ����
    mp_string strScriptPath = CPath::GetInstance().GetBinFilePath(strScriptName);
    if (!CMpFile::FileExist(strScriptPath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Script \"%s\" is not exist, iRet is %d.", strScriptName.c_str(), INTER_ERROR_SRCIPT_FILE_NOT_EXIST);
        return INTER_ERROR_SRCIPT_FILE_NOT_EXIST;
    }

    //У��ű�ǩ��
    iRet = CheckScriptSign(strScriptName);
    if (iRet != MP_SUCCESS)
    {
        //return iRet;
        //ϵͳ���ã�����ֱ�ӷ���iRet
        return ERROR_SCRIPT_COMMON_EXEC_FAILED;
    }

    //��װ����
    strScriptPath = CMpString::BlankComma(strScriptPath);
    strAgentPath = CMpString::BlankComma(strAgentPath);
    mp_char acCmd[MAX_MAIN_CMD_LENGTH] = {0};
    CHECK_FAIL(SNPRINTF_S(acCmd, sizeof(acCmd), sizeof(acCmd) - 1, "%s %s %s",
                 strScriptPath.c_str(), strAgentPath.c_str(), strUniqueID.c_str()));
    mp_string strLogCmd = strScriptName + " " + strAgentPath + " " + strUniqueID;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Script \"%s\" will be excuted.", strLogCmd.c_str());

    mp_string strCmd = acCmd;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strLogCmd.c_str());
    //ִ�нű�������ȡ����
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Script \"%s\" excuted failed, iRet = %d.", strLogCmd.c_str(), iRet);
    }

    // ���ý���ļ�Ȩ��
    iChowRet = CIPCFile::ChownResult(strUniqueID);
    if (MP_SUCCESS != iChowRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Chown result file failed, UniqueID %s.", strUniqueID.c_str());
    }

    return iRet;

}

/*------------------------------------------------------------
Function Name: GetDisk80Page
Description  : ��ȡ����80ҳ��Ϣ������ʱ�ļ��ж�ȡ�������
               ִ�гɹ��󣬽����д�����ļ���
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::GetDisk80Page(mp_string& strUniqueID)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;

    //������ʱ�����ļ�·��
    mp_string strDevice = "";
    iRet = GetParamFromTmpFile(strUniqueID, strDevice);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    mp_string strSN;
    iRet = CArray::GetDisk80Page(strDevice, strSN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CDiskPage::GetDisk80Page, ret %d.", iRet);;
        return iRet;
    }

    vector<mp_string> vecResult;
    vecResult.push_back(strSN);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/*------------------------------------------------------------
Function Name: GetDisk83Page
Description  : ��ѯdisk 83ҳ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::GetDisk83Page(mp_string& strUniqueID)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;

    mp_string strDevice = "";
    iRet = GetParamFromTmpFile(strUniqueID, strDevice);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    mp_string strLunWWN, strLunID;
    iRet = CArray::GetDisk83Page(strDevice, strLunWWN, strLunID);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CDiskPage::GetDisk80Page, ret %d.", iRet);;
        return iRet;
    }

    vector<mp_string> vecResult;
    vecResult.push_back(strLunWWN);
    vecResult.push_back(strLunID);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}



/*------------------------------------------------------------
Function Name: GetDisk00Page
Description  : ��ѯdisk 00ҳ
Return       : Support VPD pages
Call         : 
Called by    : CArray::GetDisk00Page
create by    : 
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::GetDisk00Page(mp_string& strUniqueID)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;

    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);
    mp_string strDevice = "";
    if (CMpFile::FileExist(strFilePath.c_str()))
    {
        iRet = CIPCFile::ReadInput(strUniqueID, strDevice);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ReadInput failed, ret %d.", iRet);;
            return iRet;
        }
    }
    
    vector<mp_string> vecResult;
    iRet = CArray::GetDisk00Page(strDevice, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CDiskPage::GetDisk80Page, ret %d.", iRet);;
        return iRet;
    }

    return CIPCFile::WriteResult(strUniqueID, vecResult);
}


/*------------------------------------------------------------
Function Name: GetDiskC8Page
Description  : ��ѯdisk 83ҳ
Return       : lunid
Call         :
Called by    : CArray::GetDiskC8Page
Modification : 
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::GetDiskC8Page(mp_string& strUniqueID)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;

    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);
    mp_string strDevice = "";
    if (CMpFile::FileExist(strFilePath.c_str()))
    {
        iRet = CIPCFile::ReadInput(strUniqueID, strDevice);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ReadInput failed, ret %d.", iRet);;
            return iRet;
        }
    }

    mp_string strLunID;
    iRet = CArray::GetDiskC8Page(strDevice, strLunID);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CDiskPage::GetDiskC8Page, ret %d.", iRet);;
        return iRet;
    }

    vector<mp_string> vecResult;
    if (!strLunID.empty())
    {
        vecResult.push_back(strLunID);
    }
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/*------------------------------------------------------------
Function Name: GetDiskCapacity
Description  : ��ѯ��������
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::GetDiskCapacity(mp_string& strUniqueID)
{
    LOGGUARD("");
    mp_int32 iRet = MP_FAILED;

    mp_string strDevice = "";
    iRet = GetParamFromTmpFile(strUniqueID, strDevice);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    mp_string strBuf;
    iRet = CDisk::GetDiskCapacity(strDevice, strBuf);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CDiskPage::GetDisk80Page, ret %d.", iRet);;
        return iRet;
    }

    vector<mp_string> vecResult;
    vecResult.push_back(strBuf);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/*------------------------------------------------------------
Function Name: GetDisk83Page
Description  : ��ȡ����83ҳ��Ϣ������ʱ�ļ��ж�ȡ�������
               ִ�гɹ��󣬽����д�����ļ���
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::GetVendorAndProduct(mp_string& strUniqueID)
{
    mp_int32 iRet = MP_FAILED;

    //������ʱ�����ļ�·��
    mp_string strDevice = "";
    iRet = GetParamFromTmpFile(strUniqueID, strDevice);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    mp_string strVendor, strProduct;
    iRet = CArray::GetDiskArrayInfo(strDevice, strVendor, strProduct);
    if (MP_SUCCESS != iRet)
    {
        //��¼��־
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CArray::GetVendor, ret %d.", iRet);;
        return iRet;
    }

    //��ִ�н��д�����ļ�
    vector<mp_string> vecResult;
    vecResult.push_back(strVendor);
    vecResult.push_back(strProduct);
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/*------------------------------------------------------------
Function Name: GetRawMajorMinor
Description  : ��ȡraw�豸���������ʹ˷�����Ϣ
               ִ�гɹ��󣬽����д�����ļ���
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::GetRawMajorMinor(mp_string& strUniqueID)
{
    mp_int32 iRet = MP_FAILED;
#ifdef LINUX
    //������ʱ�����ļ�·��
    mp_string strMinor = "";
    iRet = GetParamFromTmpFile(strUniqueID, strMinor);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    mp_int32 fd = open(RAW_CTRL_FILE, O_RDWR, 0);
    if (fd < 0)
    {
        fd = open(RAW_CTRL_FILE_NEW, O_RDWR, 0);
        if (fd < 0)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open raw control device \"%s\" and \"%s\" failed.",
                RAW_CTRL_FILE, RAW_CTRL_FILE_NEW);
            return MP_FAILED;
        }
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Open raw control device \"%s\" or \"%s\" succ.",
           RAW_CTRL_FILE, RAW_CTRL_FILE_NEW);

    struct raw_config_request rq;
    rq.raw_minor = atoi(strMinor.c_str());
    iRet = ioctl(fd, RAW_GETBIND, &rq);
    if (iRet < 0)
    {
        close(fd);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Get raw device bounded info failed, errno %d.", errno);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "major = %d, minor = %d", rq.block_major, rq.block_minor);
    vector<mp_string> vecResult;
    ostringstream oss, oss1;
    oss << (mp_int32)rq.block_major;
    oss1 << (mp_int32)rq.block_minor;
    vecResult.push_back(oss.str());
    vecResult.push_back(oss1.str());
    close(fd);
    iRet = CIPCFile::WriteResult(strUniqueID, vecResult);
#endif
    return iRet;
}


/*------------------------------------------------------------
Function Name: ExecThirdPartyScript
Description  : ִ�е������ű����������Ϊ�ű����ƺͽű���ʵ�������м���ð�ŷָ�

Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CSystemCall::ExecThirdPartyScript(mp_string& strUniqueID)
{
    mp_int32 iRet = MP_FAILED;
    //������ʱ�����ļ�·��
    mp_string strParam = "";
    iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    mp_string::size_type pos = strParam.find(":");
    if (pos == mp_string::npos)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Param \"%s\" is invalid.", strParam.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_string strScriptName = strParam.substr(0, pos);
    //�жϽű��Ƿ����
    mp_string strScriptPath = CPath::GetInstance().GetThirdPartyFilePath(strScriptName);
    if (!CMpFile::FileExist(strScriptPath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Script \"%s\" is not exist, iRet is %d.",
            strScriptName.c_str(), INTER_ERROR_SRCIPT_FILE_NOT_EXIST);

        return INTER_ERROR_SRCIPT_FILE_NOT_EXIST;
    }

    mp_string strCmd = CPath::GetInstance().GetThirdPartyPath() + PATH_SEPARATOR + strScriptName;
    strCmd = CMpString::BlankComma(strCmd);
    mp_string strLogCmd;
    RemoveFullPathForLog(strCmd, strLogCmd);

    //Ϊ��֤��R3C10���ݣ��ű�����˳�����ΪUID,PATH
    mp_string strAgentPath = CPath::GetInstance().GetRootPath();
    strAgentPath = CMpString::BlankComma(strAgentPath);
    strCmd = strCmd  + " " + strUniqueID + " " + strAgentPath;
    strLogCmd = strLogCmd  + " " + strUniqueID + " " + strAgentPath;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Command \"%s\" will be executed.", strLogCmd.c_str());
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ExecSystemWithoutEcho failed, iRet is %d.", iRet);
    }
    return iRet;
}

/*------------------------------------------------------------
Function Name: BatchGetLUNInfo
Description  : ������ȡLUN��Ϣ
               ����/dev/sdb;/dev/sdc
               ��ȡdevicename;vendor;product;arraysn;lunid;wwn
                         /dev/sdb;HUAWEI;S5500T;210235G6GR10D7000004;218;6200bc71001f37540769e56b000000da
                         /dev/sdc;HUAWEI;S5500T;210235G6GR10D7000004;218;6200bc71001f37540769e56b000000da
Return        :
Call            :
Called by     :
Modification :
Others        :--------------------------------------------------------*/
mp_int32 CSystemCall::BatchGetLUNInfo(mp_string& strUniqueID)
{
    mp_int32 iRet = MP_FAILED;
    vector<mp_string> vecDevs, vecResult;
    

    LOGGUARD("");    
    //������ʱ�����ļ�·��
    mp_string strParam = "";
    iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to batch get lun info %s", strParam.c_str());
    CMpString::StrSplit(vecDevs, strParam, ';');
    if (vecDevs.empty())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "BatchGetLUNInfo failed, device list is empty.");
        return MP_FAILED;
    }

    mp_string strDevice, strLUNInfo;
    // �����豸��Ϣ����ȡLUN����Ϣ
    for (vector<mp_string>::iterator iter = vecDevs.begin(); iter != vecDevs.end(); ++iter)
    {
        strDevice = *iter;
        if (strDevice.empty())
        {
            continue;
        }
        
        iRet = GetLUNInfo(strDevice, strLUNInfo);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Batch get LUN info failed.");
            return iRet;
        }
        vecResult.push_back(strLUNInfo);
        strLUNInfo.clear();
    }
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Batch get lun info success.");
    // д����ļ�
    return CIPCFile::WriteResult(strUniqueID, vecResult);
}

/*------------------------------------------------------------
Function Name: GetLUNInfo
Description  : 
Return       :
Call         :
Called by    :
Modification :
Others        :--------------------------------------------------------*/
mp_int32 CSystemCall::GetLUNInfo(mp_string &strDevice, mp_string &strLUNInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strVendor, strProduct, strSN, strLUNID, strLUNWWN;
    vector<mp_string> vecPages;
    vector<mp_string>::iterator itPg;

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to batch get this lun( %s ) info.", strDevice.c_str());
    // ���̺��ͺ�
    iRet = CArray::GetDiskArrayInfo(strDevice, strVendor, strProduct);
    if (MP_SUCCESS != iRet)
    {
        //��¼��־
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CArray::GetVendor, ret %d.", iRet);;
        return iRet;
    }
    
    // ȥ���ո�
    mp_char *pStrTmp = CMpString::Trim((mp_char *)strVendor.c_str());
    if (!pStrTmp)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get vendor info of Device name(%s) failed.", strDevice.c_str());
        strLUNInfo = strDevice + ";;;;;";
        return MP_SUCCESS;
    }
    strVendor = pStrTmp;
    
    //product���ݲ�ѯ������û��ʹ�ã����Ϊ��ֻ����ʾ�������˳�
    pStrTmp = CMpString::Trim((mp_char *)strProduct.c_str());
    if (!pStrTmp)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Get product info of Device name(%s) failed.", strDevice.c_str());
        strProduct = "";
    }
    else
    {
        strProduct = pStrTmp;
    }
    
    // not huawei lun, do not get lun info continue
    mp_bool bHuaweiLUN = CArray::CheckHuaweiLUN(strVendor);
    if (MP_FALSE == bHuaweiLUN)
    {
        COMMLOG(OS_LOG_WARN, LOG_COMMON_WARN, "Device name(%s) is not in huawei/huasai array.", strDevice.c_str());
        strLUNInfo = strDevice + ";" + strVendor + ";" + strProduct + ";;;";
        return MP_SUCCESS;
    }
    
    //����SN
    iRet = CArray::GetDisk80Page(strDevice, strSN);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get arraysn info of Device name(%s) failed.", strDevice.c_str());
        strLUNInfo = strDevice + ";" + strVendor + ";" + strProduct + ";;;";
        return MP_SUCCESS;
    }
    
    //LUN WWN��LUN ID
    iRet = CArray::GetDisk83Page(strDevice, strLUNWWN, strLUNID);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 83 page of device failed.");
        return iRet;
    }
    
    //if support C8,get lun id from C8
    //vecPages.clear();
    iRet = CArray::GetDisk00Page(strDevice, vecPages);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get 00 page of device failed.");
        return iRet;
    }
    
    // get lun id from 
    itPg = find(vecPages.begin(), vecPages.end(), "c8");
    if (itPg != vecPages.end())
    {
        iRet = CArray::GetDiskC8Page(strDevice, strLUNID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get C8 page of device failed.");
            return iRet;
        }
    }
    
    // ��װ����ļ�
    // devicename;vendor;product;arraysn;lunid;wwn
    strLUNInfo = strDevice + ";" + strVendor + ";" + strProduct + ";" + strSN + ";" + strLUNID + ";" + strLUNWWN;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Batch get this lun( %s ) info success.", strDevice.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: SyncDataFile
Description  : ͬ�������ļ�����
Return       : MP_SUCCESS �ɹ�
                    ��MP_SUCCESSʧ��
Call         :
Called by    :
Modification :
Others        :--------------------------------------------------------*/
mp_int32 CSystemCall::SyncDataFile(mp_string& strUniqueID)
{
#ifdef LINUX
    mp_int32 iRet = MP_FAILED;
    mp_int32 fd = 0;
    mp_string strParam = "";
    vector<mp_string> vecDevs;
    
    //������ʱ�����ļ�·��
    iRet = GetParamFromTmpFile(strUniqueID, strParam);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call GetParamFormTmpFile failed, ret %d.", iRet);
        return iRet;
    }
    mp_char pFilePath[PATH_MAX + 1] = {0};   
    if (strParam.length()> PATH_MAX || NULL == realpath(strParam.c_str(), pFilePath))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Check real path failed,errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Begin to sync data file.");
    
    fd = open(pFilePath, O_RDONLY | O_NONBLOCK);
    if (MP_SUCCESS > fd)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open data file failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    //��ˢ������Ӳ��
    iRet = fsync(fd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Sync data file failed, errno[%d]:%s.", errno, strerror(errno));
        close(fd);
        return MP_FAILED;
    }
    close(fd);
    
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Sync data file successful.");
    return MP_SUCCESS;
#else
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Sync data file not implemente.");
    return MP_FAILED;
#endif
}




/*------------------------------------------------------
Function Name: ReloadUDEVRules
Description     : ���¼���udev����
Input             : NA
Return           : MP_SUCCESS �ɹ�
                    ��MP_SUCCESSʧ��
Create By    :
Modification   :
--------------------------------------------------------*/
mp_int32 CSystemCall::ReloadUDEVRules(mp_string& strUniqueID)
{
#ifdef LINUX
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iOSType = HOST_OS_UNKNOWN;
    mp_string strOSVersion;
    mp_string strExecCmd = "udevadm control --reload-rules";

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin reload udev rules...");

    // ��ȡOS�汾
    GetOSType(iOSType);
    iRet = GetOSVersion(iOSType, strOSVersion);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get OS version failed.");
        return iRet;
    }

    if ((HOST_OS_REDHAT == iOSType && "5" == strOSVersion) || (HOST_OS_SUSE== iOSType && "10" == strOSVersion))
    {
        strExecCmd = "udevcontrol reload_rules";
    }
    // ��ǰ������OS����Rocky4\Rocky6\iSoftʹ���ϰ汾����
    else if (HOST_OS_OTHER_LINUX == iOSType)
    {
        strExecCmd = "udevcontrol reload_rules";
    }
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "cmd '%s' will be excute.", strExecCmd.c_str());
    iRet = CSystemExec::ExecSystemWithoutEcho(strExecCmd);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute reload udev rules failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Reload udev rules succ.");
#endif
    return MP_SUCCESS;
}

#ifdef LIN_FRE_SUPP

/*------------------------------------------------------
Function Name: FreezeFileSys
Description     : �����ļ�ϵͳ�Ĳ���
Input             : strDriveLetter���ᣬ�ⶳ����Ĺ��ص�
Return           : MP_SUCCESS �ɹ�
                    ��MP_SUCCESSʧ��
Create By    :
Modification   :
--------------------------------------------------------*/
mp_int32 CSystemCall::FreezeFileSys(mp_string& strUniqueID)
{
    mp_int32 fd = 0;
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};
    mp_long lTimeOut = OPER_TIME_OUT;
    mp_string strLogFalg;
    mp_string strDriveLetter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begian freeze file system %s...", strDriveLetter.c_str());
	//CodeDex�󱨣�FORTIFY.Path_Manipulation
    //������ʱ�����ļ�·��
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);

    if (CMpFile::FileExist(strFilePath.c_str()))
    {
        iRet = CIPCFile::ReadInput(strUniqueID, strDriveLetter);
        if (MP_SUCCESS != iRet)
        {
            //��¼��־
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ReadInput failed, ret %d.", iRet);

            return iRet;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "tmp file \"%s\" is not exist.", strFilePath.c_str());

        return ERROR_COMMON_OPER_FAILED;
    }

    fd = open(strDriveLetter.c_str(), O_RDONLY);
    if (MP_SUCCESS > fd)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open the mounted point %s failed, errno[%d]: %s.", 
            strDriveLetter.c_str(), iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));

        return ERROR_COMMON_OPER_FAILED;
    }

    iRet = ioctl(fd, FIFREEZE, &lTimeOut);
    close(fd);

    if (MP_SUCCESS > iRet)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "file system %s freeze failed, errno[%d]: %s.",
            strDriveLetter.c_str(), iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));

        return ERROR_COMMON_APP_FREEZE_FAILED;
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "File system %s freeze succ.", strDriveLetter.c_str());
    
    return MP_SUCCESS;
}

/*------------------------------------------------------
Function Name: FreezeFileSys
Description     : �ⶳ�ļ�ϵͳ�Ĳ���
Input             : strDriveLetter���ᣬ�ⶳ����Ĺ��ص�
Return           : MP_SUCCESS �ɹ�
                    ��MP_SUCCESSʧ��
Create By    :
Modification   :
--------------------------------------------------------*/
mp_int32 CSystemCall::ThawFileSys(mp_string& strUniqueID)
{
    mp_int32 fd = 0;
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iErr = 0;
    mp_long lTimeOut = OPER_TIME_OUT;
    mp_char szErr[256] = {0};
    mp_string strLogFalg;
    mp_string strDriveLetter;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin thaw file system %s...", strDriveLetter.c_str());
	//CodeDex�󱨣�FORTIFY.Path_Manipulation
    //������ʱ�����ļ�·��
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(mp_string(INPUT_TMP_FILE) + strUniqueID);

    if (CMpFile::FileExist(strFilePath.c_str()))
    {
        iRet = CIPCFile::ReadInput(strUniqueID, strDriveLetter);
        if (MP_SUCCESS != iRet)
        {
            //��¼��־
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ReadInput failed, ret %d.", iRet);

            return iRet;
        }
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "tmp file \"%s\" is not exist.", strFilePath.c_str());

        return ERROR_COMMON_OPER_FAILED;
    }

    fd = open(strDriveLetter.c_str(), O_RDONLY);
    if (MP_SUCCESS > fd)
    {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Open the mounted point %s failed, errno[%d]: %s.", 
            strDriveLetter.c_str(), iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));

        return ERROR_COMMON_OPER_FAILED;
    }

    iRet = ioctl(fd, FITHAW, &lTimeOut);
    close(fd);

    if (MP_SUCCESS > iRet)
    {
        //Rocky���νⶳ���سɹ���SuSE11\RedHat6���νⶳ���ش�����22(Invalid argument)
        //�������ͳһ����MP_SUCCESS
        //���ļ�ϵͳ���������쳣ʱ����νⶳ��������
        iErr = GetOSError();
        if (iErr != THAW_ERR)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "file system %s thaw failed, errno[%d]: %s.",
                strDriveLetter.c_str(), iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
    
            return ERROR_COMMON_APP_THAW_FAILED;
        }

        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "File system %s is thaw, so return 0.", strDriveLetter.c_str());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "File system %s thaw succ.", strDriveLetter.c_str());
    
    return MP_SUCCESS;
}

#endif //LINUX2.6.29 up

/*------------------------------------------------------------
Function Name: CCommandMap
Description  : CCommandMap���캯��
               ���������ֺͽű����ƵĶ�Ӧ��ϵ
               ���������ֺ�ϵͳ����Ķ�Ӧ��ϵ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
CCommandMap::CCommandMap()
{
    InitDB2ScriptMap();
    InitOracleScriptMap();
    InitCacheScriptMap();
    InitHostScriptMap();
    InitSybaseScriptMap();
    InitHanaScriptMap();
    InitSysCmdMap1();
    InitSysCmdMap2();
    InitSysCmdMap3();
    InitSysCmdMap4();
    InitSysCmdMap5();
    InitSysCmdMap6();
    InitNeedEchoCmdMap1();
    InitNeedEchoCmdMap2();  
}

/*------------------------------------------------------------
Function Name: InitDB2ScriptMap
Description  : ��ʼ���ű�����map
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitDB2ScriptMap()
{
    //��ʼ���ű�����,��ӽű���׺
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_INIT, "initiator.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_PACKAGELOG, "packlog.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYDB2INFO, "db2info.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYDB2LUNINFO, "db2luninfo.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_RECOVERDB2, "db2recover.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_SAMPLEDB2, "db2sample.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYDB2CLUSTERINFO, "db2clusterinfo.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_DB2RESOURCEGROUP, "db2resourcegroup.sh"));

}

/*------------------------------------------------------------
Function Name: InitOracleScriptMap
Description  : ��ʼ���ű�����map
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitOracleScriptMap()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYORACLEINFO, "oracleinfo.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYORACLEPDBINFO, "oraclepdbinfo.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STARTORACLEPDB, "oraclepdbstart.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYORACLELUNINFO, "oracleluninfo.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYORACLECLUSTERINFO, "oracleclusterinfo.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_TESTORACLE, "oracletest.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_CHECKARCHIVETHRESHOLD, "oraclecheckarchive.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_FREEZEORACLE, "oracleconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_THAWORACLE, "oracleconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_ARCHIVEORACLE, "oracleconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_TRUNCATEARCHIVELOGORACLE, "oracleconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STARTASMINSTANCE, "oraasmaction.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STOPASMINSTANCE, "oraasmaction.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STARTORACLEDB, "oradbaction.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STOPORACLEDB, "oradbaction.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_ORACLERESOURCEGROUP, "oracleresourcegroup.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_ORACLECHECKCDB, "oraclecheckcdb.sh"));
}

/*------------------------------------------------------------
Function Name: InitHostScriptMap
Description  : ��ʼ���ű�����map
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitHostScriptMap()
{
    //��ʼ���ű�����,��ӽű���׺
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_SCANDISK, "scandisk.sh"));
}

/*------------------------------------------------------------
Function Name: InitCacheScriptMap
Description  : ��ʼ���ű�����map
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitCacheScriptMap()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYCACHEINFO, "cacheinfo.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYCACHELUNINFO, "cacheluninfo.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_SAMPLECACHE, "cachesample.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYCACHECLUSTERINFO, "cacheclusterinfo.sh"));
}

/*------------------------------------------------------------
Function Name: InitSybaseScriptMap
Description  : ��ʼ���ű�����map
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitSybaseScriptMap()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_FREEZESYBASE, "sybaseconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_THAWSYBASE, "sybaseconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYFREEZESTATUS, "sybaseconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STARTSYBASE, "sybaserecover.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STOPSYBASE, "sybaserecover.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_TESTSYBASE, "sybasetest.sh"));
}

/*------------------------------------------------------------
Function Name: InitHanaScriptMap
Description  : ��ʼ���ű�����map
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitHanaScriptMap()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_FREEZEHANA, "hanaconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_THAWHANA, "hanaconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_QUERYHANAFREEZESTATUS, "hanaconsistent.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STARTHANA, "hanarecover.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_STOPHANA, "hanarecover.sh"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCRIPT_TESTHANA, "hanatest.sh"));
}


/*------------------------------------------------------------
Function Name: InitSysCmdMap1
Description  : ��ʼ��ϵͳ����map�����ȳ�̫�ߣ���ɶ��������ÿ������ֻע��10��ϵͳ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitSysCmdMap1()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_NETSTAT, "netstat"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_PS, "ps -aef"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_FSCK, "fsck"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_FSTYP, "fstyp"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_ECHO, "echo"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_DF, "df -i"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CAT, "cat"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LS, "ls -l"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_PVSCAN, "pvscan"));
    //m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXDISK_SCAN, "vxdisk scandisks"));
    //m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXDISK_E, "vxdisk -e list"));
}

/*------------------------------------------------------------
Function Name: InitSysCmdMap2
Description  : ��ʼ��ϵͳ����map�����ȳ�̫�ߣ���ɶ��������ÿ������ֻע��10��ϵͳ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitSysCmdMap2()
{
    //m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXPRINT, "vxprint -g"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_BLKID, "blkid"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LSB, "lsb_release -a"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_UDEVADM, "udevadm info"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_MOUNT, "mount"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LSLV, "lslv"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VGDISPLAY, "vgdisplay"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LVDISPLAY, "lvdisplay"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_PVS, "pvs"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LVS, "lvs"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LSVG, "lsvg"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LSPV, "lspv"));
}


/*------------------------------------------------------------
Function Name: InitSysCmdMap3
Description  : ��ʼ��ϵͳ����map�����ȳ�̫�ߣ���ɶ��������ÿ������ֻע��20��ϵͳ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitSysCmdMap3()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_HAGRP, "hagrp"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_HASYS, "hasys"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_HASTART, "hastart"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_HARES, "hares"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CLLSNODE, "/usr/es/sbin/cluster/utilities/cllsnode"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CLFINDRES, "/usr/es/sbin/cluster/utilities/clfindres"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LSSRC, "lssrc"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_STARTPOWERHA, "_SPOC_FORCE=Y /usr/es/sbin/cluster/cspoc/fix_args nop cl_rc.cluster"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_UMOUNT, "umount"));
}

/*------------------------------------------------------------
Function Name: InitSysCmdMap4
Description  : ��ʼ��ϵͳ����map�����ȳ�̫�ߣ���ɶ��������ÿ������ֻע��20��ϵͳ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitSysCmdMap4()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SED, "sed"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LSMOD, "lsmod"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LVCHANGE, "lvchange"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VGCHANGE, "vgchange"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VGS, "vgs"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_PVDISPLAY, "pvdisplay"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VARYONVG, "varyonvg"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VARYOFFVG, "varyoffvg"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_HOSTNAME, "hostname"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_HOTADD, "/usr/sbin/hot_add"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_IOSCAN, "ioscan"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_IOSCANFNC, "ioscan -fnC disk"));
}

/*------------------------------------------------------------
Function Name: InitSysCmdMap5
Description  : ��ʼ��ϵͳ����map�����ȳ�̫�ߣ���ɶ��������ÿ������ֻע��20��ϵͳ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitSysCmdMap5()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_RMSF, "rmsf"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_MKNOD, "mknod"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCSIMGR, "scsimgr"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CFGMGR, "cfgmgr -v"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CFGADM, "cfgadm -al"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_DEVFSADM, "devfsadm"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_RENDEV, "rendev -l"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CHMOD, "chmod"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CHOWN, "chown"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_ORACLEASM, "oracleasm scandisks"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SERVICE, "/sbin/service"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_RAW, "raw"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_LN, "ln -f -s"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_RM, "rm"));
}

/*------------------------------------------------------------
Function Name: InitSysCmdMap6
Description  : ��ʼ��ϵͳ����map�����ȳ�̫�ߣ���ɶ��������ÿ������ֻע��20��ϵͳ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitSysCmdMap6()
{
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_STRINGS, "strings"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VGIMPORT, "vgimport"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VGEXPORT, "vgexport"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_RMDEV, "rmdev"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_EXPORTVG, "exportvg"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_IMPORTVG, "importvg"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXDG_LIST, "vxdg list"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXDG_IMPORT, "vxdg -C import"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXDG_DEPORT, "vxdg deport"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXVOL, "vxvol"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXDISK, "vxdisk"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_UPDATE_DRV, "update_drv"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_VXPRINT, "vxprint"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_SCANASMLIB, "/usr/sbin/oracleasm scandisks"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CMVIEWCL, "cmviewcl"));//��ʾserviceguard��Ⱥ״̬
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_CMRUNCL, "cmruncl"));//��ʾserviceguard��Ⱥ״̬
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_RHCS_CLUSTAT, "clustat"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMAND_RHCS_SERVICE, "service"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMNAD_RHCS_CLUSCVADM, "cluscvadm"));
    m_mapCommand.insert(map<mp_int32, mp_string>::value_type(ROOT_COMMNAD_CLRG, "clrg"));      
}


/*------------------------------------------------------------
Function Name: InitNeedEchoCmdMap1
Description  : ��ʼ����Ҫ��������map
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitNeedEchoCmdMap1()
{
    //��ʼ�������ֶ�Ӧ���Ƿ���Ҫ����ֵ
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_NETSTAT, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_CAT, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_PS, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_LS, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_PVS, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_LSVG, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_HASYS, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_CLLSNODE, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_LSSRC, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_HOSTNAME, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_VXDISK, MP_TRUE));
}

/*------------------------------------------------------------
Function Name: InitNeedEchoCmdMap2
Description  : ��ʼ����Ҫ��������map
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CCommandMap::InitNeedEchoCmdMap2()
{
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_HAGRP, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_HARES, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_VGDISPLAY, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_LVS, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_LSVG, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_IOSCAN, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_CLFINDRES, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_CMVIEWCL, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_STRINGS, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_RHCS_CLUSTAT, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_FSTYP, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_VXDG_LIST, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMNAD_CLRG, MP_TRUE));
    m_mapNeedEcho.insert(map<mp_int32, mp_bool>::value_type(ROOT_COMMAND_MOUNT, MP_TRUE));
}
/*------------------------------------------------------------
Function Name: GetCommandString
Description  : ���������ֻ�ȡ��Ӧ�Ľű����ƻ�ϵͳ�����ַ���
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_string CCommandMap::GetCommandString(mp_int32 iCommandID)
{
    map<mp_int32, mp_string>::iterator it = m_mapCommand.find(iCommandID);
    if (it != m_mapCommand.end())
    {
        return it->second;
    }
    else
    {
        return "unknown";
    }

}

/*------------------------------------------------------------
Function Name: NeedEcho
Description  : ���������ֻ�ȡ�������Ƿ���Ҫ��ȡ������Ϣ
               MP_TRUE:��Ҫ����
               MP_FALSE:����Ҫ����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool CCommandMap::NeedEcho(mp_int32 iCommandID)
{
    map<mp_int32, mp_bool>::iterator it = m_mapNeedEcho.find(iCommandID);
    if (it != m_mapNeedEcho.end())
    {
        return MP_TRUE;
    }
    else
    {
        return MP_FALSE;
    }
}


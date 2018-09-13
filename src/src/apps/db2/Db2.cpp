/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "apps/db2/Db2.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/String.h"
#include "common/Defines.h"
#include "common/RootCaller.h"
#include "array/Array.h"

CDB2::CDB2()
{
}

CDB2::~CDB2()
{
}

/*------------------------------------------------------------ 
Description  : ������ȡ���ݿ���Ϣ�ű����ؽ���е����ݿ���Ϣ
Input        : vecResult -- ���ݿ���Ϣ�ַ���
Output       : vecdbInstInfo -- ���ݿ���Ϣ
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CDB2::AnalyseInstInfoScriptRst(vector<mp_string> vecResult, vector<db2_inst_info_t> &vecdbInstInfo)
{
    size_t idxSep, idxSepSec, idxSepTrd;
    mp_string strinstName;
    mp_string strdbName;
    mp_string strversion;
    mp_string strState;
    vector<mp_string>::iterator iter;

    db2_inst_info_t stdbInstInfo;
    const mp_string SEPARATOR = STR_COLON;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin analyse db info script result.");
    
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 1nd separator, inst info is [%s].", (*iter).c_str());;
            continue;
        }
        strinstName = iter->substr(0, idxSep);

        //find 2nd separator(;)            
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 2nd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }      
        strdbName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);  

        //find 3rd separator(;)    
        idxSepTrd = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepTrd)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db inst info failed when find 3rd separator, inst info is [%s].", (*iter).c_str());
            continue;
        }    
        strversion = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);
        strState = iter->substr(idxSepTrd + 1);

        stdbInstInfo.strinstName = strinstName;
        stdbInstInfo.strdbName = strdbName;
        stdbInstInfo.strversion= strversion;
        stdbInstInfo.istate= atoi(strState.c_str());
        vecdbInstInfo.push_back(stdbInstInfo);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "The analyse result of script is:instname(%s), dbname(%s), version(%s), state(%d).",
            stdbInstInfo.strinstName.c_str(), stdbInstInfo.strdbName.c_str(),
            stdbInstInfo.strversion.c_str(), stdbInstInfo.istate, vecdbInstInfo.size());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to analyse db info script result.");
}

/*------------------------------------------------------------ 
Description  : ������ȡ���ݿ�LUN��Ϣ�ű����ؽ���е����ݿ�洢��Ϣ
Input        : vecResult -- ���ݿ�洢��Ϣ�ַ���
Output       : vecdbStorInfo -- ���ݿ�洢��Ϣ
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_void CDB2::AnalyseLunInfoScriptRST(vector<mp_string> vecResult, vector<db2_storage_info_t> &vecdbStorInfo)
{  
    size_t idxSep, idxSepSec, idxSepTrd, idxSepFor, idxSepFiv;
    mp_string strvolName;
    mp_string strvgName;
    mp_string strdeviceName;
    mp_string strdiskName;
    mp_string strvolType;
    mp_string strstorageType;
    vector<mp_string>::iterator iter;

    db2_storage_info_t stdbStorInfo;
    const mp_string SEPARATOR = STR_COLON;

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get db storage info.");
    
    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter)
    {
        //find 1st separator(;)
        idxSep = iter->find(SEPARATOR);
        if (mp_string::npos == idxSep)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 1nd separator, storage info is [%s].", (*iter).c_str());;
            continue;
        }
        strvolName = iter->substr(0, idxSep);

        //find 2nd separator(;)            
        idxSepSec = iter->find(SEPARATOR, idxSep + 1);
        if (mp_string::npos == idxSepSec)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 2nd separator, storage info is [%s].", (*iter).c_str());
            continue;
        }      
        strvgName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);  

        //find 3rd separator(;)���ص�
        idxSepTrd = iter->find(SEPARATOR, idxSepSec + 1);
        if (mp_string::npos == idxSepTrd)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
              "Get db storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
            continue;
        }    
        strdeviceName = iter->substr(idxSepSec + 1, (idxSepTrd - idxSepSec) - 1);

        //find 4rd separator(;)    
        idxSepFor = iter->find(SEPARATOR, idxSepTrd + 1);
        if (mp_string::npos == idxSepFor)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
            continue;
        } 
        strdiskName = iter->substr(idxSepTrd + 1, (idxSepFor - idxSepTrd) - 1);

        //find 5rd separator(;)    
        idxSepFiv = iter->find(SEPARATOR, idxSepFor + 1);
        if (mp_string::npos == idxSepFiv)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR,
                "Get db storage info failed when find 3rd separator, storage info is [%s].", (*iter).c_str());
            continue;
        } 
        strvolType = iter->substr(idxSepFor + 1, (idxSepFiv - idxSepFor) - 1);
        strstorageType = iter->substr(idxSepFiv + 1);
    
        stdbStorInfo.strvolName = strvolName;
        stdbStorInfo.strvgName= strvgName;
        stdbStorInfo.strdeviceName= strdeviceName;
        stdbStorInfo.strdiskName = strdiskName;
        stdbStorInfo.ivolType = atoi(strvolType.c_str());
        stdbStorInfo.istorageType= atoi(strstorageType.c_str());
        vecdbStorInfo.push_back(stdbStorInfo);
        
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "Get db storage info structure:volName(%s), vgName(%s), deviceName(%s), diskName(%s), volType(%d), storageType(%d).", 
            stdbStorInfo.strvolName.c_str(), stdbStorInfo.strvgName.c_str(), stdbStorInfo.strdeviceName.c_str(), 
            stdbStorInfo.strdiskName.c_str(), stdbStorInfo.ivolType, stdbStorInfo.istorageType);
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to get db storage info.");
}


/*------------------------------------------------------------ 
Description  : ������ȡLUN��Ϣ�ű��������
Input        : stdbinfo -- ���ݿ���Ϣ
Output       : strParam -- ��������ַ���
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
void CDB2::BuildLunInfoScriptParam(db2_db_info_t stdbinfo, mp_string& strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbinfo.strinstName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbinfo.strdbName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbinfo.strdbUsername+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbinfo.strdbPassword;
}


/*------------------------------------------------------------ 
Description  : ����ִ�б������Ժ�����ֹͣ�ű��������
Input        : stdbinfo -- ���ݿ���Ϣ
                  strOperType --��������
Output       : strParam -- ��������ַ���
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/ 
void CDB2::BuildScriptParam(db2_db_info_t stdbinfo, mp_string strOperType, mp_string& strParam)
{
    strParam = mp_string(SCRIPTPARAM_INSTNAME) + stdbinfo.strinstName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBNAME) + stdbinfo.strdbName+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBUSERNAME) + stdbinfo.strdbUsername+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_DBPASSWORD) + stdbinfo.strdbPassword+ mp_string(STR_COLON)
        + mp_string(SCRIPTPARAM_OPERTYPE) + strOperType;
}

/*------------------------------------------------------------ 
Description  : ��ȡ����LUN��Ϣ
Input        : vecdbStorInfo -- ���ݿ�洢��Ϣ
Output       : vecdbLunInfo -- ���ݿ��LUN��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::GetDiskLunInfo(vector<db2_storage_info_t> vecdbStorInfo, vector<db2_lun_info_t> &vecdbLunInfo)
{
    mp_int32 iRet = MP_SUCCESS;
    vector<mp_string> vecResult;
    vector<db2_storage_info_t>::iterator iter;
    db2_lun_info_t stdbLunInfo;
    mp_string strLunWWN;
    mp_string strLunID;
    mp_string strSN;
    mp_string strVendor;
    mp_string strProduct;
    mp_string strDev;


    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get lun info of disk.");
    
    for (iter = vecdbStorInfo.begin(); iter != vecdbStorInfo.end(); ++iter)
    {
#ifdef HP_UX_IA

        iRet = CDisk::GetHPRawDiskName(iter->strdiskName, strDev);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get raw device info of disk(%s) failed, iRet %d.",
                iter->strdiskName.c_str(), iRet);
            return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
        }
#else
        strDev = iter->strdiskName;
#endif
        //���̺��ͺ�
        iRet = CArray::GetArrayVendorAndProduct(strDev, strVendor, strProduct);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get array info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        (void)CMpString::Trim((mp_char*)strVendor.c_str());
        (void)CMpString::Trim((mp_char*)strProduct.c_str());
        
        //�ų����ǻ�Ϊ�Ĳ�Ʒ
        if (0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI)
            && 0 != strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY))
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The disk(%s) is not huawei LUN, Vendor:%s.", 
                strDev.c_str(), strVendor.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        iRet = CArray::GetArraySN(strDev, strSN);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get SN info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        iRet = CArray::GetLunInfo(strDev, strLunWWN, strLunID);
        if (MP_SUCCESS != iRet)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info of disk(%s) failed.", strDev.c_str());
            return ERROR_COMMON_NOT_HUAWEI_LUN;
        }

        stdbLunInfo.strpvName = iter->strdiskName;
        stdbLunInfo.strvolName = iter->strvolName;
        stdbLunInfo.strvgName=iter->strvgName;
        stdbLunInfo.strdeviceName = iter->strdeviceName;
        stdbLunInfo.ivolType = iter->ivolType;
        stdbLunInfo.istorageType = iter->istorageType;
        stdbLunInfo.strarraySn = strSN;
        stdbLunInfo.strlunId = strLunID;
        stdbLunInfo.strwwn = strLunWWN;
        vecdbLunInfo.push_back(stdbLunInfo);
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, 
            "Get lun info of disk:arraySN(%s), lunId(%s), wwn(%s).", 
            stdbLunInfo.strarraySn.c_str(), stdbLunInfo.strlunId.c_str(), stdbLunInfo.strwwn.c_str());
    }

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "End to get lun info of disk.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ��ȡ���ݿ���Ϣ
Input        :
Output       : vecdbInstInfo -- ���ݿ���Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::GetInfo(vector<db2_inst_info_t> &vecdbInstInfo)
{
    mp_string strParam;
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get db info.");

    //db2�»�ȡʵ��״̬���������root����ʵ���û���ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYDB2INFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Excute get db info script failed, iRet %d.", iRet);
        return iRet;
    }
    AnalyseInstInfoScriptRst(vecResult, vecdbInstInfo);
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get db info succ.");
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : ��ȡ���ݿ�LUN��Ϣ
Input        :vecdbInstInfo -- ���ݿ���Ϣ
Output       : vecLunInfos -- ���ݿ�LUN��Ϣ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::GetLunInfo(db2_db_info_t stdbinfo, vector<db2_lun_info_t>& vecLunInfos)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string strParam;
    vector<mp_string> vecResult;
    vector<mp_string>::iterator iter;
    vector<db2_storage_info_t> vecdbStorInfo;
        
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to get db(%s) lun info.", stdbinfo.strdbName.c_str());
    
    BuildLunInfoScriptParam(stdbinfo, strParam);
    
    //db2�»�ȡʵ��״̬���������root����ʵ���û���ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYDB2LUNINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute get db(%s) lun info script failed, iRet %d.", stdbinfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    AnalyseLunInfoScriptRST(vecResult, vecdbStorInfo);

    iRet = GetDiskLunInfo(vecdbStorInfo, vecLunInfos);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get lun info of disk failed, iRet %d.", iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get db lun(%s) info succ.", stdbinfo.strdbName.c_str());
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : �������ݿ�
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::Start(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin start db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_STARTDB, strParam);
    
    //db2���������ݿ����������root����ʵ���û���ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_RECOVERDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute start db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Start db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ֹͣ���ݿ�
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::Stop(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin stop db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_STOPDB, strParam);
    
    //db2��ֹͣ���ݿ����������root����ʵ���û���ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_RECOVERDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute stop db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Stop db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : �����������ݿ�
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::Test(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to test db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_TESTDB, strParam);
    
    //db2�²������ݿ��������������root����ʵ���û���ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLEDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute test db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Test db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  : �������ݿ�
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::Freeze(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to freeze db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_FREEZEDB, strParam);
    
    //db2�¶������ݿ����������root����ʵ���û���ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLEDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute freeze db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Freeze db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : �ⶳ���ݿ�
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::UnFreeze(db2_db_info_t stdbInfo)
{
    mp_string strParam;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to thaw db(%s).", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_THAWDB, strParam);
    
    //db2�½ⶳ���ݿ����������root����ʵ���û���ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLEDB2, strParam, NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute thaw db(%s) script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Thaw db(%s) succ.", stdbInfo.strdbName.c_str());
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ��ѯ���ݿⶳ��״̬
Input        :stdbInfo -- ���ݿ���Ϣ
Output       : iFreezeState -- ���ݿⶳ��״̬
Return       :  MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CDB2::QueryFreezeState(db2_db_info_t stdbInfo, mp_int32& iFreezeState)
{
    mp_string strParam;
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Begin to query db(%s) freeze state.", stdbInfo.strdbName.c_str());    
    BuildScriptParam(stdbInfo, SCRIPTPARAM_FREEZESTATE, strParam);

	//codedex��CHECK_CONTAINER_EMPTY,����vecResult��֮ǰ�Ĵ����ܱ�֤��Ϊ�գ��˴����Բ��ж�
    //db2�²�ѯ���ݿⶳ��״̬���������root����ʵ���û���ִ��
    iRet = CRootCaller::Exec((mp_int32)ROOT_COMMAND_SCRIPT_SAMPLEDB2, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (MP_SUCCESS != iRet)
    {  
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, 
            "Excute query db(%s) freeze state script failed, iRet %d.", stdbInfo.strdbName.c_str(), iRet);
        return iRet;
    }
    iFreezeState = atoi(vecResult.front().c_str());
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, 
        "Query db(%s) freeze state(%d) succ.", stdbInfo.strdbName.c_str(), iFreezeState);
    return MP_SUCCESS;
}


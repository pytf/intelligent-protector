/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/Password.h"
#include "tools/agentcli/ChgSnmp.h"

/*------------------------------------------------------------ 
Description  :��������Ϊ����Ȧ���Ӷ���ԭhandle�������в��
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CChgSnmp::HandleInner()
{
    SNMP_CHOOSE_TYPE eChooseType = GetChoice();
    mp_int32 iRet = MP_FAILED;
    switch (eChooseType)
    {
        case SNMP_CHOOSE_SET_AUTH_PASSWD:
            printf("%s\n", SET_AUTH_PASSWD_HINT);
            if(CheckSNMPPwd(PASSWORD_SNMP_AUTH))
            {
                iRet = CPassword::ChgPwd(PASSWORD_SNMP_AUTH);
            }
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP auth.");
        break;
        
        case SNMP_CHOOSE_SET_PRI_PASSWD:
            printf("%s\n", SET_PRI_PASSWD_HINT);
            if(CheckSNMPPwd(PASSWORD_SNMP_PRIVATE))
            {
                iRet = CPassword::ChgPwd(PASSWORD_SNMP_PRIVATE);
            }
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP private.");
        break;
        
        case SNMP_CHOOSE_SET_AUTH_PROTOCOL:
            printf("%s\n", SET_AUTH_PROTOCOL_HINT);
            iRet = ChgAuthProtocol();
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP auth protocol.");
        break;
        
        case SNMP_CHOOSE_SET_PRI_PROTOCOL:
            printf("%s\n", SET_PRI_PROTOCOL_HINT);
            iRet = ChgPrivateProtocol();
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP private protocol.");
        break;
        
        case SNMP_CHOOSE_SET_SECURITY_NAME:
            iRet = ChgSecurityName();
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP security name.");
        break;
        
        default:
            iRet = HandleInner2(eChooseType);
        break;
    }
    return iRet;
}


/*------------------------------------------------------------ 
Description  :��������Ϊ����Ȧ���Ӷ���ԭHandleInner�������в��
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CChgSnmp::HandleInner2(SNMP_CHOOSE_TYPE eChooseType)
{
    mp_int32 iRet = MP_FAILED;
    switch (eChooseType)
    {        
        case SNMP_CHOOSE_SET_SECURITY_LEVEL:
            printf("%s\n", SET_SECURITY_LEVEL_HINT);
            iRet = ChgSecurityLevel();
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP security level.");
        break;
        
        case SNMP_CHOOSE_SET_SECURITY_MODEL:
            printf("%s\n", SET_SECURITY_MODEL_HINT);
            iRet = ChgSecurityModel();
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP security mode.");
        break;
        
        case SNMP_CHOOSE_SET_CONTEXT_ENGID:
            iRet = ChgContextEngineID();
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP context engid.");
        break;
        
        case SNMP_CHOOSE_SET_CONTEXT_NAME:
            iRet = ChgContextName();
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "change SNMP context name.");
        break;
        default:
            iRet = MP_SUCCESS;
        break;
    }
    return iRet;
}

/*------------------------------------------------------------ 
Description  :������
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CChgSnmp::Handle()
{
    //У�鵱ǰ����Ա������
    //�������ļ���ȡ�û���
    mp_string strUsrName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name from xml configuration file failed");
        return MP_FAILED;
    }
    mp_string strOldPwd;
    mp_uint32 iInputFailedTimes = 0;
    while (iInputFailedTimes <= MAX_FAILED_COUNT)
    {
        CPassword::InputUserPwd(strUsrName, strOldPwd, INPUT_GET_ADMIN_OLD_PWD);
        if (CPassword::CheckAdminOldPwd(strOldPwd))
        {
            break;
        }
        else
        {
            iInputFailedTimes++;
            continue;
        }
    }

    strOldPwd.replace(0, strOldPwd.length(), "");
    if (iInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("%s.\n", OPERATION_LOCKED_HINT);
        CPassword::LockAdmin();
        return MP_FAILED;
    }

    iRet = HandleInner();
    if (MP_SUCCESS != iRet)
    {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
    }
    else
    {
        printf("%s\n", OPERATION_PROCESS_SUCCESS_HINT); 
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "SNMP configuration is modified successfully.");
    }

    return iRet;
}
/*------------------------------------------------------------ 
Description  : ��ʾ����ѡ������û�����
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/  
SNMP_CHOOSE_TYPE CChgSnmp::GetChoice()
{
    printf("%s:\n", CHOOSE_OPERATION_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_AUTH_PASSWD, SET_AUTH_PASSWD_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_PRI_PASSWD, SET_PRI_PASSWD_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_AUTH_PROTOCOL, SET_AUTH_PROTOCOL_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_PRI_PROTOCOL, SET_PRI_PROTOCOL_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_SECURITY_NAME, SET_SECURITY_NAME_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_SECURITY_LEVEL, SET_SECURITY_LEVEL_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_SECURITY_MODEL, SET_SECURITY_MODEL_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_CONTEXT_ENGID, SET_CONTEXT_ENGID_HINT);
    printf("%d: %s\n", SNMP_CHOOSE_SET_CONTEXT_NAME, SET_CONTEXT_NAME_HINT);
    printf("%s\n", QUIT_HINT);
    printf("%s", CHOOSE_HINT);

    mp_string strChoice;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    if (strChoice.length() == 1 && (strChoice[0] >= '0' + SNMP_CHOOSE_SET_AUTH_PASSWD)
    && (strChoice[0] < '0' + SNMP_CHOOSE_SET_BUTT))
    {
        return SNMP_CHOOSE_TYPE(strChoice[0] - '0');
    }
    else
    {
        return SNMP_CHOOSE_SET_BUTT;
    }
}
/*------------------------------------------------------------ 
Description  : ����SNMP��֤�����㷨
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CChgSnmp::ChgAuthProtocol()
{
    printf("%d: %s\n", AUTH_PROTOCOL_NONE, NONE);
    printf("%d: %s\n", AUTH_PROTOCOL_MD5, MD5);
    printf("%d: %s\n", AUTH_PROTOCOL_SHA1, SHA1);
    printf("%d: %s\n", AUTH_PROTOCOL_SHA2, SHA2);
    printf("%s", CHOOSE_HINT);

    mp_string strChoice;
    mp_int32 iRet = MP_FAILED;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    //if (strChoice.length() != 1 || (strChoice[0] < '0' + AUTH_PROTOCOL_NONE) || (strChoice[0] > '0' + AUTH_PROTOCOL_SHA2))
    mp_bool bAuthProCheck = strChoice.length() != 1 || 
                                (      (strChoice[0] != '0' + AUTH_PROTOCOL_NONE)
                                    && (strChoice[0] != '0' + AUTH_PROTOCOL_MD5)
                                    && (strChoice[0] != '0' + AUTH_PROTOCOL_SHA1)
                                    && (strChoice[0] != '0' + AUTH_PROTOCOL_SHA2)
                                );
    if (bAuthProCheck)
    {
        printf("%s\n", AUTH_PROTOCOL_NOT_SUPPORTED);
        return MP_FAILED;
    }

    if (strChoice[0] != '0' + AUTH_PROTOCOL_SHA2) 
    {
        printf("%s\n", AUTH_PROTOCOL_NOT_SAFE);
        printf("%s", CONTINUE);
        mp_string strTmpChoice;
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice != "y" && strTmpChoice != "Y")
        {
            return MP_FAILED;
        }
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_AUTH_PROTOCOL, strChoice);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}
/*------------------------------------------------------------ 
Description  : ����SNMP˽�м����㷨
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CChgSnmp::ChgPrivateProtocol()
{
    printf("%d: %s\n", PRIVATE_PROTOCOL_NONE, NONE);
    printf("%d: %s\n", PRIVATE_PROTOCOL_DES, DES);
    printf("%d: %s\n", PRIVATE_PROTOCOL_AES128, AES128);
    printf("%s", CHOOSE_HINT);
    mp_string strChoice;
    mp_int32 iRet = MP_FAILED;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    if (strChoice.length() != 1 || (strChoice[0]  != '0' + PRIVATE_PROTOCOL_NONE) 
    && (strChoice[0] !=  '0' + PRIVATE_PROTOCOL_AES128) &&  (strChoice[0] !=  '0' + PRIVATE_PROTOCOL_DES))
    {
        printf("%s\n", PRIVATE_PROTOCOL_NOT_SUPPORTED);
        return MP_FAILED;
    }

    if (strChoice[0] != '0' + PRIVATE_PROTOCOL_AES128) 
    {
        printf("%s\n", PRIVATE_PROTOCOL_NOT_SAFE);
        printf("%s", CONTINUE);
        mp_string strTmpChoice;
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice != "y" && strTmpChoice != "Y")
        {
            return MP_FAILED;
        }
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_PRIVATE_PROTOCOL, strChoice);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;    
}
/*------------------------------------------------------------ 
Description  : ����SNMP��ȫ����
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CChgSnmp::ChgSecurityName()
{
    printf("%s", SECURITY_NAME_HINT);
    mp_string strName;
    CPassword::GetInput(SECURITY_NAME_HINT, strName);

    if (strName.length() > MAX_SNMP_PARAM_LEN)
    {
        printf("%s\n", INPUT_TOO_LONG_HINT);
        return MP_FAILED;
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_SECURITY_NAME, strName);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}
/*------------------------------------------------------------ 
Description  : ����SNMP��ȫ����
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CChgSnmp::ChgSecurityLevel()
{
    printf("%d: %s\n", SECURITY_LEVEL_NOAUTH_NOPRIV, NOAUTH_NOPRIV);
    printf("%d: %s\n", SECURITY_LEVEL_NOPRI, AUTH_NOPRIV);
    printf("%d: %s\n", SECURITY_LEVEL_AUTH_PRIV, AUTH_PRIV);
    printf("%s", CHOOSE_HINT);
    mp_string strChoice;
    mp_int32 iRet = MP_FAILED;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    if (strChoice.length() != 1 
    || (strChoice[0] < '0' + SECURITY_LEVEL_NOAUTH_NOPRIV) || (strChoice[0] > '0' + SECURITY_LEVEL_AUTH_PRIV))
    {
        printf("%s\n", SECURITY_LEVEL_NOT_SUPPORTED);
        return MP_FAILED;
    }

    if (strChoice[0] != '0' + SECURITY_LEVEL_AUTH_PRIV) 
    {
        printf("%s\n", SECURITY_LEVEL_NOT_SAFE);
        printf("%s", CONTINUE);
        mp_string strTmpChoice;
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice != "y" && strTmpChoice != "Y")
        {
            return MP_FAILED;
        }
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_SECURITY_LEVEL, strChoice);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;  
}
/*------------------------------------------------------------ 
Description  : ����SNMP��ȫģʽ
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CChgSnmp::ChgSecurityModel()
{
    printf("%d: %s\n", SECURITY_MODEL_ANY, ANY);
    printf("%d: %s\n", SECURITY_MODEL_V1, V1);
    printf("%d: %s\n", SECURITY_MODEL_V2, V2);
    printf("%d: %s\n", SECURITY_MODEL_USM, USM);
    printf("%s", CHOOSE_HINT);

    mp_string strChoice;
    mp_int32 iRet = MP_FAILED;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    if (strChoice.length() != 1 || (strChoice[0] < '0' + SECURITY_MODEL_ANY) || (strChoice[0] > '0' + SECURITY_MODEL_USM))
    {
        printf("%s\n", SECURITY_MODEL_NOT_SUPPORTED);
        return MP_FAILED;
    }
    
    if (strChoice[0] != '0' + SECURITY_MODEL_USM) 
    {
        printf("%s\n", SECURITY_MODEL_NOT_SAFE);
        printf("%s", CONTINUE);
        mp_string strTmpChoice;
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice != "y" && strTmpChoice != "Y")
        {
            return MP_FAILED;
        }
    }
    
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_SECURITY_MODEL, strChoice);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml configuration file failed.");
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  : ����SNMP����������ID
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CChgSnmp::ChgContextEngineID()
{
    mp_string strID;
    printf("%s", CONTEXT_EN_ID_HINT);
    mp_int32 iRet = MP_FAILED;
    CPassword::GetInput(CONTEXT_EN_ID_HINT, strID);

    if (strID.length() > MAX_SNMP_PARAM_LEN)
    {
        printf("%s\n", INPUT_TOO_LONG_HINT);
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_ENGINE_ID , strID);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}
/*------------------------------------------------------------ 
Description  : ����SNMP������
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_int32 CChgSnmp::ChgContextName()
{
    mp_string strName;
    printf("%s", CONTEXT_NAME_HINT);
    mp_int32 iRet = MP_FAILED;
    CPassword::GetInput(CONTEXT_NAME_HINT, strName);

    if (strName.length() > MAX_SNMP_PARAM_LEN)
    {
        printf("%s\n", INPUT_TOO_LONG_HINT);
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_CONTEXT_NAME , strName);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}

mp_bool CChgSnmp::CheckSNMPPwd(PASSWOD_TYPE eType)
{
    mp_int32 iInputFailedTimes = 0;
    mp_string strSNMPPwd;
    while (iInputFailedTimes <= MAX_FAILED_COUNT)
    {
        CPassword::InputUserPwd("", strSNMPPwd, INPUT_SNMP_OLD_PWD);
        if (CPassword::CheckOtherOldPwd(eType, strSNMPPwd))
        {
            break;
        }
        else
        {
            iInputFailedTimes++;
            continue;
        }
    }

    strSNMPPwd.replace(0, strSNMPPwd.length(), "");
    if (iInputFailedTimes > MAX_FAILED_COUNT)
    {
        return MP_FALSE;
    }
    else
    {
        return MP_TRUE;
    }
}


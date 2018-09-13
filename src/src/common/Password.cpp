/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include "common/Password.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Path.h"
#include <sstream>
#include <algorithm> 
/*------------------------------------------------------------ 
Description  :�޸�����
Input        :      eType---��������
Output       :      
Return       : MP_SUCCESS---�޸ĳɹ�
                  MP_FAILED---�޸�ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ChgPwd(PASSWOD_TYPE eType)
{
    mp_uint32 uiInputFailedTimes = 0;
    //�������ļ���ȡ�û���
    mp_string strUsrName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    //����������
    uiInputFailedTimes = 0;
    mp_string strNewPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(eType, strNewPwd))
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    //�ظ�����������
    uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (strConfirmedPwd == strNewPwd)
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            if(uiInputFailedTimes <= MAX_FAILED_COUNT) 
            {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    //��������
    if (!SaveOtherPwd(eType, strNewPwd))
    {
        printf("Save password failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Save password failed.");
        return MP_FAILED;
    }

    strNewPwd.replace(0, strNewPwd.length(), "");
    strConfirmedPwd.replace(0, strConfirmedPwd.length(), "");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :�޸�����
Input        :      eType---��������
Output       :     strPwd---����
Return       : MP_SUCCESS---�޸ĳɹ�
                  MP_FAILED---�޸�ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ChgPwd(PASSWOD_TYPE eType, mp_string& strPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    //�������ļ���ȡ�û���
    mp_string strUsrName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    //����������
    uiInputFailedTimes = 0;
    mp_string strNewPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(eType, strNewPwd))
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    //�ظ�����������
    uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (strConfirmedPwd == strNewPwd)
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            if(uiInputFailedTimes <= MAX_FAILED_COUNT)
            {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    strPwd = strNewPwd;
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  :�޸����벻�����Ⱥ����븴�Ӷ���֤
Output       :     strPwd---����
Return       : MP_SUCCESS---�޸ĳɹ�
                  MP_FAILED---�޸�ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ChgPwdNoCheck(mp_string& strPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    mp_string strUsrName;
    mp_string strNewPwd;

    //����������
    InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT, -1);

    //�ظ�����������
    uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD, -1);
        if (strConfirmedPwd == strNewPwd)
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            if(uiInputFailedTimes <= MAX_FAILED_COUNT)
            {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        printf("Input invalid password over 3 times.\n"); 
        return MP_FAILED;
    }

    strPwd = strNewPwd;
    return MP_SUCCESS;
}


/*------------------------------------------------------------ 
Description  :�޸�admin����
Input        :       
Output       :      
Return       : MP_SUCCESS---�޸ĳɹ�
                  MP_FAILED---�޸�ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ChgAdminPwd()
{
    mp_int32 iRet = MP_SUCCESS;
    //�������ļ���ȡ�û���
    mp_string strUsrName;
    mp_string strNewPwd;
    
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    //У�鵱ǰ����Ա������
    iRet = VerifyOldUserPwd(strUsrName);
    if (MP_SUCCESS != iRet)
    {
        printf("%s\n", OPERATION_LOCKED_HINT);
        CPassword::LockAdmin();
        return MP_FAILED;
    }

    //����������
    iRet = InputNewUserPwd(strUsrName, strNewPwd);
    if (MP_SUCCESS != iRet)
    {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }
    
    //У��������
    iRet = ConfirmNewUserPwd(strUsrName, strNewPwd);
    if (MP_SUCCESS != iRet)
    {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }
    
    //��������
    if (!SaveAdminPwd(strNewPwd))
    {
        printf("Save password failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Save password failed.");
        return MP_FAILED;
    }

    strNewPwd.replace(0, strNewPwd.length(), "");
    printf("Password of %s is modified successfully.\n", strUsrName.c_str());
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Password of %s is modified successfully.", strUsrName.c_str());
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :�ж�������
Input        :       strUserName---�û���
Output       :      
Return       : MP_SUCCESS---����ƥ��ɹ�
                  MP_FAILED---���벻ƥ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::VerifyOldUserPwd(mp_string& strUserName)
{
    mp_string strOldPwd;
    mp_uint32 uiInputFailedTimes = 0;
    
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUserName, strOldPwd, INPUT_GET_ADMIN_OLD_PWD);
        if (CheckAdminOldPwd(strOldPwd))
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            continue;
        }
    }

    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        return MP_FAILED;
    }

    strOldPwd.replace(0, strOldPwd.length(), "");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :���� �û�������
Input        :       strUserName---�û�����strNewPwd---������
Output       :      
Return       : MP_SUCCESS---����ɹ�
                  MP_FAILED---����ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::InputNewUserPwd(mp_string& strUserName, mp_string& strNewPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUserName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(PASSWORD_ADMIN, strNewPwd))
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            continue;
        }
    }
    
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
         return MP_FAILED;
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :ȷ�� �û�������
Input        : strUserName---�û�����strNewPwd---������
Output       :      
Return       : MP_SUCCESS---����ɹ�
                  MP_FAILED---����ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::ConfirmNewUserPwd(mp_string& strUserName, mp_string& strNewPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    
    while (uiInputFailedTimes <= MAX_FAILED_COUNT)
    {
        InputUserPwd(strUserName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (strConfirmedPwd == strNewPwd)
        {
            break;
        }
        else
        {
            uiInputFailedTimes++;
            if(uiInputFailedTimes <= MAX_FAILED_COUNT)
            {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    
    if (uiInputFailedTimes > MAX_FAILED_COUNT)
    {
        return MP_FAILED;
    }

    strConfirmedPwd.replace(0, strConfirmedPwd.length(), "");
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :�����û� ����
Input        : strUserName---�û�����strNewPwd--- ���룬eType---��������
Output       :      
Return       :  
                   
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CPassword::InputUserPwd(mp_string strUserName, mp_string &strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen)
{
    mp_uint32 uiIndex;
    char chTmpPwd[PWD_LENGTH] = {0};
    switch (eType)
    {
    case INPUT_GET_ADMIN_OLD_PWD:
        printf("%s of %s:", INPUT_OLD_PASSWORD, strUserName.c_str());
        uiIndex = mp_string(INPUT_OLD_PASSWORD + strUserName).length() + mp_string(" of ").length();
        break;
    case INPUT_CONFIRM_NEW_PWD:
        printf(CONFIRM_PASSWORD);
        uiIndex = mp_string(CONFIRM_PASSWORD).length();
        break;
    case INPUT_SNMP_OLD_PWD:
        printf(INPUT_SNMP_OLD_PASSWORD);
        uiIndex = mp_string(INPUT_SNMP_OLD_PASSWORD).length();
        break;
    default:
        printf(INPUT_NEW_PASSWORD);
        uiIndex = mp_string(INPUT_NEW_PASSWORD).length();
        break;
    }

    mp_uchar chPwd = (mp_uchar)GETCHAR;
    mp_uint32 uiLen = uiIndex;
    mp_bool bIsBackSpace=MP_TRUE;
    mp_bool bOutofLen=MP_TRUE;
    while ((chPwd & 0xff) != ENTER_SPACE && (chPwd & 0xff) != NEWLINE_SPACE)
    {
        bIsBackSpace = ( ((chPwd & 0xff) == BACK_SPACE) && (uiIndex == uiLen) );
        if ( bIsBackSpace )
        {
            chPwd = (mp_uchar)GETCHAR;
            continue;
        }

        if ((chPwd & 0xff) == BACK_SPACE)
        {
            printf("\b \b");
            uiIndex--;
        }
        else
        {
            printf(" ");
			//CodeDex��CSEC_LOOP_ARRAY_CHECKING�������±겻��Խ��
            chTmpPwd[uiIndex - uiLen] = chPwd;
            uiIndex++;
            
            bOutofLen = ( (iPwdLen > 0) && ((uiIndex - uiLen) > iPwdLen) ) ;
            if ( bOutofLen )
            {
                break;
            }
        }
        chPwd = (mp_uchar)GETCHAR;
    }
    chTmpPwd[uiIndex - uiLen] = '\0';
    printf("\n");

    strUserPwd = chTmpPwd;
    (mp_void)memset_s(chTmpPwd, PWD_LENGTH, 0, PWD_LENGTH);
}
/*------------------------------------------------------------ 
Description  :���admin������
Input        : strOldPwd---������
Output       :      
Return       :  MP_TRUE---������ƥ��
                MP_FALSE ---�����벻ƥ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckAdminOldPwd(const mp_string& strOldPwd)
{
    //��ȡ�����ļ��б������ֵ
    mp_string strSalt;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if(MP_SUCCESS != iRet)
    {
        printf("Get salt value from xml configuration file failed.\n");
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Get salt value from xml configuration file failed.");
        return MP_FALSE;
    }

    //ʹ��sha256��ȡhashֵ���������ϰ汾����
    mp_char outHashHex[SHA256_BLOCK_SIZE + 1] = {0};
    mp_string strInput = strOldPwd + strSalt;
    iRet = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, sizeof(outHashHex));
    strInput.replace(0, strInput.length(), "");
    if (iRet != MP_SUCCESS)
    {
        printf("Get sha256 hash value failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sha256 hash value failed.");
        return MP_FAILED;
    }

    //�°汾������PBKDF2����ɢ��
    mp_string strOut;
    iRet = PBKDF2Hash(strOldPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
        return iRet;
    }

    //�������ļ��л�ȡ�������hashֵ
    mp_string strOldHash;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, strOldHash);
    if (MP_SUCCESS != iRet)
    {
        printf("Parse xml config failed, key is hash.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Parse xml config failed, key is hash.");
        return MP_FAILED;
    }

    return (strOldHash == mp_string(outHashHex) || strOldHash == strOut) ? MP_TRUE : MP_FALSE;
}

/*------------------------------------------------------------ 
Description  : ��ȡnginx��key
Input        : vecResult -- nginx�����ļ�����
Output       : strKey -- key
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CPassword::GetNginxKey(mp_string & strKey, const vector<mp_string> & vecResult)
{
    mp_string strTmp;
    mp_string::size_type iPosSSLPwd = mp_string::npos;
    for (mp_uint32 i = 0; i < vecResult.size(); i++)
    {
        strTmp = vecResult[i];
        iPosSSLPwd = strTmp.find(NGINX_SSL_PWD, 0);
        if (iPosSSLPwd != mp_string::npos)
        {
            iPosSSLPwd += strlen(NGINX_SSL_PWD);
            mp_string::size_type iPosSemicolon = strTmp.find(CHAR_SEMICOLON, iPosSSLPwd);
            if (iPosSemicolon != mp_string::npos)
            {
                strKey = strTmp.substr(iPosSSLPwd, iPosSemicolon-iPosSSLPwd);
                (mp_void)CMpString::Trim((mp_char *)strKey.c_str());
            }
            break;
        }
    }

}

/*------------------------------------------------------------ 
Description  : ����������nginx��key�Ƿ���ϵ�һ��
Input        : strNewPwd -- �������nginx��key
Output       : 
Return       : MP_TRUE -- һ��
               MP_FALSE -- ��һ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckNginxOldPwd(const mp_string& strNewPwd)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile.c_str()))
    {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Nginx config file does not exist, path is \"%s\"", AGENT_NGINX_CONF_FILE);
        return MP_FALSE;
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    iRet = CMpFile::ReadFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet || vecResult.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read nginx config file failed, iRet = %d, size of vecResult is %d.",
                        iRet, vecResult.size());
        return MP_FALSE;
    }

    mp_string strOldPwd = "";
    GetNginxKey(strOldPwd, vecResult);
    mp_string strDecryptPwd = "";
    DecryptStr(strOldPwd, strDecryptPwd);
    iRet = (strDecryptPwd == strNewPwd ? MP_TRUE : MP_FALSE);
    strDecryptPwd.replace(0, strDecryptPwd.length(), "");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :���admin������
Input        : strOldPwd---������
Output       :      
Return       :  MP_TRUE---������ƥ��
                MP_FALSE ---�����벻ƥ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckOtherOldPwd(PASSWOD_TYPE eType, const mp_string& strOldPwd)
{
    //����ǵ�һ�γ�ʼ�����룬��У������룬���سɹ�
    if (eType == PASSWORD_INPUT)
    {
        return MP_SUCCESS;
    }

    if (eType == PASSWORD_NGINX_SSL)
    {
        return CheckNginxOldPwd(strOldPwd);
    }

    mp_string strCfgValue, strKeyName;
    mp_int32 iRet;
    if(eType == PASSWORD_SNMP_AUTH)
    {
        strKeyName = CFG_AUTH_PASSWORD;
    }
    else if(eType == PASSWORD_SNMP_PRIVATE)
    {
        strKeyName = CFG_PRIVATE_PASSWOD;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, strKeyName, strCfgValue);
    if (MP_SUCCESS != iRet)
    {
        printf("Get password value from xml config failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get password value from xml config failed.");
        return MP_FALSE;
    }

    mp_string strDecryptPwd = "";
    DecryptStr(strCfgValue, strDecryptPwd);
    iRet = (strDecryptPwd == strOldPwd ? MP_TRUE : MP_FALSE);
    strDecryptPwd.replace(0, strDecryptPwd.length(), "");
    return iRet;
}
/*------------------------------------------------------------ 
Description  :���������
Input        :  strNewPwd--- ���룬eType---��������
Output       :      
Return       :  bRet---������;�������ͬ
                   MP_FALSE---������;����벻ͬ
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckNewPwd(PASSWOD_TYPE eType, const mp_string& strNewPwd)
{
    //����Ƿ����û�����ת
    mp_string strUserName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUserName);
    if(MP_SUCCESS != iRet)
    {
        printf("Get name value from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get name value from xml configuration file failed.");
        return MP_FALSE;
    }
    mp_string strReverseUserName = strUserName;
    std::reverse(strReverseUserName.begin(), strReverseUserName.end());
    if (strUserName == strNewPwd || strReverseUserName == strNewPwd)
    {
        printf("Can't use username or reversed username as password.\n");
        return MP_FALSE;
    }

    //����Ƿ����������ͬ
    mp_bool bRet = MP_FALSE;
    if (eType == PASSWORD_ADMIN)
    {
        bRet = CheckAdminOldPwd(strNewPwd);
    }
    else
    {
        bRet = CheckOtherOldPwd(eType, strNewPwd);
    }
    if (bRet)
    {
        printf("New password is equal to old password.\n");
        return MP_FALSE;
    }

    bRet = CheckCommon(strNewPwd);

    //����Ƿ�ѭ��У�飬snmp©��
    if (bRet && ((eType == PASSWORD_SNMP_AUTH) || (eType == PASSWORD_SNMP_PRIVATE)))
    {
        if (CheckPasswordOverlap(strNewPwd))
        {
            return MP_FALSE;
        }
    }

    return bRet;
}

/*------------------------------------------------------------ 
Description  :����admin����
Input        :  strPwd--- ���� 
Output       :      
Return       :  MP_TRUE---����ɹ�
                   MP_FALSE---����ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::SaveAdminPwd(const mp_string& strPwd)
{
    //��ȡ��ֵ
    mp_string strSalt;
    mp_int32 iRet;
    mp_uint64 randNum;

    iRet = GetRandom(randNum);
    if (iRet != MP_SUCCESS)
    {
        printf("Get Random number failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get Random number failed.");
        return MP_FALSE;
    }
    ostringstream oss;
    oss<<randNum;
    strSalt = oss.str();

    //��ȡhashֵ
    //mp_char outHashHex[SHA256_BLOCK_SIZE + 1] = {0};
    //mp_string strInput(strPwd);
    //strInput = strInput + strSalt;
    //iRet = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, sizeof(outHashHex));
    //strInput.clear();
    //if (iRet != MP_SUCCESS)
    //{
    //    printf("Get sha256 hash value failed.\n");
    //    COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get sha256 hash value failed.");
    //    return MP_FALSE;
    //}

    //��ȡPBKDF2ɢ��ֵ
    mp_string strOut;
    iRet = PBKDF2Hash(strPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
        return iRet;
    }

    //����ֵ���������ļ�
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if(iRet != MP_SUCCESS)
    {
        printf("Set salt value failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set salt value failed.");
        return MP_FALSE;
    }
    
    //�����¼����hashֵ���������ļ�
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, strOut);
    if (iRet != MP_SUCCESS)
    {
        printf("Set PBKDF2 hash value failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set SHA256 hash value failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :������������
Input        :  strPwd--- ���� 
Output       :      
Return       :  MP_TRUE---����ɹ�
                   MP_FALSE---����ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::SaveOtherPwd(PASSWOD_TYPE eType, const mp_string& strPwd)
{
    mp_string strEncrpytPwd;
    EncryptStr(strPwd, strEncrpytPwd);
    
    mp_string strKeyName;
    if(eType == PASSWORD_NGINX_SSL)
    {
    	//CodeDex�󱨣�Dead Code
        return SaveNginxPwd(strEncrpytPwd);
    }
    else if(eType == PASSWORD_SNMP_AUTH)
    {
        strKeyName = CFG_AUTH_PASSWORD;
    }
    else if(eType == PASSWORD_SNMP_PRIVATE)
    {
        strKeyName = CFG_PRIVATE_PASSWOD;
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, strKeyName, strEncrpytPwd);
    if (MP_SUCCESS != iRet)
    {
        printf("Set value into xml config failed.\n");
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Set value into xml config failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}

/*------------------------------------------------------------ 
Description  : �����������nginx��key
Input        : strPwd -- �������nginx��key
Output       : 
Return       : MP_TRUE -- ����ɹ�
               MP_FALSE -- ����ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::SaveNginxPwd(const mp_string& strPwd)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile.c_str()))
    {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE);
        return MP_FALSE;
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    iRet = CMpFile::ReadFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet || vecResult.size() == 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read nginx config file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecResult.size());
        return MP_FALSE;
    }

    mp_string strTmp;
    mp_string::size_type iPosSSLPwd;
    for (mp_uint32 i = 0; i < vecResult.size(); i++)
    {
        strTmp = vecResult[i];
        iPosSSLPwd = strTmp.find(NGINX_SSL_PWD, 0);
        if (iPosSSLPwd != mp_string::npos)
        {
            iPosSSLPwd += strlen(NGINX_SSL_PWD);
            mp_string strInsert = " " + strPwd + STR_SEMICOLON;
            vecResult[i].replace(iPosSSLPwd, strTmp.length()-iPosSSLPwd, strInsert);
            break;
        }
    }

    iRet = CIPCFile::WriteFile(strNginxConfFile, vecResult);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write nginx config file failed, iRet = %d, size of vecResult is %d.",
                iRet, vecResult.size());
        return MP_FALSE;
    }

    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :�������븴�Ӷ�
Input        :  strPwd--- ���� 
Output       : iNum---���֣�iUppercase---��д��ĸ��iLowcase---�����Сд��ĸ��iSpecial---�����ַ�
Return       :  MP_TRUE---����ɹ�
                   MP_FALSE---����ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::CalComplexity(const mp_string& strPwd, mp_int32& iNum, mp_int32& iUppercase, mp_int32& iLowcase, mp_int32& iSpecial)
{
    for (mp_uint32 uindex = 0; uindex < strPwd.length(); uindex++)
    {
        if (string::npos == mp_string(PWD_REX).find(strPwd[uindex]))
        {
            printf("%s", WRONGPWD_HINT);
            return MP_FAILED;
        }
        if (strPwd[uindex] >= '0' && strPwd[uindex] <= '9')
            iNum++;
        if (strPwd[uindex] >= 'A' && strPwd[uindex] <= 'Z')
            iUppercase++;
        if (strPwd[uindex] >= 'a' && strPwd[uindex] <= 'z')
            iLowcase++;
        if(string::npos != mp_string(SPECIAL_REX).find(strPwd[uindex]))
        {
            iSpecial++;
        }
    }

    if (iSpecial == 0)
    {
        printf("%s", WRONGPWD_HINT);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :CheckCommon����
Input        :  strPwd--- ���� 
Output       :  
Return       :  MP_TRUE---�������Ҫ��
                   MP_FALSE---�����
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckCommon(const mp_string& strPwd)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iNum = 0;
    mp_int32 iUppercase = 0;
    mp_int32 iLowcase = 0;
    mp_int32 iSpecial = 0;
    
    //���ȼ��
    if (strPwd.length() < PWD_MIN_LEN || strPwd.length() > PWD_MAX_LEN)
    {
        printf("%s", WRONGPWD_HINT);
        return MP_FALSE;
    }

    iRet = CalComplexity(strPwd, iNum, iUppercase, iLowcase, iSpecial);
    if (MP_SUCCESS != iRet)
    {
        return MP_FALSE;
    }

    mp_int32 iComplex = CalculateComplexity(iNum, iUppercase, iLowcase);
    if (iComplex < 2)
    {
        printf("%s", WRONGPWD_HINT);
        return MP_FALSE;
    }

    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :���㸴�Ӷ�
Input        :  strPwd--- ���� 
Output       :  iNum---���֣�iUppercase---��д��ĸ��iLowcase---�����Сд��ĸ 
Return       :   iComplex---���Ӷ�ֵ
                    
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CPassword::CalculateComplexity(mp_int32 iNumber, mp_int32 iUppercase, mp_int32 iLowcase)
{
    mp_int32 iComplex = 0;
    if (iNumber > 0)
        iComplex++;
    if (iUppercase > 0)
        iComplex++;
    if (iLowcase > 0)
        iComplex++;
    return iComplex;
}
/*------------------------------------------------------------ 
Description  :�������ѭ���ص�
Input        :  strPasswd--- ���� 
Output       :      
Return       :  MP_TRUE--- ����������
                   MP_FALSE--- ��������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_bool CPassword::CheckPasswordOverlap(const mp_string& strPasswd)
{
    mp_uint32 uiIndex = 1;
    mp_uint32 uiMaxLen = strPasswd.length()/2;
    if (0 == uiMaxLen)
    {
        return MP_FALSE;
    }
    for (; uiIndex <= uiMaxLen; uiIndex++)
    {
        //�����ʱ���Ȳ��ܱ��ܳ���������˵��������ѭ��ģʽ��ֱ�ӷ���false
        mp_int32 iRemainNum = strPasswd.length()%uiIndex;
        if (0 != iRemainNum)
        {
            continue;
        }

        mp_string strMeta = strPasswd.substr(0, uiIndex);
        mp_uint32 subIndex = uiIndex; 
        mp_bool bFlag = MP_TRUE; // ȫ��ѭ����ʶ����
        for (; subIndex <= (strPasswd.length() - uiIndex); subIndex += uiIndex)
        {
            mp_string strTmp = strPasswd.substr(subIndex, uiIndex);
            if (strTmp != strMeta)
            {
                bFlag = MP_FALSE;
                break;
            }
        }

        if (bFlag)
        {
            //���ȫ��ƥ�䣬��˵����ѭ���ַ���
            //�����ѭ�������룬�򲻷���Ҫ��(SNMP©��)
            printf("%s", PASSWORD_NOT_SAFE);
            printf("%s", CONTINUE);
            mp_int32 iChoice = getchar();
            if (iChoice != 'y' && iChoice != 'Y')
            {
                //�����ѡ���ǣ����û���Ϊ��������ǲ������õģ�����Ҫ������ѭ����
                return MP_TRUE;
            }
            //����û�ѡ���������������Ϊ�û������˸�ѭ�����ʴ�������жϸ��������Ҫ��
            return MP_FALSE;
        }
    }

    return MP_FALSE;
}

/*------------------------------------------------------------ 
Description  : �����û�����
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
Create By    :
Modification : 
-------------------------------------------------------------*/  
mp_void CPassword::GetInput(mp_string strHint, mp_string& strInput, mp_int32 iInputLen)
{
    mp_char chTmpInput[PWD_LENGTH] = {0};
    mp_uchar ch = (mp_uchar)GETCHAR;
    mp_uint32 iIndex = strHint.length();
    mp_uint32 iLen = iIndex;
    while ((ch & 0xff) != ENTER_SPACE && (ch & 0xff) != NEWLINE_SPACE)
    {
        if ( ((ch & 0xff) == BACK_SPACE) && (iIndex == iLen))
        {
            ch = (mp_uchar)GETCHAR;
            continue;
        }

        if ((ch & 0xff) == BACK_SPACE)
        {
            printf("\b \b");
            iIndex--;
        }
        else
        {
            // ֧�ֲ����Ƴ�����������
            if ((iInputLen > 0) && ((iIndex - iLen) == iInputLen))
            {
                ch = (mp_uchar)GETCHAR;
                continue;
            }
            //CodeDex�󱨣�CSEC_LOOP_ARRAY_CHECKING�������±겻��Խ��
            chTmpInput[iIndex-iLen] = ch;
            printf("%c", ch);
            iIndex++;
        }
        ch = (mp_uchar)GETCHAR;
    }
    chTmpInput[iIndex-iLen] = '\0';
    strInput = chTmpInput;
    printf("\n");
}

/*------------------------------------------------------------ 
Description  :  �����û�
Input        :  
Output       :      
Return       :  
                
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CPassword::LockAdmin()
{
    mp_uint64 time = CMpTime::GetTimeSec();
    ostringstream oss;
    oss<<time;
    mp_string strTime = oss.str();
    vector<mp_string> vecInput;
    vecInput.push_back(strTime);
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    mp_int32 iRet = CIPCFile::WriteFile(strPath, vecInput);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "WriteFile failed, iRet = %d", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "agentcli is locked.");
    }
    return;
}

/*------------------------------------------------------------ 
Description  :  ��ȡ��������ʱ��
Input        :  ��
Output       :      
Return       :  
                
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_uint64 CPassword::GetLockTime()
{
    //CodeDex�󱨣�Type Mismatch:Signed to Unsigned
    vector<mp_string> vecOutput;
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    mp_int32 iRet = CMpFile::ReadFile(strPath, vecOutput);
    if (iRet != MP_SUCCESS || vecOutput.size() == 0)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "iRet = %d, vecOutput.size = %d", iRet, vecOutput.size());
        return 0;
    }

    return atol(vecOutput.front().c_str());
}

/*------------------------------------------------------------ 
Description  :  ���������Ϣ
Input        :  
Output       :      
Return       :  
                
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CPassword::ClearLock()
{
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    if(CMpFile::FileExist(strPath.c_str()))
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "agentcli is unlocked.");
    }
    mp_int32 iRet = CMpFile::DelFile(strPath.c_str());
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "DelFile failed, iRet = %d", iRet);
    }
    return;
}


/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include <sstream>
#include "openssl/sha.h"
#include "common/ConfigXmlParse.h"
#include "common/CryptAlg.h"
#include "common/Types.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/UniqueId.h"
#include "securec.h"
#include "sstream"

//��ʼ����������

/*------------------------------------------------------------
Function Name: IsRunManually
Description  : �ж��Ƿ���������
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_bool IsRunManually()
{
#ifdef WIN32
    return MP_FALSE;
#else
    pid_t my_pid, my_gid;

    my_pid = getpid();
    my_gid = getpgrp();
    if (my_pid == my_gid)
    {
        return MP_TRUE;
    }

    return MP_FALSE;
#endif // WIN32
}

/*------------------------------------------------------------
Function Name: GenSeconds
Description  : ��ȡ�ӽ���0ʱ��ʼ������
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 GenSeconds()
{
    time_t now;
    struct tm beginTime;
    struct tm* pTime = NULL;
    double seconds;

    time(&now);
    pTime = localtime(&now);
    if (NULL == pTime)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Get localtime failed.");
        return MP_FAILED;
    }

    beginTime = *pTime;
    beginTime.tm_hour = 0;
    beginTime.tm_min = 0;
    beginTime.tm_sec = 0;
    beginTime.tm_mon = 0;
    beginTime.tm_mday = 1;

    seconds = difftime(now,mktime(&beginTime));
    printf("%.f\n", seconds);

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GenSALT
Description  : �����������ֵ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 GenSALT()
{
    mp_int32 iRet;
    mp_uint64 randNum;

    // �����������SALTֵ
    iRet = GetRandom(randNum);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "get Random number failed.");
        return iRet;
    }

    //printf("%lu\n", randNum);
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ProcessCrypto
Description  : ��������������мӽ��ܴ���
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 ProcessCrypto(mp_int32 iAlg, mp_string& strFilePath, const mp_string& strSalt)
{
    if (iAlg == CRYPT_SECOND)
    {
        return GenSeconds();
    }

    if (iAlg == CRYPT_SALT)
    {
        return GenSALT();
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = MP_SUCCESS;
    iRet = CIPCFile::ReadFile(strFilePath, vecResult);

    mp_bool bReadFile = (MP_SUCCESS != iRet || vecResult.size() == 0);
    if ( bReadFile )
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read file failed, iRet = %d, size of vecResult is %d.",
            iRet, vecResult.size());
        return iRet;
    }
    //CodeDex��,KLOCWORK.ITER.END.DEREF.MIGHT
    vector<mp_string>::iterator it = vecResult.begin();
    mp_string strInput = *it;
    it++;
    for (; it != vecResult.end(); it++)
    {
        //�����β�Ļ��з���
        strInput = strInput + "\n" + *it;
    }

    mp_string strOutPut;
    if (iAlg == CRYPT_ENCYP_AES)
    {
        iRet = InitCrypt();
        if (iRet != MP_SUCCESS)
        {
            printf("Init crypt failed.\n");
            return iRet;
        }
        EncryptStr(strInput, strOutPut);
        printf("%s\n", strOutPut.c_str());
        //���򼴽��˳����˴����жϷ���ֵ
        (mp_void)FinalizeCrypt();
    }
    else if(iAlg == CRYPT_DECYP_AES)
    {
        iRet = InitCrypt();
        if (iRet != MP_SUCCESS)
        {
            printf("Init crypt failed.\n");
            return iRet;
        }
        DecryptStr(strInput, strOutPut);
        printf("%s\n", strOutPut.c_str());
        //���򼴽��˳����˴����жϷ���ֵ
        (mp_void)FinalizeCrypt();
    }
    else if (iAlg == CRYPT_PBKDF2)
    {
        mp_string strOut;
        iRet = PBKDF2Hash(strInput, strSalt, strOut);
        if (iRet != MP_SUCCESS)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
            return iRet;
        }
        // �ŵ���׼����У����ⲿ�����ȡ
        printf("%s\n", strOut.c_str());
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "algorithm [%d] not found.", iAlg);
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: main����
Description  : �û������������:
               Usage: crypto [-a <0|1|2>] [-i <inputFile>]);
               -a <0|1|2|3>    �����㷨��ö��ֵ���ο�CRYPT_ALG�Ķ��壬��ѡ����
               -i <inputFile>  ������ӽ����ļ���·�����������㷨��aes��shaʱ��ѡ;
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_int32 iOpt = 0;
    mp_int32 iAlg = MP_FAILED;
    mp_string strFilePath;
    mp_string strSalt;
//CodeDex�󱨣�Portability Flaw
#ifdef WIN32
    if (argc > 1)
    {
        for (mp_uint32 i = 1; i < argc; i = i + 2)
        {
            if (i + 1 == argc)
            {
                iAlg = MP_FAILED;
                break;
            }
            mp_char* pszOp = argv[i];
            mp_char* pszOpValue = argv[i + 1];
            if(0 == _stricmp(pszOp, "-a"))
            {
                iAlg = atoi(pszOpValue);
            }
            else if (0 == _stricmp(pszOp, "-i"))
            {
                strFilePath = pszOpValue;
            }
            else if (0 == _stricmp(pszOp, "-s"))
            {
                strSalt = pszOpValue;
            }
            else
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Invalid input param %s.", pszOp);;
            }
        }
    }
#else
    const mp_char* pszOptString = "r:a:i:s:";
    iOpt = getopt(argc, argv, pszOptString);
    while (-1 != iOpt)
    {
        switch (iOpt)
        {
        case 'a':
            iAlg = atoi(optarg);
            break;
        case 'i':
            strFilePath = optarg;
            break;
        case 's':
            strSalt = optarg;
            break;
        default:
            return MP_FAILED;
            break;
        }

        iOpt = getopt(argc, argv, pszOptString);
    }
#endif // WIN32

    if (iAlg == MP_FAILED )
    {
        printf("crypto [-a <Alg_Type>] [-i <inputFile>] [-s <Salt_Value>]\n");
        return MP_FAILED;
    }

    //��ʼ��·��
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (MP_SUCCESS != iRet)
    {
        printf("Init path %s failed.\n", argv[0]);
        return iRet;
    }

    //��ʼ��xml����
    mp_string strXMLConfFilePath = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    iRet = CConfigXmlParser::GetInstance().Init(strXMLConfFilePath);
    if (MP_SUCCESS != iRet)
    {
        printf("Init xml conf file %s failed.\n", AGENT_XML_CONF);
        return iRet;
    }

    //��ʼ����־�ļ�
    mp_string strLogFilePath = CPath::GetInstance().GetLogPath();
    CLogger::GetInstance().Init(mp_string(CRYPTO_LOG_NAME).c_str(), strLogFilePath);

    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Alg = %d, input file path = %s",
                iAlg, strFilePath.c_str());

    return ProcessCrypto(iAlg, strFilePath, strSalt);
}


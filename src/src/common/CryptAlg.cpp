/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include <stdio.h>
#include <string.h>
#include "openssl/aes.h"
#include "openssl/sha.h"
#include "openssl/evp.h"
#include "common/CryptAlg.h"
#include "common/Defines.h"
#include "common/Log.h"
#include "common/Path.h"
#include "KMC.h"
#include "securec.h"

/*------------------------------------------------------------
Function Name:getMinLen
Description  :�ַ�������С�������ƣ���ǰ��Կ��IVҪ����С48
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
size_t getMinLen(size_t len)
{
    return len < 48 ? 48 : (len / 16 + 1) * 16;
}

/*------------------------------------------------------------
Function Name:checkDecryptLen
Description  :��������Կ����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool checkDecryptLen(size_t len)
{
    return ((len >= 48) && (len % 16 == 0));
}

/*------------------------------------------------------------
Function Name:HexStr2ASCII
Description  :hex�ַ���ת����ascii
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 HexStr2ASCII(const mp_uchar *hexStr, mp_uchar *ASCIIStr, size_t hexLen, size_t ASCIILen)
{
    mp_int32 iRet = MP_SUCCESS;
	//CodeDex�󱨣�UNINIT
    mp_char *leftover;
    mp_char byteStr[5];
    if (hexLen % 2 == 1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Length of hexStr is error, [%s]", hexStr);
        return MP_FAILED;
    }

    if (hexStr == NULL || ASCIIStr == NULL)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s", "hexStr == NULL or ASCIIStr == NULL");
        return MP_FAILED;
    }
	//CodeDex�󱨣�Buffer Overflow
    iRet = memset_s(ASCIIStr, ASCIILen, 0, hexLen / 2);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
        return MP_FAILED;
    }
    for (size_t i = 0; i < hexLen / 2; ++i)
    {
        CHECK_FAIL(SNPRINTF_S(byteStr, sizeof(byteStr), sizeof(byteStr) - 1,"0x%c%c", hexStr[2 * i], hexStr[2 * i + 1]));
        ASCIIStr[i] = (mp_uchar)strtol((const mp_char *)byteStr, &leftover, 16);
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:HexStr2ASCII
Description  :asciiת����hex�ַ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 ASCII2HexStr(const mp_uchar *ASCIIStr, mp_uchar* hexStr, size_t len, size_t hexLen)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_uchar* pStr = hexStr;
    if (hexStr == NULL || ASCIIStr == NULL)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "%s", "hexStr == NULL or ASCIIStr == NULL");
        return MP_FAILED;
    }

    iRet = memset_s(hexStr, hexLen, 0, len * 2);
    if (MP_SUCCESS != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
        return MP_FAILED;
    }

    for (size_t n = 0; n < len; ++n)
    {
        CHECK_FAIL(SNPRINTF_S((char *)pStr + 2 * n, 3, 2,"%02X", ASCIIStr[n]));
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:AES_encrypt
Description  :aes���ܴ�����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void AES_encrypt(const mp_char* pBuffer, const mp_int32 bufSize, mp_string& strOut)
{
    AES_KEY key;
    mp_uchar rKey[AES_KEY_LEN] = {0};
    mp_uchar iv[AES_IV_LEN] = {0};
    mp_uchar rKey1[AES_KEY_LEN / 2] = {0};
    mp_uchar rKey2[AES_KEY_LEN / 2] = {0};
    mp_uchar iv1[AES_IV_LEN / 2] = {0};
    mp_uchar iv2[AES_IV_LEN / 2] = {0};
    const mp_int32 AES_KEY_BIT = sizeof(rKey) * 8;

    mp_uchar *in = NULL;
    mp_uchar *out = NULL;
    mp_uchar *outHex = NULL;

    if (pBuffer == NULL || bufSize == 0)
    {
        strOut.clear();
        return;
    }
    
    //Coverity&Fortify��:FORTIFY.Integer_Overflow
    size_t contentLen = getMinLen(bufSize);
	//CodeDex�󱨣�ZERO_LENGTH_ALLOCATIONS
	//CodeDex�󱨣�Memory Leak
    NEW_ARRAY_CATCH(in, mp_uchar, contentLen);
    if (!in)
    {
        return;
    }

    //Coverity&Fortify��:FORTIFY.Integer_Overflow
    NEW_ARRAY_CATCH(out, mp_uchar, contentLen);
    if (!out)
    {
        delete[] in;
        return;
    }

    //Coverity&Fortify��:FORTIFY.Integer_Overflow
    NEW_ARRAY_CATCH(outHex, mp_uchar, contentLen * 2 + 1);
    if (!outHex)
    {
        delete[] in;
        delete[] out;
        return;
    }

    CHECK_AES_DELETE_NOT_OK(memset_s(in, contentLen, 0, contentLen), in, out, outHex);
    CHECK_AES_DELETE_NOT_OK(memcpy_s(in, contentLen, pBuffer, bufSize), in, out, outHex);
    CHECK_AES_DELETE_NOT_OK(memset_s(out, contentLen, 0, contentLen), in, out, outHex);
    CHECK_AES_DELETE_NOT_OK(memset_s(outHex, contentLen * 2 + 1, 0, contentLen * 2 + 1), in, out, outHex);

    // ��װ��Կ
    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)rKeyStr1, rKey1, 
        sizeof(rKeyStr1) - 1, AES_KEY_LEN / 2), in, out, outHex);

    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)rKeyStr2, rKey2, 
        sizeof(rKeyStr2) - 1, AES_KEY_LEN / 2), in, out, outHex);

    CHECK_AES_DELETE_NOT_OK(memcpy_s(rKey, AES_KEY_LEN, rKey1, sizeof(rKey1)), in, out, outHex);
    CHECK_AES_DELETE_NOT_OK(memcpy_s(rKey + sizeof(rKey1), AES_KEY_LEN - sizeof(rKey1), rKey2, 
        sizeof(rKey2)), in, out, outHex);

    // ��װIV����
    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)rIVStr1, iv1, 
        sizeof(rIVStr1) - 1, AES_IV_LEN / 2), in, out, outHex);

    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)rIVStr2, iv2, 
        sizeof(rIVStr2) - 1, AES_IV_LEN / 2), in, out, outHex);

    CHECK_AES_DELETE_NOT_OK(memcpy_s(iv, AES_IV_LEN, iv1, sizeof(iv1)), in, out, outHex);
    CHECK_AES_DELETE_NOT_OK(memcpy_s(iv + sizeof(iv1), AES_IV_LEN - sizeof(iv1), iv2, sizeof(iv2)), in, out, outHex);

    // ��ʼ����
    AES_set_encrypt_key(rKey, AES_KEY_BIT, &key);
    AES_cbc_encrypt(in, out, contentLen, &key, iv, AES_ENCRYPT);

    CHECK_AES_DELETE_NOT_OK(ASCII2HexStr((const mp_uchar *)out, outHex, contentLen, 
        contentLen * sizeof(mp_uchar) * 2 + 1), in, out, outHex);

    strOut = (mp_char*)outHex;

    (mp_void)memset_s(in, contentLen, 0, contentLen);
    (mp_void)memset_s(rKey, AES_KEY_LEN, 0, AES_KEY_LEN);
    (mp_void)memset_s(rKey1, AES_KEY_LEN/2, 0, AES_KEY_LEN/2);
    (mp_void)memset_s(rKey2, AES_KEY_LEN/2, 0, AES_KEY_LEN/2);
    (mp_void)memset_s(iv, AES_IV_LEN, 0, AES_IV_LEN);
    (mp_void)memset_s(iv1, AES_IV_LEN/2, 0, AES_IV_LEN/2);
    (mp_void)memset_s(iv2, AES_IV_LEN/2, 0, AES_IV_LEN/2);
    delete[] in;
    delete[] out;
    delete[] outHex;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Encrypt succ.");
}

/*------------------------------------------------------------
Function Name:AES_decrypt
Description  :aes���ܴ�����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
void AES_decrypt(const mp_char *pBuffer, const mp_int32 bufSize, mp_string& strOut)
{
    AES_KEY key;
    mp_uchar rKey[AES_KEY_LEN] = {0};
    mp_uchar iv[AES_IV_LEN] = {0};
    mp_uchar rKey1[AES_KEY_LEN / 2] = {0};
    mp_uchar rKey2[AES_KEY_LEN / 2] = {0};
    mp_uchar iv1[AES_IV_LEN / 2] = {0};
    mp_uchar iv2[AES_IV_LEN / 2] = {0};
    const mp_int32 AES_KEY_BIT = sizeof(rKey) * 8;

    mp_uchar *in;
    mp_uchar *inHex;
    mp_uchar *out;
    size_t contentLen;

    if (!checkDecryptLen(bufSize))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "The length [%ld] of decrypt string is wrong.", bufSize);
        return;
    }

    if (bufSize >= MAX_SINGED_INTEGER_VALUE)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Buffer size %ld beyond the max length %d.", bufSize,
            MAX_SINGED_INTEGER_VALUE);

        return;
    }

    contentLen = bufSize / 2;
    //CodeDex�󱨣�ZERO_LENGTH_ALLOCATIONS
    //CodeDex�󱨣�HW_NSCC_CPP_SECFUNPARAMCHECKER
    //Coverity&Fortify��:FORTIFY.Integer_Overflow
    //CodeDex�󱨣�Memory Leak
    NEW_ARRAY_CATCH(in, mp_uchar, contentLen);
    if (!in)
    {
        return;
    }
    
    //Coverity&Fortify��:FORTIFY.Integer_Overflow
    NEW_ARRAY_CATCH(inHex, mp_uchar, bufSize + 1);
    if (!inHex)
    {
        delete[] in;
        return;
    }

    //Coverity&Fortify��:FORTIFY.Integer_Overflow
    NEW_ARRAY_CATCH(out, mp_uchar, contentLen);
    if (!out)
    {
        delete[] in;
        delete[] inHex;
        return;
    }

    CHECK_AES_DELETE_NOT_OK(memset_s(in, contentLen, 0, contentLen), in, inHex, out);
    CHECK_AES_DELETE_NOT_OK(memset_s(inHex, bufSize + 1, 0, bufSize + 1), in, inHex, out);
    CHECK_AES_DELETE_NOT_OK(memset_s(out, contentLen, 0, contentLen), in, inHex, out);
	 //CodeDex�󱨣�HW_NSCC_CPP_SECFUNPARAMCHECKER
    CHECK_AES_DELETE_NOT_OK(memcpy_s(inHex, bufSize + 1, pBuffer, bufSize), in, inHex, out);
    //Coverity&Fortify��:FORTIFY.Buffer_Overflow
    //HexStr2ASCII�����ڱ���ASCIIStr�������������
    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)inHex, in, bufSize, contentLen), in, inHex, out);

    // ��װ��Կ
    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)rKeyStr1, rKey1, 
        sizeof(rKeyStr1) - 1, AES_KEY_LEN / 2), in, inHex, out);
    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)rKeyStr2, rKey2, 
        sizeof(rKeyStr2) - 1, AES_KEY_LEN / 2), in, inHex, out);

    CHECK_AES_DELETE_NOT_OK(memcpy_s(rKey, AES_KEY_LEN, rKey1, sizeof(rKey1)), in, inHex, out);
    CHECK_AES_DELETE_NOT_OK(memcpy_s(rKey + sizeof(rKey1), AES_KEY_LEN - sizeof(rKey1), rKey2, 
        sizeof(rKey2)), in, inHex, out);

    // ��װIV����
    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)rIVStr1, iv1, 
        sizeof(rIVStr1) - 1, AES_IV_LEN / 2), in, inHex, out);
    CHECK_AES_DELETE_NOT_OK(HexStr2ASCII((const unsigned char *)rIVStr2, iv2, 
        sizeof(rIVStr2) - 1, AES_IV_LEN / 2), in, inHex, out);

    CHECK_AES_DELETE_NOT_OK(memcpy_s(iv, AES_IV_LEN, iv1, sizeof(iv1)), in, inHex, out);
    CHECK_AES_DELETE_NOT_OK(memcpy_s(iv + sizeof(iv1), AES_IV_LEN - sizeof(iv1), iv2, sizeof(iv2)), in, inHex, out);

    // ��ʼ����
    AES_set_decrypt_key(rKey, AES_KEY_BIT, &key);
    AES_cbc_encrypt(in, out, contentLen, &key, iv, AES_DECRYPT);

    strOut = (mp_char*)out;
    (mp_void)memset_s(out, contentLen, 0, contentLen);
    (mp_void)memset_s(rKey, AES_KEY_LEN, 0, AES_KEY_LEN);
    (mp_void)memset_s(rKey1, AES_KEY_LEN/2, 0, AES_KEY_LEN/2);
    (mp_void)memset_s(rKey2, AES_KEY_LEN/2, 0, AES_KEY_LEN/2);
    (mp_void)memset_s(iv, AES_IV_LEN, 0, AES_IV_LEN);
    (mp_void)memset_s(iv1, AES_IV_LEN/2, 0, AES_IV_LEN/2);
    (mp_void)memset_s(iv2, AES_IV_LEN/2, 0, AES_IV_LEN/2);
    delete[] in;
    delete[] inHex;
    delete[] out;
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Decrypt succ.");
}

/*------------------------------------------------------------
Function Name: GetSha256Hash
Description  : ����sha256У����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 GetSha256Hash(const mp_char* buff, const mp_int32 len, mp_char* hashHex, mp_int32 hexLen)
{
    mp_uchar acHash[SHA256_DIGEST_LENGTH] = {0};
    SHA256_CTX c;
	//CodeDex�󱨣�UNINIT
    SHA256_Init(&c);
    SHA256_Update(&c, buff, len);
    SHA256_Final(acHash, &c);
    for(mp_uint32 i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        if(MP_FAILED == sprintf_s(hashHex + 2 * i, hexLen - 2 * i, "%02x", acHash[i]))
        {
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}


/*------------------------------------------------------------
Function Name:GetRandom
Description  :��ȡ�����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 GetRandom(mp_uint64 &num)
{
#ifdef WIN32
    HCRYPTPROV hCryptProv;
    mp_uint64 lastCode;
    if (!::CryptAcquireContextW(&hCryptProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
    {
        lastCode = GetLastError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CryptAcquireContextW exec failed, errorcode [%lu].", lastCode);
        return MP_FAILED;
    }

    if (!CryptGenRandom(hCryptProv, sizeof(mp_uint64), (BYTE *)&num))
    {
        lastCode = GetLastError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CryptGenRandom exec failed, errorcode [%lu].", lastCode);
        ::CryptReleaseContext(hCryptProv, 0);
        return MP_FAILED;
    }

    if (!::CryptReleaseContext(hCryptProv, 0))
    {
        lastCode = GetLastError();
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "CryptReleaseContext exec failed, errorcode [%lu].", lastCode);
        return MP_FAILED;
    }
#else
    mp_int32 fd;
    fd = open("/dev/random", O_RDONLY);
    if (fd == -1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "open /dev/random failed[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    if (read(fd, &num, sizeof(mp_uint64)) == -1)
    {
        close(fd);
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "read file failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }
    close(fd);
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name:PBKDF2Hash
Description  :��ȡPBKDF2ɢ��ֵ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 PBKDF2Hash(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText)
{
    mp_uchar pUnChSalt[PBKDF_SALT_MAX_LEN + 1] = {0x0};
    mp_int32 saltLen = (mp_int32)strSalt.length();
    if (saltLen > PBKDF_SALT_MAX_LEN)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "strSalt is too long, len = %d", saltLen);
        return MP_FAILED;
    }
    for (mp_uint32 i = 0; i < saltLen; ++i)
    {
        pUnChSalt[i] = strSalt[i];
    }  

    //CodeDex�󱨣�Weak Cryptographic Hash:Insecure PBE Iteration Count����������5000,���ϰ�ȫҪ��
    //Call PKCS5_PBKDF2_HMAC
    mp_uchar out[PBKDF_KEY_LEN] = {0x0};
    mp_int32 iRet = PKCS5_PBKDF2_HMAC(strPlainText.c_str(), static_cast<int>(strPlainText.length()), pUnChSalt, saltLen, 
                PBKDF_ITER_TIMES, EVP_sha256(), PBKDF_KEY_LEN, out);
   
    if (iRet != 1)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "call PKCS5_PBKDF2_HMAC failed, ret = %d", iRet);
        return MP_FAILED;
    }
   
    //Convert out to ciphertext, note zero maybe in 'out' param
    //SecurityHelper::hexToStr(out, PBKDF_KEY_LEN, ciphertext);
    mp_uchar base64_array[CIPHER_BUFF_LEN] = {0x0};
    EVP_EncodeBlock(base64_array, out, PBKDF_KEY_LEN);
    strCipherText = mp_string((char *)base64_array);
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ��ʼ���������
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 InitCrypt()
{
    mp_string strKmcStoreFile = CPath::GetInstance().GetConfFilePath(KMC_STORE_FILE);
#ifdef WIN32
    mp_string strKmcErrorLog = CPath::GetInstance().GetLogPath() + "\\" + KMC_ERROR_LOG_FILE;
#else
    mp_string strKmcErrorLog = CPath::GetInstance().GetLogPath() + "/" + KMC_ERROR_LOG_FILE;
#endif
    
    mp_int32 iRet = KMC_init((char*)(strKmcStoreFile.c_str()), (char*)(strKmcErrorLog.c_str()));
    if(iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "KMC initialize failed.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : �ͷż��������Դ
Input        : 
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 FinalizeCrypt()
{
    return MP_SUCCESS;
}

/*------------------------------------------------------------ 
Description  : ���ü�������Ķ�ʱ��������
Input        : 
Output       : 
Return       : 
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void CallCryptTimer()
{
    return;
}

/*------------------------------------------------------------ 
Description  : �����ַ���
Input        : inStr -- �����ַ���
Output       : outStr -- ����ַ���
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void EncryptStr(const mp_string &inStr, mp_string &outStr)
{
    mp_int32 ret = KMC_Encrypt_Str(inStr, outStr);
    if (ret != MP_SUCCESS)
    {
        outStr = "";
        return;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "EncryptStr succ.");
}

/*------------------------------------------------------------ 
Description  : �����ַ���
Input        : inStr -- �����ַ���
Output       : outStr -- ����ַ���
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_void DecryptStr(const mp_string &inStr, mp_string &outStr)
{
    mp_int32 ret = KMC_Decrypt_Str(inStr, outStr);
    if (ret != MP_SUCCESS)
    {
        outStr = "";
        AES_decrypt(inStr.c_str(), inStr.length(), outStr);
        return;
    }
    COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "DecryptStr succ.");
}

/*------------------------------------------------------------ 
Description  : �����ļ�HMAC
Input        : filePath -- �ļ�·��
Output       : fileHMAC -- �ļ�HMACֵ
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 ComputeHMAC(const mp_string &filePath, mp_string &fileHMAC)
{
    mp_int32 ret = KMC_Gen_File_HMAC(filePath, fileHMAC);
    if (ret == MP_SUCCESS)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "ComputeHMAC succ.");
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ComputeHMAC failed.");
    }
    return ret;
}


/*------------------------------------------------------------ 
Description  : У���ļ�HMAC
Input        : filePath -- �ļ�·��
               fileHMAC -- �ļ�HMACֵ
Output       : 
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ�ܣ������ض�������
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 VerifyHMAC(const mp_string & filePath, const mp_string & fileHMAC)
{
    mp_int32 ret = KMC_Verify_File_HMAC(filePath, fileHMAC);
    if (ret == MP_SUCCESS)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "VerifyHMAC succ.");
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "VerifyHMAC failed.");
    }
    return ret;
}

/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

/******************************************************************************

Copyright (C), 2001-2019, Huawei Tech. Co., Ltd.

******************************************************************************
File Name     : CMessageBody.h
Version       : Initial Draft
Author        : 
Created       : 2015/02/02
Last Modified :
Description   : ������Ϣ�壬��Ϊagent�ڲ�ͨ����Ϣ
History       :
1.Date        :
Author      :
Modification:
******************************************************************************/
#ifndef _AGENT_MESSAGE_PROCESS_H_
#define _AGENT_MESSAGE_PROCESS_H_

#include <memory>
#include <map>

#include "common/Types.h"
#include "common/String.h"
#include "jsoncpp/include/json/value.h"
#include "jsoncpp/include/json/json.h"
#include "rest/HttpCGI.h"
#include "rest/HttpStatus.h"

enum BODY_ACTION_TYPE
{
    BODY_DECODE_JSON = 0,
    BODY_RAW_BUF = 1
};

#define REST_TAG "/agent/"
#define MSG_BLK_SIZE 2048
#define MAX_MSG_CACHE_SIZE 204800
#define MAX_ATTCHMENT_SIZE 20*1024*1024
#define ONCE_SEND_MSG_SIZE 32*1024

#define BOUNDARY_TAG "multipart/form-data; boundary="
#define CTN_DISP "Content-Disposition"
#define CTN_TYPE "Content-Type"
#define CTN_DISP_NAME "Content-Disposition: form-data; name=\""
#define CTN_TYPE_COMMA "Content-Type: "
#define FILE_NAME "; filename=\""

//�ڲ���Ϣ����飬����������ACE_Message_Block
class CMessage_Block
{
public:
    CMessage_Block();

    CMessage_Block(size_t n);

    ~CMessage_Block();

    CMessage_Block(CMessage_Block& msg_blk)
    {
    }

    CMessage_Block& operator=(CMessage_Block& msg_blk)
    {
        return *this;
    }

    /// Get the write I/O buffer pointer.
    mp_char *GetWritePtr();

     // Get the read I/O buffer pointer.
    mp_char *GetReadPtr();

    /// Set the I/O buffer ahead @a n bytes.  This is used to compute
    /// the <length> of a message.
    mp_void AddLength(mp_uint32 n);

    /// Get the length of the message
    mp_uint32 GetLength() const;


    /// Get the number of bytes in the top-level Message_Block (i.e.,
    /// does not consider the bytes in chained Message_Blocks).
    mp_uint32 GetSize() const;

    /**
   * Set the number of bytes in the top-level Message_Block,
   * reallocating space if necessary.  However, the @c rd_ptr_ and
   * @c wr_ptr_ remain at the original offsets into the buffer, even if
   * it is reallocated.  Returns 0 if successful, else -1.
   */
  mp_int32 Resize(mp_uint32 length);

private:
    mp_char* m_data_block;
    mp_char* m_wirte_ptr;
    mp_uint32 m_size;
    mp_uint32 m_length;
};

//HTTP������URL��صķ�װ
class CRequestURL
{
public:
    CRequestURL()
    {

    }
    ~CRequestURL()
    {

    }
    CRequestURL(CRequestURL& reqUrl)
    {
        m_procURL = reqUrl.m_procURL;
        m_oriURL = reqUrl.m_oriURL;
        m_queryParam = reqUrl.m_queryParam;
        m_id = reqUrl.m_id;
    }
    CRequestURL& operator=(CRequestURL& reqUrl)
    {
        m_procURL = reqUrl.m_procURL;
        m_oriURL = reqUrl.m_oriURL;
        m_queryParam = reqUrl.m_queryParam;
        m_id = reqUrl.m_id;
        return *this;
    }
    mp_string GetProcURL()
    {
        return m_procURL;
    }
    mp_string GetOriURL()
    {
        return m_oriURL;
    }
    mp_string GetServiceName();
    mp_string GetCutURL(mp_int32 index);
    mp_string GetID()
    {
        return m_id;
    }

    mp_void SetProcURL(mp_string strProcURL)
    {
        m_procURL = strProcURL;
    }
    mp_void SetOriURL(mp_string strOriURL)
    {
        m_oriURL = CMpString::ToLower((mp_char*)strOriURL.c_str());
        ParseURL();
    }
    mp_void SetQueryParam(mp_string strQueryParam, mp_bool bUTF8ToANSI = MP_TRUE);
    map<mp_string, mp_string>& GetQueryParam()
    {
        return m_queryParam;
    }
    mp_string GetSpecialQueryParam(const mp_string& strKey)
    {
        return m_queryParam.find(strKey) != m_queryParam.end() ? m_queryParam[strKey] : "";
    }
private:
    mp_string m_procURL;  //����������URL���͸�appע�����Ϣ������Ӧ
    mp_string m_oriURL;   //ԭʼURL
    mp_string m_id;       //Ԥ�������ԭʼURL�а���idʱʹ�ô˳�Ա
    map<mp_string, mp_string> m_queryParam;  //URL�еĲ�ѯ����
    mp_void ParseURL();
};

//HTTP��������Ϣ��ķ�װ��֧��json���ļ�����
class CRequestMsgBody
{
public:
    CRequestMsgBody() : m_msgLen(0), m_msgBodyType(BODY_DECODE_JSON)
    {

    }

    ~CRequestMsgBody()
    {
        //m_totalCacheBodySize = m_totalCacheBodySize - m_msgLen;
    }

    CRequestMsgBody(CRequestMsgBody& msgBody)
    {
        m_msgBodyType = msgBody.m_msgBodyType;
        m_msgLen = msgBody.m_msgLen;
        m_msgJsonData = msgBody.m_msgJsonData;
        m_raw_msg = msgBody.m_raw_msg;
    }

    CRequestMsgBody& operator=(CRequestMsgBody& msgBody)
    {
        m_msgBodyType = msgBody.m_msgBodyType;
        m_msgLen = msgBody.m_msgLen;
        m_msgJsonData = msgBody.m_msgJsonData;
        m_raw_msg = msgBody.m_raw_msg;
        return *this;
    }

    mp_int32 ReadWait(CHttpRequest &req);
    mp_int32 GetValue(const mp_string &name, mp_string &val);
    mp_int32 GetValueThrow(const mp_string & name, mp_string & val);
    mp_string JsonValueToString(const Json::Value &v);
    const Json::Value &GetJsonValueRef() const
    {
        return m_msgJsonData;
    }
    const Json::Value GetJsonValue() const
    {
        return m_msgJsonData;
    }
    mp_void SetJsonValue(Json::Value JsonValue)
    {
        m_msgJsonData = JsonValue;
    }
    mp_bool GetOriMsg(mp_char* &buf, mp_uint32&len);
    mp_void SetMsgBodyType(BODY_ACTION_TYPE bodyType)
    {
        m_msgBodyType = bodyType;
    }

public:
    Json::Value m_msgJsonData; //��Ž����õ�json����

private:
    BODY_ACTION_TYPE m_msgBodyType;  //��Ϣ������
    mp_uint32 m_msgLen;  //��Ϣ����
    //static unsigned long m_totalCacheBodySize;  //�ڴ����������ĳ���
    auto_ptr<CMessage_Block> m_raw_msg; //http������Ϣ�е�ԭʼ����

private:
    mp_int32 ReadData(CHttpRequest &req, auto_ptr<CMessage_Block> & msg);
    mp_int32 ParseJson(auto_ptr<CMessage_Block> &msg);

//������ز�����Ŀǰ������ʱ�ò�������ע�͵�
//private:
    //struct UploadFileAddr
    //{
    //    mp_string name;
    //    mp_string filename;
    //    mp_string type;
    //    const mp_char *buf;
    //    mp_uint32 len;
    //    UploadFileAddr():buf(NULL),len(0){}
    //};
    //mp_bool GetUploadFileAddr(CHttpRequest &req, std::vector<UploadFileAddr> & files );
    //struct UploadFileInfo
    //{
    //    mp_string name;
    //    mp_string filename;
    //    mp_string type;
    //    mp_uint32 begin;
    //    mp_uint32 end;
    //    UploadFileInfo():begin(0),end(0){}
    //};
    //mp_bool GetUploadFileInfo(CHttpRequest &req, std::vector<UploadFileInfo> & files );
    //mp_bool GetBoundary(CHttpRequest &req, mp_string &bnd);
    //mp_bool CheckBegin( const mp_char *buf,mp_uint32 len,mp_uint32 &cur_pos,const mp_string &sepStr );
    //mp_void GetContentDisposition(const mp_char *beg,mp_uint32 len,mp_uint32 &i,UploadFileInfo &fi);
    //mp_void GetContentDispositionSlash(const mp_char *beg,mp_uint32 len, mp_uint32 &i,UploadFileInfo &fi, std::size_t fnl);
    //mp_void GetContentType(const mp_char *beg, mp_uint32 len, mp_uint32 &i,UploadFileInfo &fi);
    //mp_bool CheckOption( const mp_char *buf,mp_uint32 len,mp_uint32 &cur_pos,UploadFileInfo &fi );
    //mp_bool CheckEnd( const mp_char *buf,mp_uint32 len,mp_uint32 &cur_pos,const mp_string &sepStr,UploadFileInfo &fi );
    //mp_bool SetFileEnd( const mp_char *buf,mp_uint32 i,mp_uint32 &cur_pos,std::size_t sl,UploadFileInfo &fi );
};

//������Ϣ��ķ�װ�����ڸ�app�õ�http������Ϣ������
class CRequestMsg
{
public:
    CRequestMsg(FCGX_Request* pFcgiReq);
    CRequestMsg()
    {

    }

    CRequestMsg(CRequestMsg& rsqMsg)
    {
        m_url = rsqMsg.m_url;
        m_msgBody = rsqMsg.m_msgBody;
        m_httpReq = rsqMsg.m_httpReq;
    }

    CRequestMsg& operator=(CRequestMsg& rsqMsg)
    {
        m_url = rsqMsg.m_url;
        m_msgBody = rsqMsg.m_msgBody;
        m_httpReq = rsqMsg.m_httpReq;
        return *this;
    }

    ~CRequestMsg()
    {

    }

    CRequestURL& GetURL()
    {
        return m_url;
    }

   CRequestMsgBody& GetMsgBody()
    {
        return m_msgBody;
    }

    CHttpRequest& GetHttpReq()
    {
        return m_httpReq;
    }

    mp_void SetProcURL(mp_string strRUL)
    {
        m_url.SetProcURL(strRUL);
    }

    mp_void SetJsonData(Json::Value jsonValue)
    {
        m_msgBody.SetJsonValue(jsonValue);
    }

    mp_int32 Parse();

public:
    CRequestURL m_url;    //URL����
    CRequestMsgBody m_msgBody; //��Ϣ�����
    CHttpRequest m_httpReq; //http�������
};

//��Ӧ��Ϣ��ķ�װ�����ڸ�app��Ӧhttp������Ϣ������
class CResponseMsg
{
public:
    CResponseMsg(FCGX_Request* pFcgiReq):m_httpRsp(pFcgiReq)
    {
        m_httpType = RSP_JSON_TYPE;
        m_iHttpStatus = SC_OK;
        m_lRetCode = MP_SUCCESS;
        m_bInternalMsg = MP_FALSE;
    }

    CResponseMsg()
    {
        m_httpType = RSP_JSON_TYPE;
        m_iHttpStatus = SC_OK;
        m_bInternalMsg = MP_TRUE;
        m_lRetCode = MP_SUCCESS;
    }

    ~CResponseMsg()
    {

    }

    enum enRspType
    {
        RSP_JSON_TYPE = 0,   //Json��ʽ������Ϣ
        RSP_ATTACHMENT_TYPE  //����������Ϣ
    };

    mp_bool IsInternalMsg()
    {
        return m_bInternalMsg;
    }

    enRspType GetHttpType()
    {
        return m_httpType;
    }

    mp_void SetHttpType(enRspType httpType)
    {
        m_httpType = httpType;
    }

    mp_int64 GetRetCode()
    {
        return m_lRetCode;
    }

    mp_void SetRetCode(mp_int64 lRet)
    {
        m_lRetCode = lRet;
    }

    mp_void SetHttpStatus(mp_int32 iStatus)
    {
        m_iHttpStatus = iStatus;
    }

    Json::Value& GetJsonValueRef()
    {
        return m_msgJsonData;
    }

    Json::Value GetJsonValue()
    {
        return m_msgJsonData;
    }

    mp_int32 Send();

public:
    Json::Value m_msgJsonData; //�����Ӧ��json����

private:
    enRspType m_httpType; //http��������
    mp_int64 m_lRetCode;  //������
    mp_int32 m_iHttpStatus;
    CHttpResponse m_httpRsp;  //http��Ӧ����
    mp_bool m_bInternalMsg;   //�Ƿ����ڲ�������Ϣ��Ĭ����false���ڴ����ڲ�����ʱ��������Ϊtrue

private:
    mp_void PackageReponse(Json::Value &root);
    mp_int32 SendJson();
    mp_int32 SendAttchment();
    mp_void SendFailedRsp();
};

//URL�������صĹ�����
#define RFC_LENGTH 256
class CUrlUtils
{
public:
    static mp_void Initrfc3986tb();
    static mp_int32 Urlencode2(mp_char *enc, mp_int32 destlen, const mp_char *src);
    static mp_int32 Urldecode2(mp_char *indec, const mp_char *s, mp_int32 len);
    static mp_string GetBetweenValue(const mp_char*eqp, const mp_char* p);
private:
    static mp_int32 IsHex(mp_int32 x)
    {
        return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F');
    }
    static mp_void Initrfc3986tb(mp_char* rfc, mp_int32 len)
    {
        for (mp_int32 i = 0; i < len; i++)
        {
            rfc[i] = isalnum(i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
        }
    }
    static mp_bool GetNum(const mp_char *s, mp_uint32 len, mp_uint32 b, mp_uint64 &ret);
};

class CJsonUtils
{
public:
    static mp_int32 GetJsonString(const Json::Value& jsValue, mp_string strKey, mp_string& strValue);
    static mp_int32 GetJsonInt32(const Json::Value& jsValue, mp_string strKey, mp_int32& iValue);
    static mp_int32 GetJsonInt64(const Json::Value& jsValue, mp_string strKey, mp_int64& lValue);
    static mp_int32 GetJsonArrayInt32(const Json::Value& jsValue, mp_string strKey, vector<mp_int32>& vecValue);
    static mp_int32 GetJsonArrayInt64(const Json::Value& jsValue, mp_string strKey, vector<mp_int64>& vecValue);
    static mp_int32 GetJsonArrayString(const Json::Value& jsValue, mp_string strKey, vector<mp_string>& vecValue);
    static mp_int32 GetJsonArrayJson(const Json::Value& jsValue, mp_string strKey, vector<Json::Value>& vecValue);
    static mp_int32 GetArrayInt32(const Json::Value& jsValue, vector<mp_int32>& vecValue);
    static mp_int32 GetArrayInt64(const Json::Value& jsValue, vector<mp_int64>& vecValue);
    static mp_int32 GetArrayString(const Json::Value& jsValue, vector<mp_string>& vecValue);
    static mp_int32 GetArrayJson(const Json::Value& jsValue, vector<Json::Value>& vecValue);
};

//���ڼ�鷵�ؽ�������岿�ֺ궨��
#define GET_JSON_STRING(jsValue, strKey, strValue) \
     CHECK_FAIL_EX(CJsonUtils::GetJsonString(jsValue, strKey, strValue))
#define GET_JSON_INT32(jsValue, strKey, iValue) \
     CHECK_FAIL_EX(CJsonUtils::GetJsonInt32(jsValue, strKey, iValue))
#define GET_JSON_INT64(jsValue, strKey, lValue) \
     CHECK_FAIL_EX(CJsonUtils::GetJsonInt64(jsValue, strKey, lValue))
#define GET_JSON_ARRAY_STRING(jsValue, strKey, vecValue) \
     CHECK_FAIL_EX(CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue))
#define GET_JSON_ARRAY_INT32(jsValue, strKey, vecValue) \
     CHECK_FAIL_EX(CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue))
#define GET_JSON_ARRAY_INT64(jsValue, strKey, vecValue) \
     CHECK_FAIL_EX(CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue))   
#define GET_JSON_ARRAY_JSON(jsValue, strKey, vecValue) \
     CHECK_FAIL_EX(CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue)) 
#define GET_ARRAY_STRING(jsValue, vecValue) \
     CHECK_FAIL_EX(CJsonUtils::GetArrayString(jsValue, vecValue))
#define GET_ARRAY_INT32(jsValue, vecValue) \
     CHECK_FAIL_EX(CJsonUtils::GetArrayInt32(jsValue, vecValue))
#define GET_ARRAY_INT64(jsValue, vecValue) \
     CHECK_FAIL_EX(CJsonUtils::GetArrayInt64(jsValue, vecValue))
#define GET_ARRAY_JSON(jsValue, vecValue) \
         CHECK_FAIL_EX(CJsonUtils::GetArrayJson(jsValue, vecValue))


#define GET_JSON_STRING_OPTION(jsValue, strKey, strValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonString(jsValue, strKey, strValue)); \
    }  \

#define GET_JSON_INT32_OPTION(jsValue, strKey, iValue) \
    if (jsValue.isMember(strKey)) \
    {  \
       CHECK_FAIL_EX(CJsonUtils::GetJsonInt32(jsValue, strKey, iValue)); \
    }  \

#define GET_JSON_INT64_OPTION(jsValue, strKey, lValue) \
    if (jsValue.isMember(strKey)) \
    {  \
       CHECK_FAIL_EX(CJsonUtils::GetJsonInt64(jsValue, strKey, lValue)); \
    }  \

#define GET_JSON_ARRAY_STRING_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isMember(strKey)) \
    {  \
        CHECK_FAIL_EX(CJsonUtils::GetJsonArrayString(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_INT32_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isMember(strKey)) \
    {  \
       CHECK_FAIL_EX(CJsonUtils::GetJsonArrayInt32(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_INT64_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isMember(strKey)) \
    {  \
       CHECK_FAIL_EX(CJsonUtils::GetJsonArrayInt64(jsValue, strKey, vecValue)); \
    }  \

#define GET_JSON_ARRAY_JSON_OPTION(jsValue, strKey, vecValue) \
    if (jsValue.isMember(strKey)) \
    {  \
       CHECK_FAIL_EX(CJsonUtils::GetJsonArrayJson(jsValue, strKey, vecValue)); \
    }  \

//���jsonԪ���Ƿ����
#define CHECK_JSON_VALUE(jsValue, strKey) \
    if (!jsValue.isMember(strKey))\
    { \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Key \"%s\" is not exist.", strKey); \
        return ERROR_COMMON_INVALID_PARAM; \
    } \

//���json�����Ƿ�Ϊ����
#define CHECK_JSON_ARRAY(jsValue) \
    if (!jsValue.isArray()) \
    { \
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Message body is not array."); \
        return ERROR_COMMON_INVALID_PARAM; \
    } \
    if(jsValue.size() == 0) \
    {\
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Size of Json array is 0."); \
        return ERROR_COMMON_INVALID_PARAM; \
    }\

#endif //_AGENT_MESSAGE_PROCESS_H_


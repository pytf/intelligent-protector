/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#include <sstream>
#include "common/Defines.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "rest/Interfaces.h"
#include "rest/MessageProcess.h"
#include <fstream>

/*------------------------------------------------------------
Function Name: CMessage_Block
Description  : CMessage_Block���캯��
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
CMessage_Block::CMessage_Block()
{
    m_wirte_ptr = NULL;
    m_data_block = NULL;
    m_length = 0;
    m_size = 0;
}

/*------------------------------------------------------------
Function Name: CMessage_Block
Description  : CMessage_Block���캯����n��ʾҪ����Ŀռ��С
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
CMessage_Block::CMessage_Block(size_t n)
{
    //CodeDex�󱨣�ZERO_LENGTH_ALLOCATIONS
    NEW_ARRAY_CATCH(m_data_block, mp_char, n);
    if (NULL == m_data_block)
    {
        m_size = 0;
    }
    else
    {
        m_size = n;
    }
    m_length = 0;
    m_wirte_ptr = m_data_block;
}

/*------------------------------------------------------------
Function Name: getWritePtr
Description  : ��ȡд�ռ�ĵ�ַ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_char *CMessage_Block::GetWritePtr()
{
    return m_wirte_ptr;
}

/*------------------------------------------------------------
Function Name: getReadPtr
Description  : ��ȡ���ռ�ĵ�ַ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_char *CMessage_Block::GetReadPtr()
{
    return m_data_block;
}

/*------------------------------------------------------------
Function Name: addLength
Description  : ���ڼ���д�ռ�ĵ�ַ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CMessage_Block::AddLength (mp_uint32 n)
{
    m_length += n;
    if (m_wirte_ptr)
    {
        m_wirte_ptr += n;
    }
}

/*------------------------------------------------------------
Function Name: getLength
Description  : ��ȡ��д��ռ�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_uint32 CMessage_Block::GetLength () const
{
    return m_length;
}

/*------------------------------------------------------------
Function Name: getSize
Description  : ��ȡ�ѷ���ռ�ĳ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_uint32 CMessage_Block::GetSize () const
{
    return m_size;
}

/*------------------------------------------------------------
Function Name: resize
Description  : ��̬�����ռ�Ĵ�С
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CMessage_Block::Resize(mp_uint32 length)
{
    if (length <= m_size)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "input length %d is smaller than size %d", length, m_size);
        return MP_FAILED;
    }

    mp_char* data_block;
	//CodeDex�󱨣�Memory Leak
    NEW_ARRAY_CATCH(data_block, mp_char, length);
    if (!data_block)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "data_block is 0.");
        return MP_FAILED;
    }

    mp_int32 iRet = memset_s(data_block, length, 0, length);
    if (EOK != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] data_block;
        return MP_FAILED;
    }

    if (!m_data_block)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "m_data_block is 0.");
        delete[] data_block;
        return MP_FAILED;
    }

    iRet = memcpy_s(data_block, length, m_data_block, m_size);
    if (EOK != iRet)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Call memcpy_s failed, ret %d.", iRet);
        delete[] data_block;
        return MP_FAILED;
    }

    delete[] m_data_block;
    m_data_block = data_block;
    m_wirte_ptr = m_data_block + m_length;
    m_size = length;

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: ~CMessage_Block
Description  : ���������������ͷŷ�����ڴ�
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
CMessage_Block::~CMessage_Block()
{
    delete[] m_data_block;
    m_data_block = NULL;
    m_wirte_ptr = NULL;
    m_size = 0;
    m_length = 0;
}

/*------------------------------------------------------------
Function Name: ParseURL
Description  : ͨ��ԭʼ�ַ������ɸ�app�ܴ�����ַ���
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CRequestURL::ParseURL()
{
    //���id����urlĩβ���ڴ����Ӵ���
    // todo
    //��ȡ��������ģ����Ҫ��URL�ַ���
    mp_string::size_type pos = m_oriURL.find(REST_TAG);
    if (pos == mp_string::npos)
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "can not find \"%s\" in url \"%s\".", REST_TAG, m_oriURL.c_str());
        m_procURL = "";
    }
    else
    {
        m_procURL = "/" + m_oriURL.substr(pos + strlen(REST_TAG));
    }
}

/*------------------------------------------------------------
Function Name: SetQueryParam
Description  : ���������ַ���Ϊmap,bUTF8ToANSIĬ��Ϊfalse��������utf8��ansiת��
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_void CRequestURL::SetQueryParam(mp_string strQueryParam, mp_bool bUTF8ToANSI)
{
    const mp_char* nm = strQueryParam.c_str();
    const mp_char* eqp = 0;
    const mp_char* p = strQueryParam.c_str();
    for (; *p != 0; ++p)
    {
        if (*p == '=')
        {
            eqp = p;
        }
        else if (*p == '&' /*|| *p == ';'*/)
        {
            if (eqp != NULL)
            {
                mp_string name(nm, eqp - nm);
                m_queryParam[name] = CUrlUtils::GetBetweenValue(eqp, p);
            }
            nm = p + 1;
            eqp = NULL;
        }
    }

    if (eqp != NULL)
    {
        mp_string name(nm, eqp - nm);
        m_queryParam[name] = CUrlUtils::GetBetweenValue(eqp, p);
    }

    for (map<mp_string, mp_string>::iterator it = m_queryParam.begin(); it != m_queryParam.end(); it++)
    {
#ifdef WIN32
        if (bUTF8ToANSI)
        {
            it->second = CMpString::UTF8ToANSI(it->second);
        }
#endif
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Write query param, %s = %s.", it->first.c_str(), it->second.c_str());
    }
}

/*------------------------------------------------------------
Function Name: GetServiceName
Description  : ͨ��ԭʼ�ַ�����ȡ����ģ�����ƣ���DB2, Oracle
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_string CRequestURL::GetServiceName()
{
    //��ȡ��������ģ����Ҫ��URL�ַ���
    if (m_procURL.empty())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "m_procURL is empty");
        return "";
    }
    mp_string strTmp = m_procURL.substr(1);

    mp_string::size_type pos = strTmp.find("/");

    return pos != mp_string::npos ? strTmp.substr(0, pos) : strTmp;
}

/*------------------------------------------------------------
Function Name: GetCutURL
Description  : ��agent/��ʼ������ָ����index���ؽضϺ��URL
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_string CRequestURL::GetCutURL(mp_int32 index)
{
    //��ȡ��������ģ����Ҫ��URL�ַ���
    if (m_procURL.empty())
    {
        return "";
    }

    mp_int32 iNum = 0; //url��"/"���ֵĸ���
    mp_string strTmp = m_procURL;
    mp_string::size_type pos = strTmp.find("/");
    while (pos != mp_string::npos)
    {
        iNum++;
        if (iNum == index)
        {
            break;
        }
        strTmp = strTmp.substr(pos);
        pos = strTmp.find("/");
    }

    if (pos != mp_string::npos)
    {
        return "";
    }

    pos = strTmp.substr(1).find("/");

    return strTmp.substr(1, pos);
}



//unsigned long CRequestMsgBody::m_totalCacheBodySize(0);

/*------------------------------------------------------------
Function Name: readWait
Description  : ��http�����ж�ȡ���ݣ�������appҪ�����ݶ����ʽ�����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CRequestMsgBody::ReadWait(CHttpRequest &req)
{
    LOGGUARD("");
    auto_ptr<CMessage_Block> msg_blk(new CMessage_Block(MSG_BLK_SIZE));

    if (0 != ReadData(req, msg_blk))
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Read data failed.");
        return MP_FAILED;
    }

    mp_int32 diff = msg_blk->GetLength() - req.GetContentLen();
    if (1 != diff)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Length of data recieved is not equal to the value descripted in head.");
        return MP_FAILED;
    }

    return ParseJson(msg_blk);
}

/*------------------------------------------------------------
Function Name: readData
Description  : ��http���ж�ȡ����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CRequestMsgBody::ReadData(CHttpRequest &req, auto_ptr<CMessage_Block> &msg)
{
    LOGGUARD("");
    CMessage_Block &msg_blk = *msg.get();

    for(; ;)
    {
        mp_uint32 len = msg_blk.GetSize() - msg_blk.GetLength();
        if (len <= 1)
        {
            if (msg_blk.Resize(msg_blk.GetSize() + MSG_BLK_SIZE))
            {
                COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Resize failed.");
                return MP_FAILED;
            }
            len = msg_blk.GetSize() - msg_blk.GetLength();
        }

        len -= 1; //Ԥ��ĩβ��0

        mp_int32 ret = req.ReadStr(msg_blk.GetWritePtr(), (mp_int32)len);
        if (ret < 0)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Fcgi read stream failed, iRet = %d", ret);
            return MP_FAILED;
        }
        else if (0 == ret)
        {
            break;
        }

        msg_blk.AddLength(ret);

        if ((mp_uint32)ret < len)
        {
            break;
        }

        //�ڴ���������
        if ((mp_int32)msg_blk.GetLength() > MAX_MSG_CACHE_SIZE)
        {
            COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Size of msg block is overload");
            return MP_FAILED;
        }
    }

    //������ļ���ĩβ����0
    if (m_msgBodyType == BODY_DECODE_JSON)
    {
        *msg_blk.GetWritePtr() = 0;
        msg_blk.AddLength(1);
    }

    m_msgLen = msg_blk.GetLength();
    //m_totalCacheBodySize += msg_blk.getSize();
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: parseJson
Description  : ����ȡ�����ַ���������json����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CRequestMsgBody::ParseJson(auto_ptr<CMessage_Block> &msg)
{
    LOGGUARD("");
    CMessage_Block &msg_block = *msg.get();

    Json::Reader r;
    if ((msg_block.GetLength() == 0) || (msg_block.GetLength() == 1))
    {
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Length of message body is 0.");
        return MP_SUCCESS;
    }

    //windowsͳһ��utf8ת����ansi����
#ifdef WIN32
    mp_string strAnsi = CMpString::UTF8ToANSI(msg_block.GetReadPtr());
    mp_bool bRet = r.parse(strAnsi.c_str(), m_msgJsonData);
#else
    mp_bool bRet = r.parse(msg_block.GetReadPtr(), m_msgJsonData);
#endif

    if (bRet != MP_TRUE)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Parse json data failed");
        return ERROR_COMMON_INVALID_PARAM;
    }  

    //ֻ������������
    if (!m_msgJsonData.isObject() && !m_msgJsonData.isArray())
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Json type is error, neither object nor array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    //��ӡ��Ϣ���е�ԭʼ����
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "[Message Body Content]: %s", msg_block.GetReadPtr());

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: jsonValueToString
Description  : json�����ַ�����ת��
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_string CRequestMsgBody::JsonValueToString(const Json::Value& v)
{
    if(v.isString())
    {
        return v.asString();
    }
    else
    {
        mp_string val;
        Json::FastWriter writer;
        val = writer.write( v);
        if(!val.empty())
        {
            val= val.substr(0,val.length() - 1);
        }
        return val;
    }
}

/*------------------------------------------------------------
Function Name: getValue
Description  : �������ƻ�ȡjson�����ж�Ӧ��ֵ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CRequestMsgBody::GetValue(const mp_string &name, mp_string &value)
{
    if( m_msgJsonData.isMember(name))
    {
        value= JsonValueToString(m_msgJsonData[name]);
        return MP_SUCCESS;
    }

    return MP_FAILED;
}

/*------------------------------------------------------------
Function Name: getOriMsg
Description  : ��ȡ��Ϣ���е�ԭʼ���ݣ������ļ����䴦��
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_bool CRequestMsgBody::GetOriMsg(mp_char *& buf, mp_uint32 &len)
{
    if( m_raw_msg.get() )
    {
        buf = m_raw_msg->GetReadPtr();
        len = (mp_uint32)m_raw_msg->GetLength();

        return MP_TRUE;
    }
    else
    {
        buf  = NULL;
        len  = 0;
        return MP_FALSE;
    }
}

/*------------------------------------------------------------
Function Name: CRequestMsg
Description  : CRequestMsg���캯����fcgiReqΪfastcgi�������
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
CRequestMsg::CRequestMsg(FCGX_Request* pFcgiReq):m_httpReq(pFcgiReq)
{
    std::string strurl = m_httpReq.GetURL();
    std::string strParam = m_httpReq.GetQueryParamStr();

    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "##### URL:%s, QueryParam:%s", 
        strurl.c_str(), strParam.c_str());

    m_url.SetOriURL(m_httpReq.GetURL());
    m_url.SetQueryParam(m_httpReq.GetQueryParamStr());
}

/*------------------------------------------------------------
Function Name: parse
Description  : ����http��������
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CRequestMsg::Parse()
{
    //����URL
    if( m_msgBody.ReadWait(m_httpReq) )
    {
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: send
Description  : ����Ӧ��Ϣд��http����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CResponseMsg::Send()
{
    if (m_httpType == RSP_JSON_TYPE)
    {
        return SendJson();
    }
    else if (m_httpType == RSP_ATTACHMENT_TYPE)
    {
        return SendAttchment();
    }
    else
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "m_httpType %d is wrong", m_httpType);
        m_httpRsp.Complete();
        return MP_FAILED;
    }
}

/*------------------------------------------------------------
Function Name: SendJson
Description  : ��json��Ӧ��Ϣд��http����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CResponseMsg::SendJson()
{
    LOGGUARD("");
    Json::Value jasonValue;
    PackageReponse(jasonValue);

    Json::FastWriter w;
    mp_string out = w.write(jasonValue);
    ostringstream ossLength;
    ossLength << out.size();

    ostringstream ossStatus;
    ossStatus << m_iHttpStatus;

    m_httpRsp.SetHead(STATUS, ossStatus.str().c_str());
    m_httpRsp.SetHead(CONTENT_TYPE, "application/json; charset=utf-8");
    m_httpRsp.SetHead(CONTENT_ENCODING, "utf-8");
    m_httpRsp.SetHead(CONTENT_LENGTH, ossLength.str().c_str());

    //д��Ϣ����
    //windowsƽ̨������Ϣͳһ��ansiת����utf8
#ifdef WIN32 
    mp_int32 iWriteLen = m_httpRsp.WriteS(CMpString::ANSIToUTF8(out).c_str());
#else
    mp_int32 iWriteLen = m_httpRsp.WriteS(out.c_str());
#endif
    
    if (iWriteLen < 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write message into fcgi stream failed");
        return MP_FAILED;
    }
    //��ӡ������Ϣ
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "[Response Message]: %s", out.c_str());
    m_httpRsp.Complete();
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: SendAttchment
Description  : ��������Ӧ��Ϣд��http����
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_int32 CResponseMsg::SendAttchment()
{
    LOGGUARD("");

    if (m_lRetCode != MP_SUCCESS)
    {
        SendFailedRsp();
        m_httpRsp.Complete();
        return MP_FAILED;
    }
     
    mp_string strAttachFileName, strAttachFilePath;
    GET_JSON_STRING_OPTION(m_msgJsonData, REST_PARAM_ATTACHMENT_NAME, strAttachFileName);
    GET_JSON_STRING_OPTION(m_msgJsonData, REST_PARAM_ATTACHMENT_PATH, strAttachFilePath);

    ifstream ifs(strAttachFilePath.c_str(), ios_base::in|ios_base::binary);
    if (!ifs)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "ifs error, strAttachFilePath = %s", strAttachFilePath.c_str());
        SendFailedRsp();
        m_httpRsp.Complete();
        return MP_FAILED;
    }

    ifs.seekg(0, ios_base::end);
    mp_uint64 fileLen = ifs.tellg();
    if (fileLen >= MAX_ATTCHMENT_SIZE)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "length[%d] of attachment is too big", fileLen);
        SendFailedRsp();
        m_httpRsp.Complete();
        return MP_FAILED;
    }

    ostringstream ossStatus;
    ossStatus << m_iHttpStatus;
    m_httpRsp.SetHead(STATUS, ossStatus.str().c_str());
    m_httpRsp.SetHead("Content-type", "application/octet-stream");
    mp_string strHeadFileName = "attachment;filename=" + strAttachFileName;
    m_httpRsp.SetHead("Content-Disposition", strHeadFileName.c_str());
    m_httpRsp.SetHead("Pragma", "public");

    //д��������
    ostringstream ossLen;
    ossLen << fileLen;
    m_httpRsp.SetHead("Content-Length", ossLen.str().c_str());
    ifs.clear();
    ifs.seekg(0);
    char tmp;
    mp_uint64 u64OneceMsg = 0;
    while(ifs.get(tmp))
    {
        m_httpRsp.WriteStr(&tmp, 1);
        u64OneceMsg++;
        //�����hp�����¸�����С����32Kʱnginx����readv��������
        if (u64OneceMsg >= ONCE_SEND_MSG_SIZE)
        {
            u64OneceMsg = 0;
            DoSleep(10);
        }
    }
    ifs.close();
    m_httpRsp.Complete();
    return MP_SUCCESS;
}


/*------------------------------------------------------------
Function Name: packageReponse
Description  : ��װhttp��Ӧ��ʽ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CResponseMsg::PackageReponse(Json::Value &root)
{
    LOGGUARD("RetCode is %d, HttpStatus is %d", m_lRetCode, m_iHttpStatus);
    if (m_lRetCode == MP_SUCCESS)
    {
        root = m_msgJsonData;
    }
    else
    {
        ostringstream oss;
        oss << m_lRetCode;
        root["errorCode"] = oss.str();
        root["errorMessage"] = m_msgJsonData;
        COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Response error code %d.", m_lRetCode);
        if (m_iHttpStatus == SC_OK)
        {
            //������߼�ģ��û������http status,Ĭ������ΪSC_NOT_ACCEPTABLE
            m_iHttpStatus = SC_NOT_ACCEPTABLE;
        }
    }
}

/*------------------------------------------------------------
Function Name: SendFailedRsp
Description  : ����ʧ�ܷ�����Ϣ
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
mp_void CResponseMsg::SendFailedRsp()
{
    ostringstream oss, ossStatus;
    oss << m_lRetCode;
    Json::Value jasonValue;
    jasonValue["errorCode"] = oss.str();
    //jasonValue["errorMessage"] = m_msgJsonData;
    COMMLOG(OS_LOG_DEBUG, LOG_COMMON_DEBUG, "Response error code %d.", m_lRetCode);
    ossStatus << SC_NOT_ACCEPTABLE;
    m_httpRsp.SetHead(STATUS, ossStatus.str().c_str());
    Json::FastWriter w;
    mp_string out = w.write(jasonValue);
    mp_int32 iWriteLen = m_httpRsp.WriteS(out.c_str());
    if (iWriteLen < 0)
    {
        COMMLOG(OS_LOG_ERROR, LOG_COMMON_ERROR, "Write message into fcgi stream failed");
    }
}

/*------------------------------------------------------------ 
Description  :���ַ���ת������ֵ
Input        : 
Output       : ret -- ת�����
Return       : MP_TRUE -- �ɹ� 
               ��MP_TRUE -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/ 
mp_bool CUrlUtils::GetNum(const mp_char *s, mp_uint32 len, mp_uint32 b, mp_uint64 &ret)
{
    mp_uint32 cv = 0;

    ret = 0;

    mp_bool cd = ((len > 16) || (b > 16) || ( NULL == s ) || ( 0 ==len ));
    if( cd )
    {
        return MP_FALSE;
    }

    struct {
        mp_char c;
        mp_int32  v;
    } m[]={
        { '0',0 },
        { '1',1 },
        { '2',2 },
        { '3',3 },
        { '4',4 },
        { '5',5 },
        { '6',6 },
        { '7',7 },
        { '8',8 },
        { '9',9 },
        { 'a',10 },
        { 'b',11 },
        { 'c',12 },
        { 'd',13 },
        { 'e',14 },
        { 'f',15 },
    };

    /*lint -e613*/
    for(mp_uint32 i = 0; i < len; i++ )
    {
        mp_uint32 j=0;
        for( ; j<sizeof(m)/sizeof(m[j]) && j<b; ++j )
        {
            mp_char c = (mp_char)tolower( s[i] );

            if( c == m[j].c )
            {
                cv = m[j].v;
                break;
            }
        }

        if( j >= b )
        {
            return MP_FALSE;
        }

        ret *= b;
        ret += cv;
    }
    /*lint +e613*/
    return MP_TRUE;
}
/*------------------------------------------------------------ 
Description  :url����
Input        : 
Output       :
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CUrlUtils::Urlencode2(mp_char *enc, mp_int32 destlen, const mp_char *src)
{
    mp_char rfc[RFC_LENGTH] = {0};
    CUrlUtils::Initrfc3986tb(rfc, RFC_LENGTH);
    const mp_uchar *s = (const mp_uchar *) src;
    for (; *s; s++)
    {
        if (rfc[*s])
        {
            CHECK_FAIL(SNPRINTF_S(enc, destlen, 2, "%c", rfc[*s]));
            //SNPRINTF_S(enc, destlen, 2, "%c", rfc3986[*s]);
        }
        else
        {
            CHECK_FAIL(SNPRINTF_S(enc, destlen, 4,"%%%02X", *s));
            //SNPRINTF_S(enc, destlen, 4, "%%%02X", *s);
        }
        while (*++enc) ;
    }
    return MP_SUCCESS;
}
/*------------------------------------------------------------ 
Description  :url����
Input        : 
Output       :
Return       : MP_SUCCESS -- �ɹ� 
               ��MP_SUCCESS -- ʧ��
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_int32 CUrlUtils::Urldecode2(mp_char *indec, const mp_char *s, mp_int32 len)
{
    if (!indec || !s)
    {
        return MP_SUCCESS;
    }
    mp_char *o;
    const mp_char *end = s + len;
    unsigned long long c;
    //lint -e440
    for (o = indec; s < end; o++)
    {
        c = *s++;
        if (c == '+')
        {
            c = ' ';
        }
        else if (c == '%' && (!CUrlUtils::IsHex(*s++) || !CUrlUtils::IsHex(*s++) || !CUrlUtils::GetNum(s-2, 2, 16, c)))
        {
            *o++ = 0;
            return ERROR_COMMON_INVALID_PARAM;
        }

        *o = (mp_char)c;
    }
    //lint +e440
    *o++ = 0;
    return (mp_int32)(o - indec);
}

/*------------------------------------------------------------ 
Description  :��ȡ�Ⱥ��ұߵ�ֵ
Input        : 
Output       :
Return       :
Create By    :
Modification : 
-------------------------------------------------------------*/
mp_string CUrlUtils::GetBetweenValue(const mp_char*eqp, const mp_char* p)
{
//CodeDex�󱨣�ZERO_LENGTH_ALLOCATIONS
    if (p - eqp <= 1)
    {
        return "";
    }
    else
    {
        mp_char *buf;
		//CodeDex�󱨣�Memory Leak
        NEW_ARRAY_CATCH(buf, mp_char, p+1-eqp);
        if (!buf)
        {
            return "";
        }
        buf[0] = 0;
        (mp_void)CUrlUtils::Urldecode2(buf, eqp + 1, (mp_int32)(p-(eqp+1)));
        mp_string ret(buf);
        delete[] buf;
        return ret;
    }
}

/*------------------------------------------------------------
Function Name: GetJsonString
Description  : ��ȡJSON����ĳ��Ԫ�ص��ַ���ֵ
               jsValue:JSON���ݣ�ʾ��{"data":"chengdu"}
               strKey:JSON���ݵ�keyֵ,ʾ���е�"data"
               strValue:key��Ӧ���ַ���ֵ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonString(const Json::Value& jsValue, mp_string strKey, mp_string& strValue)
{
    if (jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isMember(strKey))
    {
        if (jsValue[strKey].isString())
        {
            strValue = jsValue[strKey].asString();
            return MP_SUCCESS;
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The value Json key \"%s\" is not string.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }

    }
    else
    {
        strValue = "";
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}

/*------------------------------------------------------------
Function Name: GetJsonInt32
Description  : ��ȡJSON����ĳ��Ԫ�ص�int32ֵ
               jsValue:JSON���ݣ�ʾ�� {"data":2015}
               strKey:JSON���ݵ�keyֵ,ʾ���е�"data"
               iValue:key��Ӧ��int32ֵ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonInt32(const Json::Value& jsValue, mp_string strKey, mp_int32& iValue)
{
    if (jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isMember(strKey))
    {
        if (jsValue[strKey].isInt())
        {
            iValue = jsValue[strKey].asInt();
            return MP_SUCCESS;
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }
    }
    else
    {
        iValue = MP_FAILED;
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
}

/*------------------------------------------------------------
Function Name: GetJsonInt64
Description  : ��ȡJSON����ĳ��Ԫ�ص�int64ֵ
               jsValue:JSON���ݣ�ʾ�� {"data":2015}
               strKey:JSON���ݵ�keyֵ,ʾ���е�"data"
               lValue:key��Ӧ��int64ֵ
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonInt64(const Json::Value& jsValue, mp_string strKey, mp_int64& lValue)
{
    if (jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isMember(strKey))
    {
        if (jsValue[strKey].isInt())
        {
            lValue = jsValue[strKey].asInt64();
            return MP_SUCCESS;
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
            return ERROR_COMMON_INVALID_PARAM;
        }

    }
    else
    {
        lValue = MP_FAILED;
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

}

/*------------------------------------------------------------
Function Name: GetJsonArrayInt32
Description  : JSON����ĳ��Ԫ��Ϊint32����
               jsValue:JSON���ݣ�ʾ�� {"data":[2015,2016,2017]}
               strKey:JSON���ݵ�keyֵ��ʾ���е�"data"
               vecValue:key��Ӧ��int32����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonArrayInt32(const Json::Value& jsValue, mp_string strKey, vector<mp_int32>& vecValue)
{
    if (jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isMember(strKey) && jsValue[strKey].isArray())
    {
        mp_uint32 uiSize = jsValue[strKey].size();
        for (mp_uint32 i = 0; i < uiSize; i++)
        {          
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isInt())
            {
                vecValue.push_back(jsValue[strKey][i].asInt());
            }
            else
            {
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        return MP_SUCCESS;
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

}

/*------------------------------------------------------------
Function Name: GetJsonArrayInt64
Description  : JSON����ĳ��Ԫ��Ϊint64����
               jsValue:JSON���ݣ�ʾ�� {"data":[2015,2016,2017]}
               strKey:JSON���ݵ�keyֵ��ʾ���е�"data"
               vecValue:key��Ӧ��int64����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonArrayInt64(const Json::Value& jsValue, mp_string strKey, vector<mp_int64>& vecValue)
{
    if (jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isMember(strKey) && jsValue[strKey].isArray())
    {
        mp_uint32 uiSize = jsValue[strKey].size();
        for (mp_uint32 i = 0; i < uiSize; i++)
        {          
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isInt())
            {
                vecValue.push_back(jsValue[strKey][i].asInt64());
            }
            else
            {
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        return MP_SUCCESS;
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

}

/*------------------------------------------------------------
Function Name: GetJsonArrayString
Description  : JSON����ĳ��Ԫ��Ϊstring����
               jsValue:JSON���ݣ�ʾ�� {"data":["chengdu","chongqing","nanchong"]}
               strKey:JSON���ݵ�keyֵ��ʾ���е�"data"
               vecValue:key��Ӧ��string����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonArrayString(const Json::Value& jsValue, mp_string strKey, vector<mp_string>& vecValue)
{
    if (jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isMember(strKey) && jsValue[strKey].isArray())
    {
        mp_uint32 uiSize = jsValue[strKey].size();
        for (mp_uint32 i = 0; i < uiSize; i++)
        {          
            if (!jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray() && jsValue[strKey][i].isString())
            {
                vecValue.push_back(jsValue[strKey][i].asString());
            }
            else
            {
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The value of Json key \"%s\" is not int.", strKey.c_str());
                vecValue.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        return MP_SUCCESS;
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

}

/*------------------------------------------------------------
Function Name: GetJsonArrayJson
Description  : JSON����ĳ��Ԫ��ΪJson����
               jsValue:JSON���ݣ�ʾ�� {"data":[{"city":"chengdu"},{"city":"chongqing"}]}
               strKey:JSON���ݵ�keyֵ��ʾ���е�"data"
               vecValue:key��Ӧ��json����,���鵥Ԫ��Ϊjson����������{"city":"chengdu"}
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetJsonArrayJson(const Json::Value& jsValue, mp_string strKey, vector<Json::Value>& vecValue)
{
    if (jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is Array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    if (jsValue.isMember(strKey) && jsValue[strKey].isArray())
    {
        mp_uint32 uiSize = jsValue[strKey].size();
        for (mp_uint32 i = 0; i < uiSize; i++)
        {          
            if (jsValue[strKey][i].isObject() && !jsValue[strKey][i].isArray())
            {
                vecValue.push_back(jsValue[strKey][i]);
            }
            else
            {
                COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "The value of Json key \"%s\" is not Json object.", strKey.c_str());
                vecValue.clear();
                return ERROR_COMMON_INVALID_PARAM;
            }
        }
        return MP_SUCCESS;
    }
    else
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json key \"%s\" does not exist.", strKey.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }

}

/*------------------------------------------------------------
Function Name: GetArrayInt32
Description  : ��JSON����Ϊint32���飬��û��keyֵ
               jsValue:JSON���ݣ�ʾ�� {[1,2,3]}
               vecValue:ת���ɵ�int32����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayInt32(const Json::Value& jsValue, vector<mp_int32>& vecValue)
{
    if (!jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 uiSize = jsValue.size();
    for (mp_uint32 i = 0; i < uiSize; i++)
    {
        if(!jsValue[i].isObject() && !jsValue[i].isArray())
        {
            vecValue.push_back(jsValue[i].asInt());
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json value is array or object.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayInt64
Description  : ��JSON����Ϊint64���飬��û��keyֵ
               jsValue:JSON���ݣ�ʾ�� {[1,2,3]}
               vecValue:ת���ɵ�int64����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayInt64(const Json::Value& jsValue, vector<mp_int64>& vecValue)
{
    if (!jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 uiSize = jsValue.size();
    for (mp_uint32 i = 0; i < uiSize; i++)
    {
        if(!jsValue[i].isObject() && !jsValue[i].isArray())
        {
            vecValue.push_back(jsValue[i].asInt64());
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json value is array or object.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayString
Description  : ��JSON����Ϊstring���飬��û��keyֵ
               jsValue:JSON���ݣ�ʾ�� {["chengdu","chongqing","nanchong"]}
               vecValue:ת���ɵ�string����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayString(const Json::Value& jsValue, vector<mp_string>& vecValue)
{
    if (!jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 uiSize = jsValue.size();
    for (mp_uint32 i = 0; i < uiSize; i++)
    {
        if(!jsValue[i].isObject() && !jsValue[i].isArray())
        {
            vecValue.push_back(jsValue[i].asString());
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json value is array or object.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Function Name: GetArrayJson
Description  : ��JSON����Ϊjson���飬��û��keyֵ
               jsValue:JSON���ݣ�ʾ�� {[{"city":"chengdu"},{"city":"chonqing"}]}
               vecValue:ת���ɵ�json����
Return       :
Call         :
Called by    :
Modification :
Others       :-------------------------------------------------------------*/
mp_int32 CJsonUtils::GetArrayJson(const Json::Value& jsValue, vector<Json::Value>& vecValue)
{
    if (!jsValue.isArray())
    {
        COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json is not array");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 uiSize = jsValue.size();
    for (mp_uint32 i = 0; i < uiSize; i++)
    {
        if(jsValue[i].isObject() && !jsValue[i].isArray())
        {
            vecValue.push_back(jsValue[i]);
        }
        else
        {
            COMMLOG(OS_LOG_INFO, LOG_COMMON_INFO, "Json value is not object.");
            return ERROR_COMMON_INVALID_PARAM;
        }
    }

    return MP_SUCCESS;
}




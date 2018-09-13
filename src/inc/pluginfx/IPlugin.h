/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

#ifndef _AGENT_IPLUGIN_H_
#define _AGENT_IPLUGIN_H_

#include "common/Types.h"

class IPluginManager;

///////////////////////////////////////////////////////////////////////////
//��������ӿڶ���
class IPlugin
{
public:
    typedef struct {
        mp_void* Creator;           //Creator����ָ��
        const mp_char* ClassName;   //������
    }DYN_CLASS;
    
public:
    //�����ʼ������
    //param [in]  pMgr  PluginManager����ָ��
    virtual mp_void Initialize(IPluginManager* pMgr) = 0;  //��ʼ������

    //ж�ز�����
    virtual mp_int32 Destroy() = 0;
  
    //���ò�����л���
    //[in]  pszName  ��������
    //[in]  pvVal    ����ָ��
    //������л������÷����������������Ҫ����Դ�����ò���
    //ͨ�����������Push��������������������initialize()����
    virtual mp_void SetOption(const mp_char* pszName, mp_void* pvVal) = 0;
    
    //��ȡ������õķ��������Բ�ʵ�֣������ں�����չ
    //[in]  pszName  ��������
    //[out] pvVal    ����ָ��
    //[in]  sz       ����Buffer����
    //retval true  ������Ч
    //retval false ��֧�ָò���
    virtual mp_bool GetOption(const mp_char* pszName, mp_void* pvVal, mp_int32 sz) = 0;
    
    //����������������һ��������� or ����ʵ�֣�
    //[in]  pszName  ��������
    //retval  !=NULL �ɹ������Ķ����ָ��
    //retval  NULL   ��������ʧ��
    virtual mp_void* CreateObject(const mp_char* pszName) = 0;

    //��ȡ����С�������������Ϣ��
    //[out]  pClasses  ����ඨ���Buffer
    //[in]   sz        Buffer����
    //return  �����֧�ֶ�̬��������ĸ�����
    //����ֵΪ��ĸ������������ڴ治�㣬�򷵻�-1��ͬʱ���pClassses
    virtual mp_int32 GetClasses(IPlugin::DYN_CLASS *pClasses, mp_int32 sz) = 0;

    //��ȡ���ز��������
    virtual const mp_char* GetName() = 0;

    //��ȡ���ز���İ汾
    virtual const mp_char* GetVersion() = 0;
    
    //��ȡ���ز����SCN
    virtual mp_size GetSCN() = 0;
    
    //��ȡ�������
    virtual mp_int32 GetTypeId() const  { return 0; }
    
protected:
    virtual ~IPlugin() {}
};


class IPluginCallback
{
public:
    //���ж��ǰ�ص��ķ������жϸò���Ƿ����ж��
    //[in]  pOldPlg ����ӿ�
    //retval true  ����ж��
    //retval false ������ж��
    virtual mp_bool CanUnload(IPlugin* pOldPlg) = 0;

    //�²������֪ͨ����
    //[in] pOldPlg �ɲ���ӿ�
    //[in] pNewPlg �²���ӿ�
    //���²�����سɹ���PluginManager��ص����������
    //���ģ���������Ӧ����     
    virtual mp_void OnUpgraded(IPlugin* pOldPlg, IPlugin* pNewPlg) = 0;

    //���ò��ѡ��Ľӿ�
    //[in]  plg  ����ӿ�
    //���²�����سɹ���PluginManager��ص��������
    virtual mp_void SetOptions(IPlugin* plg) = 0;

    //��ȡlib�ķ����汾
    //[in]  pszLib  lib���֣�ע�ⲻ������׺��.so or .dll��
    //[out] pszVer  �汾
    //[in] sz       pszVer�ĳ���
    //retval NULL   û�а汾��Ϣ
    //retval !=NULL �汾��Ϣ
    virtual mp_char* GetReleaseVersion(const mp_char* pszLib, mp_char* pszVer, mp_size sz) = 0;

protected:
    virtual ~IPluginCallback() {}
};


//PluginManager�ӿڶ���
class IPluginManager
{
public:
    //��ʼ������
    //[in]  pCallback �ص�����
    virtual mp_void Initialize(IPluginCallback* pCallback) = 0;
    
    //����
    //�������ʵ��һ�����١����ٵĺ���
    virtual mp_void Destroy() = 0;

    //��ȡϵͳ������
    //return  ����scn��system change number��
    //ÿ��Plugin����һ��scn��Ӧ�ü�1
    virtual mp_size GetSCN() = 0;
    
    //�������ֻ�ȡ����ӿ�
    //[in]  pszPlg �������
    //retval != NULL  ���ز���ӿ�
    //retval NULL     �����û�м���
    virtual IPlugin* GetPlugin(const mp_char* pszPlg) = 0;
    
    virtual IPlugin* LoadPlugin(const mp_char* pszPlg) = 0;
    
    virtual mp_void UnloadPlugin(const mp_char* pszPlg) = 0;

    //����Ѿ����صĲ����Ҫ�����������Ҫ������Load���
    //return  �ɹ������Ĳ���ĸ�����
    virtual mp_int32 Upgrade() = 0;

protected:
    virtual ~IPluginManager() {}
};

#endif //_AGENT_IPLUGIN_H_


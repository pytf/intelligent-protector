/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

// rdvss.cpp : DLL ������ʵ�֡�
#ifdef WIN32

#include "vss/provider/stdafx.h"
#include "vss/provider/resource.h"
#include "rdvss.h"
#include "common/AppVersion.h"

// {ab23308d-639e-41ed-9b76-691b52409acb}
static const GUID g_gProviderId =
    { 0xab23308d, 0x639e, 0x41ed, { 0x9b, 0x76, 0x69, 0x1b, 0x52, 0x40, 0x9a, 0xcb } };
// {95e5091e-3123-48d9-a62b-eff6b1ef8490}
static const GUID g_gProviderVersion =
    { 0x95e5091e, 0x3123, 0x48d9, { 0xa6, 0x2b, 0xef, 0xf6, 0xb1, 0xef, 0x84, 0x90 } };
WCHAR* g_wszProviderName = L"RdAgent Software Provider";

static WCHAR* g_wszProviderVersion = RD_PROVIDER_VERSION;

class CrdvssModule : public CAtlDllModuleT<CrdvssModule>
{
public :
	DECLARE_LIBID(LIBID_rdvssLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_RDVSS, "{F749E911-2190-4482-AB86-343182FACB08}")
};

CrdvssModule _AtlModule;


// DLL ��ڵ�
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved);
}


// ����ȷ�� DLL �Ƿ���� OLE ж��
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// ����һ���๤���Դ������������͵Ķ���
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - ������ӵ�ϵͳע���
STDAPI DllRegisterServer(void)
{
    // ע��������Ϳ�����Ϳ��е����нӿ�
    HRESULT hr = _AtlModule.DllRegisterServer();

	CComPtr<IVssAdmin> pVssAdmin;
    if (SUCCEEDED( hr )) {
        hr = CoCreateInstance( CLSID_VSSCoordinator,
            NULL,
            CLSCTX_ALL,
            IID_IVssAdmin,
            (void **) &pVssAdmin);
    }

    if (SUCCEEDED( hr )) {
        hr = pVssAdmin->RegisterProvider(g_gProviderId,
            CLSID_provider,
            g_wszProviderName,
            VSS_PROV_SOFTWARE,
            g_wszProviderVersion,
            g_gProviderVersion );
    }

    pVssAdmin.Release();

	return hr;
}


// DllUnregisterServer - �����ϵͳע������Ƴ�
STDAPI DllUnregisterServer(void)
{
	CComPtr<IVssAdmin> pVssAdmin;

    HRESULT hr = CoCreateInstance(CLSID_VSSCoordinator,
                           NULL,
                           CLSCTX_ALL,
                           IID_IVssAdmin,
                           (void **) &pVssAdmin);

    if (SUCCEEDED( hr )) {
        hr = pVssAdmin->UnregisterProvider(g_gProviderId);
    }

	hr = _AtlModule.DllUnregisterServer();

	pVssAdmin.Release();

	return hr;
}

#endif


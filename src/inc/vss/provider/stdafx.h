/*******************************************************************************
* Copyright @ Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
********************************************************************************/

// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
#ifdef WIN32

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "resource.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ĳЩ CString ���캯��������ʽ��
#define _ATL_ALL_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
using namespace ATL;

#include <new>
#include <string>
#include <vector>
#include <map>

#include "vss.h"
#include "vsprov.h"
#include "vscoordint.h"

// version
#include <ntverp.h>

#endif

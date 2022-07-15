#pragma once
#include "core/WinMin.h"

// �ڵ�����������������ʽ��������Ϣ����ѡ�Ĵ��󴰿ڵ���(�Ѻ���)
// [In]strFile			��ǰ�ļ�����ͨ�����ݺ�__FILEW__
// [In]hlslFileName     ��ǰ�кţ�ͨ�����ݺ�__LINE__
// [In]hr				����ִ�г�������ʱ���ص�HRESULTֵ
// [In]strMsg			���ڰ������Զ�λ���ַ�����ͨ������L#x(����ΪNULL)
// [In]bPopMsgBox       ���ΪTRUE���򵯳�һ����Ϣ������֪������Ϣ
HRESULT WINAPI DXTraceW(
	_In_z_ const WCHAR* strFile,
	_In_ DWORD dwLine,
	_In_ HRESULT hr,
	_In_opt_ const WCHAR* strMsg,
	_In_ bool bPopMsgBox
);

#if (defined(DEBUG) || defined(_DEBUG))
	#ifndef HR
		#define HR(x) if(HRESULT hr = (x); FAILED(hr)) DXTraceW(__FILEW__, (DWORD)__LINE__, hr, L#x, TRUE)
	#endif
#else
	#ifndef HR
		#define HR(x) (x)
	#endif
#endif
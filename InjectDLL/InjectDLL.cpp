// InjectDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "detours/detours.h"
#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>
#include <stdarg.h>

#include "LdjStruct.h"

typedef DWORD SOCKET;
extern "C" int WINAPI send(SOCKET s, const char *buf, int len, int flags);
extern "C" int WINAPI recv(SOCKET s, char *buf, int len, int flags);
typedef int (WINAPI *pSendProc)(SOCKET s, const char *buf, int len, int flags);
typedef int (WINAPI *pRecvProc)(SOCKET s, char *buf, int len, int flags);
typedef LRESULT (CALLBACK * pWndProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

pSendProc pOrigSend;
pRecvProc pOrigRecv;
pWndProc pOrigWndProc;

HWND g_hWnd;

void SendKeyPress(HWND hWnd, int code)
{
	PostMessage(hWnd, WM_KEYDOWN, code, MapVirtualKey(code, 0) >> 8 | 0x01);
	PostMessage(hWnd, WM_KEYUP, code, 0xC0000000 | MapVirtualKey(code, 0) >> 8 | 0x01);
}


// �洢��һ�ε�ѡ�ֲ���
SELECT g_select;

// g_select�Ƿ��Ѿ���ʼ��
bool g_select_init = false;

// ģʽ
// ģʽ0��Ĭ�ϵĲ�������¼�����ѡ�ֲ�����Ȼ���ȡ����������ѡ��
// ģʽ1��ǿ������һ�Σ�Ȼ���л���ģʽ0
// ģʽ2����¼һ���ֵص����ݰ���Ȼ���л���ģʽ0
// ģʽ3��������һ���ֵص����ݰ���Ȼ���л���ģʽ4
// ģʽ4�������ڶ����ֵص����ݰ���Ȼ���л���ģʽ0
DWORD g_mode = 0;

#define WM_CHANGE_MODE (WM_USER + 0xc531)

void MyTrace(const char *format, ...)
{
	va_list v;
	va_start(v, format);

	char buf[1024];
	_vsnprintf_s(buf, 1024, format, v);

	OutputDebugStringA(buf);
}

HWND WINAPI MyFindWindowExW(HWND hWndParent, HWND hWndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow)
{
	MyTrace("FindWindowEx. HWND %x %x. Class is %ws, Title is %ws", hWndParent, hWndChildAfter, lpszClass, lpszWindow);
	return FindWindowExW(hWndParent, hWndChildAfter, lpszClass, lpszWindow);
}

HWND WINAPI MyFindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	MyTrace("FindWindow. Class is %ws, Title is %ws", lpClassName, lpWindowName);		
	return FindWindowW(lpClassName, lpWindowName);
}

typedef HWND (WINAPI *pFindWindowExProc)(HWND hWndParent, HWND hWndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow);
typedef HWND (WINAPI *pFindWindowProc)(LPCWSTR lpszClass, LPCWSTR lpszWindow);

pFindWindowExProc pOrigFindWindowEx;
pFindWindowProc pOrigFindWindow;

LRESULT CALLBACK fake_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_CHANGE_MODE)
	{
		MyTrace("In fake_WndProc, set g_mode to %d\n", lParam);
		g_mode = lParam;
	}
	return pOrigWndProc(hwnd, uMsg, wParam, lParam);
}


void DumpSendBuffer(const char *buf, int len)
{
	char buffer[256000];
	for(int i = 0; i < len; i++)
	{
		wsprintfA(buffer + i * 3, "%02x ", (unsigned char)buf[i]);
	}
	OutputDebugString(buffer);
}

/*
void DumpRecvBuffer(const char *buf, int len)
{
	char buffer[256000] = "Recv: ";

	if (len > 64)
		len = 64;
	for(int i = 0; i < len; i++)
	{
		wsprintfA(buffer + i * 3 + 6, "%02x ", (unsigned char)buf[i]);
	}
	OutputDebugString(buffer);
}
*/

int send_filter(char * dst, const char * src, int len)
{
	unsigned char command = (unsigned char)src[0];
	/*
	char buffer[256];
	wsprintfA(buffer, "in send_filter, %02x", (unsigned char)src[0]); 
	OutputDebugString(buffer);
	*/

	// ��ͨ��ֵĸ���
	if (g_mode == 0)
	{
		// �������µ�ѡ�ְ�
		if(command == SELECT_CODE)
		{
			memcpy(&g_select, src, sizeof(SELECT));
			g_select_init = true;
		}

		// �����ȡ������������ѡ�ְ�����
		if(command == UN_SKILL_CODE && g_select_init)
		{
			PSELECT pse = (PSELECT)src;
			PSELECT ps = (PSELECT)dst;
			memcpy(dst, &g_select, sizeof(SELECT));
			ps->syn = pse->syn;
			return sizeof(SELECT);
		}
	}
	//	ǿ�������Ĵ���
	else if (g_mode == 1)
	{
		PSELECT pse = (PSELECT)src;
		PSELECT ps = (PSELECT)dst;
		memset(dst, 0xC4, sizeof(SELECT));

		ps->syn = pse->syn;

		g_mode = 0;

		return sizeof(SELECT);
	}
	// ��¼�ֵص����ݰ�
	else if (g_mode == 2 && command == 0x7a)
	{
		MyTrace("g_mode = 2, record packet of zd");

		PZD pzd = (PZD)src;
		if (pzd->param2 != -1)
		{
			MyTrace("Second Packet. set g_mode to 0");
			memcpy(&g_zd, src, sizeof(ZD));
			g_mode = 0;
		}
		else
			MyTrace("First Packet. Do nothing.");

	}
	// �����ֵصĵ�һ�����ݰ�
	else if (g_mode == 3 && command == 0x48)
	{
		MyTrace("g_mode = 3, send first packet of zd, and set g_mode to 4");

		PZD pzd = (PZD)src;
		PZD pz = (PZD)dst;
		memcpy(dst, &g_zd, sizeof(ZD));
		pz->param2 = -1;
		pz->syn = pzd->syn;

		g_mode = 4;
		return sizeof(ZD);
	}
	// �����ֵصĵڶ������ݰ�
	else if (g_mode == 4 && command == 0x48)
	{
		MyTrace("g_mode = 3, send second packet of zd, and set g_mode to 0");

		PZD pzd = (PZD)src;
		PZD pz = (PZD)dst;
		memcpy(dst, &g_zd, sizeof(ZD));
		pz->syn = pzd->syn;

		g_mode = 0;
		return sizeof(ZD);
	}

	// �������ʲô������
	memcpy(dst, src, len);
	// DumpSendBuffer(src, len);

	return len; 
}


int WINAPI fake_send(SOCKET s, const char * buf, int len, int flags)
{ 
	char buffer[4096];
	const char * lastBuffer = buf;
	int length = 0;

	while(lastBuffer - buf < len)
	{
		int l = *(PSHORT)(lastBuffer + 2) + 6;

		if (l < 0 || lastBuffer + l - buf > len)
		{
			MyTrace("sb in fake_send.");
			return pOrigSend(s, buf, len, flags);
		}
		length += send_filter(buffer + length, lastBuffer, l);
		lastBuffer += l;
	}

	int n = pOrigSend(s, buffer, length, flags);

	if(n == length) 
		return len;
	else if(n == -1)
		return -1;
	else
		return 0;
}

int WINAPI fake_recv(SOCKET s, char * buf, int len, int flags)
{
	static int tail = 0;
	int n = pOrigRecv(s, buf, len, flags);

	/*
	if (n > 0)
	{
		const char * lastBuffer = buf + tail;

		while(lastBuffer - buf < n)
		{
			int l = *(PSHORT)(lastBuffer + 2) + 6;
			unsigned char command = (unsigned char)lastBuffer[0];
			
			if (command != 0x71)
				DumpRecvBuffer(lastBuffer, l);
			lastBuffer += l;
		}

		tail = lastBuffer - buf - n;
		if (tail)
			OutputDebugString("Tail!!");
	}
	*/

	return n;
}

void InitHook()
{
	pOrigSend = send;
	pOrigRecv = recv;
	g_hWnd = GetForegroundWindow();
	pOrigWndProc = (pWndProc)GetWindowLong(g_hWnd, GWL_WNDPROC);
	pOrigFindWindow = FindWindowW;
	pOrigFindWindowEx = FindWindowExW;

	MyTrace("I am in.");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((PVOID *)&pOrigSend, fake_send);
	DetourAttach((PVOID *)&pOrigRecv, fake_recv);
	DetourAttach((PVOID *)&pOrigWndProc, fake_WndProc);
	DetourAttach((PVOID *)&pOrigFindWindow, MyFindWindowW);
	DetourAttach((PVOID *)&pOrigFindWindowEx, MyFindWindowExW);
	DetourTransactionCommit();	
}

void FiniHook()
{
	OutputDebugString("I am out.");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach((PVOID *)&pOrigSend, fake_send);
	DetourDetach((PVOID *)&pOrigRecv, fake_recv);
	DetourDetach((PVOID *)&pOrigWndProc, fake_WndProc);
	DetourDetach((PVOID *)&pOrigFindWindow, MyFindWindowW);
	DetourDetach((PVOID *)&pOrigFindWindowEx, MyFindWindowExW);
	DetourTransactionCommit();		
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		InitHook();
		break;
	case DLL_PROCESS_DETACH:
		FiniHook();
		break;
	}
    return TRUE;
}

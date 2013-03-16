// TestInject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

char g_dllName[] = "D:\\InjectDLL\\Release\\InjectDLL.dll";

void Inject()
{
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	char * pFileName = NULL;

	__try
	{
		HWND hWnd = GetForegroundWindow();
		char buffer[256];
		GetWindowText(hWnd, buffer, 255);
		printf("��ǰ����: %s\n", buffer);

		DWORD pid = 0;
		GetWindowThreadProcessId(hWnd, &pid);
		printf("����ID: %x\n", pid);

		hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
			FALSE, pid);
		if(!hProcess)
		{
			printf("�򿪽���ʧ�ܡ�\n");
			__leave;
		}

		int cb = strlen(g_dllName) + 1;

		pFileName = (char *)VirtualAllocEx(hProcess, NULL, cb, MEM_COMMIT, PAGE_READWRITE);
		if(!pFileName)
		{
			printf("�����ڴ�ʧ�ܡ�\n");
			__leave;
		}

		if(!WriteProcessMemory(hProcess, pFileName, g_dllName, cb, NULL))
		{
			printf("д��DLL����ʧ�ܡ�\n");
			__leave;
		}

		PTHREAD_START_ROUTINE pfn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("Kernel32"), 
			"LoadLibraryA");
		if(!pfn)
		{
			printf("��ȡLoadLibraryA��ַʧ�ܡ�\n");
			__leave;
		}

		hThread = CreateRemoteThread(hProcess, NULL, 0, pfn, pFileName, 0, NULL);
		if(!hThread)
		{
			printf("����Զ���߳�ʧ�ܡ�\n");
			__leave;
		}

		WaitForSingleObject(hThread, INFINITE);
	}
	__finally
	{
		if(pFileName)
			VirtualFreeEx(hProcess, pFileName, 0, MEM_RELEASE);
		if(hThread) CloseHandle(hThread);
		if(hProcess) CloseHandle(hProcess);
	}

}

void Eject()
{
	HANDLE hth = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;

	__try
	{
		HWND hWnd = GetForegroundWindow();
		char buffer[256];
		GetWindowText(hWnd, buffer, 255);
		printf("��ǰ����: %s\n", buffer);

		DWORD pid = 0;
		GetWindowThreadProcessId(hWnd, &pid);
		printf("����ID: %x\n", pid);

		hth = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		if(!hth)
		{
			printf("����SnapShotʧ�ܡ�\n");
			__leave;
		}
		
		MODULEENTRY32 me = {sizeof(me)};
		BOOL ff = FALSE;
		for(BOOL fm = Module32First(hth, &me); fm; fm = Module32Next(hth, &me))
		{
			ff = (lstrcmpi(me.szExePath, g_dllName) == 0 || lstrcmpi(me.szModule, g_dllName) == 0);
			if(ff) break;
		}
		if(!ff)
		{
			printf("û�з������������DLLע�롣\n");
			__leave;
		}

		hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
			FALSE, pid);
		if(!hProcess)
		{
			printf("�򿪽���ʧ�ܡ�\n");
			__leave;
		}

		PTHREAD_START_ROUTINE pfn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("Kernel32"), 
			"FreeLibrary");
		if(!pfn)
		{
			printf("��ȡFreeLibrary��ַʧ�ܡ�\n");
			__leave;
		}

		hThread = CreateRemoteThread(hProcess, NULL, 0, pfn, me.modBaseAddr, 0, NULL);
		if(!hThread)
		{
			printf("����Զ���߳�ʧ�ܡ�\n");
			__leave;
		}

		WaitForSingleObject(hThread, INFINITE);
	}
	__finally
	{
		if(hth) CloseHandle(hth);
		if(hThread) CloseHandle(hThread);
		if(hProcess) CloseHandle(hProcess);
	}
}

int main(int argc, char* argv[])
{
	RegisterHotKey(0, 100, MOD_ALT, 121);
	RegisterHotKey(0, 200, MOD_CONTROL, 121);

	MSG msg;

	while(GetMessage(&msg, 0, 0, 0))
	{
		if(msg.message == WM_HOTKEY)
		{
			if(msg.wParam == 200)
			{
				Inject();
			}
			else
			{
				Eject();
			}
		}
	}
	return 0;
}

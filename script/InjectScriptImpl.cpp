#include "stdafx.h"
#include "InjectScriptImpl.h"

#include "InjectHelper.h"
#include "FileUtility.h"

void InjectScriptImpl::DoStart()
{
	StartInject(m_hWnd, GetInjectDllName());
}
void InjectScriptImpl::DoStop()
{
	StopInject(m_hWnd, GetInjectDllName());
}
char * InjectScriptImpl::GetName()
{
	if(!m_start)
		return "����ʽ���";
	else 
		return "����ʽ���(����)";
}
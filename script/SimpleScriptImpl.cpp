#include "stdafx.h"
#include "SimpleScriptImpl.h"

void SimpleScriptImpl::DoOnTimer()
{
	DWORD t = m_bFast ? 1 : 10;
	if(m_timerCount == 1)
		SendKeyPress(m_hWnd, 122);
	else if(m_timerCount == t + 1)
		SendKeyPress(m_hWnd, 123);
	if(m_timerCount == 2 * t)
		m_timerCount = 0;
}
char * SimpleScriptImpl::GetName()
{
	if(!m_start)
		return "�򵥴��";
	else if(m_bFast)
		return "�򵥴��(����)";
	else
		return "�򵥴��(����)";
}
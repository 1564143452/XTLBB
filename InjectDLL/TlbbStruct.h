#include <windows.h>

#pragma pack(push)
#pragma pack(1)

// �ż��ܵ����ݰ�
typedef struct _SKILL
{
	WORD code;
	WORD length;
	WORD syn;
	char reserved1[14];
	DWORD id;
	char reserved2[8];
} SKILL, * PSKILL;

// ѡ�ֵ����ݰ�
typedef struct _SELECT
{
	WORD code;
	WORD length;
	WORD syn;
	DWORD id;
} SELECT, * PSELECT;

#pragma pack(pop)
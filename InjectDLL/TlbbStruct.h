#include <windows.h>

#pragma pack(push)
#pragma pack(1)

// �ż��ܵ����ݰ�
#define SKILL_CODE 0x45
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
#define SELECT_CODE 0x7E
typedef struct _SELECT
{
	WORD code;
	WORD length;
	WORD syn;
	DWORD id;
} SELECT, * PSELECT;

#pragma pack(pop)
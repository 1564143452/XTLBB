#include <windows.h>

#pragma pack(push)
#pragma pack(1)

// �ֵص����ݰ�
// ֻ��Ҫ��ȡһ���ֵ����ݰ���֮��
typedef struct _ZD
{
	WORD code;
	WORD length;
	WORD syn;
	DWORD param1;
	DWORD param2;
	DWORD param3[2];
} ZD, *PZD;

ZD g_zd;

// �յص����ݰ�
typedef struct _SD
{
	WORD code;
	WORD length;
	WORD syn;
	DWORD reserved0[3];
	DWORD id;
	BYTE unused[10];
}SD, *PSD;

// �ż��ܵ����ݰ�����С��23 00
#define SKILL_CODE 0xcd
// ȡ�����ܵ����ݰ�����С��08 00
#define UN_SKILL_CODE 0xc1
typedef struct _SKILL
{
	WORD code;
	WORD length;
	WORD syn;
	DWORD reserved1;
	DWORD type;
	DWORD param0;
	DWORD param1;
	DWORD param2;
	DWORD param3;
	DWORD reserved2;
	DWORD reserved3;	
	WORD reserved4;
	BYTE reserved5;
} SKILL, * PSKILL;

// ѡ�ֵ����ݰ�����С��08 00�����8���ֽ�������ID
#define SELECT_CODE 0x15
typedef struct _SELECT
{
	WORD code;
	WORD length;
	WORD syn;
	DWORD param1;		
	DWORD reserved;
} SELECT, * PSELECT;

#pragma pack(pop)
//---------------------------------------------------------
//	����:		CWAVE.CPP
//	��������:	����� ��� ������ ������ ������� WAV
//---------------------------------------------------------

#include <windows.h>
#include "cwave.h"

#define	WAV_HEADER_SIZE		44 // ������ ������ ��������� WAV
#define	WAVEFORMATEX_SHIFT	20 // �������� �� ������ ����� �� ��������� WAVEFORMATEX

CMappedWave::CMappedWave()
{
	//�������� ��� ����� ������
	pbWaveFile=NULL;
	pWaveHeader=NULL;
	dwCurPosition=0;
	dwDataLength=0;
}


CMappedWave::~CMappedWave()
{
	//��������� ����, ���� �� ��� ������
	Close();
}


BOOL CMappedWave::Open(CHAR* szFilename)
{
	//��������� ����, ���� �� ��� ������
	Close();

	HANDLE	hFile,hMappedFile;
	DWORD	dwFileLength;
	
	//�������� ������� ����
	hFile=CreateFile(szFilename,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,NULL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return (FALSE);
	
	//���������� ������ �����
	dwFileLength=GetFileSize(hFile,NULL);

	//������ ������ - �������� �����
	hMappedFile=CreateFileMapping(hFile,NULL,PAGE_READONLY,
		0,dwFileLength,NULL);
	
	//��������� ��������� ����� (�� �� ��� ����!!!)
	CloseHandle(hFile);
	
	if (!hMappedFile)
		return (FALSE);
	
	//���������� ���� �� �������� ������������
	pbWaveFile=(PBYTE)MapViewOfFile(hMappedFile,FILE_MAP_READ,0,0,0);
	
	//��������� ��������� �������� �����
	CloseHandle(hMappedFile);

	if (!pbWaveFile)
		return (FALSE);
	
	//�������������� ���������� ����� ������
	pWaveHeader=(LPWAVEFORMATEX)(pbWaveFile+WAVEFORMATEX_SHIFT);
	dwCurPosition=WAV_HEADER_SIZE;
	dwDataLength=*((DWORD*)(pbWaveFile+WAV_HEADER_SIZE-sizeof(DWORD)));

	return (TRUE);
}

void CMappedWave::Reset()
{
	//���������� ������� ������� � �������� �� ���������
	if (pbWaveFile)
		dwCurPosition=WAV_HEADER_SIZE;
}

void CMappedWave::Close()
{
	if (pbWaveFile)
	{
		//��������� �������� ����� �� ��������� ������������
		UnmapViewOfFile(pbWaveFile);
		pbWaveFile=NULL;
	}
	
	//�������� ����� ������
	pWaveHeader=NULL;
	dwDataLength=0;
	dwCurPosition=0;

}

DWORD CMappedWave::Read(BYTE* pbData,DWORD dwSizeToRead)
{
	DWORD dwRemainder;

	//���� ���� �� ��� ������, ���� ��� ������ ���������
	//���������� 0 - ������ ������ ������
	if ((!pbWaveFile)||((dwDataLength+WAV_HEADER_SIZE)==dwCurPosition))
		return (0);

	if (dwSizeToRead<=(dwDataLength+WAV_HEADER_SIZE-dwCurPosition))
	{
		//���� ���������� ������ ��� ������
		CopyMemory(pbData,pbWaveFile+dwCurPosition,dwSizeToRead);
		dwCurPosition+=dwSizeToRead;
		dwRemainder=dwSizeToRead;
	}
	else //���� ������ ������������
	{
		//�������� ������� ����
		dwRemainder=dwDataLength-dwCurPosition+WAV_HEADER_SIZE;
		CopyMemory(pbData,pbWaveFile+dwCurPosition,dwRemainder);
        
		//��������� ������� ���������
		FillMemory(pbData+dwRemainder,dwSizeToRead - dwRemainder, 
                (pWaveHeader->wBitsPerSample==8) ? 128 : 0);

		dwCurPosition+=dwRemainder;
	}
	return (dwRemainder);
}

DWORD CMappedWave::GetWaveSize()
{
	//���������� ������ ������ �������� ������ - ��� ���������
	if (pbWaveFile)
		return (dwDataLength);
	else
		return (0);
}


////////////////////////////////////
//Filename: ZUtil.h
//Abstract: һЩͨ�ú����Ķ���ʵ��
//Create: by zdleek 20110823
//Edit history:
//
///////////////////////////////////
#pragma once
#include "windows.h"
#include "time.h"


namespace CZUtil
{

	//��ʱ�������ַ���(����"2011-06-10 11:00:00")ת��Ϊc++��ʱ����������time_t, struct tm
	time_t ParseTimeString(const char *pDateTime, size_t max_size = 20, struct tm* pOutTm = NULL);

	//ȡ��һ���Ű�ƻ��Ŀ�ʼ�ͽ���ʱ��(��һ��4:00�͵ڶ���4:00)
	void GetBETimeOfPlan(char *pszTodayTime, char *pszNextdayTime, size_t maxsize = 20);

	//��time_t��ʽ��ʱ��ת��Ϊʱ�������ַ���(����"2011-06-10 11:00:00")
	char *FormatTimeString(time_t tTimeIn, char *pOutTimeStr, size_t MaxSize = 20, const char *pFormat = "%Y-%m-%d %H:%M:%S");
	//char *TimeT2String(time_t tTimeIn, char *pOutTimeStr, size_t MaxSize = 20);

	//�������ϴ���GPSʱ��ת��ΪC++��struct tm��time_t
	time_t GpsTime2Tm(BYTE *pGpsTime, struct tm *pTmOut);

	//16�����ַ�ת��Ϊ����ֵint������Fת��Ϊ16
	inline BYTE HexByteToInt8(BYTE byHex);
	//16������ʽ���ַ���ת��Ϊ���ͣ�����12AB0A2CתΪ����
	int HexStrToInt32(const char *pHex);
	bool EncodeParam4WriteData(char *format, char *buf,... );
}

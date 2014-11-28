#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include "ZUtil.h"

using namespace std;
namespace  CZUtil{

	//��ʱ�������ַ���(����"2011-06-10 11:00:00")ת��Ϊc++��ʱ����������time_t, struct tm
	time_t ParseTimeString(const char *pDateTime, size_t max_size, struct tm* pOutTm)
	{
		const char *p = pDateTime;
		int k = 0;
		int l = 0;
		struct tm stm;
		struct tm* ptm;

		time_t curtime ;
		const char *format = "%Y-%m-%d %H:%M:%S"; //%F ��-��-�� %H 24Сʱ�Ƶ�Сʱ

		curtime = time(NULL);
		ptm = localtime(&curtime);
		stm = *ptm;
		
		//����ʱ���ַ���
		for(size_t i = 0; i < max_size; i++)
		{
			if(pDateTime[i] == '-')
			{
				k++;
				if(k == 1)
				{

					stm.tm_year = atoi(p);
				}
				else if(k == 2)
				{
					stm.tm_mon = atoi(p);
				}
				else
				{
					//error format of datetimestring
					return 0;
				}
				p = pDateTime + i + 1;
			}
			if(pDateTime[i] == ' ')
			{
				if(k == 2)
				{
					stm.tm_mday = atoi(p);
					p = pDateTime + i + 1;
				}
			}
			if(pDateTime[i] == ':')
			{
				l++;
				if(l == 1)
				{
					stm.tm_hour = atoi(p);
				}
				else if(l == 2)
				{
					stm.tm_min = atoi(p);
				}
				else
				{
					//error format of datetimestring
					return 0;
				}
				p = pDateTime + i + 1;
			}
			if(pDateTime[i] == '\0') 
			{
				if( l == 1)
					stm.tm_min = atoi(p);
				else if( l == 2)
					stm.tm_sec = atoi(p);

				break;
			}
		}//end of for 

		if( k < 2 ) return 0;//ʱ���ַ�����ʽ����ȷ
			
		curtime = mktime(&stm);
		if(pOutTm) *pOutTm = stm;

		return curtime;
	}

	//ȡ��һ���Ű�ƻ��Ŀ�ʼ�ͽ���ʱ��(��һ��4:00�͵ڶ���4:00)
	void GetBETimeOfPlan(char *pszTodayTime, char *pszNextdayTime, size_t maxsize)
	{
		time_t curtime ;
		struct tm* ptm ;
		const char *format = "%Y-%m-%d %H:%M:%S"; //%F ��-��-�� %H 24Сʱ�Ƶ�Сʱ

		curtime = time(NULL);

		//��һ��4:00
		ptm = localtime(&curtime);
		if(ptm->tm_hour < 4 )
		{
			ptm->tm_mday -= 1;//����ڽ���4:00֮ǰ����ôȡ����4:00������4:00������
		}
		ptm->tm_hour = 4;
		ptm->tm_min = 0;
		ptm->tm_sec = 0; 
		mktime(ptm);

		size_t t = strftime(pszTodayTime, maxsize, format, ptm); 

		//�ڶ���4:00
		ptm->tm_mday += 1;	
		mktime(ptm) ;
		//ptm = localtime(&nextday);
		t = strftime(pszNextdayTime, maxsize, format, ptm);

	}

	//��time_t��ʽ��ʱ��ת��Ϊʱ�������ַ���(����"2011-06-10 11:00:00")
	char *FormatTimeString(time_t tTimeIn, char *pOutTimeStr, size_t MaxSize, const char *pFormat)
	{
		if(!pOutTimeStr) return NULL;

		struct tm* ptm ;
		const char *Format = "%Y-%m-%d %H:%M:%S"; //%F ��-��-�� %H 24Сʱ�Ƶ�Сʱ
		
		if(pFormat) Format = pFormat;

		ptm = localtime(&tTimeIn);
		size_t t = strftime(pOutTimeStr, MaxSize, Format, ptm);

		return pOutTimeStr;
	}

	bool EncodeParam4WriteData(char *format, char *buf,... )
	{
		if(!buf) return false;
		va_list va;
		va_start(va, buf);
		string strFormat = format;
		char *p1 ;
		int n1;
		int offset = 0;
		char szNum[30]={0};
		for(int i = 0; format[i]!='\0' && va != NULL; i++)
		{
			if(format[i] == 's')
			{
				 p1 = va_arg(va,char *);
				 strcpy(buf + offset, p1);
				 offset += strlen(p1);	
			}
			else
			{
				 n1= va_arg(va,int);
				 _itoa(n1, szNum, 10);
				 strcpy(buf + offset,szNum);
				 offset+=strlen(szNum);
			}
			buf[offset] ='_';
			offset++;
		}
		va_end(va);

		//vsprintf(buf, format, va);
		return true;
	}

	time_t GpsTime2Tm(BYTE *pGpsTime, struct tm *pTmOut)
	{
		time_t t = time(NULL);
		struct tm *pTm = localtime(&t);

		pTm->tm_year = (2000 + pGpsTime[0]) - 1900; //tm_year: Year (current year minus 1900).
		pTm->tm_mon = pGpsTime[1] - 1;				//tm_mon:  Month (0 �C 11; January = 0).
		pTm->tm_mday = pGpsTime[2];					//tm_mday: Day of month (1 �C 31).
		pTm->tm_hour = pGpsTime[3];					//tm_hour: Hours after midnight (0 �C 23).
		pTm->tm_min = pGpsTime[4];					//tm_min: 0 - 59
		pTm->tm_sec = pGpsTime[5];					//tm_sec: 0 - 59

		t = mktime(pTm);
		if(pTmOut) *pTmOut = *pTm;

		return t;
	}

	//16�����ַ�ת��Ϊ����ֵint������Fת��Ϊ16
	BYTE HexByteToInt8(BYTE byHex)
	{
		BYTE byDec = 0x00;
		//16����ת��Ϊ10����
		if(byHex >= '0' && byHex <= '9')
		{
			byDec = byHex - '0';
		}
		else if(byHex >= 'a' && byHex <= 'f')
		{
			byDec = byHex - 'a';
		}
		else if( byHex >= 'A' && byHex <= 'F')
		{
			byDec = byHex - 'A';
		}
		return byDec;

	}

	//16������ʽ���ַ���ת��Ϊ���ͣ�����12AB0A2CתΪ����
	int HexStrToInt32(const char *pHex)
	{
		int len = strlen(pHex);
		if(len == 0) return 0;

		int nDec = 0;
		char szHex[9] = {0};
		char szDec[4] = {0};

		if(len > 8 )
			len = 8;
		
		if((len % 2) != 0)
		{
			szHex[0] = '0';
			memcpy(szHex+1, pHex, len);
		}
		else //if(len == 8)
		{
			memcpy(szHex, pHex, len);
		}

		int i = len-1;
		while (i > 0) 
		{
			//16����ת��Ϊ10��������			
			BYTE b = HexByteToInt8(szHex[i]) * 16 + HexByteToInt8(szHex[i]);
			szDec[i/2] = b;
			i = i - 2;	
		}
		memcpy(&nDec, szDec, 4);
		return nDec;
	}

}//end of namespace CZUtil
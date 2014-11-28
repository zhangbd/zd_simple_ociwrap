////////////////////////////////////
//Filename: ZUtil.h
//Abstract: 一些通用函数的定义实现
//Create: by zdleek 20110823
//Edit history:
//
///////////////////////////////////
#pragma once
#include "windows.h"
#include "time.h"


namespace CZUtil
{

	//将时间日期字符串(例如"2011-06-10 11:00:00")转换为c++的时间数据类型time_t, struct tm
	time_t ParseTimeString(const char *pDateTime, size_t max_size = 20, struct tm* pOutTm = NULL);

	//取得一天排班计划的开始和结束时间(第一天4:00和第二天4:00)
	void GetBETimeOfPlan(char *pszTodayTime, char *pszNextdayTime, size_t maxsize = 20);

	//将time_t格式的时间转换为时间日期字符串(例如"2011-06-10 11:00:00")
	char *FormatTimeString(time_t tTimeIn, char *pOutTimeStr, size_t MaxSize = 20, const char *pFormat = "%Y-%m-%d %H:%M:%S");
	//char *TimeT2String(time_t tTimeIn, char *pOutTimeStr, size_t MaxSize = 20);

	//将车载上传的GPS时间转换为C++的struct tm和time_t
	time_t GpsTime2Tm(BYTE *pGpsTime, struct tm *pTmOut);

	//16进制字符转换为整数值int，例如F转换为16
	inline BYTE HexByteToInt8(BYTE byHex);
	//16进制形式的字符串转换为整型，例如12AB0A2C转为整型
	int HexStrToInt32(const char *pHex);
	bool EncodeParam4WriteData(char *format, char *buf,... );
}

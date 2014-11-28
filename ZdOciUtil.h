////////////////////////////////////
//* Filename: ZdOciUtil.h
//* Abstract: OCI接口封装(oracle)
//* Begin on: 20110630 by zdleek 
//* Edit history:
//*
///////////////////////////////////
#pragma once
#include <oci.h>

class CZdOciUtil
{
	CZdOciUtil(void);
public:

	CZdOciUtil(char *pDBName, char *pUser, char *pPwd);
	~CZdOciUtil(void);

public:
	int CheckErr(OCIError *pErr, sword lStatus);
	char* GetType(ub2 nType, char *pTypeName);		//获取数据类型名称


	int ReadLobData();				//读写LOB型数据

	int TestQuery();
	
	int ConnectDB();
	int DisconnectDB();
	int Close();

	bool IsConnected(){return m_IsConnected;}

	int DefineByPos(ub4 uPos, ub2 uDataType, void* pVal, sb4 val_size);
	int DefineByPos_INT(ub4 uPos, int* pVal, sb4 val_size = sizeof(int));
	int DefineByPos_FLT(ub4 uPos, double* pVal, sb4 val_size = sizeof(double));
	int DefineByPos_STR(ub4 uPos, char* pVal, sb4 val_size);
	int DefineByPos_CHR(ub4 uPos, char* pVal, sb4 val_size);

	//int DefineByPosCommon(ub4 uPos, int* pVal, sb4 val_size = sizeof(int))
	//{
	//	DefineByPos_INT(uPos, pVal, val_size);
	//}
	//int DefineByPosCommon(ub4 uPos, double* pVal, sb4 val_size = sizeof(double))
	//{
	//	DefineByPos_FLT(uPos, pVal, val_size);
	//}
	//int DefineByPosCommon(ub4 uPos, char* pVal, sb4 val_size, bool TypeStrFlag = true)
	//{
	//	if(TypeStrFlag) 
	//		DefineByPos_STR(uPos, pVal, val_size); 
	//	else 
	//		DefineByPos_CHR(uPos, pVal, val_size);
	//}

	int SQLExecute(const char *pSql);

	int FetchNext();
	inline int RowFetch(){return FetchNext();}
	bool checkErr(sb4 status, OCIError *pErr = NULL);
	int RowCount();
	//bool SetIsFree(bool fIsFree){return true;}
	const char *GetOraErrMsg(){ return err_msg;}
	const int GetOraErr(){return ora_err;}
	
private:
	int BeginOciSession();
	int EndOciSession();
	void InitMember();

	/// for test by zdleek///
	int SelectXing(char *pSql);		//隐式描述一个选择列表
	int EnumFields();				//显式描述一个选择列表
	int BatchInsert();				//批量添加记录
	int SelectRecords();		//获取一个查询的结果
	int LocalTransProc();			//本地简单事务处理
	int GetRelateTableInfo();		//通过表名获取关系表的表结构信息
	/// end of test ///

private:
	
	char m_DBName[60];		//数据库服务名
	char m_UserName[33];	//用户
	char m_Password[33];	//密码

	OCIEnv *m_envhp; // 环境句柄
	OCIServer *m_srvhp; //服务器句柄
	OCISession *m_usrhp; //用户会话句柄
	OCISvcCtx *m_svchp; //服务上下文句柄
	OCIStmt* m_stmthp; //语句句柄
	OCIError *m_errhp; //错误句柄
	OCIError *m_errhp1; //错误句柄
	OCIError *m_errhp2; //错误句柄

	bool m_IsConnected; //是否已连接到DB
	int  m_ExecCount;
	
	char err_msg[512];
	int  ora_err;
};

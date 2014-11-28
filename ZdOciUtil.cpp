////////////////////////////////////
//* Filename: ZdOciUtil.h
//* Abstract: OCI接口封装(oracle)
//* Begin on: 20110630 by zdleek 
//* Edit history:
//*
///////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ZdOciUtil.h"

#if defined(WIN32)
#pragma comment(lib, "oci.lib")
#endif


CZdOciUtil::CZdOciUtil(void)
{
	InitMember();

	strcpy(m_DBName, "xmgpsbus130");	
	strcpy(m_UserName,  "xmbustest");
	strcpy(m_Password,  "xmgps");
}

CZdOciUtil::CZdOciUtil(char *pDBName, char *pUser, char *pPwd)
{
	InitMember();
	
	if(pDBName) strncpy(m_DBName, pDBName, sizeof(m_DBName));		
	if(pUser) strncpy(m_UserName, pUser, sizeof(m_UserName));	
	if(pPwd) strncpy(m_Password, pPwd, sizeof(m_Password));	
}

CZdOciUtil::~CZdOciUtil(void)
{
	Close();
}

void CZdOciUtil::InitMember()
{
	m_envhp = NULL; // 环境句柄
	m_srvhp = NULL; //服务器句柄
	m_errhp = NULL; //错误句柄
	m_errhp1 = NULL; //错误句柄
	m_errhp2 = NULL; //错误句柄
	m_usrhp = NULL; //用户会话句柄
	m_svchp = NULL; //服务上下文句柄
	m_stmthp = NULL; //语句句柄

	m_IsConnected  = false; //是否已连接到DB
	m_ExecCount = 0;
}

int CZdOciUtil::ConnectDB()
{
	if(m_IsConnected) return OCI_SUCCESS;

	//使用线程和对象模式来创建环境句柄
	OCIEnvCreate(&m_envhp, OCI_THREADED|OCI_OBJECT, (dvoid *)0,
		0, 0, 0, (size_t) 0, (dvoid **)0);
	//分配服务器句柄
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_srvhp,
		OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	//分配错误句柄
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp,
		OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp1,
		OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp2,
		OCI_HTYPE_ERROR, 0, (dvoid **) 0);

	//创建服务器上下文句柄,"orcl"为建立连接的数据库名
	if (OCIServerAttach (m_srvhp, m_errhp, (text *)m_DBName,
		strlen ((char*)m_DBName), OCI_DEFAULT) == OCI_SUCCESS)
	{
		//m_IsConnected = true;
#ifdef _DEBUG
		printf("\n已经成功连上数据库%s\n", m_DBName);
#endif
	}
	else 
	{
		m_IsConnected = false;
#ifdef _DEBUG
		printf("\n连接数据库(%s)失败!\n", m_DBName);
#endif
		return -1;
	}

	//分配服务器上下文句柄
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp,
		OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	//设置服务器上下文句柄的服务器句柄属性
	OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	//分配用户会话句柄
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp,
		OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	//为用户会话句柄设置用户名和密码属性
	OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
		(dvoid *)m_UserName, (ub4)strlen((char*)m_UserName),
		OCI_ATTR_USERNAME, m_errhp);
	OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
		(dvoid *)m_Password, (ub4)strlen((char*)m_Password),
		OCI_ATTR_PASSWORD, m_errhp);
	if (OCISessionBegin ( m_svchp, m_errhp, m_usrhp,
		OCI_CRED_RDBMS, OCI_DEFAULT) == OCI_SUCCESS)
	{
		//m_IsLogined = true;
#ifdef _DEBUG
		printf("成功建立用户会话!\n");
#endif		
	}
	else
	{
		m_IsConnected = false;
#ifdef _DEBUG
		printf("建立用户会话失败!\n");
#endif		
		return -1;
	}

	m_IsConnected = true;

	//在服务器上下文环境中设置用户会话属性
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//分配语句句柄
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));

	return OCI_SUCCESS;
}

int CZdOciUtil::BeginOciSession()
{
	if(m_IsConnected) return OCI_SUCCESS;

	//分配服务器上下文句柄
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp, 
		OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	//设置服务器上下文句柄的服务器句柄属性
	OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX, 
		(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	//分配用户会话句柄
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp, 
		OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	//为用户会话句柄设置用户名和密码属性
	OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION, 
		(dvoid *)m_UserName, (ub4)strlen((char*)m_UserName),
		OCI_ATTR_USERNAME, m_errhp);
	OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION, 
		(dvoid *)m_Password, (ub4)strlen((char*)m_Password),
		OCI_ATTR_PASSWORD, m_errhp);
	if (OCISessionBegin ( m_svchp, m_errhp, m_usrhp, 
		OCI_CRED_RDBMS, OCI_DEFAULT) == OCI_SUCCESS)
	{
		m_IsConnected = true;
#ifdef _DEBUG
		printf("成功建立用户会话!\n");
#endif		
	}
	else
	{
		m_IsConnected =false;
#ifdef _DEBUG
		printf("建立用户会话失败!\n");
#endif		
		return -1;
	}

	return OCI_SUCCESS;
}

int CZdOciUtil::EndOciSession()
{
	if(!m_IsConnected) return OCI_SUCCESS;

	//结束会话
	OCISessionEnd(m_svchp, m_errhp, m_usrhp, OCI_DEFAULT);
	
	return OCI_SUCCESS;
}

int CZdOciUtil::DisconnectDB()
{
	if(!m_IsConnected) return OCI_SUCCESS;

	EndOciSession();

	//断开连接
	OCIServerDetach(m_srvhp, m_errhp, OCI_DEFAULT);
	//释放环境句柄
	OCIHandleFree((void*)m_envhp, OCI_HTYPE_ENV);

	m_IsConnected = false;
	m_ExecCount = 0;
	return OCI_SUCCESS;
}

int CZdOciUtil::Close()
{
#ifdef _DEBUG
	printf("\n结束会话和数据库连接!\n");
#endif
	//EndOciSession();
	DisconnectDB();
	return OCI_SUCCESS;
}

int CZdOciUtil::DefineByPos(ub4 uPos, ub2 uDataType, void* pVal, sb4 val_size)
{
	OCIDefine* defhp;
	//如果成功，返回OCI_SUCCESS = 0
	return OCIDefineByPos(m_stmthp, &defhp, m_errhp, uPos, pVal, val_size,
		uDataType, 0, 0, 0, OCI_DEFAULT);
}

int CZdOciUtil::DefineByPos_INT(ub4 uPos, int* pVal, sb4 val_size)
{
	return DefineByPos( uPos, SQLT_INT,  pVal,  val_size);
}

int CZdOciUtil::DefineByPos_FLT(ub4 uPos, double* pVal, sb4 val_size)
{
	return DefineByPos( uPos, SQLT_FLT,  pVal,  val_size);
}

int CZdOciUtil::DefineByPos_STR(ub4 uPos, char* pVal, sb4 val_size)
{
	return DefineByPos( uPos, SQLT_STR,  pVal,  val_size);
}

int CZdOciUtil::DefineByPos_CHR(ub4 uPos, char* pVal, sb4 val_size)
{
	return DefineByPos( uPos, SQLT_CHR,  pVal,  val_size);
}


int CZdOciUtil::FetchNext()
{
	//如果没有数据，返回值为OCI_NO_DATA， 如果有数据，返回值为OCI_SUCCESS
	//return OCIStmtFetch(m_stmthp, m_errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT);

	int ret_code = OCIStmtFetch2(m_stmthp, m_errhp, 1, OCI_FETCH_NEXT, 1, OCI_DEFAULT);
	//checkErr(ret_code);
	return ret_code;
}

bool CZdOciUtil::checkErr(sb4 status, OCIError *pErr)
{
	if(!pErr) pErr = m_errhp;

	ora_err = status;
    switch (status)
    {
    case OCI_SUCCESS:
        strcpy(err_msg,"OCI_SUCCESS");    
        break;
    case OCI_SUCCESS_WITH_INFO:
        strcpy(err_msg, "OCI_SUCCESS_WITH_INFO");
        break;
    case OCI_ERROR:
        //if(pErr) 
			OCIErrorGet((dvoid *)pErr, (ub4)1, (text *)NULL, &ora_err,
				(unsigned char*)err_msg, (ub4)sizeof(err_msg), OCI_HTYPE_ERROR);
        break;
    case OCI_NEED_DATA:
        strcpy(err_msg, "OCI_NEED_DATA");
        break;
    case OCI_NO_DATA:
        strcpy(err_msg, "OCI_NO_DATA");
        break;
    case OCI_INVALID_HANDLE:
        strcpy(err_msg, "OCI_INVALID_HANDLE");
        break;
    case OCI_STILL_EXECUTING:
        strcpy(err_msg, "OCI_STILL_EXECUTING");
        break;
    case OCI_CONTINUE:
        strcpy(err_msg, "OCI_CONTINUE");
        break;
    default:
		err_msg[0] = 0;
        break;
    }
	printf( "CheckErr(%d) : ORA_ERR=%d, %s", status, ora_err, err_msg);

    if (status != OCI_SUCCESS && status != OCI_SUCCESS_WITH_INFO)
    {
        return false;
    } 
	return true;
}

int CZdOciUtil::SQLExecute(const char *pSql)
{
	if(!pSql) return -6;
	text * cmd ; 
	cmd = (text *)pSql;

	//if(m_ExecCount > 200) Close(); //del for test
	if( ConnectDB() != 0 ) return -7;

	int nRet = OCI_SUCCESS;

	m_ExecCount++;

	//在服务器上下文环境中设置用户会话属性
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);

	//分配语句句柄	del for test
	//CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));

	//设置预取数量为10
	int fetchCount = 10;
	CheckErr(m_errhp, OCIAttrSet(m_stmthp, OCI_HTYPE_STMT, (void*)&fetchCount, 4, OCI_ATTR_PREFETCH_ROWS, m_errhp));

	if(!CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, cmd, strlen((char*)cmd),
		OCI_NTV_SYNTAX, OCI_DEFAULT)))
	{
		nRet = -103;
		goto lb_exit;
	}

	int nstmt_type = 0;
	OCIAttrGet(m_stmthp, OCI_HTYPE_STMT, &nstmt_type, 0, OCI_ATTR_STMT_TYPE, m_errhp);
	if(nstmt_type == OCI_STMT_SELECT) //select查询
	{
		//提取结果
		if(!CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 0, 0, NULL, NULL,
			OCI_DEFAULT)))
		{
			//printf("OCI Exec Sql failed. %s \r", pSql);
			nRet = -105;
			goto lb_exit;
		}

	}
	else if(nstmt_type > OCI_STMT_SELECT) //DML语句
	{

		if(!CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
			OCI_COMMIT_ON_SUCCESS)))
		{
			//printf("OCI Exec Sql failed. %s \r", pSql);
			OCITransRollback(m_svchp, m_errhp, 0);

			nRet = -105;
			goto lb_exit;
		}
	}

lb_exit:
	return nRet;	
}

int CZdOciUtil::RowCount()
{
	int rows = 0;
	//得到已经成功处理的记录行数
	OCIAttrGet(m_stmthp, OCI_HTYPE_STMT, &rows, 0, OCI_ATTR_ROW_COUNT, m_errhp); //OCI_ATTR_ROWS_RETURNED //OCI_ATTR_ROWS_FETCHED
	return rows; 
}


//int CZdOciUtil::ExecStoredProc(char *pSql)
//{
//	return OCI_SUCCESS;
//}



int CZdOciUtil::TestQuery()
{
	////三个定义句柄，用于定义输出结果
	//OCIDefine* defhp1;
	//OCIDefine* defhp2;
	//OCIDefine* defhp3;
	//OCIDefine* defhp4;

	//输出变量
	int	nID = 0;
	char szName[128] = {0};
	int nAge = 0;
	char szAddress[256];
	sb2 indAddr = 0; //address字段的指示符

	//DML(INSERT)操作的SQL语句
	char *sql =  "SELECT id, name, age, address FROM student";
	

	//连接上数据库
	if( ConnectDB() != OCI_SUCCESS ) return OCI_ERROR;

	int ret = SQLExecute(sql);
	if(ret != OCI_SUCCESS)
	{
		return ret;
	}
	//定义输出
	DefineByPos_INT(1, &nID);
	DefineByPos_STR(2, szName, 33);
	DefineByPos_INT(3, &nAge);
	DefineByPos_STR(4, szAddress, 129);
		
	printf("\n查询结果为: \n");
	printf("ID      NAME            AGE     ADDRESS\n");
	printf("---------------------------------------------------\n");
	while( OCI_NO_DATA != this->FetchNext())
	{
		printf("%-8d%-16s%-8d", nID, szName, nAge);
		if(indAddr == 0) printf("%-32s", szAddress);
		printf("\n");
	}
	
	return OCI_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
//隐式描述一个选择列表
int CZdOciUtil::SelectXing(char *pSql)	
{
	//DML(INSERT)操作的SQL语句
	static text* select = (text*) "SELECT * FROM student";

	sb4 status = 0;

	OCIParam *paramhp; //参数句柄指针
	ub4 counter;
	ub2 dtype; //数据类型
	char szCol[128]={0}; //列名缓存
	char  szTypeName[64] ={0};//数据类型
	text *col_name; //列名
	ub4 col_name_len; //列长度
	sb4 parm_status; //状态码
	
	//连接上数据库
	if( ConnectDB() != 0 ) return -1;

	//建立用户会话
	if(BeginOciSession() != 0) return -1;

	//在服务器上下文环境中设置用户会话属性
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//分配语句句柄
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	
	//准备SQL语句
	if(!CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select, strlen((char*)select),
		OCI_NTV_SYNTAX, OCI_DEFAULT)))
		return -1;

	//获取选择表的元信息
	printf("获取选择表的元信息\n");
	CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
		OCI_DESCRIBE_ONLY));

	counter = 1;
	parm_status = OCIParamGet(m_stmthp, OCI_HTYPE_STMT, m_errhp, (void**)&paramhp,
		(ub4) counter);
	memset(szCol, 0, sizeof(szCol));

	//循环获取
	while (parm_status==OCI_SUCCESS) 
	{
		//获取数据类型属性
		CheckErr(m_errhp, OCIAttrGet((dvoid*) paramhp, (ub4) OCI_DTYPE_PARAM,
			(dvoid*) &dtype,(ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,(OCIError *) m_errhp ));
		//获取列名属性
		CheckErr(m_errhp, OCIAttrGet((dvoid*) paramhp, (ub4) OCI_DTYPE_PARAM,
			(dvoid**) &col_name,(ub4 *) &col_name_len, (ub4) OCI_ATTR_NAME,
			(OCIError *) m_errhp ));
		memcpy(szCol, col_name, col_name_len);
		printf("%d\t列名:%s\t\t数据类型:%s\n", counter, szCol, GetType(dtype, szTypeName));
		memset(szCol, 0, sizeof(szCol));
		counter++;
		parm_status = OCIParamGet(m_stmthp, OCI_HTYPE_STMT, m_errhp, (dvoid**)&paramhp,
			(ub4) counter);
	}

	printf("共得到选择列表列数为:%d\n", counter-1);

	Close();
	return OCI_SUCCESS;
}




//显式描述一个选择列表
int CZdOciUtil::EnumFields()
{
	sb4 status = 0;

	OCIParam *paramhp; //参数句柄指针
	ub4 counter;
	ub4 colcount = 0;  //列数
	ub2 dtype; //数据类型
	char szCol[128]; //列名缓存
	char szTypeName[64] = {0};
	text *col_name; //列名
	ub4 col_name_len; //列长度
	sb4 parm_status; //状态码

	static text* select = (text*) "SELECT * FROM student";

	//连接上数据库
	if( ConnectDB() != 0 ) return -1;
	//建立用户会话
	if(BeginOciSession() != 0) return -1;

	//在服务器上下文环境中设置用户会话属性
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//分配语句句柄
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	//准备SQL语句
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select, strlen((char*)select),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	//获取选择列表的元信息
	printf("获取选择表的元信息\n");
	CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
		OCI_DESCRIBE_ONLY));

	//获取选择列表的列数,保存在colcount中
	CheckErr(m_errhp, OCIAttrGet(m_stmthp, OCI_HTYPE_STMT, &colcount, 0, 
		OCI_ATTR_PARAM_COUNT, m_errhp));
	printf("选择列表中共有%d列\n", colcount);
	counter = 1;
	
	//循环获取各列的元信息
	for (counter = 1; counter<=colcount; counter++)
	{	
		//获取数据类型属性
		parm_status = OCIParamGet(m_stmthp, OCI_HTYPE_STMT, m_errhp, (dvoid**)&paramhp,
			(ub4) counter);
		CheckErr(m_errhp, OCIAttrGet((dvoid*) paramhp, (ub4) OCI_DTYPE_PARAM,
			(dvoid*) &dtype,(ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,(OCIError *) m_errhp ));
		//获取列名属性
		CheckErr(m_errhp, OCIAttrGet((dvoid*) paramhp, (ub4) OCI_DTYPE_PARAM,
			(dvoid**) &col_name,(ub4 *) &col_name_len, (ub4) OCI_ATTR_NAME,
			(OCIError *) m_errhp ));
		memcpy(szCol, col_name, col_name_len);
		szCol[col_name_len] = 0;
		printf("%d\t列名:%-32s数据类型:%-32s\n", counter, szCol, GetType(dtype, szTypeName));
	}

	Close();

	return OCI_SUCCESS;
}

int CZdOciUtil::SelectRecords()
{

	//三个定义句柄，用于定义输出结果
	OCIDefine* defhp1;
	OCIDefine* defhp2;
	OCIDefine* defhp3;
	OCIDefine* defhp4;

	//DML(INSERT)操作的SQL语句
	text* select = (text*) "SELECT id, name, age, address FROM student";
	//text* select = (text*) "select PhotoID,Dev_ID,Photosize, Photo from tab_photos where PhotoID < 9";
	text * cmd = (text*)" begin \r v2_delete_dev('0001'); \r end;";
	cmd = select;//for test select 
	//输出变量
	int	nID = 0;
	char szName[128] = {0};
	int nAge = 0;
	char szAddress[256];
	sb2 indAddr = 0; //address字段的指示符

	sb4 status = 0;

	ub4 colcount = 0;  //列数

	//连接上数据库
	if( ConnectDB() != 0 ) return -1;


	//设置预取数量为10
	int fetchCount = 2;
	CheckErr(m_errhp, OCIAttrSet(m_stmthp, OCI_HTYPE_STMT, (void*)&fetchCount, 4, OCI_ATTR_PREFETCH_ROWS, m_errhp));
	//准备SQL语句
	//CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select, strlen((char*)select),
	//	OCI_NTV_SYNTAX, OCI_DEFAULT));

	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, cmd, strlen((char*)cmd),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	//定义输出
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp1, m_errhp, 1, &nID, sizeof(nID),
		SQLT_INT, 0, 0, 0, OCI_DEFAULT));
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp2, m_errhp, 2, szName, 33,
		SQLT_STR, 0, 0, 0, OCI_DEFAULT));
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp3, m_errhp, 3, &nAge, sizeof(nAge),
		SQLT_INT, 0, 0, 0, OCI_DEFAULT));
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp4, m_errhp, 4, szAddress, 129,
		SQLT_STR, &indAddr, 0, 0, OCI_DEFAULT));


	//提取结果
	if(!CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 0, 0, NULL, NULL,
		OCI_DEFAULT)))
	{
		printf("Exec Sql failed.");
		return OCI_SUCCESS;
	}
	printf("\n查询结果为: \n");
	printf("ID      NAME            AGE     ADDRESS\n");
	printf("---------------------------------------------------\n");
	while( OCI_NO_DATA != OCIStmtFetch(m_stmthp, m_errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT))
	{
		printf("%-8d%-16s%-8d", nID, szName, nAge);
		if(indAddr == 0) printf("%-32s", szAddress);
		printf("\n");
	}
	
	Close();
	return OCI_SUCCESS;
}

//批量添加数据
int CZdOciUtil::BatchInsert()
{
	//OCIEnv *m_envhp; // 环境句柄
	//OCIServer *m_srvhp; //服务器句柄
	//OCIError *m_errhp; //错误句柄
	//OCIError *m_errhp1; //错误句柄
	//OCIError *m_errhp2; //错误句柄
	//OCISession *m_usrhp; //用户会话句柄
	//OCISvcCtx *m_svchp; //服务上下文句柄
	//OCIStmt* m_stmthp; //语句句柄

	OCIBind* bndhp1 = NULL;
	OCIBind* bndhp2 = NULL;
	
	//DML(INSERT)操作的SQL语句
	static text* insert = (text*) "INSERT INTO STUDENT(id, name, age)"
			" VALUES(:1, :name, 22)";
	int	nSID[5] = {1, 2, 1, 1, 3}; //学号
	char szName[5][32] = {"Bob", "Ying", "HeXiong", "Jenny", "Mary" }; //姓名
	sb4 nNameLen = 32;
	sb4 status = 0;

	int	rows_processed = 0;
	int err_row = 0; //错误行

	//连接上数据库
	if( ConnectDB() != 0 ) return -1;
	//建立用户会话
	if(BeginOciSession() != 0) return -1;

	//在服务器上下文环境中设置用户会话属性
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//分配语句句柄
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	//准备SQL语句
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, insert, strlen((char*)insert),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	//位置绑定
	OCIBindByPos(m_stmthp, &bndhp1, m_errhp, 1, &nSID, 
		sizeof(nSID[0]), SQLT_INT, 0, 0, 0, 0, 0, OCI_DEFAULT);

	//名字绑定
	OCIBindByName(m_stmthp, &bndhp2, m_errhp, (text*)":name", -1, &szName, 
		sizeof(szName[0]), SQLT_STR, 0, 0, 0, 0, 0, OCI_DEFAULT);

	//执行批量插入
	status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 5, 0, NULL, NULL,
		OCI_BATCH_ERRORS | OCI_COMMIT_ON_SUCCESS);

	if (status && status != OCI_SUCCESS_WITH_INFO)
	{
		OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
		exit(-1);
	}
	
	//得到已经成功处理的记录行数
	OCIAttrGet(m_stmthp, OCI_HTYPE_STMT, &rows_processed, 0, OCI_ATTR_ROW_COUNT, m_errhp);
	
	printf("已经成功处理记录%d行\n", rows_processed);
	//如果还有记录未被插入
	if (5-rows_processed != 0)
	{
		int i;
		for (i=0; i<5-rows_processed; i++)
		{
			//获取错误信息
			OCIParamGet(m_errhp, OCI_HTYPE_ERROR, m_errhp2, (void**)&m_errhp1, i);
			//获取具体错误的记录行
			OCIAttrGet(m_errhp1, OCI_HTYPE_ERROR, &err_row, 0, OCI_ATTR_DML_ROW_OFFSET, m_errhp);
			//显示错误信息
			printf("第%d行记录插入发生错误,具体错误为:\n", err_row+1);
			CheckErr(m_errhp1, OCI_ERROR);

		}
		//提交成功处理的记录行
		if (OCITransCommit(m_svchp, m_errhp2, 0) != 0)
			CheckErr(m_errhp2, status);
	}


	Close();
	return OCI_SUCCESS;
}

//通过表名获取关系表的表结构信息
int CZdOciUtil::GetRelateTableInfo()
{
	
	//OCIEnv *m_envhp;			// 环境句柄
	//OCIServer *m_srvhp;		//服务器句柄
	//OCIError *m_errhp;		//错误句柄
	//OCISession *m_usrhp;		//用户会话句柄
	//OCISvcCtx *m_svchp;		//服务上下文句柄
	

	OCIDescribe* dschp;		//描述句柄
	OCIParam	*parmhp = NULL;	//参数句柄
    OCIParam	*collsthp = NULL;	//列列句柄
	OCIParam	*colhp = NULL;		//列句柄

	OCIBind* bndhp1 = NULL;
	OCIBind* bndhp2 = NULL;

	sb4 status = 0;
	
	ub4 numcols = 0; //列个数
	ub2 dtype; //数据类型
	ub2 colsize = (ub2)0;  //列长度
	sb1 colscale = (sb1)0; //列小数位数
	ub1 colprecision = (ub1)0; //列精度
	char szCol[128]; //列名缓存
	text *col_name; //列名
	ub4 col_name_len; //列长度
	char szTypeName[64] = {0};
	
	//表名
	text* tablename = (text*) "xmbustest.student";
	ub1 ind; //列是否允许为空


	//连接上数据库
	if( ConnectDB() != 0 ) return -1;


	////使用对象模式来创建环境句柄
	//OCIEnvCreate(&m_envhp, OCI_OBJECT, (dvoid *)0,
	//	0, 0, 0, (size_t) 0, (dvoid **)0);
	////分配服务器句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_srvhp,
	//	OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	////分配错误句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);

	////创建服务器上下文句柄,"orcl"为建立连接的数据库名
	//if (OCIServerAttach (m_srvhp, m_errhp, (text *)m_DBName,
	//	strlen ((char*)m_DBName), OCI_DEFAULT) == OCI_SUCCESS)
	//	printf("\n已经成功连上数据库orcl\n");
	//else //终止程序
	//{
	//	printf("\n数据库名字不对，连接数据库失败!\n");
	//	return -1;
	//}

	//建立用户会话
	//if(BeginOciSession() != 0) return -1;

	//分配服务器上下文句柄
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp,
		OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	//设置服务器上下文句柄的服务器句柄属性
	OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	//分配用户会话句柄
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp,
		OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	//为用户会话句柄设置用户名和密码属性
	OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
		(dvoid *)m_UserName, (ub4)strlen((char*)m_UserName),
		OCI_ATTR_USERNAME, m_errhp);
	OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
		(dvoid *)m_Password, (ub4)strlen((char*)m_Password),
		OCI_ATTR_PASSWORD, m_errhp);

	//申请描述句柄
	OCIHandleAlloc((dvoid*)m_envhp, (dvoid**)&dschp, OCI_HTYPE_DESCRIBE, 0, (void**)0);
	if (OCISessionBegin ( m_svchp, m_errhp, m_usrhp,
		OCI_CRED_RDBMS, OCI_DEFAULT) == OCI_SUCCESS)
	{
		printf("成功建立用户会话!\n");
	}
	else
	{
		printf("建立用户会话失败!\n");
		return -1;
	}

	//在服务器上下文环境中设置用户会话属性
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	
	
	//获取关系表的表结构信息
	printf("获取关系表ocitest.student的表结构信息\n");

	CheckErr(m_errhp, OCIDescribeAny(m_svchp, m_errhp, (dvoid*)tablename, 
		(ub4)strlen((char*)tablename), OCI_OTYPE_NAME, OCI_DEFAULT, 
		OCI_PTYPE_TABLE, dschp));

	//获取参数描述
	CheckErr(m_errhp, OCIAttrGet(dschp, OCI_HTYPE_DESCRIBE, &parmhp, 
		0, OCI_ATTR_PARAM, m_errhp));


	//获取表的列(字段)个数
	CheckErr(m_errhp, OCIAttrGet(parmhp, OCI_DTYPE_PARAM, &numcols,
		0, OCI_ATTR_NUM_COLS, m_errhp));

	//获取列列表
	CheckErr(m_errhp, OCIAttrGet(parmhp, OCI_DTYPE_PARAM, &collsthp,
		0, OCI_ATTR_LIST_COLUMNS, m_errhp));

	printf("名称             空?      类型             列长度   列精度   小数位数\n");
	printf("---------------- -------- ---------------- -------- -------- --------\n");
	//列枚举,注意,它是以1为序
	for (ub4 i=1; i<=numcols; i++)
	{
		//获取一个字段的描述参数
		CheckErr(m_errhp, OCIParamGet(collsthp, OCI_DTYPE_PARAM, m_errhp, (void**)&colhp, i));

		//分别获取列名、是否为空、列数据类型、列长度、列精度、列小数点位数
		CheckErr(m_errhp, OCIAttrGet(colhp, OCI_DTYPE_PARAM, &col_name, &col_name_len, 
			OCI_ATTR_NAME, m_errhp));
		memcpy(szCol, col_name, col_name_len);
		szCol[col_name_len] = '\0'; //串强制以'\0'结束

		CheckErr(m_errhp, OCIAttrGet(colhp, OCI_DTYPE_PARAM, &ind, 0, 
			OCI_ATTR_IS_NULL, m_errhp));

		CheckErr(m_errhp, OCIAttrGet(colhp, OCI_DTYPE_PARAM, &dtype, 0, 
			OCI_ATTR_DATA_TYPE, m_errhp));

		CheckErr(m_errhp, OCIAttrGet(colhp, OCI_DTYPE_PARAM, &colsize, 0, 
			OCI_ATTR_DATA_SIZE, m_errhp));

		CheckErr(m_errhp, OCIAttrGet(colhp, OCI_DTYPE_PARAM, &colprecision, 0, 
			OCI_ATTR_PRECISION, m_errhp));

		CheckErr(m_errhp, OCIAttrGet(colhp, OCI_DTYPE_PARAM, &colscale, 0, 
			OCI_ATTR_SCALE, m_errhp));
		
		//输出相关信息
		printf("%-17s", szCol);
		if (ind == 0) { printf("NOT NULL "); }
		else { printf("         "); }
		printf("%-17s", GetType(dtype,szTypeName));
		printf("%-9d", colsize);
		printf("%-9d", colprecision);
		printf("%-9d\n", colscale);
	}

	Close();
	return OCI_SUCCESS;
}

//读写LOB型数据
int CZdOciUtil::ReadLobData()				
{
	
	//OCIEnv *m_envhp; // 环境句柄
	//OCIServer *m_srvhp; //服务器句柄
	//OCIError *m_errhp; //错误句柄
	//OCISession *m_usrhp; //用户会话句柄
	//OCISvcCtx *m_svchp; //服务上下文句柄
	//OCIStmt* m_stmthp; //语句句柄
	OCILobLocator* blob; //LOB定位符
	
	ub4 status;
	ub4 len = 0;
	text* insert1 = (text*)"INSERT INTO testblob VALUES(1, empty_blob())";
	text* update1 = (text*)"SELECT vb FROM testblob WHERE id=1 FOR UPDATE";
	text* select1 = (text*)"SELECT vb FROM testblob WHERE id=1";
	text* select2 = (text*)"select Photo from tab_photos where PhotoID = 6";
	char szblob[33]="我是中国人, 我爱中国";
	ub4 nLen = sizeof(szblob);
	char* szV = NULL;

	OCIBind* bndhp1 = NULL;
	OCIDefine* defhp1 = NULL;
	
	//连接上数据库
	if( ConnectDB() != 0 ) return -1;
	////使用线程和对象模式来创建环境句柄
	//OCIEnvCreate(&m_envhp, OCI_THREADED|OCI_OBJECT, (dvoid *)0,
	//	0, 0, 0, (size_t) 0, (dvoid **)0);
	////分配服务器句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_srvhp,
	//	OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	////分配错误句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);

	////创建服务器上下文句柄,"orcl"为建立连接的数据库名
	//if (OCIServerAttach (m_srvhp, m_errhp, (text *)m_DBName,
	//	strlen ((char*)m_DBName), OCI_DEFAULT) == OCI_SUCCESS)
	//	printf("\n已经成功连上数据库orcl\n");
	//else //终止程序
	//{
	//	printf("\n数据库名字不对，连接数据库失败!\n");
	//	return -1;
	//}

	////分配服务器上下文句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp,
	//	OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	////设置服务器上下文句柄的服务器句柄属性
	//OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
	//	(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	////分配用户会话句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp,
	//	OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	////为用户会话句柄设置用户名和密码属性
	//OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
	//	(dvoid *)m_UserName, (ub4)strlen((char*)m_UserName),
	//	OCI_ATTR_USERNAME, m_errhp);
	//OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
	//	(dvoid *)m_Password, (ub4)strlen((char*)m_Password),
	//	OCI_ATTR_PASSWORD, m_errhp);
	//if (OCISessionBegin ( m_svchp, m_errhp, m_usrhp,
	//	OCI_CRED_RDBMS, OCI_DEFAULT) == OCI_SUCCESS)
	//{
	//	printf("成功建立用户会话!\n");
	//}
	//else
	//{
	//	printf("建立用户会话失败!\n");
	//	return -1;
	//}

	//建立用户会话
	if(BeginOciSession() != 0) return -1;

	//在服务器上下文环境中设置用户会话属性
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	
	//分配LOB定位符句柄
	OCIDescriptorAlloc(m_envhp, (void**)&blob, OCI_DTYPE_LOB, 0, (dvoid**)0);

	//分配语句句柄
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	//准备第一个SQL语句
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, insert1, strlen((char*)insert1),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	/////////////////////////////
	////插入第一条记录
	//status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
	//	OCI_DEFAULT);

	//if (status && status != OCI_SUCCESS_WITH_INFO)
	//{
	//	OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
	//	return -1;
	//}
	//printf("插入记录完成\n");
	//printf("开始更新blob字段值\n");

	////////////////////////////////
	////准备更新blob
	//CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, update1, strlen((char*)update1),
	//	OCI_NTV_SYNTAX, OCI_DEFAULT));

	//CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp1, m_errhp, 1, &blob, -1,
	//	SQLT_BLOB, 0,0, 0, OCI_DEFAULT));
	////准备写入数据
	//status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
	//	OCI_DEFAULT);
	//if (status && status != OCI_SUCCESS_WITH_INFO)
	//{
	//	OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
	//	return -1;
	//}
	//OCILobOpen(m_svchp, m_errhp, blob, OCI_LOB_READWRITE);
	//OCILobGetLength(m_svchp, m_errhp, blob, &len);
	//printf("写入前, blob长度=%d\n", len);
	//CheckErr(m_errhp, OCILobWrite(m_svchp, m_errhp, blob, &nLen, 1, szblob, sizeof(szblob), 
	//	OCI_ONE_PIECE, 0, 0, 0, SQLCS_IMPLICIT));
	//OCILobGetLength(m_svchp, m_errhp, blob, &len);
	//printf("写入后, blob长度=%d\n", len);
	//OCILobClose(m_svchp, m_errhp, blob);

	//准备读取blob字段的值
	printf("开始读取blob字段内容\n");
	//CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select1, strlen((char*)select1),
	//	OCI_NTV_SYNTAX, OCI_DEFAULT));
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select2, strlen((char*)select2),
		OCI_NTV_SYNTAX, OCI_DEFAULT));
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp1, m_errhp, 1, &blob, -1,
		SQLT_BLOB, 0,0, 0, OCI_DEFAULT));
	//准备读出数据
	status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
		OCI_DEFAULT);
	if (status && status != OCI_SUCCESS_WITH_INFO)
	{
		OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
		return -1;
	}
	OCILobOpen(m_svchp, m_errhp, blob, OCI_LOB_READONLY);
	OCILobGetLength(m_svchp, m_errhp, blob, &len);
	if (len > 0) szV = new char[len];
	CheckErr(m_errhp, OCILobRead(m_svchp, m_errhp, blob, &nLen, 1, szV, len, 
		OCI_ONE_PIECE, 0, 0, SQLCS_IMPLICIT));
	printf("blob内容为: %s\n", szV);
	OCILobClose(m_svchp, m_errhp, blob);
	if (szV) delete[] szV;
	//释放定位符空间
	OCIDescriptorFree(blob, OCI_DTYPE_LOB);
	
	Close();

	return OCI_SUCCESS;
}

//本地简单事务处理
int CZdOciUtil::LocalTransProc()
{
	
	//OCIEnv *m_envhp;		//环境句柄
	//OCIServer *m_srvhp;		//服务器句柄
	//OCIError *m_errhp;		//错误句柄
	//OCIError *m_errhp1;		//错误句柄
	//OCIError *m_errhp2;		//错误句柄
	//OCISession *m_usrhp;	//用户会话句柄
	//OCISvcCtx *m_svchp;		//服务上下文句柄
	//OCIStmt* m_stmthp;		//语句句柄

	OCIBind* bndhp1 = NULL;
	OCIBind* bndhp2 = NULL;

	int	nSID[5] = {1, 2, 1, 1, 3}; //学号
	char szName[5][32] = {"Bob", "Ying", "HeXiong", "Jenny", "Mary" }; //姓名
	sb4 nNameLen = 32;
	sb4 status = 0;
	//DML(INSERT)操作的SQL语句
	text* insert1 = (text*) "INSERT INTO STUDENT(id, name, age)"
			" VALUES(100, '李四', 25)";

	text* insert2 = (text*) "INSERT INTO STUDENT(id, name, age)"
			" VALUES(101, '张三', 23)";

	////使用线程和对象模式来创建环境句柄
	//OCIEnvCreate(&m_envhp, OCI_THREADED|OCI_OBJECT, (dvoid *)0,
	//	0, 0, 0, (size_t) 0, (dvoid **)0);
	////分配服务器句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_srvhp,
	//	OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	////分配错误句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp1,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp2,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);

	////创建服务器上下文句柄,"orcl"为建立连接的数据库名
	//if (OCIServerAttach (m_srvhp, m_errhp, (text *)m_DBName,
	//	strlen ((char*)m_DBName), OCI_DEFAULT) == OCI_SUCCESS)
	//	printf("\n已经成功连上数据库orcl\n");
	//else //终止程序
	//{
	//	printf("\n数据库名字不对，连接数据库失败!\n");
	//	return -1;
	//}
	////分配服务器上下文句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp,
	//	OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	////设置服务器上下文句柄的服务器句柄属性
	//OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
	//	(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	////分配用户会话句柄
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp,
	//	OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	////为用户会话句柄设置用户名和密码属性
	//OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
	//	(dvoid *)m_UserName, (ub4)strlen((char*)m_UserName),
	//	OCI_ATTR_USERNAME, m_errhp);
	//OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
	//	(dvoid *)m_Password, (ub4)strlen((char*)m_Password),
	//	OCI_ATTR_PASSWORD, m_errhp);
	//if (OCISessionBegin ( m_svchp, m_errhp, m_usrhp,
	//	OCI_CRED_RDBMS, OCI_DEFAULT) == OCI_SUCCESS)
	//{
	//	printf("成功建立用户会话!\n");
	//}
	//else
	//{
	//	printf("建立用户会话失败!\n");
	//	return -1;
	//}

	//连接上数据库
	if( ConnectDB() != 0 ) return -1;
	//建立用户会话
	if(BeginOciSession() != 0) return -1;

	//在服务器上下文环境中设置用户会话属性
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//分配语句句柄
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	//准备第一个SQL语句
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, insert1, strlen((char*)insert1),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	//插入第一条记录
	status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
		OCI_DEFAULT);

	if (status && status != OCI_SUCCESS_WITH_INFO)
	{
		OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
		return -1;
	}
	printf("插入第一条记录完成\n");

	//提交结果
	OCITransCommit(m_svchp, m_errhp, (ub4) 0);
	printf("提交第一条记录的插入操作\n");
	//准备第二个SQL语句
	if(!CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, insert2, strlen((char*)insert1),
		OCI_NTV_SYNTAX, OCI_DEFAULT)))
	{
		//

		return -101;
	}

	status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
		OCI_DEFAULT);

	if (status && status != OCI_SUCCESS_WITH_INFO)
	{
		OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
		return -1;
	}
	printf("插入第二条记录完成\n");
	OCITransRollback(m_svchp, m_errhp, (ub4) 0);
	printf("回滚第二条记录的插入操作\n");
	
	Close();

	return OCI_SUCCESS;
}

//获取错误诊断信息
//@param   : pErr,错误句柄
//           lStatus, 状态码信息
//@return  : 0: 失败，出错
//			 1: 成功返回
int CZdOciUtil::CheckErr(OCIError *pErr, sword lStatus)
{
    int		m_s_nErrCode = 0;
	char	m_s_szErr[512];
    
    switch (lStatus)
    {
    case OCI_SUCCESS:
        strcpy(m_s_szErr,"OCI_SUCCESS");    
        break;
    case OCI_SUCCESS_WITH_INFO:
        strcpy(m_s_szErr, "OCI_SUCCESS_WITH_INFO");
		printf("OCI Error: %s\n", m_s_szErr);
        break;
    case OCI_ERROR:
        OCIErrorGet((dvoid *)pErr, (ub4)1, (text *)NULL, &m_s_nErrCode, 
            (unsigned char*)m_s_szErr, (ub4)sizeof(m_s_szErr), OCI_HTYPE_ERROR);
		printf("OCI Error: %s\n", m_s_szErr);
		if(m_s_nErrCode == 3114 
			|| m_s_nErrCode == 12571)//数据库连接失效
		{
			//重新连接数据库
			Close();
			ConnectDB();
		}
        break;
    case OCI_NEED_DATA:
        strcpy(m_s_szErr, "OCI_NEED_DATA");
		printf("OCI Error: %s\n", m_s_szErr);
        break;
    case OCI_NO_DATA:
        strcpy(m_s_szErr, "OCI_NO_DATA");
		printf("OCI Error: %s\n", m_s_szErr);
        break;
    case OCI_INVALID_HANDLE:
        strcpy(m_s_szErr, "OCI_INVALID_HANDLE");
		printf("OCI Error: %s\n", m_s_szErr);
        break;
    case OCI_STILL_EXECUTING:
        strcpy(m_s_szErr, "OCI_STILL_EXECUTING");
		printf("OCI Error: %s\n", m_s_szErr);
        break;
    case OCI_CONTINUE:
        strcpy(m_s_szErr, "OCI_CONTINUE");
		printf("OCI Error: %s\n", m_s_szErr);
        break;
    default:
        break;
    }

    if (lStatus != OCI_SUCCESS && lStatus != OCI_SUCCESS_WITH_INFO)
    {
        return 0;  //确实有错误
    } 
    else 
    {
        return 1;  //没有检查到错误
    }

}

//获取类型nType对应的Oracle内部类型表示
//@param   : nType, 列类型的码值
//@return  : 列类型
char* CZdOciUtil::GetType(ub2 nType, char *pTypeName)
{
	const char* pName = NULL; //pColName;
	switch(nType)
	{
	case SQLT_AFC:
	case SQLT_VCS:
	case SQLT_CHR:
		pName = "VARCHAR2";
		break;
	case SQLT_DAT:
		pName = "DATE";
		break;
	case SQLT_INT:
		pName = "INTEGER";
		break;
	case SQLT_UIN:
		pName = "INTEGER";
		break;
	case SQLT_FLT:
		pName = "FLOAT";
		break;
	case SQLT_BLOB:
	case SQLT_BIN:
		pName = "BLOB";
		break;
	case SQLT_NUM:
		pName = "NUMBER";
		break;
	default:
		pName = "UNSUPPORTED";
		break;
	}
	strcpy(pTypeName, pName);
	return pTypeName;
}
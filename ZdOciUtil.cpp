////////////////////////////////////
//* Filename: ZdOciUtil.h
//* Abstract: OCI�ӿڷ�װ(oracle)
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
	m_envhp = NULL; // �������
	m_srvhp = NULL; //���������
	m_errhp = NULL; //������
	m_errhp1 = NULL; //������
	m_errhp2 = NULL; //������
	m_usrhp = NULL; //�û��Ự���
	m_svchp = NULL; //���������ľ��
	m_stmthp = NULL; //�����

	m_IsConnected  = false; //�Ƿ������ӵ�DB
	m_ExecCount = 0;
}

int CZdOciUtil::ConnectDB()
{
	if(m_IsConnected) return OCI_SUCCESS;

	//ʹ���̺߳Ͷ���ģʽ�������������
	OCIEnvCreate(&m_envhp, OCI_THREADED|OCI_OBJECT, (dvoid *)0,
		0, 0, 0, (size_t) 0, (dvoid **)0);
	//������������
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_srvhp,
		OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	//���������
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp,
		OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp1,
		OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp2,
		OCI_HTYPE_ERROR, 0, (dvoid **) 0);

	//���������������ľ��,"orcl"Ϊ�������ӵ����ݿ���
	if (OCIServerAttach (m_srvhp, m_errhp, (text *)m_DBName,
		strlen ((char*)m_DBName), OCI_DEFAULT) == OCI_SUCCESS)
	{
		//m_IsConnected = true;
#ifdef _DEBUG
		printf("\n�Ѿ��ɹ��������ݿ�%s\n", m_DBName);
#endif
	}
	else 
	{
		m_IsConnected = false;
#ifdef _DEBUG
		printf("\n�������ݿ�(%s)ʧ��!\n", m_DBName);
#endif
		return -1;
	}

	//��������������ľ��
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp,
		OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	//���÷����������ľ���ķ������������
	OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	//�����û��Ự���
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp,
		OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	//Ϊ�û��Ự��������û�������������
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
		printf("�ɹ������û��Ự!\n");
#endif		
	}
	else
	{
		m_IsConnected = false;
#ifdef _DEBUG
		printf("�����û��Ựʧ��!\n");
#endif		
		return -1;
	}

	m_IsConnected = true;

	//�ڷ����������Ļ����������û��Ự����
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//���������
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));

	return OCI_SUCCESS;
}

int CZdOciUtil::BeginOciSession()
{
	if(m_IsConnected) return OCI_SUCCESS;

	//��������������ľ��
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp, 
		OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	//���÷����������ľ���ķ������������
	OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX, 
		(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	//�����û��Ự���
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp, 
		OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	//Ϊ�û��Ự��������û�������������
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
		printf("�ɹ������û��Ự!\n");
#endif		
	}
	else
	{
		m_IsConnected =false;
#ifdef _DEBUG
		printf("�����û��Ựʧ��!\n");
#endif		
		return -1;
	}

	return OCI_SUCCESS;
}

int CZdOciUtil::EndOciSession()
{
	if(!m_IsConnected) return OCI_SUCCESS;

	//�����Ự
	OCISessionEnd(m_svchp, m_errhp, m_usrhp, OCI_DEFAULT);
	
	return OCI_SUCCESS;
}

int CZdOciUtil::DisconnectDB()
{
	if(!m_IsConnected) return OCI_SUCCESS;

	EndOciSession();

	//�Ͽ�����
	OCIServerDetach(m_srvhp, m_errhp, OCI_DEFAULT);
	//�ͷŻ������
	OCIHandleFree((void*)m_envhp, OCI_HTYPE_ENV);

	m_IsConnected = false;
	m_ExecCount = 0;
	return OCI_SUCCESS;
}

int CZdOciUtil::Close()
{
#ifdef _DEBUG
	printf("\n�����Ự�����ݿ�����!\n");
#endif
	//EndOciSession();
	DisconnectDB();
	return OCI_SUCCESS;
}

int CZdOciUtil::DefineByPos(ub4 uPos, ub2 uDataType, void* pVal, sb4 val_size)
{
	OCIDefine* defhp;
	//����ɹ�������OCI_SUCCESS = 0
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
	//���û�����ݣ�����ֵΪOCI_NO_DATA�� ��������ݣ�����ֵΪOCI_SUCCESS
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

	//�ڷ����������Ļ����������û��Ự����
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);

	//���������	del for test
	//CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));

	//����Ԥȡ����Ϊ10
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
	if(nstmt_type == OCI_STMT_SELECT) //select��ѯ
	{
		//��ȡ���
		if(!CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 0, 0, NULL, NULL,
			OCI_DEFAULT)))
		{
			//printf("OCI Exec Sql failed. %s \r", pSql);
			nRet = -105;
			goto lb_exit;
		}

	}
	else if(nstmt_type > OCI_STMT_SELECT) //DML���
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
	//�õ��Ѿ��ɹ�����ļ�¼����
	OCIAttrGet(m_stmthp, OCI_HTYPE_STMT, &rows, 0, OCI_ATTR_ROW_COUNT, m_errhp); //OCI_ATTR_ROWS_RETURNED //OCI_ATTR_ROWS_FETCHED
	return rows; 
}


//int CZdOciUtil::ExecStoredProc(char *pSql)
//{
//	return OCI_SUCCESS;
//}



int CZdOciUtil::TestQuery()
{
	////���������������ڶ���������
	//OCIDefine* defhp1;
	//OCIDefine* defhp2;
	//OCIDefine* defhp3;
	//OCIDefine* defhp4;

	//�������
	int	nID = 0;
	char szName[128] = {0};
	int nAge = 0;
	char szAddress[256];
	sb2 indAddr = 0; //address�ֶε�ָʾ��

	//DML(INSERT)������SQL���
	char *sql =  "SELECT id, name, age, address FROM student";
	

	//���������ݿ�
	if( ConnectDB() != OCI_SUCCESS ) return OCI_ERROR;

	int ret = SQLExecute(sql);
	if(ret != OCI_SUCCESS)
	{
		return ret;
	}
	//�������
	DefineByPos_INT(1, &nID);
	DefineByPos_STR(2, szName, 33);
	DefineByPos_INT(3, &nAge);
	DefineByPos_STR(4, szAddress, 129);
		
	printf("\n��ѯ���Ϊ: \n");
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
//��ʽ����һ��ѡ���б�
int CZdOciUtil::SelectXing(char *pSql)	
{
	//DML(INSERT)������SQL���
	static text* select = (text*) "SELECT * FROM student";

	sb4 status = 0;

	OCIParam *paramhp; //�������ָ��
	ub4 counter;
	ub2 dtype; //��������
	char szCol[128]={0}; //��������
	char  szTypeName[64] ={0};//��������
	text *col_name; //����
	ub4 col_name_len; //�г���
	sb4 parm_status; //״̬��
	
	//���������ݿ�
	if( ConnectDB() != 0 ) return -1;

	//�����û��Ự
	if(BeginOciSession() != 0) return -1;

	//�ڷ����������Ļ����������û��Ự����
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//���������
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	
	//׼��SQL���
	if(!CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select, strlen((char*)select),
		OCI_NTV_SYNTAX, OCI_DEFAULT)))
		return -1;

	//��ȡѡ����Ԫ��Ϣ
	printf("��ȡѡ����Ԫ��Ϣ\n");
	CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
		OCI_DESCRIBE_ONLY));

	counter = 1;
	parm_status = OCIParamGet(m_stmthp, OCI_HTYPE_STMT, m_errhp, (void**)&paramhp,
		(ub4) counter);
	memset(szCol, 0, sizeof(szCol));

	//ѭ����ȡ
	while (parm_status==OCI_SUCCESS) 
	{
		//��ȡ������������
		CheckErr(m_errhp, OCIAttrGet((dvoid*) paramhp, (ub4) OCI_DTYPE_PARAM,
			(dvoid*) &dtype,(ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,(OCIError *) m_errhp ));
		//��ȡ��������
		CheckErr(m_errhp, OCIAttrGet((dvoid*) paramhp, (ub4) OCI_DTYPE_PARAM,
			(dvoid**) &col_name,(ub4 *) &col_name_len, (ub4) OCI_ATTR_NAME,
			(OCIError *) m_errhp ));
		memcpy(szCol, col_name, col_name_len);
		printf("%d\t����:%s\t\t��������:%s\n", counter, szCol, GetType(dtype, szTypeName));
		memset(szCol, 0, sizeof(szCol));
		counter++;
		parm_status = OCIParamGet(m_stmthp, OCI_HTYPE_STMT, m_errhp, (dvoid**)&paramhp,
			(ub4) counter);
	}

	printf("���õ�ѡ���б�����Ϊ:%d\n", counter-1);

	Close();
	return OCI_SUCCESS;
}




//��ʽ����һ��ѡ���б�
int CZdOciUtil::EnumFields()
{
	sb4 status = 0;

	OCIParam *paramhp; //�������ָ��
	ub4 counter;
	ub4 colcount = 0;  //����
	ub2 dtype; //��������
	char szCol[128]; //��������
	char szTypeName[64] = {0};
	text *col_name; //����
	ub4 col_name_len; //�г���
	sb4 parm_status; //״̬��

	static text* select = (text*) "SELECT * FROM student";

	//���������ݿ�
	if( ConnectDB() != 0 ) return -1;
	//�����û��Ự
	if(BeginOciSession() != 0) return -1;

	//�ڷ����������Ļ����������û��Ự����
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//���������
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	//׼��SQL���
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select, strlen((char*)select),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	//��ȡѡ���б��Ԫ��Ϣ
	printf("��ȡѡ����Ԫ��Ϣ\n");
	CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
		OCI_DESCRIBE_ONLY));

	//��ȡѡ���б������,������colcount��
	CheckErr(m_errhp, OCIAttrGet(m_stmthp, OCI_HTYPE_STMT, &colcount, 0, 
		OCI_ATTR_PARAM_COUNT, m_errhp));
	printf("ѡ���б��й���%d��\n", colcount);
	counter = 1;
	
	//ѭ����ȡ���е�Ԫ��Ϣ
	for (counter = 1; counter<=colcount; counter++)
	{	
		//��ȡ������������
		parm_status = OCIParamGet(m_stmthp, OCI_HTYPE_STMT, m_errhp, (dvoid**)&paramhp,
			(ub4) counter);
		CheckErr(m_errhp, OCIAttrGet((dvoid*) paramhp, (ub4) OCI_DTYPE_PARAM,
			(dvoid*) &dtype,(ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,(OCIError *) m_errhp ));
		//��ȡ��������
		CheckErr(m_errhp, OCIAttrGet((dvoid*) paramhp, (ub4) OCI_DTYPE_PARAM,
			(dvoid**) &col_name,(ub4 *) &col_name_len, (ub4) OCI_ATTR_NAME,
			(OCIError *) m_errhp ));
		memcpy(szCol, col_name, col_name_len);
		szCol[col_name_len] = 0;
		printf("%d\t����:%-32s��������:%-32s\n", counter, szCol, GetType(dtype, szTypeName));
	}

	Close();

	return OCI_SUCCESS;
}

int CZdOciUtil::SelectRecords()
{

	//���������������ڶ���������
	OCIDefine* defhp1;
	OCIDefine* defhp2;
	OCIDefine* defhp3;
	OCIDefine* defhp4;

	//DML(INSERT)������SQL���
	text* select = (text*) "SELECT id, name, age, address FROM student";
	//text* select = (text*) "select PhotoID,Dev_ID,Photosize, Photo from tab_photos where PhotoID < 9";
	text * cmd = (text*)" begin \r v2_delete_dev('0001'); \r end;";
	cmd = select;//for test select 
	//�������
	int	nID = 0;
	char szName[128] = {0};
	int nAge = 0;
	char szAddress[256];
	sb2 indAddr = 0; //address�ֶε�ָʾ��

	sb4 status = 0;

	ub4 colcount = 0;  //����

	//���������ݿ�
	if( ConnectDB() != 0 ) return -1;


	//����Ԥȡ����Ϊ10
	int fetchCount = 2;
	CheckErr(m_errhp, OCIAttrSet(m_stmthp, OCI_HTYPE_STMT, (void*)&fetchCount, 4, OCI_ATTR_PREFETCH_ROWS, m_errhp));
	//׼��SQL���
	//CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select, strlen((char*)select),
	//	OCI_NTV_SYNTAX, OCI_DEFAULT));

	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, cmd, strlen((char*)cmd),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	//�������
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp1, m_errhp, 1, &nID, sizeof(nID),
		SQLT_INT, 0, 0, 0, OCI_DEFAULT));
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp2, m_errhp, 2, szName, 33,
		SQLT_STR, 0, 0, 0, OCI_DEFAULT));
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp3, m_errhp, 3, &nAge, sizeof(nAge),
		SQLT_INT, 0, 0, 0, OCI_DEFAULT));
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp4, m_errhp, 4, szAddress, 129,
		SQLT_STR, &indAddr, 0, 0, OCI_DEFAULT));


	//��ȡ���
	if(!CheckErr(m_errhp, OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 0, 0, NULL, NULL,
		OCI_DEFAULT)))
	{
		printf("Exec Sql failed.");
		return OCI_SUCCESS;
	}
	printf("\n��ѯ���Ϊ: \n");
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

//�����������
int CZdOciUtil::BatchInsert()
{
	//OCIEnv *m_envhp; // �������
	//OCIServer *m_srvhp; //���������
	//OCIError *m_errhp; //������
	//OCIError *m_errhp1; //������
	//OCIError *m_errhp2; //������
	//OCISession *m_usrhp; //�û��Ự���
	//OCISvcCtx *m_svchp; //���������ľ��
	//OCIStmt* m_stmthp; //�����

	OCIBind* bndhp1 = NULL;
	OCIBind* bndhp2 = NULL;
	
	//DML(INSERT)������SQL���
	static text* insert = (text*) "INSERT INTO STUDENT(id, name, age)"
			" VALUES(:1, :name, 22)";
	int	nSID[5] = {1, 2, 1, 1, 3}; //ѧ��
	char szName[5][32] = {"Bob", "Ying", "HeXiong", "Jenny", "Mary" }; //����
	sb4 nNameLen = 32;
	sb4 status = 0;

	int	rows_processed = 0;
	int err_row = 0; //������

	//���������ݿ�
	if( ConnectDB() != 0 ) return -1;
	//�����û��Ự
	if(BeginOciSession() != 0) return -1;

	//�ڷ����������Ļ����������û��Ự����
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//���������
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	//׼��SQL���
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, insert, strlen((char*)insert),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	//λ�ð�
	OCIBindByPos(m_stmthp, &bndhp1, m_errhp, 1, &nSID, 
		sizeof(nSID[0]), SQLT_INT, 0, 0, 0, 0, 0, OCI_DEFAULT);

	//���ְ�
	OCIBindByName(m_stmthp, &bndhp2, m_errhp, (text*)":name", -1, &szName, 
		sizeof(szName[0]), SQLT_STR, 0, 0, 0, 0, 0, OCI_DEFAULT);

	//ִ����������
	status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 5, 0, NULL, NULL,
		OCI_BATCH_ERRORS | OCI_COMMIT_ON_SUCCESS);

	if (status && status != OCI_SUCCESS_WITH_INFO)
	{
		OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
		exit(-1);
	}
	
	//�õ��Ѿ��ɹ�����ļ�¼����
	OCIAttrGet(m_stmthp, OCI_HTYPE_STMT, &rows_processed, 0, OCI_ATTR_ROW_COUNT, m_errhp);
	
	printf("�Ѿ��ɹ������¼%d��\n", rows_processed);
	//������м�¼δ������
	if (5-rows_processed != 0)
	{
		int i;
		for (i=0; i<5-rows_processed; i++)
		{
			//��ȡ������Ϣ
			OCIParamGet(m_errhp, OCI_HTYPE_ERROR, m_errhp2, (void**)&m_errhp1, i);
			//��ȡ�������ļ�¼��
			OCIAttrGet(m_errhp1, OCI_HTYPE_ERROR, &err_row, 0, OCI_ATTR_DML_ROW_OFFSET, m_errhp);
			//��ʾ������Ϣ
			printf("��%d�м�¼���뷢������,�������Ϊ:\n", err_row+1);
			CheckErr(m_errhp1, OCI_ERROR);

		}
		//�ύ�ɹ�����ļ�¼��
		if (OCITransCommit(m_svchp, m_errhp2, 0) != 0)
			CheckErr(m_errhp2, status);
	}


	Close();
	return OCI_SUCCESS;
}

//ͨ��������ȡ��ϵ��ı�ṹ��Ϣ
int CZdOciUtil::GetRelateTableInfo()
{
	
	//OCIEnv *m_envhp;			// �������
	//OCIServer *m_srvhp;		//���������
	//OCIError *m_errhp;		//������
	//OCISession *m_usrhp;		//�û��Ự���
	//OCISvcCtx *m_svchp;		//���������ľ��
	

	OCIDescribe* dschp;		//�������
	OCIParam	*parmhp = NULL;	//�������
    OCIParam	*collsthp = NULL;	//���о��
	OCIParam	*colhp = NULL;		//�о��

	OCIBind* bndhp1 = NULL;
	OCIBind* bndhp2 = NULL;

	sb4 status = 0;
	
	ub4 numcols = 0; //�и���
	ub2 dtype; //��������
	ub2 colsize = (ub2)0;  //�г���
	sb1 colscale = (sb1)0; //��С��λ��
	ub1 colprecision = (ub1)0; //�о���
	char szCol[128]; //��������
	text *col_name; //����
	ub4 col_name_len; //�г���
	char szTypeName[64] = {0};
	
	//����
	text* tablename = (text*) "xmbustest.student";
	ub1 ind; //���Ƿ�����Ϊ��


	//���������ݿ�
	if( ConnectDB() != 0 ) return -1;


	////ʹ�ö���ģʽ�������������
	//OCIEnvCreate(&m_envhp, OCI_OBJECT, (dvoid *)0,
	//	0, 0, 0, (size_t) 0, (dvoid **)0);
	////������������
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_srvhp,
	//	OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	////���������
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);

	////���������������ľ��,"orcl"Ϊ�������ӵ����ݿ���
	//if (OCIServerAttach (m_srvhp, m_errhp, (text *)m_DBName,
	//	strlen ((char*)m_DBName), OCI_DEFAULT) == OCI_SUCCESS)
	//	printf("\n�Ѿ��ɹ��������ݿ�orcl\n");
	//else //��ֹ����
	//{
	//	printf("\n���ݿ����ֲ��ԣ��������ݿ�ʧ��!\n");
	//	return -1;
	//}

	//�����û��Ự
	//if(BeginOciSession() != 0) return -1;

	//��������������ľ��
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp,
		OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	//���÷����������ľ���ķ������������
	OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	//�����û��Ự���
	OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp,
		OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	//Ϊ�û��Ự��������û�������������
	OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
		(dvoid *)m_UserName, (ub4)strlen((char*)m_UserName),
		OCI_ATTR_USERNAME, m_errhp);
	OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
		(dvoid *)m_Password, (ub4)strlen((char*)m_Password),
		OCI_ATTR_PASSWORD, m_errhp);

	//�����������
	OCIHandleAlloc((dvoid*)m_envhp, (dvoid**)&dschp, OCI_HTYPE_DESCRIBE, 0, (void**)0);
	if (OCISessionBegin ( m_svchp, m_errhp, m_usrhp,
		OCI_CRED_RDBMS, OCI_DEFAULT) == OCI_SUCCESS)
	{
		printf("�ɹ������û��Ự!\n");
	}
	else
	{
		printf("�����û��Ựʧ��!\n");
		return -1;
	}

	//�ڷ����������Ļ����������û��Ự����
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	
	
	//��ȡ��ϵ��ı�ṹ��Ϣ
	printf("��ȡ��ϵ��ocitest.student�ı�ṹ��Ϣ\n");

	CheckErr(m_errhp, OCIDescribeAny(m_svchp, m_errhp, (dvoid*)tablename, 
		(ub4)strlen((char*)tablename), OCI_OTYPE_NAME, OCI_DEFAULT, 
		OCI_PTYPE_TABLE, dschp));

	//��ȡ��������
	CheckErr(m_errhp, OCIAttrGet(dschp, OCI_HTYPE_DESCRIBE, &parmhp, 
		0, OCI_ATTR_PARAM, m_errhp));


	//��ȡ�����(�ֶ�)����
	CheckErr(m_errhp, OCIAttrGet(parmhp, OCI_DTYPE_PARAM, &numcols,
		0, OCI_ATTR_NUM_COLS, m_errhp));

	//��ȡ���б�
	CheckErr(m_errhp, OCIAttrGet(parmhp, OCI_DTYPE_PARAM, &collsthp,
		0, OCI_ATTR_LIST_COLUMNS, m_errhp));

	printf("����             ��?      ����             �г���   �о���   С��λ��\n");
	printf("---------------- -------- ---------------- -------- -------- --------\n");
	//��ö��,ע��,������1Ϊ��
	for (ub4 i=1; i<=numcols; i++)
	{
		//��ȡһ���ֶε���������
		CheckErr(m_errhp, OCIParamGet(collsthp, OCI_DTYPE_PARAM, m_errhp, (void**)&colhp, i));

		//�ֱ��ȡ�������Ƿ�Ϊ�ա����������͡��г��ȡ��о��ȡ���С����λ��
		CheckErr(m_errhp, OCIAttrGet(colhp, OCI_DTYPE_PARAM, &col_name, &col_name_len, 
			OCI_ATTR_NAME, m_errhp));
		memcpy(szCol, col_name, col_name_len);
		szCol[col_name_len] = '\0'; //��ǿ����'\0'����

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
		
		//��������Ϣ
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

//��дLOB������
int CZdOciUtil::ReadLobData()				
{
	
	//OCIEnv *m_envhp; // �������
	//OCIServer *m_srvhp; //���������
	//OCIError *m_errhp; //������
	//OCISession *m_usrhp; //�û��Ự���
	//OCISvcCtx *m_svchp; //���������ľ��
	//OCIStmt* m_stmthp; //�����
	OCILobLocator* blob; //LOB��λ��
	
	ub4 status;
	ub4 len = 0;
	text* insert1 = (text*)"INSERT INTO testblob VALUES(1, empty_blob())";
	text* update1 = (text*)"SELECT vb FROM testblob WHERE id=1 FOR UPDATE";
	text* select1 = (text*)"SELECT vb FROM testblob WHERE id=1";
	text* select2 = (text*)"select Photo from tab_photos where PhotoID = 6";
	char szblob[33]="�����й���, �Ұ��й�";
	ub4 nLen = sizeof(szblob);
	char* szV = NULL;

	OCIBind* bndhp1 = NULL;
	OCIDefine* defhp1 = NULL;
	
	//���������ݿ�
	if( ConnectDB() != 0 ) return -1;
	////ʹ���̺߳Ͷ���ģʽ�������������
	//OCIEnvCreate(&m_envhp, OCI_THREADED|OCI_OBJECT, (dvoid *)0,
	//	0, 0, 0, (size_t) 0, (dvoid **)0);
	////������������
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_srvhp,
	//	OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	////���������
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);

	////���������������ľ��,"orcl"Ϊ�������ӵ����ݿ���
	//if (OCIServerAttach (m_srvhp, m_errhp, (text *)m_DBName,
	//	strlen ((char*)m_DBName), OCI_DEFAULT) == OCI_SUCCESS)
	//	printf("\n�Ѿ��ɹ��������ݿ�orcl\n");
	//else //��ֹ����
	//{
	//	printf("\n���ݿ����ֲ��ԣ��������ݿ�ʧ��!\n");
	//	return -1;
	//}

	////��������������ľ��
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp,
	//	OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	////���÷����������ľ���ķ������������
	//OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
	//	(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	////�����û��Ự���
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp,
	//	OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	////Ϊ�û��Ự��������û�������������
	//OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
	//	(dvoid *)m_UserName, (ub4)strlen((char*)m_UserName),
	//	OCI_ATTR_USERNAME, m_errhp);
	//OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
	//	(dvoid *)m_Password, (ub4)strlen((char*)m_Password),
	//	OCI_ATTR_PASSWORD, m_errhp);
	//if (OCISessionBegin ( m_svchp, m_errhp, m_usrhp,
	//	OCI_CRED_RDBMS, OCI_DEFAULT) == OCI_SUCCESS)
	//{
	//	printf("�ɹ������û��Ự!\n");
	//}
	//else
	//{
	//	printf("�����û��Ựʧ��!\n");
	//	return -1;
	//}

	//�����û��Ự
	if(BeginOciSession() != 0) return -1;

	//�ڷ����������Ļ����������û��Ự����
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	
	//����LOB��λ�����
	OCIDescriptorAlloc(m_envhp, (void**)&blob, OCI_DTYPE_LOB, 0, (dvoid**)0);

	//���������
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	//׼����һ��SQL���
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, insert1, strlen((char*)insert1),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	/////////////////////////////
	////�����һ����¼
	//status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
	//	OCI_DEFAULT);

	//if (status && status != OCI_SUCCESS_WITH_INFO)
	//{
	//	OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
	//	return -1;
	//}
	//printf("�����¼���\n");
	//printf("��ʼ����blob�ֶ�ֵ\n");

	////////////////////////////////
	////׼������blob
	//CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, update1, strlen((char*)update1),
	//	OCI_NTV_SYNTAX, OCI_DEFAULT));

	//CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp1, m_errhp, 1, &blob, -1,
	//	SQLT_BLOB, 0,0, 0, OCI_DEFAULT));
	////׼��д������
	//status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
	//	OCI_DEFAULT);
	//if (status && status != OCI_SUCCESS_WITH_INFO)
	//{
	//	OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
	//	return -1;
	//}
	//OCILobOpen(m_svchp, m_errhp, blob, OCI_LOB_READWRITE);
	//OCILobGetLength(m_svchp, m_errhp, blob, &len);
	//printf("д��ǰ, blob����=%d\n", len);
	//CheckErr(m_errhp, OCILobWrite(m_svchp, m_errhp, blob, &nLen, 1, szblob, sizeof(szblob), 
	//	OCI_ONE_PIECE, 0, 0, 0, SQLCS_IMPLICIT));
	//OCILobGetLength(m_svchp, m_errhp, blob, &len);
	//printf("д���, blob����=%d\n", len);
	//OCILobClose(m_svchp, m_errhp, blob);

	//׼����ȡblob�ֶε�ֵ
	printf("��ʼ��ȡblob�ֶ�����\n");
	//CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select1, strlen((char*)select1),
	//	OCI_NTV_SYNTAX, OCI_DEFAULT));
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, select2, strlen((char*)select2),
		OCI_NTV_SYNTAX, OCI_DEFAULT));
	CheckErr(m_errhp, OCIDefineByPos(m_stmthp, &defhp1, m_errhp, 1, &blob, -1,
		SQLT_BLOB, 0,0, 0, OCI_DEFAULT));
	//׼����������
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
	printf("blob����Ϊ: %s\n", szV);
	OCILobClose(m_svchp, m_errhp, blob);
	if (szV) delete[] szV;
	//�ͷŶ�λ���ռ�
	OCIDescriptorFree(blob, OCI_DTYPE_LOB);
	
	Close();

	return OCI_SUCCESS;
}

//���ؼ�������
int CZdOciUtil::LocalTransProc()
{
	
	//OCIEnv *m_envhp;		//�������
	//OCIServer *m_srvhp;		//���������
	//OCIError *m_errhp;		//������
	//OCIError *m_errhp1;		//������
	//OCIError *m_errhp2;		//������
	//OCISession *m_usrhp;	//�û��Ự���
	//OCISvcCtx *m_svchp;		//���������ľ��
	//OCIStmt* m_stmthp;		//�����

	OCIBind* bndhp1 = NULL;
	OCIBind* bndhp2 = NULL;

	int	nSID[5] = {1, 2, 1, 1, 3}; //ѧ��
	char szName[5][32] = {"Bob", "Ying", "HeXiong", "Jenny", "Mary" }; //����
	sb4 nNameLen = 32;
	sb4 status = 0;
	//DML(INSERT)������SQL���
	text* insert1 = (text*) "INSERT INTO STUDENT(id, name, age)"
			" VALUES(100, '����', 25)";

	text* insert2 = (text*) "INSERT INTO STUDENT(id, name, age)"
			" VALUES(101, '����', 23)";

	////ʹ���̺߳Ͷ���ģʽ�������������
	//OCIEnvCreate(&m_envhp, OCI_THREADED|OCI_OBJECT, (dvoid *)0,
	//	0, 0, 0, (size_t) 0, (dvoid **)0);
	////������������
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_srvhp,
	//	OCI_HTYPE_SERVER, 0, (dvoid **) 0);
	////���������
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp1,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_errhp2,
	//	OCI_HTYPE_ERROR, 0, (dvoid **) 0);

	////���������������ľ��,"orcl"Ϊ�������ӵ����ݿ���
	//if (OCIServerAttach (m_srvhp, m_errhp, (text *)m_DBName,
	//	strlen ((char*)m_DBName), OCI_DEFAULT) == OCI_SUCCESS)
	//	printf("\n�Ѿ��ɹ��������ݿ�orcl\n");
	//else //��ֹ����
	//{
	//	printf("\n���ݿ����ֲ��ԣ��������ݿ�ʧ��!\n");
	//	return -1;
	//}
	////��������������ľ��
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_svchp,
	//	OCI_HTYPE_SVCCTX, 0, (dvoid **) 0);
	////���÷����������ľ���ķ������������
	//OCIAttrSet ((dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
	//	(dvoid *)m_srvhp, (ub4) 0, OCI_ATTR_SERVER, m_errhp);
	////�����û��Ự���
	//OCIHandleAlloc ((dvoid *)m_envhp, (dvoid **)&m_usrhp,
	//	OCI_HTYPE_SESSION, 0, (dvoid **) 0);
	////Ϊ�û��Ự��������û�������������
	//OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
	//	(dvoid *)m_UserName, (ub4)strlen((char*)m_UserName),
	//	OCI_ATTR_USERNAME, m_errhp);
	//OCIAttrSet ((dvoid *)m_usrhp, OCI_HTYPE_SESSION,
	//	(dvoid *)m_Password, (ub4)strlen((char*)m_Password),
	//	OCI_ATTR_PASSWORD, m_errhp);
	//if (OCISessionBegin ( m_svchp, m_errhp, m_usrhp,
	//	OCI_CRED_RDBMS, OCI_DEFAULT) == OCI_SUCCESS)
	//{
	//	printf("�ɹ������û��Ự!\n");
	//}
	//else
	//{
	//	printf("�����û��Ựʧ��!\n");
	//	return -1;
	//}

	//���������ݿ�
	if( ConnectDB() != 0 ) return -1;
	//�����û��Ự
	if(BeginOciSession() != 0) return -1;

	//�ڷ����������Ļ����������û��Ự����
	OCIAttrSet ( (dvoid *)m_svchp, OCI_HTYPE_SVCCTX,
		(dvoid *)m_usrhp, (ub4) 0, OCI_ATTR_SESSION, m_errhp);
	//���������
	CheckErr(m_errhp, OCIHandleAlloc(m_envhp, (void**)&m_stmthp, OCI_HTYPE_STMT, 0, 0));
	//׼����һ��SQL���
	CheckErr(m_errhp, OCIStmtPrepare(m_stmthp, m_errhp, insert1, strlen((char*)insert1),
		OCI_NTV_SYNTAX, OCI_DEFAULT));

	//�����һ����¼
	status = OCIStmtExecute(m_svchp, m_stmthp, m_errhp, 1, 0, NULL, NULL,
		OCI_DEFAULT);

	if (status && status != OCI_SUCCESS_WITH_INFO)
	{
		OCIHandleFree((dvoid*)m_envhp, OCI_HTYPE_ENV);
		return -1;
	}
	printf("�����һ����¼���\n");

	//�ύ���
	OCITransCommit(m_svchp, m_errhp, (ub4) 0);
	printf("�ύ��һ����¼�Ĳ������\n");
	//׼���ڶ���SQL���
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
	printf("����ڶ�����¼���\n");
	OCITransRollback(m_svchp, m_errhp, (ub4) 0);
	printf("�ع��ڶ�����¼�Ĳ������\n");
	
	Close();

	return OCI_SUCCESS;
}

//��ȡ���������Ϣ
//@param   : pErr,������
//           lStatus, ״̬����Ϣ
//@return  : 0: ʧ�ܣ�����
//			 1: �ɹ�����
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
			|| m_s_nErrCode == 12571)//���ݿ�����ʧЧ
		{
			//�����������ݿ�
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
        return 0;  //ȷʵ�д���
    } 
    else 
    {
        return 1;  //û�м�鵽����
    }

}

//��ȡ����nType��Ӧ��Oracle�ڲ����ͱ�ʾ
//@param   : nType, �����͵���ֵ
//@return  : ������
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
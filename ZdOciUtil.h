////////////////////////////////////
//* Filename: ZdOciUtil.h
//* Abstract: OCI�ӿڷ�װ(oracle)
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
	char* GetType(ub2 nType, char *pTypeName);		//��ȡ������������


	int ReadLobData();				//��дLOB������

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
	int SelectXing(char *pSql);		//��ʽ����һ��ѡ���б�
	int EnumFields();				//��ʽ����һ��ѡ���б�
	int BatchInsert();				//������Ӽ�¼
	int SelectRecords();		//��ȡһ����ѯ�Ľ��
	int LocalTransProc();			//���ؼ�������
	int GetRelateTableInfo();		//ͨ��������ȡ��ϵ��ı�ṹ��Ϣ
	/// end of test ///

private:
	
	char m_DBName[60];		//���ݿ������
	char m_UserName[33];	//�û�
	char m_Password[33];	//����

	OCIEnv *m_envhp; // �������
	OCIServer *m_srvhp; //���������
	OCISession *m_usrhp; //�û��Ự���
	OCISvcCtx *m_svchp; //���������ľ��
	OCIStmt* m_stmthp; //�����
	OCIError *m_errhp; //������
	OCIError *m_errhp1; //������
	OCIError *m_errhp2; //������

	bool m_IsConnected; //�Ƿ������ӵ�DB
	int  m_ExecCount;
	
	char err_msg[512];
	int  ora_err;
};

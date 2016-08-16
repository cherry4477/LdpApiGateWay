/*

 */

#include "UserQueryCount.h"

MYSQL_SERVERINFO_S g_mySqlInfoCount;


CUserQueryCount::CUserQueryCount(const STATISTICSPRM_S& stStatisticsPrm,const MYSQL_SERVERINFO_S &mySqlInfo)
 : m_stStatisticsPrm(stStatisticsPrm)
{	
	struct stat statbuf;
	std::string strHiveLogDir;
	srand((int)time(0)+123456789);
	int randomInt = rand()%100000;

	m_RandomInt = randomInt;	
	char randomchar[5];
	memset(randomchar,0,5);
	sprintf(randomchar,"%d",m_RandomInt);


	// TODO Auto-generated constructor stub
	std::string statusDir= m_stStatisticsPrm.m_strStatisticsPath+"/"+BdxUserQueryCountGetDate();
	if(!m_clFile.FileBeExists(statusDir.c_str())) {
		m_clFile.FileCreatDir(statusDir.c_str());
	}
	
	//std::string strFileName = statusDir + "/"+m_stStatisticsPrm.m_strStatisticsFileName+"_"+ BdxUserQueryCountGetDate() + ".txt";
	std::string strFileName = statusDir + "/"+m_stStatisticsPrm.m_strStatisticsFileName+"_"+ BdxUserQueryCountGetDate()+"_"+ std::string(randomchar) + ".txt";
	m_pFile = fopen(strFileName.c_str(), "a");
	if (!stat(strFileName.c_str(),&statbuf))
	{
		if( statbuf.st_size == 0 )
		{
			BdxQueryCountWriteTitle();
		}

	}
	for(u_int i = 0; i < CUserQueryWorkThreads::m_vecReport.size(); ++i) 
	{
		m_stReport += CUserQueryWorkThreads::m_vecReport[i];
		CUserQueryWorkThreads::m_vecReport[i] = 0;
	}
	//memset(&m_stReport,0,sizeof(UESRQUERYRPORT_S));

	m_stMysqlServerInfoCount = new CMYSQL;
	sprintf(g_mySqlInfoCount.m_stMysqlLinkInfo.m_pchIp,"%s",mySqlInfo.m_stMysqlLinkInfo.m_pchIp);	
	g_mySqlInfoCount.m_stMysqlLinkInfo.m_uiPort = mySqlInfo.m_stMysqlLinkInfo.m_uiPort;	
	sprintf(g_mySqlInfoCount.pchUserName,"%s",mySqlInfo.pchUserName);	
	sprintf(g_mySqlInfoCount.pchPassWord,"%s",mySqlInfo.pchPassWord);	
	sprintf(g_mySqlInfoCount.pchDbName,"%s",mySqlInfo.pchDbName);
	m_stMysqlServerInfoCount->InitMysql(mySqlInfo.m_stMysqlLinkInfo.m_pchIp,mySqlInfo.m_stMysqlLinkInfo.m_uiPort,mySqlInfo.pchUserName,mySqlInfo.pchPassWord,mySqlInfo.pchDbName);



}

CUserQueryCount::~CUserQueryCount() {
	// TODO Auto-generated destructor stub
}
void CUserQueryCount::Core()
{

	std::string pchSqlPermissions ;//= "insert into rtb_hive_data";
	//char insertMysqlValue[2000];
	std::string insertMysqlValue;
	std::string createTable;

	std::string tableValue ="( id int(10) unsigned NOT NULL AUTO_INCREMENT ,\
							log_time datetime DEFAULT NULL ,\
							accessKeyId varchar(200) DEFAULT NULL ,\
							reqCount varchar(20) DEFAULT NULL ,\
							resCount varchar(20) DEFAULT NULL,\
							emptyCount varchar(20) DEFAULT NULL,\
							PRIMARY KEY (id)) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 ;";
	

	char logDate[30];
	char chReqCount[20];
	char chResCount[20];
	char chEmptyCount[20];
	std::string strHiveDate;

	while(true) {

		createTable =   "create table if not exists apigateway_req_count" + tableValue;
		memset(chReqCount,0,10);
		memset(chResCount,0,10);
		memset(chEmptyCount,0,10);


		memset(logDate,0,30);

		pchSqlPermissions = "insert into apigateway_req_count(log_time,accessKeyId,reqCount,resCount,emptyCount)values";


		std::map<std::string,USERINFO_S>::iterator itr;
		BdxQueryCountOpenFile();
		BdxQueryCountGetReport();
        time_t timep;
        time(&timep);
        struct tm* timeinfo = localtime(&timep);
		if(!m_stMysqlServerInfoCount->GetMysqlInitState())
		{
			m_stMysqlServerInfoCount->InitMysql(g_mySqlInfoCount.m_stMysqlLinkInfo.m_pchIp,g_mySqlInfoCount.m_stMysqlLinkInfo.m_uiPort,g_mySqlInfoCount.pchUserName,g_mySqlInfoCount.pchPassWord,g_mySqlInfoCount.pchDbName);
		}
		m_stMysqlServerInfoCount->ExecuteMySql(createTable.c_str());//create table
		//printf("Line:%d,create table %s\n",__LINE__,createTable.c_str());
		//LOG(DEBUG,"create table %s",createTable.c_str());

        //fprintf(m_pFile, "%04d-%02d-%02d %02d:%02d:%02d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		//printf("m_stReport.m_strUserInfo_size=%d\n",m_stReport.m_strUserInfo.size());
		for(itr = m_stReport.m_strUserInfo.begin();itr!= m_stReport.m_strUserInfo.end();itr++)
		{
			fprintf(m_pFile, "%04d-%02d-%02d %02d:%02d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
			fprintf(m_pFile,"%s\t%llu\t%llu\t%llu\t%llu\n",itr->first.c_str(),itr->second.m_ullReqNum,itr->second.m_ullResNum,itr->second.m_ullEmptyResNum,itr->second.m_ullResTagNum);
			sprintf(logDate, "%04d-%02d-%02d %02d:%02d",timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);

			sprintf(chReqCount,"%d",itr->second.m_ullReqNum);
			sprintf(chResCount,"%d",itr->second.m_ullResNum);
			sprintf(chEmptyCount,"%d",itr->second.m_ullEmptyResNum);


			insertMysqlValue =pchSqlPermissions + "('"+std::string(logDate)+"','" + itr->first + "','" + std::string(chReqCount) + "','" + std::string(chResCount) + "','"  + std::string(chEmptyCount) + "');";
			
			//printf("Line:%d,insertMysqlValue=%s\n",__LINE__,insertMysqlValue.c_str());
			
			m_stMysqlServerInfoCount->ExecuteMySql(std::string("set names utf8").c_str());//set charset
			m_stMysqlServerInfoCount->ExecuteMySql((std::string("alter database ")+g_mySqlInfoCount.pchDbName+ std::string(" default character set utf8")).c_str());//set charset
			if(m_stMysqlServerInfoCount->ExecuteMySql(insertMysqlValue.c_str()))
			{
				//LOG(DEBUG,"%s success",insertMysqlValue.c_str());
				//printf("Line:%d,Insert into mysql is success.....%s\n",__LINE__,insertMysqlValue.c_str());
			}
			else
			{
				//LOG(DEBUG," %s error!!!!1",insertMysqlValue.c_str());
				//printf("Line:%d,Insert into mysql is error.....\n",__LINE__);					
			}	
		

		}


		
		fflush(m_pFile);
		m_stReport = 0;
        sleep(m_stStatisticsPrm.m_uiStatisticsTime);
	}
}

std::string CUserQueryCount::BdxUserQueryCountGetDate(const time_t ttime)
{
	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);
	sprintf(dt, "%4d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}

void CUserQueryCount::BdxQueryCountOpenFile()
{
	struct stat statbuf;

	std::string strHiveLogDir;
	//DIR *pstDir = NULL;
	srand((int)time(0)+123456789);
	int randomInt = rand()%100000;
	char randomchar[5];
	memset(randomchar,0,5);
	sprintf(randomchar,"%d",m_RandomInt);
	
	std::string statusDir= m_stStatisticsPrm.m_strStatisticsPath+"/"+BdxUserQueryCountGetDate();
	//std::string strFileName = m_stStatisticsPrm.m_strStatisticsPath+"/"+BdxUserQueryCountGetDate()+ "/" + m_stStatisticsPrm.m_strStatisticsFileName +"_"+ BdxUserQueryCountGetDate() + ".txt";
	std::string strFileName = statusDir + "/"+m_stStatisticsPrm.m_strStatisticsFileName+"_"+ BdxUserQueryCountGetDate()+"_"+ std::string(randomchar) + ".txt";
	if(!m_clFile.FileBeExists(strFileName.c_str())) {
		if(m_pFile){
			fclose(m_pFile);
			m_pFile = NULL;
		}
		m_pFile = fopen(strFileName.c_str(), "a");
		if (!stat(strFileName.c_str(),&statbuf))
		{
			if( statbuf.st_size == 0 )
			{
				BdxQueryCountWriteTitle();
			}
			
		}
/*
		for(u_int i = 0; i < CUserQueryWorkThreads::m_vecReport.size(); ++i) 
		{
			CUserQueryWorkThreads::m_vecReport[i] = 0;
		}
*/
	}
}

void CUserQueryCount::BdxQueryCountWriteTitle()
{
	printf("BdxQueryCountWriteTitle.....\n");
	std::string strTitle = std::string("DateTime\t") + "AccessKeyID\t" + "QueryReq\t" +"QueryRes\t" +"QueryEmptyRes\t"+ "QueryLimit\t\n";
	fprintf(m_pFile, "%s", strTitle.c_str());
	fflush(m_pFile);
}

void CUserQueryCount::BdxQueryCountGetReport()
{
	//memset(&m_stReport.m_strUserInfo,0,sizeof(std::map<std::string,USERINFO_S>));
	//memset(&m_stReport, 0, sizeof(UESRQUERYRPORT_S));
	for(u_int i = 0; i < CUserQueryWorkThreads::m_vecReport.size(); ++i) {
		m_stReport += CUserQueryWorkThreads::m_vecReport[i];
		CUserQueryWorkThreads::m_vecReport[i] = 0;
	}
}

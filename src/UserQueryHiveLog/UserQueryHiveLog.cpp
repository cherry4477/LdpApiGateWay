/*

 */

#include "UserQueryHiveLog.h"
extern CLog *gp_log;



MYSQL_SERVERINFO_S g_mySqlInfoHive;


CUserQueryHiveLog::CUserQueryHiveLog(const STATISTICSPRM_S& stHiveLogPrm,const MYSQL_SERVERINFO_S &mySqlInfo )
 :m_stHiveLogPrm(stHiveLogPrm)
{	
	struct stat statbuf;
	std::string strHiveLogDir;
	m_stMysqlServerInfoHive = new CMYSQL;


	sprintf(g_mySqlInfoHive.m_stMysqlLinkInfo.m_pchIp,"%s",mySqlInfo.m_stMysqlLinkInfo.m_pchIp);	
	g_mySqlInfoHive.m_stMysqlLinkInfo.m_uiPort = mySqlInfo.m_stMysqlLinkInfo.m_uiPort;	
	sprintf(g_mySqlInfoHive.pchUserName,"%s",mySqlInfo.pchUserName);	
	sprintf(g_mySqlInfoHive.pchPassWord,"%s",mySqlInfo.pchPassWord);	
	sprintf(g_mySqlInfoHive.pchDbName,"%s",mySqlInfo.pchDbName);
	

	strHiveLogDir = m_stHiveLogPrm.m_strStatisticsPath + "/" +	BdxUserQueryHiveLogGetDate();
	if(!m_clFile.FileBeExists(strHiveLogDir.c_str())) {
		m_clFile.FileCreatDir(strHiveLogDir.c_str());
	}
	std::string strFileName = strHiveLogDir + "/"+m_stHiveLogPrm.m_strStatisticsFileName+"_"+ BdxUserQueryHiveLogGetDate()+"_"+BdxUserQueryHiveLogGetHour() + ".txt";
	m_pFile = fopen(strFileName.c_str(), "a");
	if (!stat(strFileName.c_str(),&statbuf))
	{
		if( statbuf.st_size == 0 )
		{
			BdxQueryHiveLogWriteTitle();
		}
	}

	//m_stMysqlServerInfoHive->InitMysql(mySqlInfo.m_stMysqlLinkInfo.m_pchIp,mySqlInfo.m_stMysqlLinkInfo.m_uiPort,mySqlInfo.pchUserName,mySqlInfo.pchPassWord,mySqlInfo.pchDbName);


	
}

CUserQueryHiveLog::~CUserQueryHiveLog() {
	// TODO Auto-generated destructor stub
}
void CUserQueryHiveLog::Core()
{
	std::string pchSqlPermissions ;//= "insert into rtb_hive_data";
	//char insertMysqlValue[2000];
	std::string insertMysqlValue;
	std::string createTable;
	std::string tableValue ="( id int(10) unsigned NOT NULL AUTO_INCREMENT ,\
							log_time datetime DEFAULT NULL ,\
							accessKeyId varchar(20) DEFAULT NULL ,\
							data_source varchar(20) DEFAULT NULL ,\
							data_provider varchar(20) DEFAULT NULL,\
							data_carrier varchar(20) DEFAULT NULL,\
							phoneNo int(15) DEFAULT NULL ,\
							reqTimeStamp varchar(30) DEFAULT NULL ,\
							reqLiveTime varchar(30) DEFAULT NULL ,\
							reqSignature varchar(50) DEFAULT NULL ,\
							authID varchar(50) DEFAULT NULL ,\
							custName varchar(50) DEFAULT NULL ,\
							reqAction varchar(50) DEFAULT NULL ,\
							reqMd5Store varchar(50) DEFAULT NULL ,\
							reqParams varchar(200) DEFAULT NULL ,\
							resultValue bigint(20) DEFAULT NULL ,\
							queryTime bigint(20) DEFAULT NULL,\
							log_dayID bigint(20) DEFAULT NULL ,\
							log_HourID bigint(20) DEFAULT NULL ,\
							PRIMARY KEY (id)) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 ;";
	
	char chSource[10];
	char chProvider[10];
	char logDate[30];
	std::string strHiveDate;
	while(true) {

		strHiveDate = BdxUserQueryHiveLogGetDate();

		createTable =   "create table if not exists rtb_hive_data_"+ strHiveDate + tableValue;
		
		//memset(insertMysqlValue,0,2000);
		memset(chSource,0,10);
		memset(chProvider,0,10);
		memset(logDate,0,30);
		pchSqlPermissions = "insert into rtb_hive_data_" + strHiveDate +"(log_time,accessKeyId,data_source,data_provider,data_carrier,phoneNo,reqTimeStamp,reqLiveTime,reqSignature,authID,custName,reqAction,reqMd5Store,reqParams,resultValue,queryTime,log_dayID,log_hourID)values";
		std::vector<HIVELOCALLOG_S>::iterator itr;
		BdxQueryHiveLogOpenFile();
		//BdxQueryHiveLogGetReport();
        time_t timep;
        time(&timep);

		#if 0
		if(!m_stMysqlServerInfoHive->GetMysqlInitState())
		{
			m_stMysqlServerInfoHive->InitMysql(g_mySqlInfoHive.m_stMysqlLinkInfo.m_pchIp,g_mySqlInfoHive.m_stMysqlLinkInfo.m_uiPort,g_mySqlInfoHive.pchUserName,g_mySqlInfoHive.pchPassWord,g_mySqlInfoHive.pchDbName);
		}
		m_stMysqlServerInfoHive->ExecuteMySql(createTable.c_str());//create table
		printf("Line:%d,create table %s\n",__LINE__,createTable.c_str());
		LOG(DEBUG,"create table %s",createTable.c_str());

		#endif
		
        struct tm* timeinfo = localtime(&timep);
		for(u_int i = 0; i < CUserQueryWorkThreads::m_vecHiveLog.size(); ++i) {
			while(!CUserQueryWorkThreads::m_vecHiveLog[i].empty())
			{
				m_stHiveLog = CUserQueryWorkThreads::m_vecHiveLog[i].front();
							  CUserQueryWorkThreads::m_vecHiveLog[i].pop();		
				fprintf(m_pFile, "%04d-%02d-%02d %02d:%02d\t", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
				fprintf(m_pFile,"%s\t%s\t%s\t%s\t%s\t%s\n",m_stHiveLog.strReqParams.c_str(),m_stHiveLog.strAction.c_str(),m_stHiveLog.strValue.c_str(),m_stHiveLog.strQuerytime.c_str(),m_stHiveLog.strDayId.c_str(),m_stHiveLog.strHourId.c_str());

				#if 0
				sprintf(logDate, "%04d-%02d-%02d %02d:%02d",timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
				sprintf(chSource,"%d",m_stHiveLog.iSource);
				sprintf(chProvider,"%d",m_stHiveLog.iProvider);
				
				insertMysqlValue =pchSqlPermissions + "('"+std::string(logDate)+"','" + m_stHiveLog.strAccessKeyId + "','" + std::string(chSource) + "','" + std::string(chProvider)+ "','"  + m_stHiveLog.strCarrier +"','" + m_stHiveLog.strTelNo +"','" + m_stHiveLog.strTimeStamp +"','" + m_stHiveLog.strLiveTime +"','" + m_stHiveLog.strSinature +"','" + m_stHiveLog.strAuthId + "','" + m_stHiveLog.strCustName +"','" + m_stHiveLog.strAction +"','" + m_stHiveLog.strMd5Key +"','" + m_stHiveLog.strReqParams +"','" + m_stHiveLog.strValue + "','" + m_stHiveLog.strQuerytime +"','" + m_stHiveLog.strDayId + "','" + m_stHiveLog.strHourId + "');";;

				printf("Line:%d,insertMysqlValue=%s\n",__LINE__,insertMysqlValue.c_str());

                m_stMysqlServerInfoHive->ExecuteMySql(std::string("set names utf8").c_str());//set charset
                m_stMysqlServerInfoHive->ExecuteMySql((std::string("alter database ")+g_mySqlInfoHive.pchDbName+ std::string("default character set utf8")).c_str());//set charset

				
				if(m_stMysqlServerInfoHive->ExecuteMySql(insertMysqlValue.c_str()))
				{
					LOG(DEBUG,"%s success",insertMysqlValue.c_str());
					printf("Line:%d,Insert into mysql is success.....%s\n",__LINE__,insertMysqlValue.c_str());
				}
				else
				{
					LOG(DEBUG," %s error!!!!1",insertMysqlValue.c_str());
					printf("Line:%d,Insert into mysql is error.....\n",__LINE__);					
				}	
				#endif
				
			}		
			//CUserQueryWorkThreads::m_vecHiveLog[i].clear();
		}
		
		fflush(m_pFile);
		//sleep(3000);
        sleep(m_stHiveLogPrm.m_uiStatisticsTime);
	}
}

std::string CUserQueryHiveLog::BdxUserQueryHiveLogGetDate(const time_t ttime)
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

std::string CUserQueryHiveLog::BdxUserQueryHiveLogGetHour(const time_t ttime)
{
	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);
	sprintf(dt, "%02d",timeinfo->tm_hour);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}
void CUserQueryHiveLog::BdxQueryHiveLogOpenFile()
{
	struct stat statbuf;
	std::string strFileName = m_stHiveLogPrm.m_strStatisticsPath +"/" +BdxUserQueryHiveLogGetDate() + "/" + m_stHiveLogPrm.m_strStatisticsFileName +"_"+ BdxUserQueryHiveLogGetDate()+"_"+BdxUserQueryHiveLogGetHour() + ".txt";
	if(!m_clFile.FileBeExists(strFileName.c_str())) 
	{
		if(m_pFile){
			printf("close last hour file.....\n");
			fclose(m_pFile);
			m_pFile = NULL;
		}
	}

	m_pFile = fopen(strFileName.c_str(), "a");
	if (!stat(strFileName.c_str(),&statbuf))
	{
		if( statbuf.st_size == 0 )
		{
			BdxQueryHiveLogWriteTitle();
		}		
	}
}

void CUserQueryHiveLog::BdxQueryHiveLogWriteTitle()
{
	printf("BdxQueryHiveLogWriteTitle.....\n");
}

void CUserQueryHiveLog::BdxQueryHiveLogGetReport()
{
	//memset(&m_stReport.m_strUserInfo,0,sizeof(std::map<std::string,USERINFO_S>));
	//memset(&m_stReport, 0, sizeof(UESRQUERYRPORT_S)); 结构体中有map 
	// 不需要初始化，否则会出错
	//for(u_int i = 0; i < CUserQueryWorkThreads::m_vecHiveLog.size(); ++i) {
		//m_stHiveLog += CUserQueryWorkThreads::m_vecHiveLog[i];
		//CUserQueryWorkThreads::m_vecHiveLog[i] = 0;
	//}
}

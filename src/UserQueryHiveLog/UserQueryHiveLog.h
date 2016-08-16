/*

 */

#ifndef __USERQUERYHIVELOG_H__
#define __USERQUERYHIVELOG_H__

#include "../UserQueryStruct/UserQueryStruct.h"
#include "../CommonTools/Mysql/mysqlClient.h"

#include "../CommonTools/File/File.h"
#include "../UserQueryWorkThreads/UserQueryWorkThreads.h"

class CUserQueryHiveLog {
public:
			CUserQueryHiveLog(const STATISTICSPRM_S& stHiveLogPrm,const MYSQL_SERVERINFO_S &mySqlInfo);
	virtual ~CUserQueryHiveLog();

	void Core();
protected:
	std::string BdxUserQueryHiveLogGetDate(const time_t ttime = 0);
	std::string BdxUserQueryHiveLogGetHour(const time_t ttime = 0);
	void BdxQueryHiveLogOpenFile();
	void BdxQueryHiveLogWriteTitle();
	void BdxQueryHiveLogGetReport();
private:
	CFile m_clFile;
	FILE* m_pFile;
	STATISTICSPRM_S m_stStatisticsPrm;
	STATISTICSPRM_S m_stHiveLogPrm;
	HIVELOCALLOG_S m_stHiveLog;
	CMYSQL *m_stMysqlServerInfoHive;	
	MYSQL_ROW mysqlRowHive;
	std::string currentHour;
	int m_RandomInt;


};

#endif /* __USERQUERYHIVELOG_H__ */


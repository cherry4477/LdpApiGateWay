/*

 */

#ifndef __USERQUERYCOUNT_H__
#define __USERQUERYCOUNT_H__

#include "../UserQueryStruct/UserQueryStruct.h"
#include "../CommonTools/Mysql/mysqlClient.h"

#include "../CommonTools/File/File.h"
#include "../UserQueryWorkThreads/UserQueryWorkThreads.h"

class CUserQueryCount {
public:
	CUserQueryCount(const STATISTICSPRM_S& stStatisticsPrm,const MYSQL_SERVERINFO_S &mySqlInfo);
	virtual ~CUserQueryCount();

	void Core();
protected:
	std::string BdxUserQueryCountGetDate(const time_t ttime = 0);
	void BdxQueryCountOpenFile();
	void BdxQueryCountWriteTitle();
	void BdxQueryCountGetReport();
private:
	CFile m_clFile;
	FILE* m_pFile;
	STATISTICSPRM_S m_stStatisticsPrm;
	UESRQUERYRPORT_S m_stReport;
	std::string currentHour;
	int m_RandomInt;
	CMYSQL *m_stMysqlServerInfoCount;	
	MYSQL_ROW mysqlRowCount;


};

#endif /* __USERQUERYCOUNT_H__ */


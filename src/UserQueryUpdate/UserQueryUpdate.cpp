/*
 * UserQueryUpdate.cpp
 */

#include "UserQueryUpdate.h"

#include "../UserQueryWorkThreads/UserQueryWorkThreads.h"
#include "../UserQueryServer/UserQueryServer.h"


extern CLog *gp_log;
extern std::string strServerName;
extern std::string strConfigFileName;
pthread_rwlock_t p_rwlock;
pthread_rwlockattr_t p_rwlock_attr;
pthread_mutex_t mutex;
std::string g_strTokenString = "";
u_int  g_iNeedUpdateToken = 0;
extern u_int  InitSSLFlag ;

extern	IPPORT_S m_stMonitorApi;
extern	IPPORT_S m_stEmailServer;
extern	string m_strUserName;
extern	string m_strPassWord;
extern	string m_strMailFrom;
extern	string m_strRcptTo;
extern	string m_strSubject;
extern	string m_strErrorMsg;
	
extern std::string g_strDataHubToken;
std::string g_strCurrentDate ;


std::map<std::string,BDXPERMISSSION_S> g_mapUserInfo;
MYSQL_SERVERINFO_S g_mySqlInfo;
std::map<std::string,BDXAPIGATEWAYCONFIF_S> g_MapApiGateWayconfig;


CUserQueryUpdate::CUserQueryUpdate(const IPPORT_S &stTokenServer,const MYSQL_SERVERINFO_S &mySqlInfo ) {
	// TODO Auto-generated constructor stub
	char chCount[10];
	char chPort [10];
	char chIdx  [10];
    m_pTokenRedis = new CDataAdapter;
    cTaskMonitorHuaWei = new CTaskMain;
    m_stMysqlServerInfo = new CMYSQL;
	sprintf(chCount,"%d",stTokenServer.m_count);
	sprintf(chPort,"%d",stTokenServer.m_uiPort);
	sprintf(chIdx,"%d",stTokenServer.m_idx);
	string strServerList = string(chCount)+";"+string(stTokenServer.m_pchIp)+":"+string(chPort)+","+string(chIdx)+";";

	//g_mySqlInfo.m_stMysqlLinkInfo.m_pchIp  = mySqlInfo.m_stMysqlLinkInfo.m_pchIp;
	sprintf(g_mySqlInfo.m_stMysqlLinkInfo.m_pchIp,"%s",mySqlInfo.m_stMysqlLinkInfo.m_pchIp);
	g_mySqlInfo.m_stMysqlLinkInfo.m_uiPort = mySqlInfo.m_stMysqlLinkInfo.m_uiPort;
	sprintf(g_mySqlInfo.pchUserName,"%s",mySqlInfo.pchUserName);
	sprintf(g_mySqlInfo.pchPassWord,"%s",mySqlInfo.pchPassWord);
	sprintf(g_mySqlInfo.pchDbName,"%s",mySqlInfo.pchDbName);
	
  	printf("Redis Server=%s\n",strServerList.c_str());
	m_pTokenRedis->Init(strServerList.c_str());
  
	printf("==============mysql info==============\n");
	printf("%s\n",mySqlInfo.m_stMysqlLinkInfo.m_pchIp);
	printf("%d\n",mySqlInfo.m_stMysqlLinkInfo.m_uiPort);
	printf("%s\n",mySqlInfo.pchUserName);
	printf("%s\n",mySqlInfo.pchPassWord);
	printf("%s\n",mySqlInfo.pchDbName);

	LOG(DEBUG,"mySqlInfo.m_stMysqlLinkInfo.m_pchIp=%s",mySqlInfo.m_stMysqlLinkInfo.m_pchIp);
	LOG(DEBUG,"mySqlInfo.m_stMysqlLinkInfo.m_uiPort=%d",mySqlInfo.m_stMysqlLinkInfo.m_uiPort);
	LOG(DEBUG,"mySqlInfo.pchUserName=%s",mySqlInfo.pchUserName);
	LOG(DEBUG,"mySqlInfo.pchPassWord=%s",mySqlInfo.pchPassWord);
	LOG(DEBUG,"mySqlInfo.pchDbName=%s",mySqlInfo.pchDbName);
	printf("======================================\n");


	//m_stMysqlServerInfo->InitMysql(mySqlInfo.m_stMysqlLinkInfo.m_pchIp,mySqlInfo.m_stMysqlLinkInfo.m_uiPort,mySqlInfo.pchUserName,mySqlInfo.pchPassWord,mySqlInfo.pchDbName);

}

CUserQueryUpdate::~CUserQueryUpdate() {
	// TODO Auto-generated destructor stub
	//delete m_pTokenRedis ;
    delete m_stMysqlServerInfo ;
}

void CUserQueryUpdate::GetMysqlFieldsUserInfo(std::string strUserInfo,BDXPERMISSSION_S &mVecFieldsUser,std::string &mUserName) 
{
	char *buf;
	int  index = 0;
	char bufTemp[PACKET];
	char *outer_ptr = NULL;  
	char *inner_ptr = NULL;  
	char *temp[PACKET]; 
	
	memset(temp,0,PACKET);
	memset(bufTemp,0,PACKET);
	mVecFieldsUser.mVecFields.clear();
					printf("strUserInfo=%s\n",strUserInfo.c_str());
					buf = const_cast<char*>(strUserInfo.c_str());
					while((temp[index] = strtok_r(buf,";", &outer_ptr))!=NULL)   
					{  	
					    buf=temp[index];
					    while((temp[index]=strtok_r(buf,"|", &inner_ptr))!=NULL)   
					    {   			    
					    	if(index >= 5)
					        {
					            //g_vecUrlAPIS[temp[index-1]]=temp[index];
					            mVecFieldsUser.mVecFields.push_back(temp[index]);
					        }					     	       					        
					        index++;
					        buf=NULL;  
					    }  
					    buf=NULL;  
					}  
					mUserName =	std::string(temp[0]);
					mVecFieldsUser.mResToken =	temp[1];
					mVecFieldsUser.mIntQueryTimes =	atoi(temp[2]);
					mVecFieldsUser.mIntGoodsTimes =	atoi(temp[3]);
					mVecFieldsUser.mGoodsFields =	std::string(temp[4]);
					std::vector<std::string>::iterator itr;
					#if 0
					for(itr=mVecFieldsUser.mVecFields.begin();itr!=mVecFieldsUser.mVecFields.end();itr++)
					{
						printf("=====%s\n",(*itr).c_str());
					}
					#endif


}


std::string CUserQueryUpdate::BdxUserGetCurrentDate(const time_t ttime)
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

std::string CUserQueryUpdate::BdxApiGateWayGetDataHubToken(std::string AuthUser,std::string PassWord) 
{
		std::string retDataHub;
		retDataHub = BdxGetDatafromDataHub(AuthUser,PassWord,"","","",0,2);
		//printf("Line:%d,BdxApiGateWayGetDataHubToken retDataHub=%s",__LINE__,retDataHub.c_str());
		if(retDataHub.find("\"msg\": \"OK\"")!=-1)
		{
			retDataHub = retDataHub.substr(retDataHub.find("token")+ 9 ,32);
			//retDataHub = retDataHub;
		}
		else
		{
			retDataHub = "";
		}
		return retDataHub;   
}

std::string CUserQueryUpdate::BdxApiUpdateUserOrder(std::string user,std::string subid,std::string repo,std::string item,std::string token,long used) 
{
		std::string retDataHub;
		retDataHub = BdxGetDatafromDataHub(user,token,repo,item,subid,used,5);
		printf("Line:%d,BdxApiGateWayGetDataHubToken retDataHub=%s",__LINE__,retDataHub.c_str());
		if(retDataHub.find("\"msg\": \"OK\"")!=-1)
		{
			retDataHub = retDataHub.substr(retDataHub.find("token")+ 9 ,32);
			//retDataHub = retDataHub;
		}
		else
		{
			retDataHub = "";
		}
		return retDataHub;   
}



std::string CUserQueryUpdate::BdxGetDatafromDataHub(std::string AuthUser,std::string AuthToken,std::string repo,std::string item,std::string subid,long used,int type)
{
	//type 1 verifytoken 2 get token 3 get order 4 update order
	char m_httpReqVerifyToken[_8KBLEN];
	char sslReadBuffer[_8KBLEN];
	char tempBuffer[PACKET];
	std::string strReadBuffer;
	//std::string datahubIP = "10.1.235.98";
	//uint16_t 	datahubPort = 443;
	
	std::string datahubIP = getenv("DATAHUB_IP");
	uint16_t datahubPort = atoi(getenv("DATAHUB_PORT"));

	CTcpSocket* sslLocalSocket; 	
	sslLocalSocket=new CTcpSocket(datahubPort,datahubIP);
	string strType;
	memset(m_httpReqVerifyToken,0,sizeof(m_httpReqVerifyToken));
	if( type == 1 )
	{
		strType ="verify user token";
		sprintf(m_httpReqVerifyToken,"GET /api/valid HTTP/1.1\r\nHost: %s\r\nAuthorization:Token %s\r\nAuthuser: %s\r\n\r\n",datahubIP.c_str(),AuthToken.c_str(),AuthUser.c_str());
		//printf("Line:%d,BdxVerifyDataHubToken\n",__LINE__);
	}
	if( type == 2 )
	{
		strType ="get gateway token";
		sprintf(m_httpReqVerifyToken,"GET /api/ HTTP/1.1\r\nHost: %s\r\nAuthorization: Basic YWRtaW5fY2hlbnlnQGFzaWFpbmZvLmNvbTo4ZGRjZmYzYTgwZjQxODljYTFjOWQ0ZDkwMmMzYzkwOQ==\r\n\r\n",datahubIP.c_str());
	}
	if( type == 3 )
	{
		strType ="get order info";
		sprintf(m_httpReqVerifyToken,"GET /api/subscriptions/pull/%s/%s?username=%s HTTP/1.1\r\nHost: %s\r\nAccept: application/json; charset=utf-8\r\nAuthorization: Token %s\r\n\r\n",repo.c_str(),item.c_str(),AuthUser.c_str(),datahubIP.c_str(),AuthToken.c_str());
	}
	if( type == 4 )
	{
		strType ="set_retrieved";
		memset(tempBuffer,0,sizeof(tempBuffer));
		sprintf(tempBuffer,"{\"action\":\"set_retrieved\",\"repname\":\"%s\",\"itemname\":\"%s\",\"username\":\"%s\"}",repo.c_str(),item.c_str(),AuthUser.c_str());
	    //std::string putValue = std::string(tempBuffer);
		sprintf(m_httpReqVerifyToken,"PUT /api/subscription/%s HTTP/1.1\r\nHost: %s\r\nAuthorization: Token %s\r\nContent-Length: %d\r\nAccept: application/json; charset=utf-8\r\n\r\n%s",subid.c_str(),datahubIP.c_str(),AuthToken.c_str(),std::string(tempBuffer).length(),std::string(tempBuffer).c_str());
		//printf("Line:%d,m_httpReqVerifyToken=%s\n",__LINE__,m_httpReqVerifyToken);


	}
	if( type == 5 )
	{
		strType ="set_plan_used";
		memset(tempBuffer,0,sizeof(tempBuffer));
		sprintf(tempBuffer,"{\"action\":\"set_plan_used\",\"used\":%ld,\"repname\":\"%s\",\"itemname\":\"%s\",\"username\":\"%s\"}",used,repo.c_str(),item.c_str(),AuthUser.c_str());
	    //std::string putValue = std::string(tempBuffer);
		sprintf(m_httpReqVerifyToken,"PUT /api/subscription/%s HTTP/1.1\r\nHost: %s\r\nAuthorization: Token %s\r\nContent-Length: %d\r\nAccept: application/json; charset=utf-8\r\n\r\n%s",subid.c_str(),datahubIP.c_str(),AuthToken.c_str(),std::string(tempBuffer).length(),std::string(tempBuffer).c_str());
		//printf("Line:%d,m_httpReqVerifyToken=%s\n",__LINE__,m_httpReqVerifyToken);

	}

	if(sslLocalSocket->TcpConnect()==0)
	{
		pthread_mutex_lock (&mutex);
		if ( InitSSLFlag == 0 )
		{
			sslLocalSocket->TcpSslInitParams();
			InitSSLFlag = 1;
			
		}
		pthread_mutex_unlock(&mutex);
		if(sslLocalSocket->TcpSslInitEnv()==0)
		{
			if(sslLocalSocket->TcpSslConnect())
			{
				if(sslLocalSocket->TcpSslWriteLen(m_httpReqVerifyToken,strlen(m_httpReqVerifyToken))!=0)
				{
					memset(sslReadBuffer,0,sizeof(sslReadBuffer));
					sslLocalSocket->TcpSslReadLen(sslReadBuffer,sizeof(sslReadBuffer));
					if( strlen(sslReadBuffer)>0 )
					{
						strReadBuffer = std::string(sslReadBuffer);
						if(strReadBuffer.find("Transfer-Encoding: chunked")!=-1)
						{
							int itrunkStart = strReadBuffer.find("\r\n",strReadBuffer.find("\r\n\r\n")+4);
							int itrunkEnd = strReadBuffer.find("\r\n",itrunkStart + 1);
							strReadBuffer=strReadBuffer.substr(itrunkStart + 2 ,itrunkEnd - itrunkStart -1);
						}
						else
						{
							if(strReadBuffer.find("\r\n\r\n")!=std::string::npos)
							{
							  int lenStrTemp = strReadBuffer.length();
							  strReadBuffer = strReadBuffer.substr(strReadBuffer.find("\r\n\r\n")+4,lenStrTemp -(strReadBuffer.find("\r\n\r\n")+4));
							}
						}
						printf("Line:%d,receive from datahub type[%d] [%s] sslReadBuffer=%s\n",__LINE__,type,strType.c_str(),strReadBuffer.c_str());
					}
	
				}
	
			}
		}
	}
	sslLocalSocket->TcpSslDestroy();
	return strReadBuffer;
}



#if 0
bool CUserQueryUpdate::VectorIsEqual(std::vector<std::string> srcVector,std::vector<std::string> destVector) 
{
	std::vector<std::string>::iterator itrSrcVec;
	std::vector<std::string>::iterator itrDestVec;
	std::vector<std::string>::iterator itr2;

	if( srcVector.size() != srcVector.size())
	{
		return false;
	}
	for(itrSrcVec=srcVector.begin();itrSrcVec!=srcVector.end();itrSrcVec++)
	{
		itrDestVec = destVector.find(*itrSrcVec);
		if(itrDestVec !=itrDestVec.end())
		{
			continue;
		}
		else
		{
			return false;
		}

	}
	
return true;
}

#endif

bool CUserQueryUpdate::MapIsEqual(std::map<std::string,BDXPERMISSSION_S> &srcMap,std::map<std::string,BDXPERMISSSION_S> &destMap) 
{
	std::map<std::string,BDXPERMISSSION_S>::iterator itrSrcMap;
	std::map<std::string,BDXPERMISSSION_S>::iterator itrDestMap;
	std::vector<std::string>::iterator itr2;

	if( srcMap.size() != destMap.size())
	{
		return false;
	}
	
	for(itrSrcMap=srcMap.begin();itrSrcMap!=srcMap.end();itrSrcMap++)
	{
		itrDestMap = destMap.find(itrSrcMap->first);
		if(itrDestMap !=destMap.end())
		{
				if(itrSrcMap->second.mResToken!=itrDestMap->second.mResToken||
				itrSrcMap->second.mIntQueryTimes!=itrDestMap->second.mIntQueryTimes||
				itrSrcMap->second.mVecFields!=itrDestMap->second.mVecFields||
				itrSrcMap->second.mGoodsFields!=itrDestMap->second.mGoodsFields||
				itrSrcMap->second.mIntGoodsTimes!=itrDestMap->second.mIntGoodsTimes)
				//VectorIsEqual(itrSrcMap->second.mVecFields,)
				{
					return false;

				}
				

		}
		else
		{
			return false;
		}

	}

return true;
}

void CUserQueryUpdate::SwapMap(std::map<std::string,BDXPERMISSSION_S> &srcMap,std::map<std::string,BDXPERMISSSION_S> &destMap) 
{
	std::map<std::string,BDXPERMISSSION_S>::iterator itrSrcMap;
	BDXPERMISSSION_S mUserInfoVecFields;
	std::vector<std::string>::iterator itrSrcVector;

	destMap.clear();
	for(itrSrcMap=srcMap.begin();itrSrcMap!=srcMap.end();itrSrcMap++)
	{
		mUserInfoVecFields.mVecFields.clear();
		mUserInfoVecFields.mVecFields 		= itrSrcMap->second.mVecFields;
		mUserInfoVecFields.mResToken  		= itrSrcMap->second.mResToken;
		mUserInfoVecFields.mIntQueryTimes   = itrSrcMap->second.mIntQueryTimes;
		mUserInfoVecFields.mIntGoodsTimes   = itrSrcMap->second.mIntGoodsTimes;
		mUserInfoVecFields.mGoodsFields   = itrSrcMap->second.mGoodsFields;
		destMap.insert(std::pair<std::string,BDXPERMISSSION_S>(itrSrcMap->first,mUserInfoVecFields));

	}
	
}

void CUserQueryUpdate::Core()
{

	std::string strToken,strTokenValue;
	Json::Value jValue,jRoot,jResult;
	Json::Reader jReader;
	Json::FastWriter jFastWriter;
	std::string sts="yd_zhejiang_mobile_token";
	std::string strMysqlRecord;
	const char *pchSqlPermissions = "select access_keyid,secret_privatekey,query_count,goods_count,goods_perm,permissions from dmp_user_permissions";
	BDXPERMISSSION_S mUserInfoVecFields;
	std::string strUserName;
	int times = 0;
	std::map<std::string,BDXPERMISSSION_S> temp_mapUserInfo;
	BDXAPIGATEWAYCONFIF_S temp_MapApiGateWayconfig;


	std::string tempCurrentDate;
	deque<string> setMembers;
	std::string needUpdateOrder="need_updated_datahub_order";
	std::string alreadyUpdateOrder="already_updated_datahub_order";
	deque<string>::iterator itSetMembers;
	std::string statusOrder,countOrder;
	std::string mid_statusOrder= "order_status";
	std::string mid_countOrder = "order_count";
	std::string ssmoidValue,ssmoidValue2;
	int iLimitOrderId,iCountOrderId;
	std::string startOrderID = KEY_startOrderID;
	std::string startToken = KEY_startToken;
	std::string apiGateWayAdmin = KEY_apiGateWayAdmin;
	std::string startOrderCount  = KEY_startOrderCount;
	std::string startOrderLimit  = KEY_startOrderLimit;
	std::string startOrderStatus = KEY_startOrderStatus;
	std::string startOrderExpire = KEY_startOrderExpire;
	std::string strCountOrderId,strLimitOrderId,strStatusOrderId,strExpireOrderId,strTempSubID;


	std::string sModValue,updatedmember,strUserOrderId;
	std::string keyValue,strAuthUser,strRepo,strItem,strSubid;
	int orderUsed;
	g_strCurrentDate = BdxUserGetCurrentDate();
	
	mid_statusOrder += KEY_DELIMITER;
	mid_countOrder  += KEY_DELIMITER;


	
	while(true)
	{
		times=1;	

		tempCurrentDate = BdxUserGetCurrentDate();
		if( tempCurrentDate.compare(g_strCurrentDate)!=0 )

		{
					g_strCurrentDate = tempCurrentDate;
					setMembers.clear();
					if(m_pTokenRedis->UserSmembers(needUpdateOrder,setMembers))
					{
						if(!setMembers.empty())
						{
							for(itSetMembers = setMembers.begin();itSetMembers!=setMembers.end();itSetMembers++)
							{
								sModValue.clear();
								keyValue = *itSetMembers;
								statusOrder = mid_statusOrder + *itSetMembers;
								countOrder	= mid_countOrder  + *itSetMembers;
								
								if( m_pTokenRedis ->UserGet(statusOrder,sModValue))
								{
									printf("Line:%d,get sModValue=%s\n",__LINE__,sModValue.c_str());
									if( atoi(sModValue.c_str()) == 0 )
									{//update datahub order 
		
										int userStart = keyValue.find(KEY_DELIMITER);
										strAuthUser = keyValue.substr(0,userStart);
										
										int repoStart = keyValue.find(KEY_DELIMITER,userStart+1);
										strRepo = keyValue.substr(userStart + 1,repoStart - (userStart+1));
		
										int itemStart = keyValue.find(KEY_DELIMITER,repoStart+1);
										strItem = keyValue.substr(repoStart + 1,itemStart - (repoStart+1));
		
										int subIdStart = keyValue.find(KEY_DELIMITER,itemStart+1);
										strSubid= keyValue.substr(itemStart + 1,subIdStart - (itemStart+1));
										ssmoidValue.clear();
										ssmoidValue2.clear();
										

										printf("Line:%d,strAuthUser=%s,strRepo=%s\n",__LINE__,strAuthUser.c_str(),strRepo.c_str());
										printf("Line:%d,strItem=%s,strSubid=%s\n",__LINE__,strItem.c_str(),strSubid.c_str());
										printf("Line:%d,keyValue=%s\n",__LINE__,keyValue.c_str());

		
										updatedmember = strAuthUser+KEY_DELIMITER + strRepo+KEY_DELIMITER+strItem+KEY_DELIMITER+strSubid;
										strUserOrderId =  strAuthUser +std::string(KEY_DELIMITER + startOrderID ); 
										strCountOrderId = startOrderCount + KEY_DELIMITER + updatedmember;
										strLimitOrderId = startOrderLimit + KEY_DELIMITER + updatedmember;
										strExpireOrderId =	startOrderExpire + KEY_DELIMITER + updatedmember;
										printf("Line:%d,strCountOrderId=%s\n",__LINE__,strCountOrderId.c_str());
										printf("Line:%d,strLimitOrderId=%s\n",__LINE__,strLimitOrderId.c_str());
										if((m_pTokenRedis->UserGet(strCountOrderId,ssmoidValue))&&(m_pTokenRedis->UserGet(strLimitOrderId,ssmoidValue2)))
										{
											
											iCountOrderId = atoi(ssmoidValue.c_str());
											iLimitOrderId = atoi(ssmoidValue2.c_str());
											printf("Line:%d,iCountOrderId=%d\n",__LINE__,iCountOrderId);
											printf("Line:%d,iLimitOrderId=%d\n",__LINE__,iLimitOrderId);									
											if(( iCountOrderId >= iLimitOrderId )&&(!m_pTokenRedis->UserGet(strExpireOrderId,ssmoidValue)))
											{
												m_pTokenRedis->UserPut(statusOrder,std::string("1"));
												m_pTokenRedis->UserSmove(needUpdateOrder,alreadyUpdateOrder,updatedmember);
												m_pTokenRedis->UserRemoveSortedSet(strUserOrderId,ssmoidValue);
											}
											if(g_strDataHubToken.empty())
											{
												g_strDataHubToken = BdxApiGateWayGetDataHubToken();
											}
											BdxApiUpdateUserOrder(strAuthUser,strSubid,strRepo,strItem,g_strDataHubToken,iCountOrderId);
										}								

									}
								}
							}
						}
					}
				}

	  
		while(times--)
		{
				temp_mapUserInfo.clear();
				g_MapApiGateWayconfig.clear();

				#if 0
				if(m_stMysqlServerInfo->GetMysqlInitState())
				{
					if(m_stMysqlServerInfo->ExecuteMySql(pchSqlPermissions))
					{

						if(m_stMysqlServerInfo->MysqlUseResult())
						{	
							//m_stMysqlServerInfo->DisplayHeader();
							while(m_stMysqlServerInfo->MysqlFetchRow())
							{			
								//if(!first_row)
								{	
									strMysqlRecord = m_stMysqlServerInfo->GetColumnValue();
									//printf("strMysqlRecord = %s\n",strMysqlRecord.c_str());
									GetMysqlFieldsUserInfo(strMysqlRecord,mUserInfoVecFields,strUserName);
									temp_mapUserInfo.insert(std::pair<std::string,BDXPERMISSSION_S>(strUserName,mUserInfoVecFields));
								}
								//first_row = 0;
							}

							std::map<std::string,BDXPERMISSSION_S>::iterator itr;
							std::vector<std::string>::iterator itr2;
							#if 1
							printf("===================temp_mapUserInfo========================\n");
							for(itr=temp_mapUserInfo.begin();itr!=temp_mapUserInfo.end();itr++)
							{	
								printf("%s ",itr->first.c_str());
								printf("%s ",itr->second.mResToken.c_str());
								printf("%d ",itr->second.mIntQueryTimes);
								printf("%d ",itr->second.mIntGoodsTimes);
								printf("%s ",itr->second.mGoodsFields.c_str());
								
								//LOG(DEBUG,"itr->first.c_str()=%s",itr->first.c_str());
								//LOG(DEBUG,"itr->second.mResToken.c_str()=%s",itr->second.mResToken.c_str());
								//LOG(DEBUG,"itr->second.mIntQueryTimes=%d",itr->second.mIntQueryTimes);
								//LOG(DEBUG,"itr->second.mIntGoodsTimes=%d",itr->second.mIntGoodsTimes);
								//LOG(DEBUG,"itr->second.mGoodsFields.c_str()=%s",itr->second.mGoodsFields.c_str());
								for(itr2=itr->second.mVecFields.begin();itr2!=itr->second.mVecFields.end();itr2++)
								{
									printf("%s ",(*itr2).c_str());
									
								}
								printf("\n");
							}
							#endif
							#if 1
							printf("===================g_mapUserInfo========================\n");
							for(itr=g_mapUserInfo.begin();itr!=g_mapUserInfo.end();itr++)
							{	
								printf("%s ",itr->first.c_str());
								printf("%s ",itr->second.mResToken.c_str());
								printf("%d ",itr->second.mIntQueryTimes);
								printf("%d ",itr->second.mIntGoodsTimes);
								printf("%s ",itr->second.mGoodsFields.c_str());
								//LOG(DEBUG,"itr->first.c_str()=%s",itr->first.c_str());
								//LOG(DEBUG,"itr->second.mResToken.c_str()=%s",itr->second.mResToken.c_str());
								//LOG(DEBUG,"itr->second.mIntQueryTimes=%d",itr->second.mIntQueryTimes);
								//LOG(DEBUG,"itr->second.mIntGoodsTimes=%d",itr->second.mIntGoodsTimes);
								//LOG(DEBUG,"itr->second.mGoodsFields.c_str()=%s",itr->second.mGoodsFields.c_str());
								for(itr2=itr->second.mVecFields.begin();itr2!=itr->second.mVecFields.end();itr2++)
								{
									printf("%s ",(*itr2).c_str());
								}
								printf("\n");
							}
							#endif
							if( !MapIsEqual(temp_mapUserInfo,g_mapUserInfo) )
							{
							//	g_mapUserInfo = temp_mapUserInfo;
								
								SwapMap(temp_mapUserInfo,g_mapUserInfo);
								printf("\nswap map g_mapUserInfo\n\n");				
							}

						}
						
					}
					else
					{
						m_stMysqlServerInfo->DestroyResultEnv();
					}
				}
				else
				{
					printf("Line:%d,Reconnect mysql .....\n",__LINE__);

					printf("==============reconnect mysql info==============\n");
					printf("%s\n",g_mySqlInfo.m_stMysqlLinkInfo.m_pchIp);
					printf("%d\n",g_mySqlInfo.m_stMysqlLinkInfo.m_uiPort);
					printf("%s\n",g_mySqlInfo.pchUserName);
					printf("%s\n",g_mySqlInfo.pchPassWord);
					printf("%s\n",g_mySqlInfo.pchDbName);

					m_stMysqlServerInfo->InitMysql(g_mySqlInfo.m_stMysqlLinkInfo.m_pchIp,g_mySqlInfo.m_stMysqlLinkInfo.m_uiPort,g_mySqlInfo.pchUserName,g_mySqlInfo.pchPassWord,g_mySqlInfo.pchDbName);
				}


				#endif
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getThemeByTk?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("secID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("secShortName=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("ticker=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("exchangeCD=0");
				g_MapApiGateWayconfig["ThemeByTk"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getThemeInfo?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("isMain=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("themeID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("themeName=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("themeSource=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["ThemeInfo"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getThemeHeat?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("isMain=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("themeID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("themeName=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["ThemeHeat"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getTkByTheme?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("isMain=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("themeID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("themeName=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("isNew=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["TkByTheme"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getThemeByNews?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("newsID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("insertDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginTime=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endTime=0");;
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["ThemeByNews"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getActiveTheme?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("date=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["ActiveTheme"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getNewsContent?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("newsID=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["NewsContent"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getNewsByTk?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("secID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("secShortName=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("ticker=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("exchangeCD=0"); 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");	 
				g_MapApiGateWayconfig["NewsByTk"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getNewsByComp?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("partyID=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");	 
				g_MapApiGateWayconfig["NewsByComp"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getSecID?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("partyID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("ticker=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("assetClass=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("cnSpell=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["SecID"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getTradeCal?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("exchangeCD=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endDate=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=1");
				g_MapApiGateWayconfig["TradeCal"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getEquInfo?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("ticker=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("pagesize=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("pagenum=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["EquInfo"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/haodai/blacklist_personal?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("name=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("idcard=1");
				g_MapApiGateWayconfig["Blacklist"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getFdmtEe?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("secID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginDate=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endDate=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("publishDateBegin=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("publishDateEnd=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("reportType=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("ticker=0");
				g_MapApiGateWayconfig["FdmtEe"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getFdmtEf?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("secID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("beginDate=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("endDate=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("forecastType=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("publishDateBegin=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("publishDateEnd=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("reportType=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("ticker=0");
				g_MapApiGateWayconfig["FdmtEf"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/KMA/CreditRank?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("comnumber=1");
				g_MapApiGateWayconfig["CreditRank"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/datayes/getEqu?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("secID=0");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("ticker=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("equTypeCD=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("field=0");
				g_MapApiGateWayconfig["Equ"] = temp_MapApiGateWayconfig;			
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/simplybrand/ProductQuery?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("Keyword=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("SiteId=0");
				g_MapApiGateWayconfig["ProductQuery"] = temp_MapApiGateWayconfig;			
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getCategories?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("search_word=0");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("type=0"); 																									 
				g_MapApiGateWayconfig["Categories"] = temp_MapApiGateWayconfig; 	
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getHighCategories?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("search_word=0");	 
				g_MapApiGateWayconfig["HighCategories"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getSuperCategories?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("search_word=0");																										
				g_MapApiGateWayconfig["SuperCategories"] = temp_MapApiGateWayconfig;	
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getBrands?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("search_word=0");																										
				g_MapApiGateWayconfig["Brands"] = temp_MapApiGateWayconfig; 	
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getPlatforms?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("search_word=0");																										
				g_MapApiGateWayconfig["Platforms"] = temp_MapApiGateWayconfig;	
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getShops?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("search_word=0");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("per=0");	   
				temp_MapApiGateWayconfig.mStrReqParams.push_back("page=0");  
				g_MapApiGateWayconfig["Shops"] = temp_MapApiGateWayconfig;	
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getSkus?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("search_word=0");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("brand_id=0"); 	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("category_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("shop_id=0");	   
				temp_MapApiGateWayconfig.mStrReqParams.push_back("platform_id=0");
				g_MapApiGateWayconfig["Skus"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getBrandCategory?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("search_word=0");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("brand_id=0"); 	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("category_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("type=0"); 	
				g_MapApiGateWayconfig["BrandCategory"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getSalesVolume?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("category_id=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("platform_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("brand_id=0"); 	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("product_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_type=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("start_time=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("end_time=0"); 	
				g_MapApiGateWayconfig["SalesVolume"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getSalesAmount?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("category_id=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("platform_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("brand_id=0"); 	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("product_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_type=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("start_time=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("end_time=0"); 	
				g_MapApiGateWayconfig["SalesAmount"] = temp_MapApiGateWayconfig;
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getAveragePrice?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("category_id=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("platform_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("brand_id=0"); 	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("product_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_type=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("start_time=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("end_time=0"); 	
				g_MapApiGateWayconfig["AveragePrice"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getTop10Platforms?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("category_id=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("platform_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("brand_id=0"); 	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("product_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_type=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("start_time=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("end_time=0"); 	
				g_MapApiGateWayconfig["Top10Platforms"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getTop10Brands?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("category_id=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("platform_id=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("brand_id=0"); 	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("product_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_type=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("start_time=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("end_time=0"); 	
				g_MapApiGateWayconfig["Top10Brands"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/syntun/getTop10Skus?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("access_key_id=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("live_time=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_stamp=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("signature=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("category_id=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("platform_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("brand_id=0"); 	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("product_id=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("time_type=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("start_time=0");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("end_time=0"); 	
				g_MapApiGateWayconfig["Top10Skus"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/pm25/cities?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("token=1");   
				g_MapApiGateWayconfig["Cities"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/pm25/stations?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("token=1");
				temp_MapApiGateWayconfig.mStrReqParams.push_back("city=1");
				g_MapApiGateWayconfig["Stations"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/pm25/aqi_by_station?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("token=1");  
				temp_MapApiGateWayconfig.mStrReqParams.push_back("station_code=1"); 
				g_MapApiGateWayconfig["AqiStation"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/pm25/aqi_by_city?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("token=1"); 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("city=1");
				g_MapApiGateWayconfig["Aqicity"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "211.152.122.185:443";
				temp_MapApiGateWayconfig.mStrCname = "way.jd.com";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/pm25/aqi_ranking?";
				temp_MapApiGateWayconfig.mStrApiKey = "appkey=537e2e5f0c948d4a6cd88c097918feee";
				temp_MapApiGateWayconfig.mStrReqParams.push_back("token=1");   
				g_MapApiGateWayconfig["Aqiranking"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "115.182.0.203:443";
				temp_MapApiGateWayconfig.mStrCname = "apis.juhe.cn";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/goodbook/catalog?";
				temp_MapApiGateWayconfig.mStrApiKey = "key=ade09cde074729c00b07c35925da1ee7";						  
				temp_MapApiGateWayconfig.mStrReqParams.push_back("key=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("dtype=0");																							  
				g_MapApiGateWayconfig["Catalog"] = temp_MapApiGateWayconfig;	
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "115.182.0.203:443";
				temp_MapApiGateWayconfig.mStrCname = "apis.juhe.cn";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/goodbook/query?";
				temp_MapApiGateWayconfig.mStrApiKey = "key=ade09cde074729c00b07c35925da1ee7";						  
				temp_MapApiGateWayconfig.mStrReqParams.push_back("key=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("catalog_id=1");	
				temp_MapApiGateWayconfig.mStrReqParams.push_back("pn=1");	   
				temp_MapApiGateWayconfig.mStrReqParams.push_back("rn=1");	  
				temp_MapApiGateWayconfig.mStrReqParams.push_back("dtype=0");					 
				g_MapApiGateWayconfig["content"] = temp_MapApiGateWayconfig;	
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "115.182.0.157:443";
				temp_MapApiGateWayconfig.mStrCname = "op.juhe.cn";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/onebox/train/query?";
				temp_MapApiGateWayconfig.mStrApiKey = "key=945b843f2d163144a686646e2ef2d2b2";						  
				temp_MapApiGateWayconfig.mStrReqParams.push_back("train=1");   
				temp_MapApiGateWayconfig.mStrReqParams.push_back("key=1");																								
				g_MapApiGateWayconfig["Train"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "115.182.0.157:443";
				temp_MapApiGateWayconfig.mStrCname = "op.juhe.cn";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/onebox/train/query_ab?";
				temp_MapApiGateWayconfig.mStrApiKey = "key=945b843f2d163144a686646e2ef2d2b2";						  
				temp_MapApiGateWayconfig.mStrReqParams.push_back("from=1");   
				temp_MapApiGateWayconfig.mStrReqParams.push_back("to=1");  
				temp_MapApiGateWayconfig.mStrReqParams.push_back("key=1");																								
				g_MapApiGateWayconfig["TrainSite"] = temp_MapApiGateWayconfig; 
				
				
				temp_MapApiGateWayconfig.mStrReqParams.clear();
				temp_MapApiGateWayconfig.mStrHostInfo = "115.182.0.157:443";
				temp_MapApiGateWayconfig.mStrCname = "op.juhe.cn";
				temp_MapApiGateWayconfig.mIntIsHttps = 1;
				temp_MapApiGateWayconfig.mIntIsVerify = 1;
				temp_MapApiGateWayconfig.mIntQueryTimesLimit = 100000000;
				temp_MapApiGateWayconfig.mStrUrlPath = "/onebox/weather/query?";
				temp_MapApiGateWayconfig.mStrApiKey = "key=d7f95f29b0111e73bd72eef056a1b2a5";						  
				temp_MapApiGateWayconfig.mStrReqParams.push_back("cityname=1");   
				temp_MapApiGateWayconfig.mStrReqParams.push_back("key=1");	 
				temp_MapApiGateWayconfig.mStrReqParams.push_back("dtype=0");																							 
				g_MapApiGateWayconfig["Weather"] = temp_MapApiGateWayconfig;		 
																														
																														
																														
																																																																																												
				







				
								
				sleep(600);
		}	
	}

}


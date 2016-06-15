/*
 * TaskMain.cpp
 */

#include "TaskMain.h"
#include "../../CommonTools/UrlEncode/UrlEncode.h"
#include "../../UserQueryWorkThreads/UserQueryWorkThreads.h"

#include "../../CommonTools/Base64Encode/Base64.h"
#include "../../CommonTools/Base64Encode/Base64_2.h"
#include "../../../include/json/json.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>



extern CLog *gp_log;
const char* CTaskMain::m_pszHttpHeaderEnd = "\r\n\r\n";
const char* CTaskMain::m_pszHttpLineEnd = "\r\n";
const std::string CTaskMain::keyEdcpMd5Sign="edc_543_key_155&";
extern std::map<std::string,BDXPERMISSSION_S> g_mapUserInfo;
extern std::map<std::string,int> g_mapUserQueryLimit;
extern std::map<std::string,QUERYAPIINFO_S> g_vecUrlAPIS;


extern pthread_rwlock_t p_rwlock;
extern pthread_rwlockattr_t p_rwlock_attr;
extern pthread_mutex_t mutex;
extern std::string g_strTokenString ;
extern std::string ssToken;
extern u_int  g_iNeedUpdateToken ;
extern int iAPIQpsLimit;
extern std::map<int,std::string>mapIntStringOperator;
extern std::map<std::string,BDXAPIGATEWAYCONFIF_S> g_MapApiGateWayconfig;
std::string g_strDataHubToken;



int InitSSLFlag = 0;

static const string http=" HTTP/1.1";

static const char http200ok[] = "HTTP/1.1 200 OK\r\nServer: Bdx DMP/0.1.0\r\nCache-Control: must-revalidate\r\nExpires: Thu, 01 Jan 1970 00:00:00 GMT\r\nPragma: no-cache\r\nConnection: Keep-Alive\r\nContent-Type: application/json;charset=UTF-8\r\nDate: ";
//static const char http200ok[] = "";
static const char httpReq[]="GET %s HTTP/1.1\r\nHost: %s\r\nAccept-Encoding: identity\r\n\r\n";


#define __HTTPS__


CTaskMain::CTaskMain(CTcpSocket* pclSock):CUserQueryTask(pclSock)
{
	// TODO Auto-generated constructor stub
	m_piKLVLen = (int*)m_pszKLVBuf;
	m_piKLVContent = m_pszKLVBuf + sizeof(int);
	*m_piKLVLen = 0;
	m_httpType = 0;
}

CTaskMain::CTaskMain()
{

}

CTaskMain::~CTaskMain() {
	// TODO Auto-generated destructor stub

}

int CTaskMain::BdxRunTask(BDXREQUEST_S& stRequestInfo, BDXRESPONSE_S& stResponseInfo)
{
	string keyReq = "Req_"+BdxTaskMainGetTime();
	string keyEmptyRes = "EmptyRes_"+BdxTaskMainGetTime();
	string strErrorMsg,errValue;
    HIVELOCALLOG_S stHiveEmptyLog;
	int iRes = 0;
	if(!m_pclSock) {
		LOG(ERROR, "[thread: %d]m_pclSock is NULL.", m_uiThreadId);
		return LINKERROR;
	}

	iRes = 	BdxGetHttpPacket(stRequestInfo,stResponseInfo,strErrorMsg);	
	//printf("Line:%d,stResponseInfo=%s\n",__LINE__,stResponseInfo.mResValue.c_str());
	if(iRes == SUCCESS )
	{
		//printf("Line:%d,stResponseInfo=%s\n",__LINE__,stResponseInfo.mResValue.c_str());
		return BdxSendRespones( stRequestInfo, stResponseInfo,strErrorMsg);
	}
	else
	{
		return BdxSendEmpyRespones(strErrorMsg);
	}

}


int CTaskMain::BdxGetHttpPacket(BDXREQUEST_S& stRequestInfo,BDXRESPONSE_S &stResponseInfo,std::string &errorMsg)
{

	int iRes = 0;
	char* pszBody = NULL;
	u_int uiBodyLen = 0;	

	memset(m_pszAdxBuf,0,_8KBLEN);
	iRes = m_pclSock->TcpRead(m_pszAdxBuf, _8KBLEN);
 
	//printf("ThreadID: %d,res %d,Line%d,Requrest= %s\n",m_uiThreadId,iRes,__LINE__,m_pszAdxBuf);  
	if( iRes <= 0 ) 
	{		
		LOG(DEBUG, "[thread: %d]Read Socket Error [%d].", m_uiThreadId, iRes);
		//printf("[thread: %d]Read Socket Error [%d].\n", m_uiThreadId, iRes);
		errorMsg="1101 Read Socket Error";
		return LINKERROR;
	}

	iRes = BdxParseHttpPacket(pszBody, uiBodyLen, iRes); // pszBody no use
	//printf("Line:%d,[thread: %d] iRes=%d,m_pszAdxBuf=%s\n",__LINE__,m_uiThreadId,iRes,m_pszAdxBuf);

	if(iRes == SUCCESS) {
		return BdxParseBody(pszBody, uiBodyLen, stRequestInfo,stResponseInfo,errorMsg);
	}

	printf("Line:%d,iRes=%d\n",__LINE__,iRes);
	return iRes;
	
}

int CTaskMain::BdxParseHttpPacket(char*& pszBody, u_int& uiBodyLen, const u_int uiParseLen)
{

	char* pszTmp = NULL;
	char* pszPacket = m_pszAdxBuf;
	if(strncmp(m_pszAdxBuf, "GET", strlen("GET"))) {
		LOG(ERROR, "[thread: %d]It is not GET request.", m_uiThreadId);
		return PROTOERROR;
	}

	//find body
	pszTmp = strstr(pszPacket, m_pszHttpHeaderEnd);
	if(pszTmp == NULL) {
		LOG(ERROR, "[thread: %d]can not find Header End.", m_uiThreadId);
		return PROTOERROR;
	}

	#if 0

	//u_int uiHeadLen = 0;
	//int iRes = 0;
	//u_int uiReadLen = uiParseLen;
	//bool bChunked = false;

	if(!bChunked)	
	{
			pszTmp = strstr(pszPacket, "Content-Length:");
			if(pszTmp == NULL) {
				LOG(ERROR, "[thread: %d]can not find Content-Length", m_uiThreadId);
				return PROTOERROR;
			}
			uiBodyLen = atoi(pszTmp + strlen("Content-Length:"));
			LOG(DEBUG, "[thread: %d]body length is %u.", m_uiThreadId,uiBodyLen);
			if(!uiBodyLen) {
				LOG(ERROR, "[thread: %d]body length is 0.", m_uiThreadId);
				return PROTOERROR;
			}	
			while(uiBodyLen > uiReadLen - uiHeadLen) {

				iRes = m_pclSock->TcpRead(m_pszAdxBuf + uiReadLen,uiBodyLen-(uiReadLen - uiHeadLen));
				if(iRes <= 0){
				LOG(ERROR, "[thread: %d]Get Http Body error[return:%d][body length:%d][read length:%d][head length:%d].", m_uiThreadId, iRes, uiBodyLen, uiReadLen, uiHeadLen);
				//m_pclSock->EmTcpClose();
				return LINKERROR;
	
				}
				uiReadLen += iRes;
			}
		}
	#endif
	
	return SUCCESS;
}

int CTaskMain::BdxParseBody(char *pszBody, u_int uiBodyLen, BDXREQUEST_S& stRequestInfo,BDXRESPONSE_S &stResponseInfo,std::string & errorMsg)
{
	std::string strTimeStamp,strLiveTime,strAccessKeyId,strUserToken,strApiGateWayToken,strAccessPrivatekey,strSinature,strItem,strRepo;
	std::string postValue;
	std::map<std::string,std::string> map_UserValueKey;
	//std::map<std::string,std::string>::iterator iter2;
	std::map<std::string,BDXPERMISSSION_S>::iterator iter;
	std::map<std::string,int> mapActionFilter;
	std::string strAction,ssmoidValue,ssmoidValue2,strDataHubToken;
	std::string strUserOrderId,strMidCountOrderId,strMidLimitOrderId,strMidStatusOrderId,strMidExpireOrderId;

	std::string startOrderID = KEY_startOrderID;
	std::string startToken = KEY_startToken;
	std::string apiGateWayAdmin = KEY_apiGateWayAdmin;
	std::string startOrderCount  = KEY_startOrderCount;
	std::string startOrderLimit  = KEY_startOrderLimit;
	std::string startOrderStatus = KEY_startOrderStatus;
	std::string startOrderExpire = KEY_startOrderExpire;

	std::string startUserRepoItemSubid,valueUserRepoItemSubid;
	std::string strCountOrderId,strLimitOrderId,strStatusOrderId,strExpireOrderId,strTempSubID;

	std::string needUpdateOrder="need_updated_datahub_order";
	std::string alreadyUpdateOrder="already_updated_datahub_order";
	int lenStrTemp,iLimitOrderId,iCountOrderId;
	std::vector<DATAHUB_ORDER_INFO_S> vecDataHubOrder;

	
	//int lenStrTemp;
	//struct tm ts;
	//time_t timetNow,timetCreateTime,timetCreateTime2;
	//Json::Reader jReader;
	//Json::Reader *jReader= new Json::Reader(Json::Features::strictMode()); // turn on strict verify mode
	//Json::FastWriter jFastWriter;
	//Json::Value jValue,jRoot,jResult,jTemp;
	//Json::Reader *jReader= new Json::Reader(Json::Features::all());
	HIVELOCALLOG_S stHiveLog;
	
	char *temp[PACKET]; 
	int  index = 0;
	char bufTemp[PACKET];
	char *buf;
	char *outer_ptr = NULL;  
	char *inner_ptr = NULL;  

	memset(bufTemp, 0, PACKET);

	printf("Line:%d,m_pszAdxBuf=%s\n",__LINE__,m_pszAdxBuf);
	
	std::string ssContent = std::string(m_pszAdxBuf);
	ssContent=url_decode(ssContent.c_str());
	std::string tempssContent,strActionUrl,strReqParams;
	unsigned int ipos = ssContent.find(CTRL_N,0);
	unsigned int jpos = ssContent.find(REQ_TYPE,0);
	
	if( std::string::npos !=jpos )
	{
		m_httpType = 1;
	}
	if(m_httpType )
	{
		ssContent = ssContent.substr(jpos+4,ipos-http.length()-(jpos+4));
		stHiveLog.strReqParams = ssContent;
		
		int ibegin = ssContent.find(SEC_Q,0);
		int iend =   ssContent.find(BLANK,0);
		//int iend = ssContent.rfind(BLANK,ssContent.length());
		strAction =  ssContent.substr(0,ssContent.find(SEC_Q,0)); //strActionUrl api name
		strAction = strAction.substr(strAction.rfind("/",strAction.length())+1);

		printf("Line:%d,ssContent=%s\n",__LINE__,ssContent.c_str());
		
		if (ibegin!=-1 && iend !=-1)
		{
				ssContent = ssContent.substr(ibegin+1,iend - ibegin-1);	
				strReqParams = ssContent;
				//retUser=strReqParams;
				memcpy(bufTemp,ssContent.c_str(),ssContent.length());
				buf=bufTemp;
				while((temp[index] = strtok_r(buf, STRING_AND, &outer_ptr))!=NULL)   
				{  	
				    buf=temp[index];  
				    while((temp[index]=strtok_r(buf, STRING_EQUAL, &inner_ptr))!=NULL)   
				    {   if(index%2==1)
				        {
				            map_UserValueKey[temp[index-1]]=temp[index];
				            
				            
				        }
				        index++;
				        buf=NULL;  
				    }  
				    buf=NULL;  
				}  
				printf("Line:%d,strActionUrl=%s\n",__LINE__,strAction.c_str());

				std::map<std::string,std::string>::iterator itss;

				#if 0
				for(itss = map_UserValueKey.begin();itss!=map_UserValueKey.end();itss++)
				{
					printf("Line:%d,%s %s\n",__LINE__,itss->first.c_str(),itss->second.c_str());

				}
				#endif
		
				if(map_UserValueKey.find(KEY_ACCESS_KEY_ID)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_REPOSITIRY)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_ITEM)!=map_UserValueKey.end()&&map_UserValueKey.find(KEY_SIGNATURE)!=map_UserValueKey.end())
				{	
					strAccessKeyId = map_UserValueKey.find(KEY_ACCESS_KEY_ID)->second;
					strRepo = map_UserValueKey.find(KEY_REPOSITIRY)->second;
					strItem = map_UserValueKey.find(KEY_ITEM)->second;
					strSinature = map_UserValueKey.find(KEY_SIGNATURE)->second;

					stResponseInfo.ssUserName=strAccessKeyId;
					//stResponseInfo.ssOperatorName=strAccessKeyId+"_" + strRepo +"_" + strItem + "_"+strAction;//+"_"+strSinature;

					//stResponseInfo.ssUserCountKeyUserLimitReq = "Limit_"+BdxTaskMainGetDate()+"_"+strAccessKeyId+"_"+strAction;
					//stResponseInfo.ssUserCountKeyUserLimitReq = "Limit_"+strAccessKeyId;
					
					//req user count
					stResponseInfo.ssUserCountKeyReq = "Req_" + BdxTaskMainGetTime()+"_"+stResponseInfo.ssUserName;//+"_"+strAction;
					stResponseInfo.ssUserCountKeyRes = "Res_" + BdxTaskMainGetTime()+"_"+stResponseInfo.ssUserName;//"_"+strAction;
					stResponseInfo.ssUserCountKeyEmptyRes ="EmptyRes_"+BdxTaskMainGetTime()+"_"+ stResponseInfo.ssUserName;//+"_"+strAction;
					stResponseInfo.ssUserCountKeyLimitReq = "Limit_"+stResponseInfo.ssUserName;
					

					//remote server count
					//stResponseInfo.ssOperatorNameKeyReq = "Req_" + BdxTaskMainGetTime()+"_"+stResponseInfo.ssOperatorName;//+"_"+strAction;
					//stResponseInfo.ssOperatorNameKeyRes = "Res_" + BdxTaskMainGetTime()+"_"+stResponseInfo.ssOperatorName;//"_"+strAction;
					//stResponseInfo.ssOperatorNameKeyEmptyRes ="EmptyRes_"+BdxTaskMainGetTime()+"_"+ stResponseInfo.ssOperatorName;//+"_"+strAction;
					//stResponseInfo.ssOperatorNameKeyLimit = "Limit_"+stResponseInfo.ssOperatorName;

					
					m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyReq); //req++
					CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullReqNum++;


					strUserToken = strAccessKeyId +std::string(KEY_DELIMITER + startToken);



					printf("Line:%d,===========\n",__LINE__);

					m_pDataRedis->UserGet(strUserToken,ssmoidValue);
					if( strSinature.compare(ssmoidValue.c_str())!= 0 )
					{	
						#ifdef __HTTPS__
						if (BdxVerifyDataHubToken(strAccessKeyId,strSinature)==1)
						#endif
						{	
							m_pDataRedis->UserPutExpire(strUserToken,strSinature,3*3600);
						}
						else
						{
							errorMsg ="8001 user token is error"; //user token is error
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							return OTHERERROR;
						}
						//get local token,if verify is not ok,then get new token
						//BdxGetNewDataHubToken(authUser,authToken);						
					}

					strApiGateWayToken = apiGateWayAdmin +std::string(KEY_DELIMITER + startToken);
					if(m_pDataRedis->UserGet(strApiGateWayToken,ssmoidValue))
					{
						strDataHubToken  =	ssmoidValue;
						printf("Line:%d,Local strDataHubToken =%s\n",__LINE__,strDataHubToken.c_str());
					}
					else
					{	
						#ifdef __HTTPS__
						strDataHubToken = BdxApiGateWayGetDataHubToken();
						#endif
						if(strDataHubToken.empty())
						{
							errorMsg ="8002 gateway token is error"; //gateway token is error
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							return OTHERERROR;
						}
						printf("Line:%d,datahub strDataHubToken =%s\n",__LINE__,strDataHubToken.c_str());
						m_pDataRedis->UserPutExpire(strApiGateWayToken,strDataHubToken,3*3600); 				
					}
					g_strDataHubToken = strDataHubToken;
					strUserOrderId =  strAccessKeyId +std::string(KEY_DELIMITER + startOrderID );

					ssmoidValue = "";
					startUserRepoItemSubid = strAccessKeyId + KEY_DELIMITER + strRepo + KEY_DELIMITER + strItem  + KEY_DELIMITER;
					strMidCountOrderId   = startOrderCount  + KEY_DELIMITER + startUserRepoItemSubid;
					strMidLimitOrderId   = startOrderLimit  + KEY_DELIMITER + startUserRepoItemSubid;
					strMidStatusOrderId  = startOrderStatus + KEY_DELIMITER + startUserRepoItemSubid;
					strMidExpireOrderId  = startOrderExpire + KEY_DELIMITER + startUserRepoItemSubid;

					if(m_pDataRedis->UserGetSortedSet(strUserOrderId,ssmoidValue))
					{
						strCountOrderId = strMidCountOrderId + ssmoidValue;
						strLimitOrderId = strMidLimitOrderId + ssmoidValue;
						strStatusOrderId = strMidStatusOrderId + ssmoidValue;
						strExpireOrderId = strMidExpireOrderId + ssmoidValue;
						valueUserRepoItemSubid = startUserRepoItemSubid + ssmoidValue;
						strTempSubID = ssmoidValue;
						

						printf("Line:%d,Order is  =%s\n",__LINE__,ssmoidValue.c_str());
					}
					else
					{	
						printf("Line:%d,get datahub Order\n",__LINE__);
						//get user order
						vecDataHubOrder = BdxApiGetUserOrder(strRepo,strItem,strAccessKeyId,strDataHubToken);
						printf("Line:%d,vecDataHubOrder size=%d\n",__LINE__,vecDataHubOrder.size());

						//local store order info to redis
						for(int i=0;i<vecDataHubOrder.size();i++)
						{
							m_pDataRedis->UserPutOrderSet(strUserOrderId,vecDataHubOrder[i].m_subscriptionID,vecDataHubOrder[i].m_subscriptionID);

							valueUserRepoItemSubid = startUserRepoItemSubid + vecDataHubOrder[i].m_subscriptionID;
							//m_pDataRedis->UserSadd(needUpdateOrder,valueUserRepoItemSubid);
							printf("Line:%d,vecDataHubOrder[%d].m_subscriptionID=%s\n",__LINE__,i,vecDataHubOrder[i].m_subscriptionID.c_str());
							strCountOrderId = strMidCountOrderId + vecDataHubOrder[i].m_subscriptionID;
							strLimitOrderId = strMidLimitOrderId + vecDataHubOrder[i].m_subscriptionID;
							strStatusOrderId = strMidStatusOrderId + vecDataHubOrder[i].m_subscriptionID;
							strExpireOrderId = strMidExpireOrderId + vecDataHubOrder[i].m_subscriptionID;
							strTempSubID = vecDataHubOrder[i].m_subscriptionID;
							m_pDataRedis->UserPut(strCountOrderId,std::string("0"));
							m_pDataRedis->UserPutExpire(strExpireOrderId,vecDataHubOrder[i].m_subscriptionID,atol(vecDataHubOrder[i].m_expireTime.c_str())*86400);
							m_pDataRedis->UserPut(strLimitOrderId,vecDataHubOrder[i].m_units);
							m_pDataRedis->UserPut(strStatusOrderId,std::string("0"));							
							if(!BdxApiSetUserOrderStatus(strAccessKeyId,strTempSubID,strRepo,strItem,strDataHubToken))
							{	
								//update datahub status retrieve faild,then jump it
								printf("Line:%d,BdxApiSetUserOrderStatus\n",__LINE__);
								m_pDataRedis->UserRemoveSortedSet(strUserOrderId,ssmoidValue);
								//m_pDataRedis->UserDel(strCountOrderId,std::string("0"));
								//m_pDataRedis->UserDel(strExpireOrderId,vecDataHubOrder[i].m_subscriptionID);
								//m_pDataRedis->UserDel(strLimitOrderId,vecDataHubOrder[i].m_units);
								//m_pDataRedis->UserDel(strStatusOrderId,std::string("0"));	
								continue;
							}

						}
						ssmoidValue.clear();
						m_pDataRedis->UserGetSortedSet(strUserOrderId,ssmoidValue);
						strCountOrderId = strMidCountOrderId + ssmoidValue;
						strLimitOrderId = strMidLimitOrderId + ssmoidValue;
						strStatusOrderId = strMidStatusOrderId + ssmoidValue;
						strExpireOrderId = strMidExpireOrderId + ssmoidValue;
						valueUserRepoItemSubid = startUserRepoItemSubid + ssmoidValue;
						strTempSubID = ssmoidValue;
						printf("Line:%d,valueUserRepoItemSubid=%s\n",__LINE__,valueUserRepoItemSubid.c_str());

			
					}	

					ssmoidValue.clear();
					ssmoidValue2.clear();

					if((m_pDataRedis->UserGet(strCountOrderId,ssmoidValue))&&(m_pDataRedis->UserGet(strLimitOrderId,ssmoidValue2)))
					{	
						iCountOrderId = atoi(ssmoidValue.c_str());
						iLimitOrderId = atoi(ssmoidValue2.c_str());

						printf("Line:%d,iCountOrderId==%d\n",__LINE__,iCountOrderId);
						printf("Line:%d,iLimitOrderId==%d\n",__LINE__,iLimitOrderId);
						printf("Line:%d,strExpireOrderId=%s\n",__LINE__,strExpireOrderId.c_str());

						if(( iCountOrderId >= iLimitOrderId )||(!m_pDataRedis->UserGet(strExpireOrderId,ssmoidValue)))
						{
							printf("Line:%d,over order limit\n",__LINE__);
							//delete order
							ssmoidValue.clear();
							printf("Line:%d,################################\n",__LINE__);
							m_pDataRedis->UserRemoveSortedSet(strUserOrderId,ssmoidValue);

							ssmoidValue.clear();
							if(m_pDataRedis->UserGet(strStatusOrderId,ssmoidValue))
							{
								if(atoi(ssmoidValue.c_str())==0)
								{
									//update datahub order used
									BdxApiUpdateUserOrder(strAccessKeyId,strTempSubID,strRepo,strItem,strDataHubToken,iCountOrderId);
									//update local order status 
									m_pDataRedis->UserSmove(needUpdateOrder,alreadyUpdateOrder,valueUserRepoItemSubid);
									m_pDataRedis->UserPut(strStatusOrderId,std::string("1"));
								}
							
							}		
							errorMsg = "8003 over quato limit or order is expired";//over quato limit or order is expired
							LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
							return OTHERERROR;
						}
					}
					else
					{
							errorMsg = "8004 can't get order info";//can't get order info
							LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
							return OTHERERROR;
					}				
					
				 }
				 else
				 {
						errorMsg ="1100 param missing or error"; //param missing or error
						LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
						printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
						return OTHERERROR;
				 }

				stResponseInfo.ssOperatorName=strAccessKeyId+"_" + strRepo +"_" + strItem + "_"+strAction+"_"+strTempSubID;//+"_"+strSinature;
				//remote server count
				stResponseInfo.ssOperatorNameKeyReq = "Req_" + BdxTaskMainGetTime()+"_"+stResponseInfo.ssOperatorName;//+"_"+strAction;
				stResponseInfo.ssOperatorNameKeyRes = "Res_" + BdxTaskMainGetTime()+"_"+stResponseInfo.ssOperatorName;//"_"+strAction;
				stResponseInfo.ssOperatorNameKeyEmptyRes ="EmptyRes_"+BdxTaskMainGetTime()+"_"+ stResponseInfo.ssOperatorName;//+"_"+strAction;
				stResponseInfo.ssOperatorNameKeyLimit = "Limit_"+stResponseInfo.ssOperatorName;


				ssContent = ssContent.substr(0,ipos);

				std::map<std::string,BDXAPIGATEWAYCONFIF_S>::iterator itr = g_MapApiGateWayconfig.find(strAction);
				BDXAPIGATEWAYCONFIF_S strGateWayconfig;
				std::string tempValue,realParam,realValue;
				int equalPosition;
				//printf("Line:%d,g_MapApiGateWayconfig=%d\n",__LINE__,g_MapApiGateWayconfig.size());
				if(itr!=g_MapApiGateWayconfig.end())
				{

					printf("Line:%d,g_MapApiGateWayconfig....\n",__LINE__);
					strGateWayconfig.mStrHostInfo 	=  itr->second.mStrHostInfo;
					strGateWayconfig.mStrCname  	=  itr->second.mStrCname;
					strGateWayconfig.mStrUrlPath  	=  itr->second.mStrUrlPath;
					strGateWayconfig.mStrApiKey   	=  itr->second.mStrApiKey;
					strGateWayconfig.mIntIsHttps 	=  itr->second.mIntIsHttps;
					strGateWayconfig.mIntIsVerify 	=  itr->second.mIntIsVerify;
					strGateWayconfig.mStrReqParams	=  itr->second.mStrReqParams;
					strGateWayconfig.mIntQueryTimesLimit  =  itr->second.mIntQueryTimesLimit;


					for(std::vector<std::string>::iterator itr2 = strGateWayconfig.mStrReqParams.begin();itr2!=strGateWayconfig.mStrReqParams.end();itr2++ )
					{
						printf("Line:%d,####################### (*itr2)=%s\n",__LINE__,(*itr2).c_str());

					}
					

					for(std::vector<std::string>::iterator itr = strGateWayconfig.mStrReqParams.begin();itr!=strGateWayconfig.mStrReqParams.end();)
					{
						//printf("Line:%d,before (*itr)=%s\n",__LINE__,(*itr).c_str());
						equalPosition = (*itr).find("=",0);
						if( equalPosition == -1 )
						{
							errorMsg ="1400 config req param is not wrong";
							printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
							return OTHERERROR;
						}
						tempValue = (*itr).substr(equalPosition + 1 );
						realParam = (*itr).substr(0,equalPosition);
						if(map_UserValueKey.find(realParam)!=map_UserValueKey.end())
						{
							//printf("#########################\n");
							realValue = map_UserValueKey.find(realParam)->second;
							(*itr)= BdxTaskMainReplace_FirstOnece((*itr),tempValue,realValue);
							itr++;
						}
						else if( atoi(tempValue.c_str()) == 1 )
						{	
								errorMsg ="1401  req param is missing " + realParam;
								printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
								return OTHERERROR;
						}	
						else
						{
							itr = strGateWayconfig.mStrReqParams.erase(itr);
							//itr--;
							continue;
							
						}
						//printf("Line:%d,after (*itr)=%s\n",__LINE__,(*itr).c_str());

					}
					for(std::vector<std::string>::iterator itr2 = strGateWayconfig.mStrReqParams.begin();itr2!=strGateWayconfig.mStrReqParams.end();itr2++ )
					{
						printf("Line:%d,$$$$$$$$$$$$$$$$4 (*itr2)=%s\n",__LINE__,(*itr2).c_str());

					}
					m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyReq); 
					CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullReqNum++;

					
					int iRes = BdxGetRemoteGateWayData(strGateWayconfig,stRequestInfo,stResponseInfo,errorMsg);	
					stHiveLog.strAction = stRequestInfo.m_strRequestParam;
					stHiveLog.strAction = base64_encode(reinterpret_cast<const unsigned char*>(stHiveLog.strAction.c_str()),stHiveLog.strAction.length());
					stHiveLog.strValue = stResponseInfo.mResValue;

					stHiveLog.strQuerytime=BdxTaskMainGetFullTime();
					stHiveLog.strDayId=BdxTaskMainGetDate();
					stHiveLog.strHourId=stHiveLog.strQuerytime.substr(8,2);
					CUserQueryWorkThreads::m_vecHiveLog[m_uiThreadId].push(stHiveLog);
					
					if( iRes != 0)
					{
						errorMsg ="1104 get remote api data is  error"; //
						stResponseInfo.ssUserCountKeyEmptyRes ="EmptyRes_"+BdxTaskMainGetTime()+"_"+ strAccessKeyId;//+"_"+strAction;
		
						m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyEmptyRes); 
						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullEmptyResNum++;

						m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyEmptyRes); 
						CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullEmptyResNum++;

						LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
						printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
						return OTHERERROR;
					}
					m_pDataRedis->UserIncr(strCountOrderId);
					m_pDataRedis->UserIncr(stResponseInfo.ssOperatorNameKeyRes); 
					CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssOperatorName].m_ullResNum++;
					
					m_pDataRedis->UserIncr(stResponseInfo.ssUserCountKeyRes); 
					CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stResponseInfo.ssUserName].m_ullResNum++;
					return SUCCESS;
				

				}
				else
				{
					errorMsg ="1103 apiname is not exists"; //accekeykey is not exists
					LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
					printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
					return OTHERERROR;
				}
		}
		else
		{
			 errorMsg = "1100  request param is error";	// request param is error
			 printf("line %d,s Error: %s\n",__LINE__,errorMsg.c_str());
			 LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
			 return OTHERERROR;
		}
	}
	else
	{
		errorMsg = "1101 request type is error";	// request type  is error ( GET )
		LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
		return OTHERERROR;
	}

    return SUCCESS;
}



int CTaskMain::BdxVerifyDataHubToken(std::string AuthUser,std::string AuthToken) 
{
		int verifyOK = 0;
		std::string retDataHub;

		retDataHub = BdxGetDatafromDataHub(AuthUser,AuthToken,"","","",0,1);

		printf("Line:%d,BdxVerifyDataHubToken retDataHub=%s\n",__LINE__,retDataHub.c_str());
		if(retDataHub.find("\"msg\": \"OK\"")!=-1)
		{
			verifyOK = 1;
		}
		return verifyOK;      
}

std::string CTaskMain::BdxGetDatafromDataHub(std::string AuthUser,std::string AuthToken,std::string repo,std::string item,std::string subid,long used,int type)
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
	string strType;
	sslLocalSocket=new CTcpSocket(datahubPort,datahubIP);
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

std::string CTaskMain::BdxApiGateWayGetDataHubToken(std::string AuthUser,std::string PassWord) 
{
		std::string retDataHub;
		retDataHub = BdxGetDatafromDataHub(AuthUser,PassWord,"","","",0,2);
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

bool CTaskMain::BdxApiSetUserOrderStatus(std::string user,std::string subid,std::string repo,std::string item,std::string token) 
{
		std::string retDataHub;
		bool status = false;
		retDataHub = BdxGetDatafromDataHub(user,token,repo,item,subid,0,4);
		if(retDataHub.find("\"msg\":\"OK\"")!=-1)
		{
			status = true;
		}
		return status;   
}

std::vector<DATAHUB_ORDER_INFO_S> CTaskMain::BdxApiGetUserOrder(std::string repo,std::string item,std::string user,std::string apitoken) 
{
		std::string retDataHub;
		std::vector<DATAHUB_ORDER_INFO_S> vecDataHubOrder;
		DATAHUB_ORDER_INFO_S dataHubInfo;
		//Json::Reader *jReader = new Json::Reader(Json::Features::strictMode()); // turn on strict verify mode
		Json::Reader *jReader = new Json::Reader();
		Json::Value jValue,jValue2,jRoot,jResult,jTemp;
		Json::FastWriter jFastWriter;
		int lenStrTemp;
		int jsonFormatValid = 0;
		char tempSubid[50];

		retDataHub = BdxGetDatafromDataHub(user,apitoken,repo,item,"",0,3);
		if(retDataHub.find("\"msg\":\"OK\"")!=-1)
		{
			//retDataHub = retDataHub.substr(retDataHub.find("token")+ 9 ,32);
			if(jReader->parse(retDataHub,jValue))
			{ 
				if(jReader->parse(jValue["data"].toStyledString(), jValue))
				{
					if(jReader->parse(jValue["results"].toStyledString(), jValue))
					{						
							for(unsigned int i=0;i<jValue.size();i++)
							{
								//printf("Line:%d,i=%d,jValue[i][\"phase\"]=%s\n",__LINE__,i,jValue[i]["phase"].toStyledString().c_str());
								
								if(jValue[i]["phase"].toStyledString()=="1\n")
								{

									memset(tempSubid,0,50);
									sprintf(tempSubid,"%ld",atol(jValue[i]["subscriptionid"].toStyledString().c_str()));
									//dataHubInfo.m_subscriptionID= jValue[i]["subscriptionid"].toStyledString();
									dataHubInfo.m_subscriptionID = std::string(tempSubid);
					
									if(jReader->parse(jValue[i]["plan"].toStyledString(), jValue2))
									{
										jsonFormatValid = 1;
										memset(tempSubid,0,50);
										sprintf(tempSubid,"%ld",atol(jValue2["units"].toStyledString().c_str()));
										dataHubInfo.m_units= std::string(tempSubid);

										memset(tempSubid,0,50);
										sprintf(tempSubid,"%ld",atol(jValue2["expire"].toStyledString().c_str()));
										dataHubInfo.m_expireTime= std::string(tempSubid);
										
										printf("Line:%d,dataHubInfo.m_units=%s",__LINE__,dataHubInfo.m_units.c_str());
										printf("Line:%d,dataHubInfo.m_expireTime=%s",__LINE__,dataHubInfo.m_expireTime.c_str());
										printf("Line:%d,dataHubInfo.m_subscriptionID=%s\n",__LINE__,dataHubInfo.m_subscriptionID.c_str());

										vecDataHubOrder.push_back(dataHubInfo);

									}
								}
							}
						
					}
				 }
			}
			if( !jsonFormatValid )
			{
				retDataHub = "";
			}			
		  //retDataHub = retDataHub;
		}

		delete jReader; 
		return vecDataHubOrder;   
}


std::string CTaskMain::BdxApiUpdateUserOrder(std::string user,std::string subid,std::string repo,std::string item,std::string token,long used) 
{
		std::string retDataHub;
		retDataHub = BdxGetDatafromDataHub(user,token,repo,item,subid,used,5);
		printf("Line:%d,BdxApiUpdateUserOrder retDataHub=%s",__LINE__,retDataHub.c_str());
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

void CTaskMain::BdxGenerateReqUrl(BDXAPIGATEWAYCONFIF_S stGataWayReqParam,char *buffer)
{
	 std::string strReqUrl;
	 
	 for(std::vector<std::string>::iterator itr = stGataWayReqParam.mStrReqParams.begin();itr!=stGataWayReqParam.mStrReqParams.end();itr++ )
	 {
		strReqUrl += (*itr)+ "&";
	 }
	 if(stGataWayReqParam.mIntIsVerify == 1)
	 {
		strReqUrl = strReqUrl + stGataWayReqParam.mStrApiKey;
		strReqUrl = stGataWayReqParam.mStrUrlPath + strReqUrl;
	 }
	 else
	 {
		strReqUrl = strReqUrl.substr(0,strReqUrl.length() -1);
		strReqUrl = stGataWayReqParam.mStrUrlPath + strReqUrl;
	 }
	 printf("Line:%d,strReqUrl=%s\n",__LINE__,strReqUrl.c_str());
	 sprintf(buffer,httpReq,strReqUrl.c_str(),stGataWayReqParam.mStrCname.c_str());
}

int  CTaskMain::BdxGetRemoteGateWayData(BDXAPIGATEWAYCONFIF_S stGataWayReqParam, BDXREQUEST_S& stRequestInfo,BDXRESPONSE_S &stResponseInfo,std::string & errorMsg)
{
	char remoteBuffer[_8KBLEN];
	std::string strremoteBuffer;
	CTcpSocket* remoteSocket;
	std::string remoteIp;
	uint16_t remotePort;
	char m_httpReq[_8KBLEN];

	int uiReadLen = 0;
	int uiHeadLen = 0;
	int iRes = 0;
	char* pszTmp = NULL;
	char* pszBody;
	int uiBodyLen;
	char* pszPacket = NULL;

	printf("Line:%d,BdxGetRemoteGateWayData....\n",__LINE__);
	memset(m_httpReq,0,sizeof(m_httpReq));

	BdxGenerateReqUrl(stGataWayReqParam,m_httpReq);

	printf("Line:%d,m_httpReq=%s\n",__LINE__,m_httpReq);

	remoteIp.assign(stGataWayReqParam.mStrHostInfo,0,stGataWayReqParam.mStrHostInfo.find(":",0));
	remotePort = atoi(stGataWayReqParam.mStrHostInfo.substr(stGataWayReqParam.mStrHostInfo.find(":",0)+1).c_str());

	stRequestInfo.m_strRequestParam = stGataWayReqParam.mStrHostInfo + "###" + std::string(m_httpReq);
	
	printf("Line:%d,InitSSLFlag=%d\n",__LINE__,InitSSLFlag);
	printf("Line:%d,remotePort=%d,remoteIp=%s\n",__LINE__,remotePort,remoteIp.c_str());
	remoteSocket=new CTcpSocket(remotePort,remoteIp);
	if(remoteSocket->TcpConnect()==0)
	{
		if(stGataWayReqParam.mIntIsHttps == 1)
		{
			printf("Line:%d,https  https https\n",__LINE__);
			pthread_mutex_lock (&mutex);
			if ( InitSSLFlag == 0 )
			{
				remoteSocket->TcpSslInitParams();
				InitSSLFlag = 1;				
			}
			pthread_mutex_unlock(&mutex);
			if(remoteSocket->TcpSslInitEnv()==0)
			{
				printf("Line:%d,TcpSslInitEnv...\n",__LINE__);
				if(remoteSocket->TcpSslConnect())
				{
					printf("Line:%d,TcpSslConnect...\n",__LINE__);
					if(remoteSocket->TcpSslWriteLen(m_httpReq,strlen(m_httpReq))!=0)
					{
						printf("Line:%d,TcpSslWriteLen...\n",__LINE__);
						memset(remoteBuffer,0,sizeof(remoteBuffer));
						uiReadLen = remoteSocket->TcpSslReadLen(remoteBuffer,sizeof(remoteBuffer));
						//printf("Line%d,remoteBuffer=%s\n",__LINE__,remoteBuffer);

						pszPacket = remoteBuffer;
						if(uiReadLen < 0 )
						{
							remoteSocket->TcpSslDestroy();
							errorMsg = "2003 read socket is error";
							//printf("Line:%d,1111111111111111111111\n",__LINE__);
							LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
							return OTHERERROR;
						}
						pszTmp = strstr(pszPacket, m_pszHttpHeaderEnd);
						if(pszTmp == NULL) 
						{
							remoteSocket->TcpSslDestroy();
							//printf("Line:%d,2222222222222222222222222222\n",__LINE__);
							errorMsg = "2003 read socket is error";
							LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
							return OTHERERROR;
						}

						pszBody = pszTmp + strlen(m_pszHttpHeaderEnd);
						uiHeadLen = pszBody - remoteBuffer;
						pszTmp = strstr(pszPacket, "Content-Length:");
						if(pszTmp == NULL) 
						{
							remoteSocket->TcpSslDestroy();
							//printf("Line:%d,33333333333333333333\n",__LINE__);
							errorMsg = "2003 read socket is error";
							LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
							return OTHERERROR;
						}
						uiBodyLen = atoi(pszTmp + strlen("Content-Length:"));
						if(!uiBodyLen) 
						{
							remoteSocket->TcpSslDestroy();
							errorMsg = "2003 read socket is error";
							LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
							return OTHERERROR;
						
						}
						while(uiBodyLen > uiReadLen - uiHeadLen) 
						{
							iRes = remoteSocket->TcpSslReadLen(remoteBuffer + uiReadLen,uiBodyLen-(uiReadLen - uiHeadLen));
							if(iRes <= 0){
								remoteSocket->TcpClose();
								continue;
						
							}
							uiReadLen += iRes;
						}
						if( strlen(remoteBuffer)>0 )
						{
							strremoteBuffer = std::string(remoteBuffer);
							if(strremoteBuffer.find("Transfer-Encoding: chunked")!=std::string::npos)
							{
								int itrunkStart = strremoteBuffer.find("\r\n",strremoteBuffer.find("\r\n\r\n")+4);
								int itrunkEnd = strremoteBuffer.find("\r\n",itrunkStart + 1);
								strremoteBuffer=strremoteBuffer.substr(itrunkStart + 2 ,itrunkEnd - itrunkStart -1);
							}
							else
							{
								if(strremoteBuffer.find("\r\n\r\n")!=std::string::npos)
								{
								  int lenStrTemp = strremoteBuffer.length();
								  strremoteBuffer = strremoteBuffer.substr(strremoteBuffer.find("\r\n\r\n")+4,lenStrTemp -(strremoteBuffer.find("\r\n\r\n")+4));
								}
							}
						}
					
					}
				
				}
			}
			remoteSocket->TcpSslDestroy();
		}
		else
		{
			if(remoteSocket->TcpWrite(m_httpReq,strlen(m_httpReq))!=0)
			{
				memset(remoteBuffer,0,sizeof(remoteBuffer));
				uiReadLen = remoteSocket->TcpRead(remoteBuffer,sizeof(remoteBuffer));
				//printf("Line:%d,remoteBuffer=%s\n",__LINE__,remoteBuffer);
				pszPacket = remoteBuffer;
				if(uiReadLen < 0 )
				{
					remoteSocket->TcpClose();
					errorMsg = "2002 read socket is error";
					printf("Line:%d,1111111111111111111111\n",__LINE__);
					LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
					return OTHERERROR;
				}
				pszTmp = strstr(pszPacket, m_pszHttpHeaderEnd);
				if(pszTmp == NULL) 
				{
					remoteSocket->TcpClose();
					printf("Line:%d,2222222222222222222222222222\n",__LINE__);
					errorMsg = "2002 read socket is error";
					LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
					return OTHERERROR;
				}
				pszBody = pszTmp + strlen(m_pszHttpHeaderEnd);
				uiHeadLen = pszBody - remoteBuffer;
				pszTmp = strstr(pszPacket, "Content-Length:");
				if(pszTmp == NULL) 
				{
					remoteSocket->TcpClose();
					printf("Line:%d,33333333333333333333\n",__LINE__);
					errorMsg = "2002 read socket is error";
					LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
					return OTHERERROR;
				}
				uiBodyLen = atoi(pszTmp + strlen("Content-Length:"));
				if(!uiBodyLen) 
				{
					remoteSocket->TcpClose();
					errorMsg = "2002 read socket is error";
					LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
					return OTHERERROR;

				}

				while(uiBodyLen > uiReadLen - uiHeadLen) 
				{
					iRes = remoteSocket->TcpRead(remoteBuffer + uiReadLen,uiBodyLen-(uiReadLen - uiHeadLen));
					//LOG(ERROR,"iRes	   **************=%d",iRes);
					//printf("Line:%d,remoteBuffer=%s\n",__LINE__,remoteBuffer);
					if(iRes <= 0){
						remoteSocket->TcpClose();
						continue;
				
					}
					uiReadLen += iRes;
				}
				if( strlen(remoteBuffer)>0 )
				{
					strremoteBuffer = std::string(remoteBuffer);
					if(strremoteBuffer.find("\r\n\r\n")!=std::string::npos)
					{
					  int lenStrTemp = strremoteBuffer.length();
					  strremoteBuffer = strremoteBuffer.substr(strremoteBuffer.find("\r\n\r\n")+4,lenStrTemp -(strremoteBuffer.find("\r\n\r\n")+4));
					}
				}
			}
			remoteSocket->TcpClose();				
		}
	}
	else
	{
		errorMsg = "2001 connect remote is error";
		LOG(DEBUG,"errorMsg=%s",errorMsg.c_str());
		return OTHERERROR;
	}
	//printf("Line:%d,receive from  ReadBuffer=%s\n",__LINE__,strremoteBuffer.c_str());				
	stResponseInfo.mResValue = strremoteBuffer;
	return SUCCESS;
}

int CTaskMain::BdxSendEmpyRespones(std::string errorMsg)
{
	m_clEmTime.TimeOff();
	std::string strOutput=errorMsg;	
	char pszDataBuf[_8KBLEN];
	memset(pszDataBuf, 0, _8KBLEN);
	sprintf((char *)pszDataBuf, "%s%sContent-Length: %d\r\n\r\n", http200ok,BdxGetHttpDate().c_str(),(int)strOutput.length());
	int iHeadLen = strlen(pszDataBuf);
	
	memcpy(pszDataBuf + iHeadLen, strOutput.c_str(), strOutput.length());
	//printf("Line:%d,AdAdxSendEmpyRespones=%s\n",__LINE__,pszDataBuf);
	LOG(DEBUG,"Thread : %d ,AdAdxSendEmpyRespones=%s\n",m_uiThreadId,pszDataBuf);
	if(!m_pclSock->TcpWrite(pszDataBuf, iHeadLen + strOutput.length())) {
		LOG(ERROR, "[tread: %d]write empty response data error.", m_uiThreadId);
		return LINKERROR;
	}

	return SUCCESS;
}

int CTaskMain::BdxSendRespones(BDXREQUEST_S& stRequestInfo, BDXRESPONSE_S& stAdxRes,std::string errorMsg)
{
	memset(m_pszAdxResponse, 0, _64KBLEN);
	if( stAdxRes.mResValue.empty())
	{		
		std::string strOutput=errorMsg;
	}
	if(m_httpType)
	{
		sprintf((char *)m_pszAdxResponse, "%s%sContent-Length: %d\r\n\r\n", http200ok,BdxGetHttpDate().c_str(),(int)stAdxRes.mResValue.length());
		int iHeadLen = strlen(m_pszAdxResponse);
		memcpy(m_pszAdxResponse + iHeadLen, stAdxRes.mResValue.c_str(),stAdxRes.mResValue.length());
	}
	else
	{
		sprintf((char *)m_pszAdxResponse,"%s",stAdxRes.mResValue.c_str());
	}
	
	int iBodyLength = strlen(m_pszAdxResponse);
	iBodyLength=strlen(m_pszAdxResponse);



	if(!m_pclSock->TcpWrite(m_pszAdxResponse, iBodyLength)) 
	{
		CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stAdxRes.ssUserName].m_ullEmptyResNum++;
		CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stAdxRes.ssUserName].m_ullTotalEmptyResNum++;
		if(stAdxRes.queryType==1)// 1 query user index ,2 query goods 
		{
			m_pDataRedis->UserIncr(stAdxRes.ssUserCountKeyRes);

		}
		LOG(ERROR, "[thread: %d]write  response error.", m_uiThreadId);
		return LINKERROR;
	}

	//CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stAdxRes.ssUserName].m_ullResNum++;
	//CUserQueryWorkThreads::m_vecReport[m_uiThreadId].m_strUserInfo[stAdxRes.ssUserName].m_ullTotalResNum++;
	
	if(stAdxRes.queryType==1)// 1 query user index ,2 query goods 
	{
		m_pDataRedis->UserIncr(stAdxRes.ssUserCountKeyRes);

	}
	
	LOG(DEBUG, "[thread: %d]write response iBodyLength=%d.",m_uiThreadId,iBodyLength);
	
    return SUCCESS;
}

std::string CTaskMain::BdxTaskMainGetTime(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);
	sprintf(dt, "%4d%02d%02d%02d", timeinfo->tm_year + 1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour);
	//sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}

std::string CTaskMain::BdxTaskMainGetMinute(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);
	sprintf(dt, "%4d%02d%02d%02d%02d", timeinfo->tm_year + 1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min);
	//sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}

std::string CTaskMain::BdxTaskMainGetFullTime(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);
	sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}
std::string CTaskMain::BdxTaskMainGetUCTime(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
	{
		tmpTime = time(0);
	}
	else
	{
		tmpTime = ttime;
	}
	tmpTime -= 8*3600;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);

	sprintf(dt, "%4d-%02d-%02dT%02d:%02d:%02dZ", timeinfo->tm_year + 1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//sprintf(dt, "%4d%02d%02d%02d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}

std::string CTaskMain::BdxTaskMainGetDate(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);
	sprintf(dt, "%4d%02d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1,timeinfo->tm_mday);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}

std::string CTaskMain::BdxTaskMainGetLastTwoMonth(const time_t ttime)
{

	time_t tmpTime;
	if(ttime == 0)
		tmpTime = time(0);
	else
		tmpTime = ttime;
	tmpTime -= 86400*61;
	struct tm* timeinfo = localtime(&tmpTime);
	char dt[20];
	memset(dt, 0, 20);
	sprintf(dt, "%4d%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon+1);
	//return (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;
	return std::string(dt);
}

std::string CTaskMain::BdxGenNonce(int length) 
{
        char CHAR_ARRAY[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b','c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x','y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H','I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T','U', 'V', 'W', 'X', 'Y', 'Z'};
        srand((int)time(0));
         
        std::string strBuffer ;
        //int nextPos = strlen(CHAR_ARRAY);
        int nextPos = sizeof(CHAR_ARRAY);
        //printf("nextPos=%d\n",nextPos);
        int tmp = 0;
        for (int i = 0; i < length; ++i) 
        { 
            tmp = rand()%nextPos;
            
            strBuffer.append(std::string(1,CHAR_ARRAY[tmp]));
        }
        return strBuffer;
}

std::string CTaskMain::GenPasswordDigest(std::string utcTime, std::string nonce, std::string appSecret)
{
		std::string strDigest;

		std::string strValue = nonce + utcTime + appSecret;

        unsigned char *dmg = mdSHA1.SHA1_Encode(strValue.c_str());
        const  char *pchTemp = (const  char *)(char*)dmg;
        //std::string strDmg = base64_encode((const unsigned char*)pchTemp,strlen(pchTemp));
        std::string strDmg = base64_encode((const unsigned char*)pchTemp,SHA_DIGEST_LENGTH);
		//std::string strDmg = base64_encode(reinterpret_cast<const char *>(static_cast<void*>(dmg)),strlen(dmg));
        return strDmg;
}

string   CTaskMain::BdxTaskMainReplace_All(string    str,   string   old_value,   string   new_value)   
{   
    while(true)   {   
		//printf("$$$$$$$$$$\n");
		//sleep(1);
		//printf("LIne:%d,str=%s\n",__LINE__,str.c_str());
		//printf("LIne:%d,old_value=%s\n",__LINE__,old_value.c_str());
		//printf("LIne:%d,new_value=%s\n",__LINE__,new_value.c_str());
        string::size_type   pos(0);   
        if(   (pos=str.find(old_value))!=string::npos   )  
        {
            	str.replace(pos,old_value.length(),new_value); 
				//printf("Line:%d,replaced...\n",__LINE__);
        }
        else 
        {
         	break;   
		}
    }   
    return   str;   
}   


string   CTaskMain::BdxTaskMainReplace_FirstOnece(string    str,   string   old_value,   string   new_value)   
{   
		//printf("LIne:%d,str=%s\n",__LINE__,str.c_str());
		//printf("LIne:%d,old_value=%s\n",__LINE__,old_value.c_str());
		//printf("LIne:%d,new_value=%s\n",__LINE__,new_value.c_str());
        string::size_type   pos(0);   
        if(   (pos=str.find(old_value))!=string::npos   )  
        {
            	str.replace(pos,old_value.length(),new_value); 
				//printf("Line:%d,replaced...\n",__LINE__);
        }
    return   str;   
}   


std::string CTaskMain::BdxGetParamSign(const std::string& strParam, const std::string& strSign)
{
	char pszMd5Hex[33];
	std::string strParamKey = strParam + strSign;
	printf("Line:%d,strParamKey=%s\n",__LINE__,strParamKey.c_str());

    //计算参数串的128位MD5
    m_clMd5.Md5Init();
    m_clMd5.Md5Update((u_char*)strParamKey.c_str(), strParamKey.length());

    u_char pszParamSign[16];
    m_clMd5.Md5Final(pszParamSign);

    //以16进制数表示
    for (unsigned char i = 0; i < sizeof(pszParamSign); i++) {
    	sprintf(&pszMd5Hex[i * 2], "%c", to_hex(pszParamSign[i] >> 4));
    	sprintf(&pszMd5Hex[i * 2 + 1], "%c", to_hex((pszParamSign[i] << 4) >> 4));
    }
    pszMd5Hex[32] = '\0';
    return std::string(pszMd5Hex);
}


#pragma once

#include <string>
#include <vector>
#include "../curl/curl.h"
#pragma comment(lib, "libcurl_a.lib") 

typedef struct tagDownResourceInfo
{
	int			rowNum;				//�к�
	std::string	strResourceName;	//��Դ����
	std::string	strResourceCurl;	//��Դid
	BOOL		bComment;			//�Ƿ����۹�
}DownResourceInfo;

// ��¼post
typedef struct tagPostInfo
{
	std::string	username;
	std::string password;
	std::string lt;
	std::string	execution;
	std::string _eventId;
}PostInfo;

// ��¼���
typedef struct tagReturnInfo
{
	BOOL		bReturn;
	std::string strErrorInfo;
	std::string	data;
}ReturnInfo;

static BOOL UrlEncode(const char* szSrc, char* pBuf, int cbBufLen, BOOL bUpperCase)
{
	if (szSrc == NULL || pBuf == NULL || cbBufLen <= 0)
		return FALSE;

	size_t len_ascii = strlen(szSrc);
	if (len_ascii == 0)
	{
		pBuf[0] = 0;
		return TRUE;
	}

	//��ת����UTF-8
	char baseChar = bUpperCase ? 'A' : 'a';
	int cchWideChar = MultiByteToWideChar(CP_ACP, 0, szSrc, len_ascii, NULL, 0);
	LPWSTR pUnicode = (LPWSTR)malloc((cchWideChar + 1) * sizeof(WCHAR));
	if (pUnicode == NULL)
		return FALSE;
	MultiByteToWideChar(CP_ACP, 0, szSrc, len_ascii, pUnicode, cchWideChar + 1);

	int cbUTF8 = WideCharToMultiByte(CP_UTF8, 0, pUnicode, cchWideChar, NULL, 0, NULL, NULL);
	LPSTR pUTF8 = (LPSTR)malloc((cbUTF8 + 1) * sizeof(CHAR));
	if (pUTF8 == NULL)
	{
		free(pUnicode);
		return FALSE;
	}
	WideCharToMultiByte(CP_UTF8, 0, pUnicode, cchWideChar, pUTF8, cbUTF8 + 1, NULL, NULL);
	pUTF8[cbUTF8] = '\0';

	unsigned char c;
	int cbDest = 0; //�ۼ�
	unsigned char *pSrc = (unsigned char*)pUTF8;
	unsigned char *pDest = (unsigned char*)pBuf;
	while (*pSrc && cbDest < cbBufLen - 1)
	{
		c = *pSrc;
		if (isalpha(c) || isdigit(c) || c == '-' || c == '.' || c == '~')
		{
			*pDest = c;
			++pDest;
			++cbDest;
		}
		else if (c == ' ')
		{
			*pDest = '+';
			++pDest;
			++cbDest;
		}
		else
		{
			//��黺������С�Ƿ��ã�
			if (cbDest + 3 > cbBufLen - 1)
				break;
			pDest[0] = '%';
			pDest[1] = (c >= 0xA0) ? ((c >> 4) - 10 + baseChar) : ((c >> 4) + '0');
			pDest[2] = ((c & 0xF) >= 0xA) ? ((c & 0xF) - 10 + baseChar) : ((c & 0xF) + '0');
			pDest += 3;
			cbDest += 3;
		}
		++pSrc;
	}
	//null-terminator
	*pDest = '\0';
	free(pUnicode);
	free(pUTF8);
	return TRUE;
}

//�������utf-8����
static BOOL UrlDecode(const char* szSrc, char* pBuf, int cbBufLen)
{
	if (szSrc == NULL || pBuf == NULL || cbBufLen <= 0)
		return FALSE;

	size_t len_ascii = strlen(szSrc);
	if (len_ascii == 0)
	{
		pBuf[0] = 0;
		return TRUE;
	}

	char *pUTF8 = (char*)malloc(len_ascii + 1);
	if (pUTF8 == NULL)
		return FALSE;

	int cbDest = 0; //�ۼ�
	unsigned char *pSrc = (unsigned char*)szSrc;
	unsigned char *pDest = (unsigned char*)pUTF8;
	while (*pSrc)
	{
		if (*pSrc == '%')
		{
			*pDest = 0;
			//��λ
			if (pSrc[1] >= 'A' && pSrc[1] <= 'F')
				*pDest += (pSrc[1] - 'A' + 10) * 0x10;
			else if (pSrc[1] >= 'a' && pSrc[1] <= 'f')
				*pDest += (pSrc[1] - 'a' + 10) * 0x10;
			else
				*pDest += (pSrc[1] - '0') * 0x10;

			//��λ
			if (pSrc[2] >= 'A' && pSrc[2] <= 'F')
				*pDest += (pSrc[2] - 'A' + 10);
			else if (pSrc[2] >= 'a' && pSrc[2] <= 'f')
				*pDest += (pSrc[2] - 'a' + 10);
			else
				*pDest += (pSrc[2] - '0');

			pSrc += 3;
		}
		else if (*pSrc == '+')
		{
			*pDest = ' ';
			++pSrc;
		}
		else
		{
			*pDest = *pSrc;
			++pSrc;
		}
		++pDest;
		++cbDest;
	}
	//null-terminator
	*pDest = '\0';
	++cbDest;

	int cchWideChar = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, cbDest, NULL, 0);
	LPWSTR pUnicode = (LPWSTR)malloc(cchWideChar * sizeof(WCHAR));
	if (pUnicode == NULL)
	{
		free(pUTF8);
		return FALSE;
	}
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, cbDest, pUnicode, cchWideChar);
	WideCharToMultiByte(CP_ACP, 0, pUnicode, cchWideChar, pBuf, cbBufLen, NULL, NULL);
	free(pUTF8);
	free(pUnicode);
	return TRUE;
}

static void StringToWstring(std::wstring& szDst, std::string& str)
{
	std::string temp = str;
	int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)temp.c_str(), -1, NULL, 0);
	wchar_t * wszUtf8 = new wchar_t[len + 1];
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)temp.c_str(), -1, (LPWSTR)wszUtf8, len);
	szDst = wszUtf8;
	delete[] wszUtf8;
}

static std::string FormatString(const char * lpcszFormat, ...)
{
	char *pszStr = NULL;
	if (NULL != lpcszFormat)
	{
		va_list marker = NULL;
		va_start(marker, lpcszFormat); //��ʼ����������  
		size_t nLength = _vscprintf(lpcszFormat, marker) + 1; //��ȡ��ʽ���ַ�������  
		pszStr = new char[nLength];
		memset(pszStr, '\0', nLength);
		_vsnprintf_s(pszStr, nLength, nLength, lpcszFormat, marker);
		va_end(marker); //���ñ�������  
	}
	std::string strResult(pszStr);
	delete[]pszStr;
	return strResult;
}

static bool Utf8ToMb(char* strStcText, int nLen, std::string &strDstText)
{
	// convert an UTF8 string to widechar
	int nWLen = MultiByteToWideChar(CP_UTF8, 0, strStcText, nLen, NULL, 0);
	WCHAR *strTemp = (WCHAR*)malloc(sizeof(WCHAR)* nWLen);
	if (NULL == strTemp)
	{
		return false;
	}
	int nRtn = MultiByteToWideChar(CP_UTF8, 0, strStcText, nLen, strTemp, nWLen);
	if (nRtn == 0)
	{
		free(strTemp);
		return false;
	}
	// convert an widechar string to Multibyte  
	int MBLen = WideCharToMultiByte(CP_UTF8, 0, strTemp, nWLen, NULL, 0, NULL, NULL);
	if (0 == MBLen)
	{
		free(strTemp);
		return false;
	}
	char *str = (char*)malloc(sizeof(char)*MBLen);
	if (NULL == str)
	{
		return false;
	}
	nRtn = WideCharToMultiByte(CP_UTF8, 0, strTemp, nWLen, str, MBLen, NULL, NULL);
	if (0 == nRtn)
	{
		free(strTemp);
		free(str);
		return false;
	}
	//����ͷ����еı���
	strDstText = str;
	free(strTemp);
	free(str);
	return true;
}

static long writer(void *data, int size, int nmemb, std::string &content)
{
	long sizes = size * nmemb;
	std::string temp((char*)data, sizes);
	content += temp;
	return sizes;
}

static std::string GetPostParam(const char * sourses, const char* user, const char* pwd)
{
	bool isSuccess = false;
	std::string chunk;
	char postparam[256];
	long http_response_code = 0;

	PostInfo postInfo;
	postInfo.username = "460560842%40qq.com";
	postInfo.password = "198584";

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	/* initalize custom header list (stating that Expect: 100-continue is not
	wanted */

	if (curl) {
		/* what URL that receives this GET */
		curl_easy_setopt(curl, CURLOPT_URL, sourses);

		//ͨ��write_data������������������д�뵽data��
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		/* Perform the request, res will get the return code */

		res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
			isSuccess = false;
		}
		else
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response_code);
			if (http_response_code == 200)
			{
				int nPos = 0;
				int n = 0;
				int flag = 1;
				if ((nPos = chunk.find("name=\"lt\" value=", n)) != -1)
				{
					n = nPos + 17;
					int nEndPos = chunk.find("\" />", n);
					std::string strlt = chunk.substr(n, nEndPos - n);
					postInfo.lt = strlt;
				}
				if ((nPos = chunk.find("name=\"execution\" value=", n)) != -1)
				{
					n = nPos + 24;
					int nEndPos = chunk.find("\" />", n);
					std::string strexecution = chunk.substr(n, nEndPos - n);
					postInfo.execution = strexecution;
				}
				if ((nPos = chunk.find("name=\"_eventId\" value=", n)) != -1)
				{
					n = nPos + 23;
					int nEndPos = chunk.find("\" />", n);
					std::string strsubmit = chunk.substr(n, nEndPos - n);
					postInfo._eventId = strsubmit;
				}
				
				isSuccess = true;
			}
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	if (isSuccess)
	{
		sprintf(postparam, "username=%s&password=%s&lt=%s&execution=%s&_eventId=%s", postInfo.username.c_str(), postInfo.password.c_str(), postInfo.lt.c_str(), postInfo.execution.c_str(), postInfo._eventId.c_str());
	}
	return std::string(postparam);
}

/************************************************************************
* �������ã���Ա��¼����
* ����˵����strUser			�û���
strPass			��¼����
strErrMsg		������Ϣ
* �� �� ֵ��EnumResult		��¼���
* ��ע��Ϣ����
************************************************************************/
static ReturnInfo LoginServer(std::string strUser, std::string strPass)
{
	long http_response_code = 0;

	ReturnInfo returnInfo;
	returnInfo.bReturn = FALSE;

	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		//��ʼ��cookie����
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		//http����ͷ
		headers = curl_slist_append(headers, "User-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64; rv:25.0) Gecko/20100101 Firefox/25.0");	//ģ�������
		headers = curl_slist_append(headers, "Host:passport.csdn.net");
		headers = curl_slist_append(headers, "Accept:*/*");
		headers = curl_slist_append(headers, "Accept-Language:zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3");
		//headers = curl_slist_append(headers, "Accept-Encoding:gzip, deflate");
		headers = curl_slist_append(headers, "X-Requested-With:XMLHttpRequest");
		headers = curl_slist_append(headers, "Referer:https://passport.csdn.net/account/login?from=http%3A%2F%2Fdownload.csdn.net%2Fmy%2Fdownloads");
		headers = curl_slist_append(headers, "Connection:keep-alive");

		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookie.txt");		//�ѷ�������������cookie���浽cookie.txt

																		//����http����ͷ
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		char url[256] = "https://passport.csdn.net/account/login?from=http%3A%2F%2Fdownload.csdn.net%2Fmy%2Fdownloads";
		//char strpost[256] = "username=460560842%40qq.com&password=198584&lt=LT-445957-IEfKkNqFPFIdCXTfLM7ZeDcqxItUnd&execution=e4s1&_eventId=submit";
		std::string strpost = GetPostParam(url, strUser.c_str(), strPass.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strpost.c_str());

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		std::string content;
		//���ûص�����
		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

		//ִ��http����
		res = curl_easy_perform(curl);

		std::string returnVal;
		Utf8ToMb((char*)content.c_str(), content.length(), returnVal);
		int pos = returnVal.find("redirect_back");
		if (pos >= 0) {
			returnInfo.bReturn = TRUE;
			int nStartPos = content.find("data =");
			int nEndPos = content.rfind("\"}}");
			returnInfo.data = content.substr(nStartPos + 6, nEndPos - nStartPos - 4);
		}
		else {
			returnInfo.strErrorInfo = "failure";
		}

		//�ͷ���Դ
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
		headers = NULL;
	}
	curl_global_cleanup();
	return returnInfo;
}

static bool InitCurl(CURL *easy_handle, CURLcode &res, std::string &url, std::string &content)
{
	res = curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
	if (res != CURLE_OK)
		return FALSE;

	//�ص�����
	res = curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, writer);
	if (res != CURLE_OK)
		return FALSE;

	//�ص������Ĳ�����content
	res = curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &content);
	if (res != CURLE_OK)
		return FALSE;

	//ִ��http����
	res = curl_easy_perform(easy_handle);
	if (res != CURLE_OK)
		return FALSE;

	return TRUE;
}
//��ȡ֮ǰ���ص���Դ�б�
static std::string GetHtmlPage(std::string url)
{
	CURL *easy_handle;
	CURLcode res;
	std::string content;
	curl_global_init(CURL_GLOBAL_ALL);
	easy_handle = curl_easy_init();
	if (easy_handle)
	{
		//��ʼ��cookie����
		curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "");
		curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 5);			//��������ʱʱ��
																	//curl_easy_setopt(easy_handle,CURLOPT_VERBOSE,1);			//�������ͷ����Ӧͷ
																	//curl_easy_setopt(easy_handle,CURLOPT_HEADER,1);
		curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);

		//http����ͷ
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Host: download.csdn.net");
		headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:25.0) Gecko/20100101 Firefox/25.0");
		headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		headers = curl_slist_append(headers, "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3");
		headers = curl_slist_append(headers, "Referer: http://www.csdn.net/");
		headers = curl_slist_append(headers, "Connection: keep-alive");
		curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "cookie.txt");		//��ȡ���ش洢��cookie

		if (!InitCurl(easy_handle, res, url, content))
		{
			//�ͷ���Դ
			curl_slist_free_all(headers);
			curl_easy_cleanup(easy_handle);
			return NULL;
		}
		//�ͷ���Դ
		curl_slist_free_all(headers);
		curl_easy_cleanup(easy_handle);
	}
	curl_global_cleanup();
	std::string tt;
	Utf8ToMb((char *)content.c_str(), content.length(), tt);
	return tt;
}

//��ȡ������Դ��ҳ��
static int GetTotalPageNum()
{
	std::string url = "http://download.csdn.net/my/downloads/1";

	std::string html = GetHtmlPage(url);
	int nPos = html.rfind("βҳ");
	if (nPos == -1)
		return -1;
	nPos -= 2;
	int nStartPos = html.rfind("/", nPos);
	std::string strTotal = html.substr(nStartPos + 1, nPos - nStartPos - 1);
	return atoi(strTotal.c_str());
}

//��ȡ�����۵���Դ�б�
static std::vector<DownResourceInfo> GetToCommentList(int pageNum)
{
	std::vector<DownResourceInfo> vtDownload;
	char url[128] = { 0 };
	sprintf(url, "http://download.csdn.net/my/downloads/%d", pageNum);
	std::string html = GetHtmlPage(url);
	int nPos = 0;
	int n = 0;
	int flag = 1;
	while ((nPos = html.find("#comment", n)) != -1)
	{
		n = nPos + 1;
		int nStartPos = html.rfind("/", nPos);
		std::string strUrl = html.substr(nStartPos + 1, nPos - nStartPos - 1);
		DownResourceInfo info;
		info.strResourceCurl = strUrl;
		//��ȡ��Դ������
		nStartPos = html.find(strUrl, nPos + 1);
		if (nStartPos == -1)
			return vtDownload;
		nStartPos += 2;
		nStartPos += strUrl.length();
		int nEndPos = html.find("</a>", nStartPos);
		std::string ResourceName = html.substr(nStartPos, nEndPos - nStartPos);
		info.strResourceName = ResourceName;
		vtDownload.push_back(info);
	}
	return vtDownload;
}
//��������
static BOOL AddComment(std::string sourceId)
{
	CURL *easy_handle;
	CURLcode res;
	std::string content;
	curl_global_init(CURL_GLOBAL_ALL);
	easy_handle = curl_easy_init();
	if (easy_handle)
	{
		//��ʼ��cookie����
		curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "");
		curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
		std::string url = "http://download.csdn.net/index.php/comment/post_comment?jsonpcallback=jsonp1385304626524&sourceid=" + sourceId + "&content=%E9%9D%9E%E5%B8%B8%E6%84%9F%E8%B0%A2%EF%BC%8C%E8%BF%99%E8%B5%84%E6%BA%90%E6%88%91%E6%89%BE%E4%BA%86%E5%A5%BD%E4%B9%85%E4%BA%86%EF%BC%81&rating=5&t=1385304679900";
		std::string referer = "Referer: http://download.csdn.net/detail/wasdzxce/" + sourceId;
		//http����ͷ
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Host: download.csdn.net");
		headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:25.0) Gecko/20100101 Firefox/25.0");
		headers = curl_slist_append(headers, "Accept: text/javascript, application/javascript, */*");
		headers = curl_slist_append(headers, "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3");
		headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
		headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
		headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
		headers = curl_slist_append(headers, referer.c_str());
		headers = curl_slist_append(headers, "Connection: keep-alive");
		curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "cookie.txt");		//��ȡ���ش洢��cookie
		if (!InitCurl(easy_handle, res, url, content))
		{
			//�ͷ���Դ
			curl_slist_free_all(headers);
			curl_easy_cleanup(easy_handle);
			curl_global_cleanup();
			return FALSE;
		}
		//�ͷ���Դ
		curl_slist_free_all(headers);
		curl_easy_cleanup(easy_handle);
	}
	curl_global_cleanup();
	int pos = content.find("\"succ\":1");
	if (pos >= 0)
		return TRUE;
	else
		return FALSE;
}
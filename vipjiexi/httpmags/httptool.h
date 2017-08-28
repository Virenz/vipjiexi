#pragma once

#include <string>
#include <vector>
#include "../curl/curl.h"
#pragma comment(lib, "libcurl_a.lib") 

typedef struct tagDownResourceInfo
{
	int			rowNum;				//行号
	std::string	strResourceName;	//资源名称
	std::string	strResourceCurl;	//资源id
	BOOL		bComment;			//是否评论过
}DownResourceInfo;


// 登录结果
typedef struct tagReturnInfo
{
	BOOL		bReturn;
	std::string strErrorInfo;
	std::string	data;
}ReturnInfo;

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
	int MBLen = WideCharToMultiByte(CP_ACP, 0, strTemp, nWLen, NULL, 0, NULL, NULL);
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
	nRtn = WideCharToMultiByte(CP_ACP, 0, strTemp, nWLen, str, MBLen, NULL, NULL);
	if (0 == nRtn)
	{
		free(strTemp);
		free(str);
		return false;
	}
	//最后释放所有的变量
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

static bool InitCurl(CURL *easy_handle, CURLcode &res, std::string &url, std::string &content)
{
	res = curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
	if (res != CURLE_OK)
		return FALSE;

	//回调函数
	res = curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, writer);
	if (res != CURLE_OK)
		return FALSE;

	//回调函数的参数：content
	res = curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &content);
	if (res != CURLE_OK)
		return FALSE;

	//执行http请求
	res = curl_easy_perform(easy_handle);
	if (res != CURLE_OK)
		return FALSE;

	return TRUE;
}
//获取之前下载的资源列表
static std::string GetHtmlPage(std::string url)
{
	CURL *easy_handle;
	CURLcode res;
	std::string content;
	curl_global_init(CURL_GLOBAL_ALL);
	easy_handle = curl_easy_init();
	if (easy_handle)
	{
		//初始化cookie引擎
		curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "");
		curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, 5);			//设置请求超时时间
																	//curl_easy_setopt(easy_handle,CURLOPT_VERBOSE,1);			//输出请求头和响应头
																	//curl_easy_setopt(easy_handle,CURLOPT_HEADER,1);
		curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);

		//http请求头
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Host: download.csdn.net");
		headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:25.0) Gecko/20100101 Firefox/25.0");
		headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
		headers = curl_slist_append(headers, "Accept-Language: zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3");
		headers = curl_slist_append(headers, "Referer: http://www.csdn.net/");
		headers = curl_slist_append(headers, "Connection: keep-alive");
		curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "cookie.txt");		//读取本地存储的cookie

		if (!InitCurl(easy_handle, res, url, content))
		{
			//释放资源
			curl_slist_free_all(headers);
			curl_easy_cleanup(easy_handle);
			return NULL;
		}
		//释放资源
		curl_slist_free_all(headers);
		curl_easy_cleanup(easy_handle);
	}
	curl_global_cleanup();
	std::string tt;
	Utf8ToMb((char *)content.c_str(), content.length(), tt);
	return tt;
}

//获取下载资源总页数
static int GetTotalPageNum()
{
	std::string url = "http://download.csdn.net/my/downloads/1";

	std::string html = GetHtmlPage(url);
	int nPos = html.rfind("尾页");
	if (nPos == -1)
		return -1;
	nPos -= 2;
	int nStartPos = html.rfind("/", nPos);
	std::string strTotal = html.substr(nStartPos + 1, nPos - nStartPos - 1);
	return atoi(strTotal.c_str());
}

//获取待评论的资源列表
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
		//获取资源的名字
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
//发表评论
static BOOL AddComment(std::string sourceId)
{
	CURL *easy_handle;
	CURLcode res;
	std::string content;
	curl_global_init(CURL_GLOBAL_ALL);
	easy_handle = curl_easy_init();
	if (easy_handle)
	{
		//初始化cookie引擎
		curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "");
		curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
		std::string url = "http://download.csdn.net/index.php/comment/post_comment?jsonpcallback=jsonp1385304626524&sourceid=" + sourceId + "&content=%E9%9D%9E%E5%B8%B8%E6%84%9F%E8%B0%A2%EF%BC%8C%E8%BF%99%E8%B5%84%E6%BA%90%E6%88%91%E6%89%BE%E4%BA%86%E5%A5%BD%E4%B9%85%E4%BA%86%EF%BC%81&rating=5&t=1385304679900";
		std::string referer = "Referer: http://download.csdn.net/detail/wasdzxce/" + sourceId;
		//http请求头
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

		curl_easy_setopt(easy_handle, CURLOPT_COOKIEFILE, "cookie.txt");		//读取本地存储的cookie
		if (!InitCurl(easy_handle, res, url, content))
		{
			//释放资源
			curl_slist_free_all(headers);
			curl_easy_cleanup(easy_handle);
			curl_global_cleanup();
			return FALSE;
		}
		//释放资源
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
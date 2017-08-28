#include <stdio.h>
#include <stdlib.h>
#include "httpmags\httptool.h"

int main()
{
	HttpTool* httpTool = new HttpTool();
	bool isGet = httpTool->httpGet("http://www.sjzvip.com/jiexi1.php?url=https://v.qq.com/x/cover/m8i6uooilmandtf/b0024weo2b7.html?ptag=baidu.aladdin.tv.pay");
	if (isGet)
		std::string html_content(httpTool->getReponseHTML());
	delete httpTool;

	system("pause");
	return 0;
}
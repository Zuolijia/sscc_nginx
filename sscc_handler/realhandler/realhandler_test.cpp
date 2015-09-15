#include <request_test.h>
#include <reply_test.h>
#include <fstream>

using namespace std;

extern "C"{
	int ngx_http_sscctest_realhandler_test(struct Request *req,struct Reply *rep);
}

int ngx_http_sscctest_realhandler_test(struct Request *req,struct Reply *rep){
	fstream fpdebug("/home/zuolj/sscc_handler/realhandler/error",std::fstream::out|std::fstream::app);
	fpdebug << "open realhandler_test success\n";
	printf("In realhandler\n");

	rep->status = Reply::ok;
	rep->content = "<center><strong><font color='red'>RemoteAddress : </font>"+req->remoteAddress +"</strong></center>";

	printf("content : %s\n",rep->content.c_str());

	
	/*struct Header h1; //返回头部信息
	h1.name.assign("Expires");
	h1.value.assign("Web, 24 Aug 2016 23:00:00 GMT");
	rep->headers.push_back(h1);

	struct Header h2;
	h2.name.assign("Cache-Control");
	h2.value.assign("max-age=7200");
	rep->headers.push_back(h2);

	struct Header h3;
	h3.name.assign("Content-Type");
	h3.value.assign("text/html");
	rep->headers.push_back(h3);*/

	return 1;
}
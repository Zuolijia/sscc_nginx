#include <request_test.h>
#include <reply_test.h>
#include <fstream>

using namespace std;

extern "C"{
	int ngx_http_sscctest_realhandler_test2(struct Request *req,struct Reply *rep);
}

int ngx_http_sscctest_realhandler_test2(struct Request *req,struct Reply *rep){
	fstream fpdebug("/home/zuolj/sscc_handler/realhandler/error",std::fstream::out|std::fstream::app);
	time_t now_time;
	time(&now_time);
	fpdebug << ctime(&now_time);
	fpdebug << "open realhandler_test2 success\n\n";

	rep->status = Reply::ok;
	rep->content = "<center><strong><font color='red'>Expires : </font>Web, 24 Aug 2016 23:00:00 GMT</strong></center>";

	fpdebug << ctime(&now_time);
	fpdebug << "content : " << rep->content << "\n\n";
	printf("content : %s\n",rep->content.c_str());
	
	struct Header h; //返回头部信息
	h.name.assign("Expires");
	h.value.assign("Web, 24 Aug 2016 23:00:00 GMT");
	rep->headers.push_back(h);

	h.name.clear();
	h.value.clear();

	h.name.assign("Content-Type");
	h.value.assign("text/html");
	rep->headers.push_back(h);

	h.name.clear();
	h.value.clear();

	h.name.assign("Cache-Control");
	h.value.assign("max-age=7200");
	rep->headers.push_back(h);

	return 1;
}
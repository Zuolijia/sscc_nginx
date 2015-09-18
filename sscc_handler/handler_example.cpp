#include <fstream>
#include <map>
#include <dlfcn.h> //显式调用动态链接库文件需要加载的头文件
#include <ngx_http_sscctest_module.h>
#include <request_test.h>
#include <reply_test.h>
#include <iostream>
#include <stdio.h>
#include <ctime>



extern "C" {
ngx_int_t init();
ngx_int_t example_handler(ngx_http_sscctest_request_t *req_c,ngx_http_sscctest_response_t *resp_c);
}


using namespace std;

typedef int (*real_handler_pt)(struct Request *req,struct Reply *rep);
void request_C_2_CPP(ngx_http_sscctest_request_t *req_c,struct Request *req_cpp);
void response_CPP_2_C(struct Reply *resp_cpp,ngx_http_sscctest_response_t *resp_c);

static map<string,real_handler_pt> m;	//保存所有的动态库文件中realhandler函数的指针地址
void *pdl = NULL; 						//动态链接库文件解析句柄
char *pErr = NULL; 						//动态链接库解析错误指针
ngx_table_elt_t ex;

/**
 * @初始化载入所有的动态链接库
 */
ngx_int_t init()
{
	fstream fperror("/home/zuolj/sscc_handler/error",std::fstream::out|std::fstream::app);
	time_t now_time;
	time(&now_time);
	fperror << ctime(&now_time);
	fperror << "Ready Init\n\n";

	FILE *file = NULL;
	file=fopen("/home/zuolj/sscc_handler/sscctest","r");
	if(file == NULL){
		fperror << ctime(&now_time);
		fperror << "Error opening file\n\n";
		return 0;
	}
	char ch;
	char k[1024] = {0};
	char v[1024] = {0};
	
	m.clear();
	while(fscanf(file,"%s %s",k,v)==2){
		string key(k);
		string value(v);
		
		string path_of_so = "/home/zuolj/sscc_handler/realhandler/"+key+".so";
		pdl = dlopen(path_of_so.c_str(),RTLD_LAZY);
		pErr = dlerror();
		if(!pdl || pErr)
		{
			fperror << ctime(&now_time);
			fperror << "Load " << key << ".so failed\n\n";
			return 0;
		}
		fperror << ctime(&now_time);
		fperror << "Load " << key << ".so successful\n\n";
		real_handler_pt pt = (real_handler_pt)dlsym(pdl,value.c_str());
		if(pt == NULL){
			fperror << ctime(&now_time);
			fperror << "real_handler_pt is NULL\n\n";
		}
		else{
			m.insert(pair<string,real_handler_pt>(key,pt));
			fperror << ctime(&now_time);
			fperror << "push one element to m\n\n"; 
			key.clear();
			value.clear();
		}
	}
	fclose(file);
	fperror << ctime(&now_time);
	fperror << "Init over\n\n";
	fperror.close();
	return 1;
}

/**
 * @处理函数，根据不同的uri调用不同real插件进行处理
 */
ngx_int_t example_handler(ngx_http_sscctest_request_t *req_c,ngx_http_sscctest_response_t *resp_c)
{
	struct Request req_cpp;
	struct Reply resp_cpp;
	fstream fperror("/home/zuolj/sscc_handler/error",std::fstream::out|std::fstream::app);
	time_t now_time;
	time(&now_time);
	fperror << ctime(&now_time);
	fperror << "example_handler is called\n\n";

	request_C_2_CPP(req_c,&req_cpp);
	multimap<string, string>::iterator it = req_cpp.querys.find("handler_name");
	if (it == req_cpp.querys.end()){
		fperror << ctime(&now_time);
		fperror << "can not find handler_name arg\n\n";
		fperror.close();
		return 0;
	}

	map<string,real_handler_pt>::iterator ih = m.find((*it).second);
	if (ih == m.end()){
		fperror << ctime(&now_time);
		fperror << "cannot find realhandler\n\n";
		fperror.close();
		return 0;
	}

	if ((*ih).second){
		(*ih).second(&req_cpp,&resp_cpp);
	}
	else{
		fperror << ctime(&now_time);
		fperror << "realhandler is null\n\n";
		return 0;
	}

	response_CPP_2_C(&resp_cpp,resp_c);
	fperror.close();
	return 1;
}


void request_C_2_CPP(ngx_http_sscctest_request_t *req_c,struct Request *req_cpp)
{
	/** 浏览器地址赋值 **/
	req_cpp->remoteAddress.assign((char *)req_c->remoteAddr.data,req_c->remoteAddr.len);	
	/** 浏览器端口赋值 **/
	req_cpp->remotePort = req_c->remotePort;
	/** 请求模式赋值(GET/POST) **/
	req_cpp->method.assign((char *)req_c->method.data,req_c->method.len);
	/** 用户请求的完整uri赋值 **/
	req_cpp->uri.assign((char *)req_c->uri.data,req_c->uri.len);
	req_cpp->uri += "?";
	/** HTTP主版本号赋值 **/
	req_cpp->httpVersionMajor = req_c->httpVersionMajor;
	/** HTTP副版本号赋值 **/
	req_cpp->httpVersionMinor = req_c->httpVersionMinor;

	/** 用户请求的数据 **/
	//vector<char>     body

	/** HTTP头 **/
	//头信息包括Host,User-Agent,Accept,Accept-Language,Content-Type
	struct Header h;
	h.name.assign("Host");
	h.value.assign((char *)req_c->headers_in.host->value.data,req_c->headers_in.host->value.len);
	req_cpp->headers.push_back(h);

	h.name.clear();
	h.value.clear();

	h.name.assign("User-Agent");
	h.value.assign((char *)req_c->headers_in.user_agent->value.data,req_c->headers_in.user_agent->value.len);
	req_cpp->headers.push_back(h);

	/*h.name.clear();
	h.value.clear();
	h.name.assign("Accept");
	h.value.assign(req_c->headers_in.accept->value.data,req_c->headers_in.accept->value.len);
	req_cpp->headers.push_back(h);*/

	/*h.name.clear();
	h.value.clear();
	h.name.assign("Accept-Language");
	h.value.assign(req_c->headers_in.accept_language->value.data,req_c->headers_in.accept_language->value.len);
	req_cpp->headers.push_back(h);*/

	/*h.name.clear();
	h.value.clear();
	h.name.assign("Content-Type");
	h.value.assign((char *)req_c->headers_in.content_type->value.data,req_c->headers_in.content_type->value.len);
	req_cpp->headers.push_back(h);*/

	/** cookies赋值 **/
	int i;
	for(i=0;i<req_c->headers_in.cookies.nelts;i++){
		char *p = (char *)((ngx_table_elt_t **)req_c->headers_in.cookies.elts)[i]->value.data;
		int len = ((ngx_table_elt_t **)req_c->headers_in.cookies.elts)[i]->value.len;
		string key="",value="";
		for(int j=0;j<len;j++){
			if(*(p+j) == '='){
				key.assign(p,j);
				value.assign(p+j+1,len-j-1);
				req_cpp->cookies.insert(pair<string,string>(key,value));
			}
		}
	}

	/** 查询参数赋值 **/
	for(i=0;i<req_c->query->nelts;i++){
		string key="",value="";
		key.assign((char *)((ngx_table_elt_t *)req_c->query->elts)[i].key.data,((ngx_table_elt_t *)req_c->query->elts)[i].key.len);
		value.assign((char *)((ngx_table_elt_t *)req_c->query->elts)[i].value.data,((ngx_table_elt_t *)req_c->query->elts)[i].value.len);
		req_cpp->querys.insert(pair<string,string>(key,value));
		req_cpp->uri += (key + "=" + value);
	}

	/** 用户请求的路径，不含查询串 **/
	//string           requestPath;
	req_cpp->requestPath.assign((char *)req_c->uri.data,req_c->uri.len);
	//string           contentType;
}

void response_CPP_2_C(struct Reply *resp_cpp,ngx_http_sscctest_response_t *resp_c)
{
	/** 状态赋值 **/
	if(resp_cpp->status == Reply::ok){
		resp_c->status = ngx_http_sscctest_response_s::ok;
	}
	else if(resp_cpp->status == Reply::created){
		resp_c->status = ngx_http_sscctest_response_s::created;
	}
	else if(resp_cpp->status == Reply::accepted){
		resp_c->status = ngx_http_sscctest_response_s::accepted;
	}
	else if(resp_cpp->status == Reply::no_content){
		resp_c->status = ngx_http_sscctest_response_s::no_content;
	}
	else if(resp_cpp->status == Reply::multiple_choices){
		resp_c->status = ngx_http_sscctest_response_s::multiple_choices;
	}
	else if(resp_cpp->status == Reply::moved_permanently){
		resp_c->status = ngx_http_sscctest_response_s::moved_permanently;
	}
	else if(resp_cpp->status == Reply::moved_temporarily){
		resp_c->status = ngx_http_sscctest_response_s::moved_temporarily;
	}
	else if(resp_cpp->status == Reply::not_modified){
		resp_c->status = ngx_http_sscctest_response_s::not_modified;
	}
	else if(resp_cpp->status == Reply::bad_request){
		resp_c->status = ngx_http_sscctest_response_s::bad_request;
	}
	else if(resp_cpp->status == Reply::unauthorized){
		resp_c->status = ngx_http_sscctest_response_s::unauthorized;
	}
	else if(resp_cpp->status == Reply::forbidden){
		resp_c->status = ngx_http_sscctest_response_s::forbidden;
	}
	else if(resp_cpp->status == Reply::not_found){
		resp_c->status = ngx_http_sscctest_response_s::not_found;
	}
	else if(resp_cpp->status == Reply::internal_server_error){
		resp_c->status = ngx_http_sscctest_response_s::internal_server_error;
	}
	else if(resp_cpp->status == Reply::not_implemented){
		resp_c->status = ngx_http_sscctest_response_s::not_implemented;
	}
	else if(resp_cpp->status == Reply::bad_gateway){
		resp_c->status = ngx_http_sscctest_response_s::bad_gateway;
	}
	else if(resp_cpp->status == Reply::service_unavailable){
		resp_c->status = ngx_http_sscctest_response_s::service_unavailable;
	}
	else{
		;
	}

	/** 内容赋值 **/
	resp_c->content.len = resp_cpp->content.length();
	resp_c->content.data = (u_char *)malloc(sizeof(u_char)*resp_c->content.len);
	resp_cpp->content.copy((char*)resp_c->content.data,resp_c->content.len);

	/** Header_out赋值 **/
	resp_c->headers_out->status = resp_c->status;
    resp_c->headers_out->content_length_n = resp_c->content.len;
	for(int i = 0; i<resp_cpp->headers.size(); i++){
		if(resp_cpp->headers[i].name == "Expires"){
			ex.key.data = (u_char *)resp_cpp->headers[i].name.c_str();
			ex.key.len = resp_cpp->headers[i].name.length();
			ex.value.data = (u_char *)resp_cpp->headers[i].value.c_str();
			ex.value.len = resp_cpp->headers[i].value.length();
			resp_c->headers_out->expires = &ex;
		}
		else if(resp_cpp->headers[i].name == "Cache-Control"){
			ngx_table_elt_t **ccp = NULL;
			ngx_table_elt_t *set_cache = NULL;
			ccp = (ngx_table_elt_t **)resp_c->headers_out->cache_control.elts;
			if(ccp == NULL){
				resp_c->headers_out->cache_control.nelts = 0;
				resp_c->headers_out->cache_control.size = sizeof(ngx_table_elt_t *);
				resp_c->headers_out->cache_control.nalloc = 1;
				resp_c->headers_out->cache_control.elts = (ngx_table_elt_t **)malloc(sizeof(ngx_table_elt_t *));
			}
			ccp = (ngx_table_elt_t **)resp_c->headers_out->cache_control.elts;
			
			set_cache = (ngx_table_elt_t *)resp_c->headers_out->headers.last->elts + resp_c->headers_out->headers.size * resp_c->headers_out->headers.last->nelts;

			if(set_cache == NULL){
				printf("address of set_cache is NULL\n");
				return ;
			}

			set_cache->hash = 1;
			string str_cache = "Cache-Control";
			set_cache->key.data = (u_char*)malloc(sizeof(u_char)*str_cache.length());
			str_cache.copy((char*)set_cache->key.data,str_cache.length());
			set_cache->key.len = str_cache.length();

			set_cache->value.len = resp_cpp->headers[i].value.length();
			set_cache->value.data = (u_char*)malloc(sizeof(u_char)*set_cache->value.len);
			resp_cpp->headers[i].value.copy((char*)set_cache->value.data,set_cache->value.len);

			*ccp = set_cache;

			resp_c->headers_out->cache_control.nelts = 1;
		}
		else if(resp_cpp->headers[i].name == "Content-Type"){
			resp_c->headers_out->content_type.len = resp_cpp->headers[i].value.length();
			resp_c->headers_out->content_type.data = (u_char *)malloc(sizeof(u_char)*resp_c->headers_out->content_type.len);
			resp_cpp->headers[i].value.copy((char*)resp_c->headers_out->content_type.data,resp_c->headers_out->content_type.len);
		}
		else{
			;
		}
	}
}



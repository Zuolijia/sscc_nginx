/*
	Copyright (C) Paul 
	rwhsysu@163.com

    /home/renwh/sscctest
    /home/renwh/mylib.so
    /nginx stop的时候添加一行关闭动态链接库文件的程序：未添加

	for nginx.conf 

	location /sscctest {
		flag;
	} 
*/
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <dlfcn.h> //显式调用动态链接库文件需要加载的头文件
#include "ngx_http_sscctest_module.h"


static char * ngx_http_sscctest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);//handler挂载函数, 按需挂载；

static ngx_int_t ngx_http_sscctest_postconfiguration(ngx_conf_t *cf);//打开handler函数定义文件，调用所有函数，
																//挂载到loc_conf的handlers的数组中

static ngx_int_t ngx_http_sscctest_handler(ngx_http_request_t *r); //handler 函数定义

static ngx_int_t ngx_get_args_array(ngx_http_request_t *r, ngx_array_t *a); //args数组赋值
static ngx_http_sscc_handler_pt handler = NULL;
ngx_http_sscctest_request_t sscc_C_request;
ngx_http_sscctest_response_t sscc_C_response;

static ngx_command_t ngx_http_sscctest_commands[]={
	{
		ngx_string("flag"),
		NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        /*
		NGX_CONF_FLAG：配置指令可以接受的值是”on”或者”off”，最终会被转成bool值。
		*/
		ngx_http_sscctest,
		NGX_HTTP_LOC_CONF_OFFSET,
		0,
		NULL
	},
	ngx_null_command
};

static ngx_http_module_t ngx_http_sscctest_module_ctx = {
        NULL,                          /* preconfiguration */
        ngx_http_sscctest_postconfiguration,                          /* postconfiguration */

        NULL,                          /* create main configuration */
        NULL,                          /* init main configuration */

        NULL,                          /* create server configuration */
        NULL,                          /* merge server configuration */

        NULL, /* create location configuration */
        NULL                            /* merge location configuration */
};

ngx_module_t ngx_http_sscctest_module = {
        NGX_MODULE_V1,
        &ngx_http_sscctest_module_ctx,    /* module context */
        ngx_http_sscctest_commands,       /* module directives */
        NGX_HTTP_MODULE,               /* module type */
        NULL,                          /* init master */
        NULL,                          /* init module */
        NULL,                          /* init process */
        NULL,                          /* init thread */
        NULL,                          /* exit thread */
        NULL,                          /* exit process */
        NULL,                          /* exit master */
        NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_get_args_array(ngx_http_request_t *r, ngx_array_t *a)
{
    u_char  *p, *last;
    ngx_table_elt_t *k2v = NULL;
    
    if (r->args.len == 0) {
        return NGX_DECLINED;
    }
    //ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is called! init  \n");
    p = r->args.data;
    last = p + r->args.len;
    

    for (  ; p < last; p++) {

        if (p == r->args.data || *(p - 1) == '&') {

            //ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is called! a->nelts:%d ; a->elts:%d \n",a->nelts,a->elts);
            k2v = (ngx_table_elt_t *) a->elts + a->size * a->nelts;
            a->nelts++;
            //ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is called! k2v:%l ; a->elts:%l \n",k2v,a->elts);
            k2v->key.data = p;
            p = ngx_strlchr(p, last, '=');
            k2v->key.len = p - k2v->key.data;
            p++;
            k2v->value.data =p; 
            p = ngx_strlchr(p, last, '&');

            if (p == NULL) {
                p = r->args.data + r->args.len;
            }
            k2v->value.len = p - k2v->value.data;
        }
    }
    return NGX_OK;
}

static char * ngx_http_sscctest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) //handler挂载函数, 按需挂载
{
	ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_sscctest_handler;

    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_sscctest_postconfiguration(ngx_conf_t *cf)
{
    //my_cf = ngx_http_get_module_loc_conf(r,ngx_http_sscctest_module);
    void *pdlHandle;//动态链接库文件解析句柄
    char *pszErr;//动态链接库解析错误指针
    pdlHandle = dlopen("/home/zuolj/sscc_handler/mylib.so", RTLD_LAZY); // RTLD_LAZY 延迟加载
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "after dlopen\n");
    pszErr = dlerror();
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "after dlerror\n");
    if( !pdlHandle || pszErr )
    {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_sscctest_create_loc_conf: Load mylib failed!\n");
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "error : %s\n",pszErr);
    }
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "Load success\n");
    ngx_http_handler_init init_pt;
    init_pt = dlsym(pdlHandle,"init");
    if (init_pt == NULL){
        printf("init_pt is null\n");
    }
    handler = dlsym(pdlHandle,"example_handler");
    if (handler == NULL){
        printf("example_handler is not found\n");
    }
    init_pt();
    return NGX_OK;
}

static ngx_int_t ngx_http_sscctest_handler(ngx_http_request_t *r)
{
	ngx_int_t rc = 0;
    ngx_buf_t *b;
	ngx_chain_t out;

    sscc_C_request.query = NULL;
    /**************************    sscc_C_request结构体query赋值       *******************************/
    sscc_C_request.query = ngx_array_create(r->pool, 20, sizeof(ngx_table_elt_t));
    //static ngx_int_t ngx_get_args_array(ngx_http_request_t *r, ngx_array_t *a)
    // 可以拿HTTP GET的参数。
    // 第一个参数是ngx_http_request_t；
    // 第二个参数是已经创建的用于存储args的key value对的数组指针
    rc = ngx_get_args_array(r,sscc_C_request.query);
    if(rc != NGX_OK){
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_get_args_array is error!\n");
    }

    /**************************    sscc_C_request结构体remoteAddr赋值      *******************************/
    struct sockaddr_in *ip = (struct sockaddr_in *) (r->connection->sockaddr); //inet_ntoa(ip->sin_addr) , ntohs(ip->sin_port)
    sscc_C_request.remoteAddr = r->connection->addr_text;

    /**************************    sscc_C_request结构体remotePort赋值      *******************************/
    sscc_C_request.remotePort = ntohs(ip->sin_port); // 大小端转化

    /**************************    sscc_C_request结构体 method 赋值      *******************************/
    sscc_C_request.method = r->method_name;

    /**************************    sscc_C_request结构体 uri 赋值      *******************************/
    sscc_C_request.uri = r->uri;
    /**************************    sscc_C_request结构体 httpVersionMajor 赋值      *******************************/
    // nginx 宏定义如下：
    // #define NGX_HTTP_VERSION_9                 9
    // #define NGX_HTTP_VERSION_10                1000
    // #define NGX_HTTP_VERSION_11                1001
    if(r->http_version == NGX_HTTP_VERSION_9){
        sscc_C_request.httpVersionMajor = 0;
        sscc_C_request.httpVersionMinor = 9;
    }
    if(r->http_version == NGX_HTTP_VERSION_10){
        sscc_C_request.httpVersionMajor = 1;
        sscc_C_request.httpVersionMinor = 0;
    }
    if(r->http_version == NGX_HTTP_VERSION_11){
        sscc_C_request.httpVersionMajor = 1;
        sscc_C_request.httpVersionMinor = 1;
    }
    /**************************    sscc_C_request结构体 headers_in 赋值      *******************************/
    //复制了一个ngx_http_headers_in_t结构体
    sscc_C_request.headers_in = r->headers_in;

    sscc_C_response.headers_out = &r->headers_out;

    /*************************  调用的handler  **********************/
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "Ready to call handler\n");

    if(handler == NULL){
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "example_handler can not found here\n");
        return NGX_ERROR;
    }
    if(sscc_C_response.content.len == 0){
        printf("free response content data\n");
        free(sscc_C_response.content.data);
    }
    handler(&sscc_C_request, &sscc_C_response);

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "handler in nginx is called over\n");

    /* discard request body, since we don't need it here */
    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
            return rc;
    }

    /* send the header only, if the request type is http 'HEAD' */
    if (r->method == NGX_HTTP_HEAD) {
            return ngx_http_send_header(r);
    }

    /* send the headers of your response */
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
    }
    
    b = ngx_pcalloc(r->pool,sizeof(ngx_buf_t));
    if (b == NULL){
        printf("malloc ngx_buf_t fault\n");
        return NGX_ERROR;
    }

    b->pos = sscc_C_response.content.data;
    b->last = sscc_C_response.content.data + sscc_C_response.content.len;
    b->memory = 1;
    b->last_buf = 1;

    out.buf = b;
    out.next = NULL;

    /* send the buffer chain of your response */
    ngx_array_destroy(sscc_C_request.query);
    sscc_C_response.content.len = 0;
    return ngx_http_output_filter(r, &out);
}

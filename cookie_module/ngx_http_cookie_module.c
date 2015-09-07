#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char * ngx_http_cookie(ngx_conf_t *cf,ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_cookie_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_cookie_commands[] = {
	{
		ngx_string("get_cookie"),
		NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
		ngx_http_cookie,
		NGX_HTTP_LOC_CONF_OFFSET,
		0,
		NULL
	},

	ngx_null_command	//数组结束的标志
};


static ngx_http_module_t ngx_http_cookie_module_ctx = {
	NULL,			/* preconfiguration */	
	NULL,			/* postconfiguration */	
	NULL,			/* create main configuration */	
	NULL,			/* init main configuration */
	NULL,			/* create server configuration */
	NULL,			/* merge server configuration */
	NULL,			/* create location configuration */
	NULL,			/* merge location configuration */
};

ngx_module_t ngx_http_cookie_module = {
	NGX_MODULE_V1,				/* 填充处理 */
	&ngx_http_cookie_module_ctx,	/* module context */
	ngx_http_cookie_commands,		/* module directives */
	NGX_HTTP_MODULE,			/* module type */
	NULL,						/* init master */	//在nginx初始化过程的特定时间点调用
	//初始化完所有模块后调用，在ngx_int_cycle函数(ngx_cycle.c)中
	NULL,						/* init module */	
	//初始化完worker进程后调用，在ngx_worker_process_init函数(ngx_process_cycle.c)中
	NULL,						/* init process */
	NULL,						/* init thread */
	NULL,						/* exit thread */
	NULL,						/* exit process */
	NULL,						/* exit master */
	NGX_MODULE_V1_PADDING		/* 填充处理 */
};

static char * ngx_http_cookie(ngx_conf_t *cf,ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	clcf = ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
	clcf->handler = ngx_http_cookie_handler;
	return NGX_CONF_OK;
}
static ngx_uint_t i=0;
static ngx_int_t ngx_http_cookie_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;	//存储返回值
	ngx_buf_t *b;	//缓冲区
	ngx_chain_t out;	//缓冲区链表
	ngx_table_elt_t **cookies;
	ngx_uint_t nelts;
	ngx_table_elt_t *set_cookie;
	u_char ngx_string[2048]={0};	//存储返回的字符串
	u_char temp[1024]={0};
	ngx_uint_t content_length = 0;	//返回的字符串长度
	ngx_str_t type = ngx_string("text/html");
	ngx_uint_t j;
	
	//在r->headers_in结构有一个ngx_array_t类型成员cookies,这是nginx存储请求cookie的变量。
	cookies = r->headers_in.cookies.elts;
 	nelts = r->headers_in.cookies.nelts;
	if (cookies == NULL){
		ngx_sprintf(ngx_string,"cookies is not exsit!");
		//在r->headers_out中没有cookies成员，需要构造一个ngx_table_elt_t结构添加到r->headers_out.headers中
		set_cookie = ngx_list_push(&r->headers_out.headers);
		if (set_cookie == NULL){
			ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"set_cookie is NULL nelts is %d",nelts);
		}
		set_cookie->hash = 1;
		ngx_str_set(&set_cookie->key,"Set-Cookie");
		i++;
		//在nginx中，所有的cookie键值对都格式化为一个字符串value，形式为keyname=valuename
		ngx_sprintf(temp,"CookieName%d=Visit %d times! <br>",i,i);
		set_cookie->value.data = temp;
		set_cookie->value.len = ngx_strlen(temp);
	}
	else {
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"number of cookie is %d",nelts);
		for(j=0;j<nelts;j++){
			ngx_sprintf(ngx_string+ngx_strlen(ngx_string),"Cookie %d : %V",j+1,&cookies[j]->value);
		}
		set_cookie = ngx_list_push(&r->headers_out.headers);
		set_cookie->hash = 1;
		ngx_str_set(&set_cookie->key,"Set-Cookie");
		i++;
		ngx_sprintf(temp,"CookieName%d=Visit %d times! <br>",i,i);
		set_cookie->value.data = temp;
		set_cookie->value.len = ngx_strlen(temp);
	}
	/*
	** ngx_sprintf()函数用于拼接字符串，原型如下：
	** u_char * ngx_cdecl ngx_sprintf(u_char *buf, const char *fmt, ...)
	** u_char *buf: 某个字符串的地址，即拼接后存放的位置
	** const char *fmt: 传格式，nginx自定义的格式，最常用的是%V，代表ngx_str_t
	*/
	content_length = ngx_strlen(ngx_string);

	if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))){
		return NGX_HTTP_NOT_ALLOWED;
	}

	rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK) {
		return rc;
	}

	
	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = content_length;
	r->headers_out.content_type = type;

	/* HTTP框架提供的发送HTTP头部的方法 */
	/* 返回NGX_ERROR或返回值大于0就表示不正常 */
	rc = ngx_http_send_header(r);
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only){
		return rc;
	}

	// 构造ngx_buf_t结构体准备发送包体
	b = ngx_pcalloc(r->pool,sizeof(ngx_buf_t));
	if(b==NULL){
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	// 将需要返回的字符串复制到ngx_buf_t指向的内存中
	b->pos = ngx_string;
	b->last = ngx_string+content_length;
	b->memory = 1;
	b->last_buf = 1;	//最后一块缓冲区

	// 缓冲区链表chain把需要往外发送的数据串起来，这里只有一个buf里的数据需要发送
	out.buf = b;
	out.next = NULL;

	// 发送包体，结束请求
	return ngx_http_output_filter(r,&out);
}
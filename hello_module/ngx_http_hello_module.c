#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct 
{
	ngx_str_t hello_string;
}ngx_http_hello_loc_conf_t;

// r中包含请求的所有信息（如方法、URI、协议版本号和头部等）
static ngx_int_t ngx_http_hello_handler(ngx_http_request_t *r);
static char * ngx_http_hello(ngx_conf_t *cf,ngx_command_t *cmd,void *conf);//函数指针

static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf);				//解析完http{}内的所有配置项后回调
static ngx_int_t ngx_http_hello_preconfiguration(ngx_conf_t *cf);	//在解析http{}内的配置项前回调
/*
** 创建用于存储HTTP全局配置项的结构体，该结构体中的成员将保存直属于http{}块的配置项参数。
** 它会在解析main配置项前调用
*/
static void *ngx_http_hello_create_main_conf(ngx_conf_t *cf);			
static char *ngx_http_hello_init_main_conf(ngx_conf_t *cf,void *conf);//解析完main配置项后回调
/*
** 创建用于存储可同时出现在main/srv级别配置项的结构体，该结构体中的成员与server配置是相关联的
*/	
static void *ngx_http_hello_create_srv_conf(ngx_conf_t *cf);
/*
** create_srv_conf产生的结构体所要解析的配置项，可能同时出现在main/srv级别中，
** merge_srv_conf方法可以把出现在main级别中的配置项值合并到srv级别配置项中
*/
static char *ngx_http_hello_merge_srv_conf(ngx_conf_t *cf,void *prev,void *conf);
/*
** 创建用于存储可同时出现在main、srv、loc级别的结构体，该结构体中的成员与location配置是相关联的
*/
static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf);
/*
** create_loc_conf产生的结构体所要解析的配置项，可能出现在main/srv/loc级别中，
** merge_loc_conf方法可以分别把出现在main、srv级别的配置项合并到loc级别的配置项中
*/
static char *ngx_http_hello_merge_loc_conf(ngx_conf_t *cf,void *prev,void *conf);

void ngx_http_test_body_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_hello_commands[] = {
	{
		ngx_string("hello_string"),		/* 配置项名称 */
		NGX_HTTP_MAIN_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LMT_CONF|NGX_CONF_TAKE1,
		ngx_http_hello,				/* 处理配置项的参数 */
		NGX_HTTP_LOC_CONF_OFFSET,	/* 配置文件中的偏移量 */
		offsetof(ngx_http_hello_loc_conf_t,hello_string),
		NULL
	},
	ngx_null_command
};

/*
** 8个阶段的调用顺序与定义顺序不同，
** HTTP框架调用这些回调方法的实际顺序与nginx.conf配置项有关
*/
static ngx_http_module_t ngx_http_hello_module_ctx = {
	ngx_http_hello_preconfiguration,		/* preconfiguration */
	ngx_http_hello_init,					/* postconfiguration */
	ngx_http_hello_create_main_conf,		/* create main configuration */
	ngx_http_hello_init_main_conf,			/* init main configuration */
	ngx_http_hello_create_srv_conf,			/* create server configuration */
	ngx_http_hello_merge_srv_conf,			/* merge server configuration */
	ngx_http_hello_create_loc_conf,			/* create location configuration */
	ngx_http_hello_merge_loc_conf,			/* merge location configuration */
};

ngx_module_t ngx_http_hello_module = {
	NGX_MODULE_V1,				/* 填充处理 */
	&ngx_http_hello_module_ctx,	/* module context */
	ngx_http_hello_commands,	/* module directives */
	NGX_HTTP_MODULE,			/* module type */
	NULL,						/* init master */
	NULL,						/* init module */
	NULL,						/* init process */
	NULL,						/* init thread */
	NULL,						/* exit thread */
	NULL,						/* exit process */
	NULL,						/* exit master */
	NGX_MODULE_V1_PADDING		/* 填充处理 */
};
static ngx_int_t ngx_http_hello_preconfiguration(ngx_conf_t *cf)
{
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"ngx_http_hello_preconfiguration is called");
	return NGX_OK;
}	
/*
** 11个阶段中有四个阶段不调用挂载的任何handler，所以不需要挂载到这几个阶段：
** NGX_HTTP_FIND_CONFIG_PHASE
** NGX_HTTP_POST_ACCESS_PHASE
** NGX_HTTP_POST_REWRITE_PHASE
** NGX_HTTP_TRY_FILES_PHASE
*/
static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf)
{
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"ngx_http_hello_postconfiguration is called");
	ngx_http_handler_pt *h;
	ngx_http_core_main_conf_t *cmcf;

	cmcf = ngx_http_conf_get_module_main_conf(cf,ngx_http_core_module);

	h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
	if(h == NULL){
		return NGX_ERROR;
	}
	*h = ngx_http_hello_handler;
	return NGX_OK;
}

static void *ngx_http_hello_create_main_conf(ngx_conf_t *cf)
{
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"ngx_http_hello_create_main_conf is called");
	ngx_http_hello_loc_conf_t* main_conf = NULL;
	main_conf = ngx_pcalloc(cf->pool,sizeof(ngx_http_hello_loc_conf_t));
	if(main_conf == NULL)
	{
		return NULL;
	}
	ngx_str_null(&main_conf->hello_string);
	return main_conf;
}
static char *ngx_http_hello_init_main_conf(ngx_conf_t *cf,void *conf)
{
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"ngx_http_hello_init_main_conf is called");
	return NULL;
}
static void *ngx_http_hello_create_srv_conf(ngx_conf_t *cf)
{
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"ngx_http_hello_create_srv_conf is called");
	ngx_http_hello_loc_conf_t* srv_conf = NULL;
	srv_conf = ngx_pcalloc(cf->pool,sizeof(ngx_http_hello_loc_conf_t));
	if(srv_conf == NULL)
	{
		return NULL;
	}
	ngx_str_null(&srv_conf->hello_string);
	return srv_conf;
}
static char *ngx_http_hello_merge_srv_conf(ngx_conf_t *cf,void *prev,void *conf)
{
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"ngx_http_hello_merge_srv_conf is called");
	return NULL;
}
static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf)
{
	ngx_http_hello_loc_conf_t* local_conf = NULL;
	local_conf = ngx_pcalloc(cf->pool,sizeof(ngx_http_hello_loc_conf_t));
	if(local_conf == NULL)
	{
		return NULL;
	}
	ngx_str_null(&local_conf->hello_string);
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"ngx_http_hello_create_loc_conf is called");
	return local_conf;
}
static char *ngx_http_hello_merge_loc_conf(ngx_conf_t *cf,void *prev,void *conf)
{
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"ngx_http_hello_merge_loc_conf is called");
	return NULL;
}

static char * ngx_http_hello(ngx_conf_t *cf,ngx_command_t *cmd, void *conf)
{
	ngx_http_hello_loc_conf_t* local_conf;
	local_conf = conf;
	char* rv = ngx_conf_set_str_slot(cf,cmd,conf);
	ngx_conf_log_error(NGX_LOG_EMERG,cf,0,"hello_string:%s",local_conf->hello_string.data);
	return rv;
}

static ngx_int_t ngx_http_hello_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_http_hello_loc_conf_t* my_conf;
	u_char ngx_hello_string[1024]= {0};
	ngx_uint_t content_length = 0;

	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"ngx_http_hello_handler is called");

	my_conf = ngx_http_get_module_loc_conf(r,ngx_http_hello_module);
	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"get ngx_http_hello_loc_conf_t");

	if (my_conf == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"my_conf is null");
	}
	if (my_conf->hello_string.len == 0)
	{
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"empty!");
		return NGX_DECLINED;
	}
	else
	{
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"string is not empty");
		ngx_sprintf(ngx_hello_string,"%V<br>",&my_conf->hello_string);

		rc = ngx_http_read_client_request_body(r,ngx_http_test_body_handler);

		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"rc : %d",rc);
	}
	content_length = ngx_strlen(ngx_hello_string);
	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"els");
	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Host->key: %s",r->headers_in.host->key.data);
	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Host->value: %s",r->headers_in.host->value.data);
	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"User-Agent->key: %s",r->headers_in.user_agent->key.data);
	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"User-Agent->value: %s",r->headers_in.user_agent->value.data);
	/*if(r->headers_in.accept == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Accept is NULL");
	}*/
	//ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Accept->key: %s",r->headers_in.accept->key.data);
	//ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Accept->value: %s",r->headers_in.accept->value.data);
	//ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Accept-Language->key: %s",r->headers_in.accept_language->key.data);
	//ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Accept-Language->value: %s",r->headers_in.accept_language->value.data);
	//ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Content-Type->key: %s",r->headers_in.content_type->key.data);
	//ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"Content-Type->value: %s",r->headers_in.content_type->value.data);

	/*if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))){
		return NGX_HTTP_NOT_ALLOWED;
	}*/
	rc = ngx_http_discard_request_body(r);
	if(rc != NGX_OK){
		return rc;
	}

	ngx_str_t type = ngx_string("text/html");
	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = content_length;
	r->headers_out.content_type = type;

	/* HTTP框架提供的发送HTTP头部的方法 */
	/* 返回NGX_ERROR或返回值大于0就表示不正常 */
	rc = ngx_http_send_header(r);
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only){
		return rc;
	}

	b = ngx_pcalloc(r->pool,sizeof(ngx_buf_t));
	if(b == NULL){
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	b->pos = ngx_hello_string;
	b->last = ngx_hello_string + content_length;
	b->memory = 1;
	b->last_buf = 1;

	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r,&out);
}

void ngx_http_test_body_handler(ngx_http_request_t *r)
{
	u_char *p;
	size_t len;
	ngx_buf_t *buf;
	ngx_chain_t *cl;

	if(r->request_body == NULL || r->request_body->bufs == NULL || r->request_body->temp_file){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"request_body is empty");
	}

	cl = r->request_body->bufs;
	if(cl == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"buffer_chain is NULL");
	}
	buf = cl->buf;
	if(buf == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"buffer is NULL");
	}

	if (cl->next == NULL){
		len = buf->last - buf->pos;
		p = buf->pos;
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"cl->next is NULL p:%s",p);
	}

	len = buf->last - buf->pos;
	cl = cl->next;
	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"len : %d",len);

	for(; cl ; cl = cl->next){
		buf = cl->buf;
		len += buf->last - buf->pos;
	}

	ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"after calculate len : %d",len);

	p = malloc(len*sizeof(u_char));

	cl = r->request_body->bufs;
	for(; cl ; cl = cl->next){
		buf = cl->buf;
		p = (u_char*)memcpy(p,buf->pos,buf->last - buf->pos) + (buf->last - buf->pos);
	}

	p = p-len;

	if(p == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"p is NULL");
	}
	else{
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"body : %s",p);
		//ngx_sprintf(ngx_hello_string+ngx_strlen(ngx_hello_string),"body:%s",p);
	}
}
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char * ngx_http_getbody(ngx_conf_t *cf,ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_getbody_handler(ngx_http_request_t *r);
void ngx_http_test_body_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_getbody_commands[] = {
	{
		ngx_string("get_body"),
		NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
		ngx_http_getbody,
		NGX_HTTP_LOC_CONF_OFFSET,
		0,
		NULL
	},

	ngx_null_command	//数组结束的标志
};

static ngx_http_module_t ngx_http_getbody_module_ctx = {
	NULL,			/* preconfiguration */	
	NULL,			/* postconfiguration */	
	NULL,			/* create main configuration */	
	NULL,			/* init main configuration */
	NULL,			/* create server configuration */
	NULL,			/* merge server configuration */
	NULL,			/* create location configuration */
	NULL,			/* merge location configuration */
};

ngx_module_t ngx_http_getbody_module = {
	NGX_MODULE_V1,					/* 填充处理 */
	&ngx_http_getbody_module_ctx,	/* module context */
	ngx_http_getbody_commands,		/* module directives */
	NGX_HTTP_MODULE,				/* module type */
	NULL,							/* init master */	//在nginx初始化过程的特定时间点调用
	//初始化完所有模块后调用，在ngx_int_cycle函数(ngx_cycle.c)中
	NULL,							/* init module */	
	//初始化完worker进程后调用，在ngx_worker_process_init函数(ngx_process_cycle.c)中
	NULL,							/* init process */
	NULL,							/* init thread */
	NULL,							/* exit thread */
	NULL,							/* exit process */
	NULL,							/* exit master */
	NGX_MODULE_V1_PADDING			/* 填充处理 */
};

static char * ngx_http_getbody(ngx_conf_t *cf,ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	clcf = ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
	clcf->handler = ngx_http_getbody_handler;
	return NGX_CONF_OK;
}


static ngx_int_t ngx_http_getbody_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;	//存储返回值

	if (!(r->method & (NGX_HTTP_POST | NGX_HTTP_HEAD))){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"return method is not post");
		return NGX_HTTP_NOT_ALLOWED;
	}

	rc = ngx_http_read_client_request_body(r,ngx_http_test_body_handler);
	
	if (rc >= NGX_HTTP_SPECIAL_RESPONSE) {
		return rc;
	}
	if(r->request_body->bufs == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"body is NULL");
	}

	return NGX_DONE;
}

void ngx_http_test_body_handler(ngx_http_request_t *r)
{
	u_char *p;
	size_t len;
	ngx_buf_t *buf;
	ngx_chain_t *cl;
	ngx_chain_t out;
	ngx_int_t rc;
	ngx_str_t type = ngx_string("text/html");

	if(r->request_body == NULL || r->request_body->bufs == NULL || r->request_body->temp_file){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"request_body is empty");
	}

	cl = r->request_body->bufs;
	if(cl == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"buffer_chain is NULL");
	}
	buf = cl->buf;
	buf->memory = 1;
	if(buf == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"buffer is NULL");
	}

	if (cl->next == NULL){
		len = buf->last - buf->pos;
		p = buf->pos;
		//out.buf = buf;
		//out.next = NULL;
		out = *cl;
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"cl->next is NULL p:%s",p);
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"buffer is in memory : %d",buf->memory);
	}
	else{
		len = buf->last - buf->pos;
		cl = cl->next;
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"len : %d",len);

		for(; cl ; cl = cl->next){
			buf = cl->buf;
			buf->memory = 1;
			len += buf->last - buf->pos;
		}

		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"after calculate len : %d",len);

		p = malloc(len*sizeof(u_char));

		cl = r->request_body->bufs;
		for(; cl ; cl = cl->next){
			buf = cl->buf;
			p = (u_char*)memcpy(p,buf->pos,buf->last - buf->pos) + (buf->last - buf->pos);
		}
		out = *(r->request_body->bufs);

		p = p-len;
	}

	if(p == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"p is NULL");
	}
	else{
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"body : %s",p);
	}

	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = len;
	r->headers_out.content_type = type;

	rc = ngx_http_send_header(r);
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"send header is error");
	}
	ngx_http_output_filter(r,&out);
}
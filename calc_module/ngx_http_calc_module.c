#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char * ngx_http_calc(ngx_conf_t *cf,ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_calc_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_calc_commands[] = {
	{
		ngx_string("calculate"),	/* 配置项名称 */
		NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE2,
		ngx_http_calc,				/* 处理配置项的参数 */
		NGX_HTTP_LOC_CONF_OFFSET,	/* 配置文件中的偏移量 */
		0,
		NULL
	},

	ngx_null_command
};

static ngx_http_module_t ngx_http_calc_module_ctx = {
	NULL,			/* preconfiguration */
	NULL,			/* postconfiguration */
	NULL,			/* create main configuration */
	NULL,			/* init main configuration */
	NULL,			/* create server configuration */
	NULL,			/* merge server configuration */
	NULL,			/* create location configuration */
	NULL,			/* merge location configuration */
};

ngx_module_t ngx_http_calc_module = {
	NGX_MODULE_V1,				/* 填充处理 */
	&ngx_http_calc_module_ctx,	/* module context */
	ngx_http_calc_commands,		/* module directives */
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

static char * ngx_http_calc(ngx_conf_t *cf,ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	clcf = ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
	clcf->handler = ngx_http_calc_handler;
	return NGX_CONF_OK;
}


static ngx_int_t ngx_http_calc_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	u_char ngx_string[1024]={0};
	ngx_uint_t content_length = 0;
	
	ngx_str_t arg1;
	ngx_str_t arg2;
	ngx_int_t ans;

	ngx_http_arg(r,(u_char *)"arg1",4,&arg1);
	ngx_http_arg(r,(u_char *)"arg2",4,&arg2);
	ans = ngx_atoi(arg1.data,arg1.len)+ngx_atoi(arg2.data,arg2.len);

	ngx_sprintf(ngx_string,"%V + %V = %d",&arg1,&arg2,ans);
	content_length = ngx_strlen(ngx_string);

	if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD))){
		return NGX_HTTP_NOT_ALLOWED;
	}

	rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK) {
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
	if(b==NULL){
		return NGX_HTTP_INTERNAL_SERVER_ERROR;
	}

	b->pos = ngx_string;
	b->last = ngx_string+content_length;
	b->memory = 1;
	b->last_buf = 1;

	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r,&out);
}
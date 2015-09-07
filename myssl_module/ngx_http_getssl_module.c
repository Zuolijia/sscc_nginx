#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

static char * ngx_http_getssl(ngx_conf_t *cf,ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_getssl_handler(ngx_http_request_t *r);
time_t ASN1_GetTimeT(ASN1_TIME *time);
char * X509_getCountryName(X509_NAME *name,char * msginfo);
char * X509_getProvinceName(X509_NAME *name,char * msginfo);
char * X509_getLocalityName(X509_NAME *name,char * msginfo);
char * X509_getOrganizationName(X509_NAME *name,char * msginfo);
char * X509_getOrganizationUnitName(X509_NAME *name,char * msginfo);
char * X509_getCommonName(X509_NAME *name,char * msginfo);
char * X509_getEmailAddress(X509_NAME *name,char * msginfo);

static ngx_command_t ngx_http_getssl_commands[] = {
	{
		ngx_string("get_ssl"),
		NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
		ngx_http_getssl,
		NGX_HTTP_LOC_CONF_OFFSET,
		0,
		NULL
	},

	ngx_null_command	//数组结束的标志
};


static ngx_http_module_t ngx_http_getssl_module_ctx = {
	NULL,			/* preconfiguration */	
	NULL,			/* postconfiguration */	
	NULL,			/* create main configuration */	
	NULL,			/* init main configuration */
	NULL,			/* create server configuration */
	NULL,			/* merge server configuration */
	NULL,			/* create location configuration */
	NULL,			/* merge location configuration */
};

ngx_module_t ngx_http_getssl_module = {
	NGX_MODULE_V1,				/* 填充处理 */
	&ngx_http_getssl_module_ctx,	/* module context */
	ngx_http_getssl_commands,		/* module directives */
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

static char * ngx_http_getssl(ngx_conf_t *cf,ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	clcf = ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
	clcf->handler = ngx_http_getssl_handler;
	return NGX_CONF_OK;
}

static ngx_int_t ngx_http_getssl_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;	//存储返回值
	ngx_buf_t *b;	//缓冲区
	ngx_chain_t out;	//缓冲区链表
	u_char ngx_string[2048]={0};	//存储返回的字符串
	ngx_uint_t content_length = 0;	//返回的字符串长度
	ngx_connection_t *c;
	ngx_http_ssl_srv_conf_t *sscf;
	char *sp = NULL;
	char *up = NULL;
	char scn[1024] = {0};
	char spn[1024] = {0};
	char sln[1024] = {0};
	char son[1024] = {0};
	char soun[1024] = {0};
	char scom[1024] = {0};
	char semadd[1024] = {0};
	char ucn[1024] = {0};
	char upn[1024] = {0};
	char uln[1024] = {0};
	char uon[1024] = {0};
	char uoun[1024] = {0};
	char ucom[1024] = {0};
	char uemadd[1024] = {0};

	c = r->connection;
	if (c->ssl == NULL){
		ngx_log_error(NGX_LOG_EMERG,r->connection->log,0,"client sent plain HTTP request to HTTPS port ");
		return NGX_DECLINED;
	}
	sscf = ngx_http_get_module_srv_conf(r,ngx_http_ssl_module);
	if (sscf == NULL){
		ngx_log_error(NGX_LOG_EMERG,c->log,0,"sscf is NULL");
		return NGX_ERROR;
	}
	ngx_sprintf(ngx_string,"verify:%d",sscf->verify);
	if (sscf->verify){
		ngx_log_error(NGX_LOG_EMERG,c->log,0,"In sscf->verify: %d",sscf->verify);
		rc=SSL_get_verify_result(c->ssl->connection);
		if(rc != X509_V_OK && (sscf->verify != 3 || !ngx_ssl_verify_error_optional(rc))){
			ngx_log_error(NGX_LOG_EMERG,c->log,0,"client SSL certificate verify error: (%l:%s)",
                              rc, X509_verify_cert_error_string(rc));
			return NGX_DECLINED;
		}
		ngx_log_error(NGX_LOG_EMERG,c->log,0,"After get verify result!");
		if(sscf->verify == 1){
			X509 *cert;
			X509_NAME *subject_name,*issuer_name;
			long version;
			ASN1_INTEGER *serial;
			ASN1_TIME *before,*after;
			cert = SSL_get_peer_certificate(c->ssl->connection);

			if(cert == NULL){
				ngx_log_error(NGX_LOG_EMERG,c->log,0,"client sent no required SSL certificate");
				return NGX_DECLINED;
			}
			issuer_name = X509_get_issuer_name(cert);
			subject_name = X509_get_subject_name(cert);
			version = X509_get_version(cert);
			serial = X509_get_serialNumber(cert);
			before =  X509_get_notBefore(cert);
			after = X509_get_notAfter(cert);
			long ser = ASN1_INTEGER_get(serial);
			time_t tbefore;
			tbefore = ASN1_GetTimeT(before);
			time_t tafter;
			tafter = ASN1_GetTimeT(after);
			char str[100]={0};
			strcpy(str,ctime(&tbefore));
			
			if (subject_name == NULL || issuer_name == NULL){
				X509_free(cert);
				ngx_log_error(NGX_LOG_EMERG,c->log,0,"name is NULL");
				return NGX_DECLINED;
			}
			up = X509_NAME_oneline(issuer_name,NULL,0);
			sp = X509_NAME_oneline(subject_name,NULL,0);
			ngx_log_error(NGX_LOG_EMERG,c->log,0,"After X509_NAME_oneline");

			ngx_sprintf(ngx_string,"<strong>SubjectName:</strong> %s <br> \
				<font color='red'>CountryName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>ProvinceName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp; \
				<font color='red'>LocalityName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>OrganizationName:</font> %s <br> \
				<font color='red'>OrganizationUnitName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>CommonName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp; \
				<font color='red'>EmailAddress:</font> %s <br> \
				<strong>IssuerName:</strong> %s <br> \
				<font color='red'>CountryName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>ProvinceName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp; \
				<font color='red'>LocalityName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>OrganizationName:</font> %s  <br> \
				<font color='red'>OrganizationUnitName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp;<font color='red'>CommonName:</font> %s &nbsp;&nbsp;&nbsp;&nbsp; \
				<font color='red'>EmailAddress:</font> %s <br> \
				<strong>version:</strong> %l <br><strong>serialNumber:</strong> %Xl <br> \
				<strong>before:</strong> %s <br><strong>after:</strong> %s <br>" , \
				sp,X509_getCountryName(subject_name,scn), X509_getProvinceName(subject_name,spn),X509_getLocalityName(subject_name,sln), \
				X509_getOrganizationName(subject_name,son),X509_getOrganizationUnitName(subject_name,soun), \
				X509_getCommonName(subject_name,scom),X509_getEmailAddress(subject_name,semadd), \
				up,X509_getCountryName(issuer_name,ucn), X509_getProvinceName(issuer_name,upn),X509_getLocalityName(issuer_name,uln), \
				X509_getOrganizationName(issuer_name,uon),X509_getOrganizationUnitName(issuer_name,uoun), \
				X509_getCommonName(issuer_name,ucom),X509_getEmailAddress(issuer_name,uemadd), \
				version,ser,str,ctime(&tafter));
			
			OPENSSL_free(up);
			OPENSSL_free(sp);
			X509_free(cert);
		}
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


time_t ASN1_GetTimeT(ASN1_TIME *time)
{
	struct tm t;
	char * str = (char *)time->data;
	size_t i = 0;
	//memset(&t,0,sizeof(t));

	if (time->type == V_ASN1_UTCTIME)
	{
		int temp = 0;
		temp += (str[i] - '0') * 10;
		t.tm_year = temp + (str[++i] - '0');
		i++;
		if (t.tm_year < 70)
			t.tm_year += 100;
	}
	else if (time->type == V_ASN1_GENERALIZEDTIME)
	{
		int temp = 0;
		temp += (str[i++] - '0') * 1000;
		temp += (str[i++] - '0') * 100;
		temp += (str[i++] - '0') * 10;
		t.tm_year = temp + (str[i++] - '0');
		t.tm_year -=1900;
	}
	int temp = 0;
	temp += (str[i++] - '0') * 10;
	t.tm_mon = (temp + (str[i++] - '0')) - 1;
	temp = 0;
	temp += (str[i++] - '0') * 10;
	t.tm_mday = temp + (str[i++] - '0');
	temp = 0;
	temp += (str[i++] - '0') * 10;
	t.tm_hour = temp + (str[i++] - '0');
	temp = 0;
	temp += (str[i++] - '0') * 10;
	t.tm_min = temp + (str[i++] - '0');
	temp = 0;
	temp += (str[i++] - '0') * 10;
	t.tm_sec = temp + (str[i++] - '0');

	return mktime(&t);
}

char * X509_getCountryName(X509_NAME *name,char * msginfo)
{
	int entriesNum;
	X509_NAME_ENTRY *name_entry;
	int msginfoLen;
	long Nid;
	int i;

	entriesNum = sk_X509_NAME_ENTRY_num(name->entries);
	for (i=0;i<entriesNum;i++)
	{
		name_entry = sk_X509_NAME_ENTRY_value(name->entries,i);
		Nid = OBJ_obj2nid(name_entry->object);
		msginfoLen=name_entry->value->length;   
        memcpy(msginfo,name_entry->value->data,msginfoLen);   
        msginfo[msginfoLen]='\0';
		if (Nid == NID_countryName){
			return msginfo;
		}
	}
	return msginfo;
}
char * X509_getProvinceName(X509_NAME *name,char * msginfo)
{
	int entriesNum;
	X509_NAME_ENTRY *name_entry;
	int msginfoLen;
	long Nid;
	int i;

	entriesNum = sk_X509_NAME_ENTRY_num(name->entries);
	for (i=0;i<entriesNum;i++)
	{
		name_entry = sk_X509_NAME_ENTRY_value(name->entries,i);
		Nid = OBJ_obj2nid(name_entry->object);
		msginfoLen=name_entry->value->length;   
        memcpy(msginfo,name_entry->value->data,msginfoLen);   
        msginfo[msginfoLen]='\0';  
		if (Nid == NID_stateOrProvinceName)
			return msginfo;
	}
	return msginfo;
}
char * X509_getLocalityName(X509_NAME *name,char * msginfo)
{
	int entriesNum;
	X509_NAME_ENTRY *name_entry;
	int msginfoLen;
	long Nid;
	int i;

	entriesNum = sk_X509_NAME_ENTRY_num(name->entries);
	for (i=0;i<entriesNum;i++)
	{
		name_entry = sk_X509_NAME_ENTRY_value(name->entries,i);
		Nid = OBJ_obj2nid(name_entry->object);
		msginfoLen=name_entry->value->length;   
        memcpy(msginfo,name_entry->value->data,msginfoLen);   
        msginfo[msginfoLen]='\0';  
		if (Nid == NID_localityName)
			return msginfo;
	}
	return msginfo;
}
char * X509_getOrganizationName(X509_NAME *name,char * msginfo)
{
	int entriesNum;
	X509_NAME_ENTRY *name_entry;
	int msginfoLen;
	long Nid;
	int i;

	entriesNum = sk_X509_NAME_ENTRY_num(name->entries);
	for (i=0;i<entriesNum;i++)
	{
		name_entry = sk_X509_NAME_ENTRY_value(name->entries,i);
		Nid = OBJ_obj2nid(name_entry->object);
		msginfoLen=name_entry->value->length;   
        memcpy(msginfo,name_entry->value->data,msginfoLen);   
        msginfo[msginfoLen]='\0';  
		if (Nid == NID_organizationName)
			return msginfo;
	}
	return msginfo;
}
char * X509_getOrganizationUnitName(X509_NAME *name,char * msginfo)
{
	int entriesNum;
	X509_NAME_ENTRY *name_entry;
	int msginfoLen;
	long Nid;
	int i;

	entriesNum = sk_X509_NAME_ENTRY_num(name->entries);
	for (i=0;i<entriesNum;i++)
	{
		name_entry = sk_X509_NAME_ENTRY_value(name->entries,i);
		Nid = OBJ_obj2nid(name_entry->object);
		msginfoLen=name_entry->value->length;   
        memcpy(msginfo,name_entry->value->data,msginfoLen);   
        msginfo[msginfoLen]='\0';  
		if (Nid == NID_organizationalUnitName)
			return msginfo;
	}
	return msginfo;
}
char * X509_getCommonName(X509_NAME *name,char * msginfo)
{
	int entriesNum;
	X509_NAME_ENTRY *name_entry;
	int msginfoLen;
	long Nid;
	int i;

	entriesNum = sk_X509_NAME_ENTRY_num(name->entries);
	for (i=0;i<entriesNum;i++)
	{
		name_entry = sk_X509_NAME_ENTRY_value(name->entries,i);
		Nid = OBJ_obj2nid(name_entry->object);
		msginfoLen=name_entry->value->length;   
        memcpy(msginfo,name_entry->value->data,msginfoLen);   
        msginfo[msginfoLen]='\0';  
		if (Nid == NID_commonName)
			return msginfo;
	}
	return msginfo;
}
char * X509_getEmailAddress(X509_NAME *name,char * msginfo)
{
	int entriesNum;
	X509_NAME_ENTRY *name_entry;
	int msginfoLen;
	long Nid;
	int i;

	entriesNum = sk_X509_NAME_ENTRY_num(name->entries);
	for (i=0;i<entriesNum;i++)
	{
		name_entry = sk_X509_NAME_ENTRY_value(name->entries,i);
		Nid = OBJ_obj2nid(name_entry->object);
		msginfoLen=name_entry->value->length;   
        memcpy(msginfo,name_entry->value->data,msginfoLen);   
        msginfo[msginfoLen]='\0';  
		if (Nid == NID_pkcs9_emailAddress)
			return msginfo;
	}
	return msginfo;
}

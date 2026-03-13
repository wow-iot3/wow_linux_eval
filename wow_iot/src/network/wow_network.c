#include <stdio.h>
#include <string.h>

#include "network/wow_tcp.h"
#include "network/wow_network.h"
#include "network/wow_socket.h"
#include "system/wow_countdown.h"
#include "system/wow_time.h"

#include "wow_lte.h"

#include "prefix/prefix.h"
#include "wow_iot_config.h"




typedef struct {
	int (*open)(void);
	int (*connect)(int sktfd, char *host, uint16_t port, uint32_t timeout_ms);
    int (*recv)(int sktfd, uint8_t *buffer, size_t len, uint32_t timeout_ms);
    int (*send)(int sktfd, uint8_t *buffer, size_t len, uint32_t timeout_ms);
    int (*close)(int sktfd);
} aiot_net_al_t;


typedef enum {
    CORE_SYSDEP_SOCKET_TCP_CLIENT,
    CORE_SYSDEP_SOCKET_TCP_SERVER,
    CORE_SYSDEP_SOCKET_UDP_CLIENT,
    CORE_SYSDEP_SOCKET_UDP_SERVER
}CoreSysdepSocketType_E;
	

	
typedef struct {
	char addr[16]; /* ipv4地址点分十进制字符串, 最大长度15字节.  */
	uint16_t port; /* 端口号 */
} core_sysdep_addr_t;

typedef struct {
    int fd;
    CoreSysdepSocketType_E socket_type;
    char *host;
    char backup_ip[16];
    uint16_t port;
    uint32_t connect_timeout_ms;
	aiot_net_al_t *net_api;
} core_network_handle_t;

static int gi_network_mode = NETWORK_MODE_PHY;

static int wow_hal_socket_open(void)
{
	int sktfd = wow_socket_open(SOCKET_TYPE_TCP);
	CHECK_RET_VAL_P(sktfd >= 0,-SOCKET_OPEN_FAILED,"wow_socket_open failed!\n");
	wow_socket_setopt(sktfd,SOCKET_CTRL_SET_BLOCK,0);

	return sktfd;
}

static int wow_hal_socket_connect(int sock, char *host, uint16_t port, uint32_t timeout_ms)
{
	timeout_ms =(timeout_ms == 0)?NETWORK_DEFAULT_CONNECT_TIMEOUT:timeout_ms; 

	int ret = wow_socket_connect(sock,host,port,timeout_ms);
	return (ret == 0)?0:-SOCKET_CONNECT_FAILED;
}

static int wow_hal_socket_send(int sock, uint8_t *buffer, size_t nSize, uint32_t timeout_ms)
{
	CHECK_RET_VAL_P(sock > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(buffer,-PARAM_INPUT_STRUCT_IS_NULL,"param input data invalid!\n");

    int   ret = -1;
    int   slen = 0;

	CountDown_T* cdown = wow_cdown_new();
	timeout_ms =(timeout_ms == 0)?NETWORK_DEFAULT_SEND_TIMEOUT:timeout_ms; 
	wow_cdown_set(cdown,timeout_ms);

    while ((slen < nSize) && (wow_cdown_is_expired(cdown) == 0)){

        ret = wow_socket_tsend(sock,buffer + slen, nSize - slen);
		CHECK_RET_BREAK(ret > 0);
        slen += ret;
    } 
	wow_cdown_free(&cdown);
	
    return slen;
}

static int wow_hal_socket_recv(int sock, uint8_t *buffer, size_t nSize, uint32_t timeout_ms)
{
	CHECK_RET_VAL_P(sock > 0,-PARAM_INPUT_ARG_INVALID,"param input arg invalid!\n");
	CHECK_RET_VAL_P(buffer,-PARAM_INPUT_STRUCT_IS_NULL,"param input data invalid!\n");

	int rlen = 0;
	int ret  = 0;
	CountDown_T* cdown = wow_cdown_new();
	wow_cdown_set(cdown,timeout_ms);
	
	do{
		ret = wow_socket_trecv(sock, buffer + rlen, nSize - rlen);
		if(ret > 0){
			rlen += ret;
		}else if(ret == -1){
			fd_set	 sets;
			struct timeval tv,*ptv;
			if(timeout_ms < 0){
				ptv = NULL;
			}else{
			
				tv.tv_sec  = timeout_ms / 1000;
				tv.tv_usec = (timeout_ms % 1000) * 1000;
				ptv = &tv;
			}
			
			FD_ZERO(&sets);
			FD_SET(sock, &sets);
			ret = select(sock + 1, &sets, NULL, NULL, ptv);
			CHECK_RET_BREAK(ret > 0);
			
			 /* 检查fd是否可读 */
			CHECK_RET_BREAK(FD_ISSET(sock, &sets));
		}else{
			break;
		}
	}while((rlen < nSize)&& (wow_cdown_is_expired(cdown) == 0));

	wow_cdown_free(&cdown);
	
    return rlen;
}
static int wow_hal_socket_disconnect(int sock)
{
	wow_socket_close(sock);

	return 0;
}
aiot_net_al_t  g_net_gprs_api = {
	.open      = wow_hal_lte_open,
	.connect   = wow_hal_lte_connect,
    .recv 	   = wow_hal_lte_recv,
    .send 	   = wow_hal_lte_send,
    .close 	   = wow_hal_lte_disconnect,
};

aiot_net_al_t  g_net_phy_api = {
	.open      = wow_hal_socket_open,
	.connect   = wow_hal_socket_connect,
    .recv 	   = wow_hal_socket_recv,
    .send 	   = wow_hal_socket_send,
    .close 	   = wow_hal_socket_disconnect,
};


static void *core_sysdep_network_init(void)
{
    core_network_handle_t *handle = CALLOC(1,core_network_handle_t);
	CHECK_RET_VAL_P(handle,NULL, "malloc core_network_handle_t failed!\n");
	
    handle->connect_timeout_ms = 5*1000;
	handle->socket_type        = CORE_SYSDEP_SOCKET_TCP_CLIENT;
	
	switch(gi_network_mode){
		case NETWORK_MODE_GPRS:
			wow_hal_lte_init();
			handle->net_api = &g_net_gprs_api;
			break;
		case NETWORK_MODE_PHY:
		default:
			handle->net_api = &g_net_phy_api;
			break;
	}

    return handle;
}

static int core_sysdep_network_setopt(void *handle, CoreSysdepNetOpt_E option, void *data)
{
    core_network_handle_t *network_handle = (core_network_handle_t *)handle;
 	CHECK_RET_VAL_P(network_handle,-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invaid!\n");
	CHECK_RET_VAL_P(data,-PARAM_INPUT_DATA_IS_NULL,"param input data invaid!\n");

    switch (option) {
    case CORE_SYSDEP_NETWORK_SOCKET_TYPE: 
        network_handle->socket_type = *(CoreSysdepSocketType_E *)data;
	    break;
    case CORE_SYSDEP_NETWORK_HOST: 
        network_handle->host = MALLOC(strlen(data) + 1);
		CHECK_RET_VAL_P(network_handle->host,-SYSTEM_MALLOC_FAILED,"malloc size:%zu failed!\n",strlen(data) + 1);
        memset(network_handle->host, 0, strlen(data) + 1);
        memcpy(network_handle->host, data, strlen(data));
	    break;
    case CORE_SYSDEP_NETWORK_BACKUP_IP: 
        memcpy(network_handle->backup_ip, data, strlen(data));
	    break;
    case CORE_SYSDEP_NETWORK_PORT: 
        network_handle->port = *(uint16_t *)data;
	    break;
    case CORE_SYSDEP_NETWORK_CONNECT_TIMEOUT_MS: 
        network_handle->connect_timeout_ms = *(uint32_t *)data;
	    break;
    default: 
        break;
    }

    return 0;
}

static int core_sysdep_network_open(void *handle)
{
    core_network_handle_t *network_handle = (core_network_handle_t *)handle;
	CHECK_RET_VAL_P(network_handle && network_handle->net_api && network_handle->net_api->open,
								-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invaid!\n");

	int fd = network_handle->net_api->open();
	CHECK_RET_VAL_P(fd >=0 ,-1, "net api open failed!\n");

	network_handle->fd = fd;
	return 0;
}

static int core_sysdep_network_connect(void *handle)
{
    core_network_handle_t *network_handle = (core_network_handle_t *)handle;
	CHECK_RET_VAL_P(network_handle && network_handle->net_api && network_handle->net_api->connect,
						-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invaid!\n");
	CHECK_RET_VAL_P(network_handle->socket_type == CORE_SYSDEP_SOCKET_TCP_CLIENT,-PARAM_INPUT_ARG_INVALID,"socket type not supoort!\n");
	CHECK_RET_VAL_P(network_handle->host,-PARAM_INPUT_ARG_INVALID,"host is not set\n");

	return network_handle->net_api->connect(network_handle->fd,network_handle->host,network_handle->port, network_handle->connect_timeout_ms);
}

static int core_sysdep_network_recv(void *handle, uint8_t *buffer, uint32_t len, uint32_t timeout_ms)
{
    core_network_handle_t *network_handle = (core_network_handle_t *)handle;
	CHECK_RET_VAL_P(network_handle && network_handle->net_api && network_handle->net_api->recv,
						-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invaid!\n");

    return network_handle->net_api->recv(network_handle->fd, buffer, len, timeout_ms);
}

static int core_sysdep_network_send(void *handle, uint8_t *buffer, uint32_t len, uint32_t timeout_ms)
{
    core_network_handle_t *network_handle = (core_network_handle_t *)handle;
   CHECK_RET_VAL_P(network_handle && network_handle->net_api && network_handle->net_api->send,
   						-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invaid!\n");
   
    return network_handle->net_api->send(network_handle->fd, buffer, len, timeout_ms);
}

static int core_sysdep_network_close(void *handle)
{
    core_network_handle_t *network_handle = (core_network_handle_t *)handle;
	CHECK_RET_VAL_P(network_handle && network_handle->net_api && network_handle->net_api->close,
						-PARAM_INPUT_STRUCT_IS_NULL,"param input struct invaid!\n");

	network_handle->net_api->close(network_handle->fd);
	network_handle->fd = -1;
	
	return 0;
}

static int core_sysdep_network_deinit(void **handle)
{
	CHECK_RET_VAL_P(handle && *handle,-PARAM_INPUT_DATA_IS_NULL,"param input data invaid!\n");

    core_network_handle_t *network_handle = *(core_network_handle_t **)handle;

    /* Shutdown both send and receive operations. */
    if (network_handle->socket_type == CORE_SYSDEP_SOCKET_TCP_CLIENT && network_handle->host != NULL) {
        network_handle->net_api->close(network_handle->fd);
    }

    if (network_handle->host != NULL) {
        FREE(network_handle->host);
        network_handle->host = NULL;
    }

    FREE(network_handle);
    *handle = NULL;

    return 0;
}

#ifdef WOW_MBEDTLS_SUPPORT
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/platform.h"
#include "mbedtls/timing.h"



typedef struct {
	mbedtls_net_context 		 net_ctx;
	mbedtls_ssl_context 		 ssl_ctx;
	mbedtls_ssl_config			 ssl_config;
	mbedtls_timing_delay_context timer_delay_ctx;
	mbedtls_x509_crt			 x509_server_cert;
	mbedtls_x509_crt			 x509_client_cert;
	mbedtls_pk_context			 x509_client_pk;
} core_sysdep_mbedtls_t;
#endif

typedef struct {
    char *psk_id;
    char *psk;
} core_sysdep_psk_t;	



struct network_adapter_t{
	void *network_handle;
	bool tls;
	CoreSysdepSocketType_E socket_type;
	CoreSysdepNetworkCred_T *cred;
	char *host;
	char backup_ip[16];
	uint16_t port;
	uint32_t connect_timeout_ms;
#ifdef WOW_MBEDTLS_SUPPORT
	core_sysdep_psk_t psk;
	core_sysdep_mbedtls_t mbedtls;
#endif
};



#ifdef WOW_MBEDTLS_SUPPORT
static uint8_t _host_is_ip(char *host)
{
	uint32_t idx = 0;

	if (strlen(host) >= 16) {
		return 0;
	}

	for (idx = 0; idx < strlen(host); idx++) {
		if ((host[idx] != '.') && (host[idx] < '0' || host[idx] > '9')) {
			return 0;
		}
	}

	return 1;
}


static int _core_mbedtls_random(void *handle, uint8_t *output, size_t output_len)
{
    uint32_t rnglen = output_len;
    uint8_t rngoffset = 0;

    while (rnglen > 0) {
        *(output + rngoffset) = (uint8_t)rand();
        rngoffset++;
        rnglen--;
    }

    return 0;
}

static void _core_mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
	((void) level);
	printf("-----%s\r\n", (char *)str);
}

static int _core_mbedtls_net_send(void *ctx, const uint8_t *buf, size_t len)
{
	return core_sysdep_network_send(ctx, (uint8_t *)buf, len, NETWORK_DEFAULT_SEND_TIMEOUT);
}

static int _core_mbedtls_net_recv(void *ctx, uint8_t *buf, size_t len)
{
	return core_sysdep_network_recv(ctx, buf, len, NETWORK_DEFAULT_RECV_TIMEOUT);
}
static int _core_mbedtls_net_recv_timeout(void *ctx, uint8_t *buf, size_t len,uint32_t timeout)
{
	return core_sysdep_network_recv(ctx, buf, len, timeout);
}


unsigned long _core_mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val, int reset)
{
	unsigned long delta;
	unsigned long offset;
	unsigned long *p_hr_time = (unsigned long *)&val->opaque;

	offset = wow_time_get_msec();
	if (reset) {
		*p_hr_time = offset;
		return (0);
	}
	delta = offset - *p_hr_time;

	return (delta);
}

void _core_mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms)
{
	mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;

	ctx->int_ms = int_ms;
	ctx->fin_ms = fin_ms;
	if (fin_ms != 0) {
		(void) _core_mbedtls_timing_get_timer(&ctx->timer, 1);
	}
}

/*
 * Get number of delays expired
 */
int _core_mbedtls_timing_get_delay(void *data)
{
	mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *) data;
	unsigned long elapsed_ms;

	if (ctx->fin_ms == 0) {
		return (-1);
	}

	elapsed_ms = _core_mbedtls_timing_get_timer(&ctx->timer, 0);
	if (elapsed_ms >= ctx->fin_ms) {
		return (2);
	}

	if (elapsed_ms >= ctx->int_ms) {
		return (1);
	}

	return (0);
}

int _tls_network_establish(void *handle)
{
	
	int ret = 0;
	NetworkAdapter_T *adpater = (NetworkAdapter_T *)handle;
	CHECK_RET_VAL_P(adpater && adpater->cred->max_tls_fragment > 0,-1,"param input invalid!\n");
	
	if (adpater->cred->max_tls_fragment <= 512) {
		ret = mbedtls_ssl_conf_max_frag_len(&adpater->mbedtls.ssl_config, MBEDTLS_SSL_MAX_FRAG_LEN_512);
	} else if (adpater->cred->max_tls_fragment <= 1024) {
		ret = mbedtls_ssl_conf_max_frag_len(&adpater->mbedtls.ssl_config, MBEDTLS_SSL_MAX_FRAG_LEN_1024);
	} else if (adpater->cred->max_tls_fragment <= 2048) {
		ret = mbedtls_ssl_conf_max_frag_len(&adpater->mbedtls.ssl_config, MBEDTLS_SSL_MAX_FRAG_LEN_2048);
	} else if (adpater->cred->max_tls_fragment <= 4096) {
		ret = mbedtls_ssl_conf_max_frag_len(&adpater->mbedtls.ssl_config, MBEDTLS_SSL_MAX_FRAG_LEN_4096);
	} else {
		ret = mbedtls_ssl_conf_max_frag_len(&adpater->mbedtls.ssl_config, MBEDTLS_SSL_MAX_FRAG_LEN_NONE);
	}
	CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_ssl_conf_max_frag_len error, ret: %x\r\n", ret);

	if (adpater->socket_type == CORE_SYSDEP_SOCKET_TCP_CLIENT) {
		ret = mbedtls_ssl_config_defaults(&adpater->mbedtls.ssl_config, MBEDTLS_SSL_IS_CLIENT,
										  MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	} else if (adpater->socket_type == CORE_SYSDEP_SOCKET_UDP_CLIENT) {
		ret = -1;
	}
	
	CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_ssl_config_defaults error, ret: %x\r\n", ret);
	

	mbedtls_ssl_conf_max_version(&adpater->mbedtls.ssl_config, MBEDTLS_SSL_MAJOR_VERSION_3,
								 MBEDTLS_SSL_MINOR_VERSION_3);
	mbedtls_ssl_conf_min_version(&adpater->mbedtls.ssl_config, MBEDTLS_SSL_MAJOR_VERSION_3,
								 MBEDTLS_SSL_MINOR_VERSION_3);
	
	mbedtls_ssl_conf_handshake_timeout(&adpater->mbedtls.ssl_config, (MBEDTLS_SSL_DTLS_TIMEOUT_DFL_MIN * 2),
									   (MBEDTLS_SSL_DTLS_TIMEOUT_DFL_MIN * 2 * 4));
	mbedtls_ssl_conf_rng(&adpater->mbedtls.ssl_config, _core_mbedtls_random, NULL);
	mbedtls_ssl_conf_dbg(&adpater->mbedtls.ssl_config, _core_mbedtls_debug, stdout);

	CHECK_RET_VAL_P(adpater->cred->option == CORE_SYSDEP_NETWORK_CRED_SVRCERT_CA || adpater->cred->option == CORE_SYSDEP_NETWORK_CRED_SVRCERT_PSK
				, -1, "unsupported security option\r\n");

	if (adpater->cred->option == CORE_SYSDEP_NETWORK_CRED_SVRCERT_CA) {
		CHECK_RET_VAL_P(adpater->cred->x509_server_cert && adpater->cred->x509_server_cert_len > 0,
								-1,"mbedtls_ssl_config_defaults error, ret: %x\r\n", ret);
		

		mbedtls_x509_crt_init(&adpater->mbedtls.x509_server_cert);
		ret = mbedtls_x509_crt_parse(&adpater->mbedtls.x509_server_cert,
									 (const uint8_t *)adpater->cred->x509_server_cert, (size_t)adpater->cred->x509_server_cert_len + 1);
		CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_x509_crt_parse server cert  error, ret: %x\r\n", ret);

		if (adpater->cred->x509_client_cert != NULL && adpater->cred->x509_client_cert_len > 0 &&
			adpater->cred->x509_client_privkey != NULL && adpater->cred->x509_client_privkey_len > 0) {
			mbedtls_x509_crt_init(&adpater->mbedtls.x509_client_cert);
			mbedtls_pk_init(&adpater->mbedtls.x509_client_pk);
			ret = mbedtls_x509_crt_parse(&adpater->mbedtls.x509_client_cert,
										 (const uint8_t *)adpater->cred->x509_client_cert, (size_t)adpater->cred->x509_client_cert_len + 1);
			CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_x509_crt_parse server cert  error, ret: %x\r\n", ret);
			ret = mbedtls_pk_parse_key(&adpater->mbedtls.x509_client_pk,
									   (const uint8_t *)adpater->cred->x509_client_privkey,
									   (size_t)adpater->cred->x509_client_privkey_len + 1, NULL, 0);
			CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_pk_parse_key client pk  error, ret: %x\r\n", ret);

			ret = mbedtls_ssl_conf_own_cert(&adpater->mbedtls.ssl_config, &adpater->mbedtls.x509_client_cert,
											&adpater->mbedtls.x509_client_pk);
			CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_ssl_conf_own_cert error, ret: %x\r\n", ret);
		}
		mbedtls_ssl_conf_ca_chain(&adpater->mbedtls.ssl_config, &adpater->mbedtls.x509_server_cert, NULL);
	} else if (adpater->cred->option == CORE_SYSDEP_NETWORK_CRED_SVRCERT_PSK) {
		static const int ciphersuites[1] = {MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA};
		ret = mbedtls_ssl_conf_psk(&adpater->mbedtls.ssl_config,
								   (const uint8_t *)adpater->psk.psk, (size_t)strlen(adpater->psk.psk),
								   (const uint8_t *)adpater->psk.psk_id, (size_t)strlen(adpater->psk.psk_id));
		CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_ssl_conf_psk error, ret: %x\r\n", ret);

		mbedtls_ssl_conf_ciphersuites(&adpater->mbedtls.ssl_config, ciphersuites);
	} 

	ret = mbedtls_ssl_setup(&adpater->mbedtls.ssl_ctx, &adpater->mbedtls.ssl_config);
	CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_ssl_setup error, ret: %x\r\n", ret);

	if (_host_is_ip(adpater->host) == 0) {
		ret = mbedtls_ssl_set_hostname(&adpater->mbedtls.ssl_ctx, adpater->host);
		CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_ssl_mbedtls_ssl_set_hostnamesetup error, ret: %x\r\n", ret);
	}


	mbedtls_ssl_set_bio(&adpater->mbedtls.ssl_ctx, adpater->network_handle, _core_mbedtls_net_send,
						_core_mbedtls_net_recv, _core_mbedtls_net_recv_timeout);
	mbedtls_ssl_conf_read_timeout(&adpater->mbedtls.ssl_config, adpater->connect_timeout_ms);

	/* Handshake */
    while ((ret = mbedtls_ssl_handshake(&adpater->mbedtls.ssl_ctx)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            printf("mbedtls_ssl_handshake() failed, ret:-0x%x.\r\n", -ret);
            break;
        }
    }
	
	ret = mbedtls_ssl_get_verify_result(&adpater->mbedtls.ssl_ctx);
	CHECK_RET_VAL_P(ret == 0,-1,"mbedtls_ssl_get_verify_result error, ret: %x.\r\n", ret);

	return 0;
}

int _tls_network_recv(void *handle, uint8_t *buffer, size_t len, uint32_t timeout_ms)
{
	int ret = 0;
	int recv_bytes = 0;

	NetworkAdapter_T *adpater = (NetworkAdapter_T *)handle;
	CHECK_RET_VAL_P(adpater,-1,"param input invalid!\n");
	
	CountDown_T* cdown = wow_cdown_new();
	wow_cdown_set(cdown,timeout_ms);

	_core_mbedtls_timing_set_delay(&adpater->mbedtls.timer_delay_ctx, 0, 0);
	mbedtls_ssl_conf_read_timeout(&adpater->mbedtls.ssl_config, timeout_ms);
	do {
		ret = mbedtls_ssl_read(&adpater->mbedtls.ssl_ctx, buffer + recv_bytes, len - recv_bytes);
		if (ret < 0) {
			if (ret == MBEDTLS_ERR_SSL_TIMEOUT) {
				break;
			} else if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
					   ret != MBEDTLS_ERR_SSL_WANT_WRITE &&
					   ret != MBEDTLS_ERR_SSL_CLIENT_RECONNECT) {
				CHECK_RET_VAL(recv_bytes > 0,-1);	   
				break;
			}
		} else if (ret == 0) {
			break;
		} else {
			recv_bytes += ret;
		}
	} while (recv_bytes < len || wow_cdown_is_expired(cdown) == 0);
	wow_cdown_free(&cdown);
	
	return recv_bytes;
}

int _tls_network_send(void *handle, uint8_t *buffer, size_t len, uint32_t timeout_ms)
{
	int ret = 0;
	int send_bytes = 0;
	
	NetworkAdapter_T *adpater = (NetworkAdapter_T *)handle;
	CHECK_RET_VAL_P(adpater,-1,"param input invalid!\n");
	
	CountDown_T* cdown = wow_cdown_new();
	wow_cdown_set(cdown,timeout_ms);

	do {
		ret = mbedtls_ssl_write(&adpater->mbedtls.ssl_ctx, buffer + send_bytes, len - send_bytes);
		if (ret < 0) {
			if (ret != MBEDTLS_ERR_SSL_WANT_READ &&
				ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
				CHECK_RET_VAL(send_bytes > 0,-1);	
				break;
			}
		} else if (ret == 0) {
			break;
		} else {
			send_bytes += ret;
		}
	} while (wow_cdown_is_expired(cdown) == 0);
	wow_cdown_free(&cdown);

	return send_bytes;
}
#endif

/*brief    配置网络模式
 *param ： eMode   :网络模式
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_network_config(NetworkMode_E eMode)
{
	CHECK_RET_VAL_P(eMode < NETWORK_MODE_MAX,-1,"param input invalid!\n");
	gi_network_mode = eMode;

	return 0;
}

/*brief    网络适配器初始化
 *param ： tls   :是否为tls模式
 *return： 成功返回操作描述符 失败返回-1
 */
__EX_API__ NetworkAdapter_T* wow_network_adapter_init(bool tls)
{
	NetworkAdapter_T *adapter = CALLOC(1,NetworkAdapter_T);
	CHECK_RET_VAL_P(adapter,NULL,"malloc NetworkAdapter_T failed!\n");
	
	adapter->network_handle = core_sysdep_network_init();
	adapter->tls = tls;

#ifdef WOW_MBEDTLS_SUPPORT
	if(tls){
		adapter->psk.psk_id = NULL;
		adapter->psk.psk = NULL;
		mbedtls_debug_set_threshold(0);
		mbedtls_ssl_init(&adapter->mbedtls.ssl_ctx);
		mbedtls_ssl_config_init(&adapter->mbedtls.ssl_config);
	}
#endif

	return adapter;
}

/*brief    网络适配器参数设置
 *param ： ptNetAdp :操作描述符
 *param ： option :操作标识
 *param ： eOption   :标识数据
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_network_adapter_setopt(NetworkAdapter_T* ptNetAdp, CoreSysdepNetOpt_E eOption, void *pData)
{
	CHECK_RET_VAL_P(ptNetAdp, -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pData, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	CHECK_RET_VAL_P(eOption < CORE_SYSDEP_NETWORK_MAX, -PARAM_INPUT_ARG_INVALID, "param input arg invalid!\n");

	int ret = 0;

	ret = core_sysdep_network_setopt(ptNetAdp->network_handle, eOption, pData);
	
#ifdef WOW_MBEDTLS_SUPPORT
	if(ptNetAdp->tls){
		switch (eOption) {
			case CORE_SYSDEP_NETWORK_SOCKET_TYPE: {
				ptNetAdp->socket_type = *(CoreSysdepSocketType_E *)pData;
			}
			break;
			case CORE_SYSDEP_NETWORK_HOST: {
				ptNetAdp->host = MALLOC(strlen(pData) + 1);
				CHECK_RET_VAL_P(ptNetAdp->host,-1,"malloc size:%zu failed!\n",strlen(pData) + 1);
				memset(ptNetAdp->host, 0, strlen(pData) + 1);
				memcpy(ptNetAdp->host, pData, strlen(pData));
			}
			break;
			case CORE_SYSDEP_NETWORK_BACKUP_IP: {
				memcpy(ptNetAdp->backup_ip, pData, strlen(pData));
			}
			break;
			case CORE_SYSDEP_NETWORK_PORT: {
				ptNetAdp->port = *(uint16_t *)pData;
			}
			break;
			case CORE_SYSDEP_NETWORK_CONNECT_TIMEOUT_MS: {
				ptNetAdp->connect_timeout_ms = *(uint32_t *)pData;
			}
			break;

			case CORE_SYSDEP_NETWORK_CRED: {
				ptNetAdp->cred = CALLOC(1,CoreSysdepNetworkCred_T);
				CHECK_RET_VAL_P(ptNetAdp->cred,-1,"malloc CoreSysdepNetworkCred_T failed!\n");
				memcpy(ptNetAdp->cred, pData, sizeof(CoreSysdepNetworkCred_T));
			}
			break;
			case CORE_SYSDEP_NETWORK_PSK: {
				core_sysdep_psk_t *psk = (core_sysdep_psk_t *)pData;
				ptNetAdp->psk.psk_id = MALLOC(strlen(psk->psk_id) + 1);
				CHECK_RET_VAL_P(ptNetAdp->host,-1,"malloc size:%zu failed!\n",strlen(psk->psk_id) + 1);
				memset(ptNetAdp->psk.psk_id, 0, strlen(psk->psk_id) + 1);
				memcpy(ptNetAdp->psk.psk_id, psk->psk_id, strlen(psk->psk_id));
				
				ptNetAdp->psk.psk = MALLOC(strlen(psk->psk) + 1);
				CHECK_RET_VAL_EXE_P(ptNetAdp->host,-1,FREE(ptNetAdp->psk.psk_id),
								"malloc size:%zu failed!\n",strlen(psk->psk_id) + 1);
				memset(ptNetAdp->psk.psk, 0, strlen(psk->psk) + 1);
				memcpy(ptNetAdp->psk.psk, psk->psk, strlen(psk->psk));
			}
			break;
			case CORE_SYSDEP_NETWORK_MAX:
			break;
		}
	}
#endif

	return ret;
}

/*brief    网络适配器建立连接
 *param ： ptNetAdp :操作描述符
 *return： 成功返回0 失败返回-1
 */
__EX_API__ int wow_network_adapter_connect(NetworkAdapter_T* ptNetAdp)
{
	int ret = 0;
	CHECK_RET_VAL_P(ptNetAdp, -PARAM_INPUT_STRUCT_IS_NULL, "param input invalid!\n");

	ret = core_sysdep_network_open(ptNetAdp->network_handle); 
	CHECK_RET_VAL_P(ret == 0, ret, "core_sysdep_network_open failed!\n");

	ret = core_sysdep_network_connect(ptNetAdp->network_handle);
	CHECK_RET_GOTO_P(ret == 0, out, "core_sysdep_network_connect failed!\n");
#ifdef WOW_MBEDTLS_SUPPORT
	if (ptNetAdp->tls && ptNetAdp->cred != NULL && ptNetAdp->cred->option != CORE_SYSDEP_NETWORK_CRED_NONE) {
		ret = _tls_network_establish(ptNetAdp);
	}
#endif
	return ret;

out:
	core_sysdep_network_close(ptNetAdp->network_handle);
	return ret;
}

/*brief    网络适配器接收数据
 *param ： ptNetAdp :操作描述符
 *param ： pu8Buff  :数据内容
 *param ： snLen    :数据长度
 *param ： u32TimeoutMs:超时时间
 *return： 成功返回接收长度 失败返回-1
 */
__EX_API__ int wow_network_adapter_recv(NetworkAdapter_T* ptNetAdp, uint8_t *pu8Buff, size_t snLen, uint32_t u32TimeoutMs)
{
	int ret = 0;
	CHECK_RET_VAL_P(ptNetAdp, -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Buff, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	CHECK_RET_VAL_P(snLen > 0, -PARAM_INPUT_ARG_INVALID, "param input arg invalid!\n");
	
#ifdef WOW_MBEDTLS_SUPPORT
	if (ptNetAdp->tls && ptNetAdp->cred != NULL && ptNetAdp->cred->option != CORE_SYSDEP_NETWORK_CRED_NONE) {
		ret = _tls_network_recv(ptNetAdp, pu8Buff, snLen, u32TimeoutMs);
	} else
#endif
	{
		ret = core_sysdep_network_recv(ptNetAdp->network_handle, pu8Buff, snLen, u32TimeoutMs);
	}

	return ret;
}

/*brief    网络适配器发送数据
 *param ： ptNetAdp :操作描述符
 *param ： pu8Buff  :数据内容
 *param ： snLen    :数据长度
 *param ： u32TimeoutMs:超时时间
 *return： 成功返回发送长度 失败返回-1
 */
__EX_API__ int wow_network_adapter_send(NetworkAdapter_T* ptNetAdp, uint8_t *pu8Buff, size_t snLen, uint32_t u32TimeoutMs)
{
	int ret = 0;
	CHECK_RET_VAL_P(ptNetAdp, -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");
	CHECK_RET_VAL_P(pu8Buff, -PARAM_INPUT_DATA_IS_NULL, "param input data invalid!\n");
	CHECK_RET_VAL_P(snLen > 0, -PARAM_INPUT_ARG_INVALID, "param input arg invalid!\n");
	
#ifdef WOW_MBEDTLS_SUPPORT
	if (ptNetAdp->tls && ptNetAdp->cred != NULL && ptNetAdp->cred->option != CORE_SYSDEP_NETWORK_CRED_NONE) {
		ret = _tls_network_send(ptNetAdp, pu8Buff, snLen, u32TimeoutMs);
	} else
#endif
	{
		ret = core_sysdep_network_send(ptNetAdp->network_handle,pu8Buff, snLen, u32TimeoutMs);
	}

	return ret;
}

/*brief    网络适配器断开连接
 *param ： ptNetAdp :操作描述符
 *return： 成功返0 失败返回-1
 */
int wow_network_adapter_close(NetworkAdapter_T* ptNetAdp)
{
	CHECK_RET_VAL_P(ptNetAdp, -PARAM_INPUT_STRUCT_IS_NULL, "param input struct invalid!\n");

#ifdef WOW_MBEDTLS_SUPPORT
	if(ptNetAdp->tls){
		mbedtls_ssl_close_notify(&ptNetAdp->mbedtls.ssl_ctx);
		if (ptNetAdp->cred != NULL && ptNetAdp->cred->option == CORE_SYSDEP_NETWORK_CRED_SVRCERT_CA) {
			mbedtls_x509_crt_free(&ptNetAdp->mbedtls.x509_server_cert);
			mbedtls_x509_crt_free(&ptNetAdp->mbedtls.x509_client_cert);
			mbedtls_pk_free(&ptNetAdp->mbedtls.x509_client_pk);
		}
		mbedtls_ssl_free(&ptNetAdp->mbedtls.ssl_ctx);
		mbedtls_ssl_config_free(&ptNetAdp->mbedtls.ssl_config);
	}
#endif

	return core_sysdep_network_close(ptNetAdp->network_handle);
}

/*brief    网络适配器去初始化
 *param ： pptNetAdp :操作描述符
 *return： 成功返回0 失败返回-1
 */
int wow_network_adapter_deinit(NetworkAdapter_T** pptNetAdp)
{
	CHECK_RET_VAL(pptNetAdp && *pptNetAdp, -PARAM_INPUT_STRUCT_IS_NULL);
	NetworkAdapter_T *adapter = *(NetworkAdapter_T **)pptNetAdp;


	if (adapter->host != NULL) {
		FREE(adapter->host);
		adapter->host = NULL;
	}
#ifdef WOW_MBEDTLS_SUPPORT
	if(adapter->tls){
		mbedtls_ssl_close_notify(&adapter->mbedtls.ssl_ctx);
		if (adapter->cred != NULL && adapter->cred->option == CORE_SYSDEP_NETWORK_CRED_SVRCERT_CA) {
			mbedtls_x509_crt_free(&adapter->mbedtls.x509_server_cert);
			mbedtls_x509_crt_free(&adapter->mbedtls.x509_client_cert);
			mbedtls_pk_free(&adapter->mbedtls.x509_client_pk);
		}
		mbedtls_ssl_free(&adapter->mbedtls.ssl_ctx);
		mbedtls_ssl_config_free(&adapter->mbedtls.ssl_config);

		if (adapter->psk.psk_id != NULL) {
			FREE(adapter->psk.psk_id);
			adapter->psk.psk_id = NULL;
		}
		if (adapter->psk.psk != NULL) {
			FREE(adapter->psk.psk);
			adapter->psk.psk = NULL;
		}
	}
#endif
	if (adapter->cred != NULL) {
		FREE(adapter->cred);
		adapter->cred = NULL;
	}
	
	core_sysdep_network_deinit(&adapter->network_handle);
	
	FREE(adapter);
	*pptNetAdp = NULL;

	return 0;
}

	



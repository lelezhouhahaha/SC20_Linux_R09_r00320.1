#SUBDIRS=client core shared
AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -fpie -pie \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat \
		-DLWM2M_CLIENT_MODE \
		-DLWM2M_BOOTSTRAP \
		-DAUTHORIZATION_SUPPORT \
		-DLWM2M_LITTLE_ENDIAN \
		-DDEBUG_DUMPSTACK \
		-DLWM2M_WITH_LOGS \
		-DWITH_SHA256 \
		-DWITH_TLS \
		-DWITH_MBEDTLS \
		-DLWM2M_VERSION_1_0 \
		-DENABLE_SOFTWARE_MGNT_OBJ \
		-DCONNECTIVITY_STATS_ENHANCEMENT \
		-DMBEDTLS_SSL_CIPHERSUITES=MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256 \
		-DENABLE_LWM2M_NETWORK_MANAGERMENT \
		-DVZW_FATAL_ALERT_SUPPORT \
		-DLWM2M_SUPPORT_QLRIL_API \
		-DENABLE_QUECTEL_LWM2M_DEFAULT

#-DDTLS_SESSION_FREQUENTLY_RESUMPTION 
#-DENABLE_QUECTEL_LWM2M_DEFAULT

AM_CXXFLAGS = \
        $(AM_CFLAGS) \
        -fpermissive

AM_CPPFLAGS = -fpie -pie \
		-Dstrlcpy=g_strlcpy \
		-Dstrlcat=g_strlcat \
		-I$(WORKSPACE)/quectel-core/ql-mcm-api/inc \
		-I$(WORKSPACE)/quectel-core/ql-ota/include
        
#AM_CPPFLAGS = -I$(WORKSPACE)/quectel-core/ql-mbedtls/library \
#              -I$(WORKSPACE)/quectel-core/ql-mbedtls/include/
#-lmbedtls -lmbedx509 -lmbedcrypto 

requiredlibs = -lgthread-2.0 \
		 -lglib-2.0 -lpthread \
		 -lql_power \
                 -lql_ril \
		 -lql_factory_reset\
		 $(QLMCMAPI_LIBS) \
		 -lql_ota \
		 -lcurl \
		 -lql_mcm_api \
		 -lcutils

library_include_INCLUDES = $(QLMCMAPI_CFLAGS) 
library_includedir = $(pkgincludedir)

INCLUDES=-I$(top_srcdir)/src/examples/client \
         -I$(top_srcdir)/src/core \
		 -I$(top_srcdir)/src/examples/shared \
		 -I$(top_srcdir)/src/utils \
		 -I$(top_srcdir)/src/examples/shared/mbedtls/include

lwm2mclient_sources =examples/client/lwm2mclient.c \
				examples/client/object_access_control.c \
				examples/client/object_connectivity_moni.c \
				examples/client/object_connectivity_stat.c \
				examples/client/object_device.c \
				examples/client/object_firmware.c \
				examples/client/object_location.c \
				examples/client/object_security.c \
				examples/client/object_server.c \
				examples/client/system_api.c \
				examples/client/test_object.c \
				examples/client/object_host_device.c \
				examples/client/object_apn_connection_profile.c \
				examples/client/object_connectivity_exte.c \
				examples/client/object_devicecap.c \
				examples/client/object_host_device_vzw.c \
				examples/client/object_software_mgnt.c \
                examples/client/lwm2m_configure.c \
                examples/client/cJSON.c \
                examples/client/quectel_sms.c \
                examples/client/j_aes.c \
                examples/client/quectel_getinfo.c \
				core/er-coap-13/er-coap-13.c \
				core/block1.c \
				core/bootstrap.c \
				core/data.c \
				core/discover.c \
				core/json.c \
				core/json_common.c \
				core/liblwm2m.c \
				core/list.c \
				core/management.c \
				core/objects.c \
				core/observe.c \
				core/packet.c \
				core/registration.c \
				core/senml_json.c \
				core/tlv.c \
				core/transaction.c \
				core/uri.c \
				core/utils.c \
				examples/shared/commandline.c \
				examples/shared/dtlsconnection.c \
				examples/shared/memtrace.c \
				examples/shared/platform.c \
				examples/shared/mbedtls/library/aes.c \
       			examples/shared/mbedtls/library/aesni.c \
		        examples/shared/mbedtls/library/arc4.c \
		        examples/shared/mbedtls/library/aria.c \
		        examples/shared/mbedtls/library/asn1parse.c \
		        examples/shared/mbedtls/library/asn1write.c \
		        examples/shared/mbedtls/library/base64.c \
		        examples/shared/mbedtls/library/bignum.c \
		        examples/shared/mbedtls/library/blowfish.c \
		        examples/shared/mbedtls/library/camellia.c \
		        examples/shared/mbedtls/library/ccm.c \
		        examples/shared/mbedtls/library/chacha20.c \
		        examples/shared/mbedtls/library/chachapoly.c \
		        examples/shared/mbedtls/library/cipher.c \
		        examples/shared/mbedtls/library/cipher_wrap.c \
		        examples/shared/mbedtls/library/cmac.c \
		        examples/shared/mbedtls/library/ctr_drbg.c \
		        examples/shared/mbedtls/library/des.c \
		        examples/shared/mbedtls/library/dhm.c \
		        examples/shared/mbedtls/library/ecdh.c \
		        examples/shared/mbedtls/library/ecdsa.c \
		        examples/shared/mbedtls/library/ecjpake.c \
		        examples/shared/mbedtls/library/ecp.c \
		        examples/shared/mbedtls/library/ecp_curves.c \
		        examples/shared/mbedtls/library/entropy.c \
		        examples/shared/mbedtls/library/entropy_poll.c \
		        examples/shared/mbedtls/library/error.c \
		        examples/shared/mbedtls/library/gcm.c \
		        examples/shared/mbedtls/library/havege.c \
		        examples/shared/mbedtls/library/hkdf.c \
		        examples/shared/mbedtls/library/hmac_drbg.c \
		        examples/shared/mbedtls/library/md.c \
		        examples/shared/mbedtls/library/md2.c \
		        examples/shared/mbedtls/library/md4.c \
		        examples/shared/mbedtls/library/md5.c \
		        examples/shared/mbedtls/library/md_wrap.c \
		        examples/shared/mbedtls/library/memory_buffer_alloc.c \
		        examples/shared/mbedtls/library/nist_kw.c \
		        examples/shared/mbedtls/library/oid.c \
		        examples/shared/mbedtls/library/padlock.c \
		        examples/shared/mbedtls/library/pem.c \
		        examples/shared/mbedtls/library/pk.c \
		        examples/shared/mbedtls/library/pk_wrap.c \
		        examples/shared/mbedtls/library/pkcs12.c \
		        examples/shared/mbedtls/library/pkcs5.c \
		        examples/shared/mbedtls/library/pkparse.c \
		        examples/shared/mbedtls/library/pkwrite.c \
		        examples/shared/mbedtls/library/platform.c \
		        examples/shared/mbedtls/library/platform_util.c \
		        examples/shared/mbedtls/library/poly1305.c \
		        examples/shared/mbedtls/library/ripemd160.c \
		        examples/shared/mbedtls/library/rsa.c \
		        examples/shared/mbedtls/library/rsa_internal.c \
		        examples/shared/mbedtls/library/sha1.c \
		        examples/shared/mbedtls/library/sha256.c \
		        examples/shared/mbedtls/library/sha512.c \
		        examples/shared/mbedtls/library/threading.c \
		        examples/shared/mbedtls/library/timing.c \
		        examples/shared/mbedtls/library/version.c \
		        examples/shared/mbedtls/library/version_features.c \
		        examples/shared/mbedtls/library/xtea.c \
 		        examples/shared/mbedtls/library/certs.c \
 		        examples/shared/mbedtls/library/pkcs11.c \
 		        examples/shared/mbedtls/library/x509.c \
 		        examples/shared/mbedtls/library/x509_create.c \
 		        examples/shared/mbedtls/library/x509_crl.c \
 		        examples/shared/mbedtls/library/x509_crt.c \
 		        examples/shared/mbedtls/library/x509_csr.c \
 		        examples/shared/mbedtls/library/x509write_crt.c \
 		        examples/shared/mbedtls/library/x509write_csr.c \
		        examples/shared/mbedtls/library/debug.c \
		        examples/shared/mbedtls/library/net_sockets.c \
		        examples/shared/mbedtls/library/ssl_cache.c \
		        examples/shared/mbedtls/library/ssl_ciphersuites.c \
		        examples/shared/mbedtls/library/ssl_cli.c \
		        examples/shared/mbedtls/library/ssl_cookie.c \
		        examples/shared/mbedtls/library/ssl_srv.c \
		        examples/shared/mbedtls/library/ssl_ticket.c \
		        examples/shared/mbedtls/library/ssl_tls.c \
		        utils/quectel_info_util.c
	
#QCMAP_CLI_h_sources =

bin_PROGRAMS = lwm2mclient 
lwm2mclient_CC = @CC@
lwm2mclient_SOURCES = $(lwm2mclient_sources)
#if USE_GLIB
lwm2mclient_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) #@GLIB_CFLAGS@
lwm2mclient_LDFLAGS = -lpthread $(XMLLIB_LIBS) #@GLIB_LIBS@
lwm2mclient_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) #@GLIB_CFLAGS@
#else
lwm2mclient_CFLAGS = $(AM_CFLAGS) -fPIC
lwm2mclient_LDFLAGS = -lpthread $(XMLLIB_LIBS) 
lwm2mclient_CPPFLAGS = $(AM_CPPFLAGS)
#endif
lwm2mclient_LDADD = $(requiredlibs)

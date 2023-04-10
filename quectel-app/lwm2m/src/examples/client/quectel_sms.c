#include <ql-mcm-api/ql_in.h>
#include "quectel_sms.h"
#include "liblwm2m.h"
#include "internals.h"
static sms_client_handle_type      h_sms;
#define BUF_SIZE 32
#define MAX_CHARACTER_SIZE    8   
static lwm2m_context_t * lwm2m_ptr;
extern lwm2m_context_t *g_lwm2mH_ptr;

unsigned char* UnicodeToUTF8( int unicode, unsigned char *p)
{  
    unsigned char *e = NULL;  
    if((e = p))  
    {  
        if(unicode < 0x80)  
        {  
            *e++ = unicode;  
        }  
        else if(unicode < 0x800)  
        {
            *e++ = ((unicode >> 6) & 0x1f)|0xc0;  
            *e++ = (unicode & 0x3f)|0x80;   
        }  
        else if(unicode < 0x10000)  
        {
            *e++ = ((unicode >> 12) & 0x0f)|0xe0;   
            *e++ = ((unicode >> 6) & 0x3f)|0x80;  
            *e++ = (unicode & 0x3f)|0x80;   
        }  
        else if(unicode < 0x200000)  
        {
            *e++ = ((unicode >> 18) & 0x07)|0xf0;   
            *e++ = ((unicode >> 12) & 0x3f)|0x80;  
            *e++ = ((unicode >> 6) & 0x3f)|0x80;  
            *e++ = (unicode & 0x3f)|0x80;   
        }  
        else if(unicode < 0x4000000)  
        {
            *e++ = ((unicode >> 24) & 0x03)|0xf8 ;   
            *e++ = ((unicode >> 18) & 0x3f)|0x80;  
            *e++ = ((unicode >> 12) & 0x3f)|0x80;  
            *e++ = ((unicode >> 6) & 0x3f)|0x80;  
            *e++ = (unicode & 0x3f)|0x80;   
        }  
        else 
        {
            *e++ = ((unicode >> 30) & 0x01)|0xfc;
            *e++ = ((unicode >> 24) & 0x3f)|0x80;
            *e++ = ((unicode >> 18) & 0x3f)|0x80;
            *e++ = ((unicode >> 12) & 0x3f)|0x80;
            *e++ = ((unicode >> 6) & 0x3f)|0x80;  
            *e++ = (unicode & 0x3f)|0x80;   
        }  
    }  
     
    return e;  
}  

int UTF8ToUnicode (unsigned char *ch, int *unicode)
{  
    unsigned char *p = NULL;  
    int e = 0, n = 0;  
    if((p = ch) && unicode)  
    {  
        if(*p >= 0xfc)  
        {
            e = (p[0] & 0x01) << 30;  
            e |= (p[1] & 0x3f) << 24;  
            e |= (p[2] & 0x3f) << 18;  
            e |= (p[3] & 0x3f) << 12;  
            e |= (p[4] & 0x3f) << 6;  
            e |= (p[5] & 0x3f);  
            n = 6;  
        }  
        else if(*p >= 0xf8)   
        {
            e = (p[0] & 0x03) << 24;  
            e |= (p[1] & 0x3f) << 18;  
            e |= (p[2] & 0x3f) << 12;  
            e |= (p[3] & 0x3f) << 6;  
            e |= (p[4] & 0x3f);  
            n = 5;  
        }  
        else if(*p >= 0xf0)  
        {
            e = (p[0] & 0x07) << 18;  
            e |= (p[1] & 0x3f) << 12;  
            e |= (p[2] & 0x3f) << 6;  
            e |= (p[3] & 0x3f);  
            n = 4;  
        }  
        else if(*p >= 0xe0)  
        {
            e = (p[0] & 0x0f) << 12;  
            e |= (p[1] & 0x3f) << 6;  
            e |= (p[2] & 0x3f);  
            n = 3;  
        }  
        else if(*p >= 0xc0)   
        {
            e = (p[0] & 0x1f) << 6;  
            e |= (p[1] & 0x3f);  
            n = 2;  
        }  
        else   
        {  
            e = p[0];  
            n = 1;  
        }  
        *unicode = e;  
    }  
     
    return n;  
}  

unsigned char* UnicodeStrToUTF8Str (unsigned short *unicode_str,  
                                    unsigned char   *utf8_str, 
                                    int             utf8_str_size)
{
    int unicode = 0;  
    unsigned char *e = NULL, *s = NULL;  
    unsigned char utf8_ch[MAX_CHARACTER_SIZE];   
    s = utf8_str;  
    if ((unicode_str) && (s))  
    {  
        while ((unicode = (int) (*unicode_str++)))  
        {  
            #if 1//LE
                unicode = ((unicode&0xFF)<<8) | ((unicode&0xFF00) >> 8);  //error
            #else//BE
                unicode = unicode;  
            #endif
            memset (utf8_ch, 0, sizeof (utf8_ch));  
              
            if ((e = UnicodeToUTF8 (unicode, utf8_ch)) > utf8_ch)  
            {  
                *e = '\0';
                 
                if ((s - utf8_str + strlen ((const char *) utf8_ch)) >= utf8_str_size)  
                {  
                    return s;  
                }  
                else 
                {  
                    memcpy (s, utf8_ch, strlen ((const char *) utf8_ch));  
                    s += strlen ((const char *) utf8_ch);  
                    *s = '\0';  
                }  
            }  
            else 
            {
                return s;  
            }  
        }  
    }  
      
    return s;  
}  

int UTF8StrToUnicodeStr(unsigned char   *utf8_str,  
                        uint16_t        *unicode_str, 
                        int             unicode_str_size)
{  
    int unicode = 0;  
    int n = 0;  
    int count = 0;  
    unsigned char *s = NULL;  
    uint16_t *e = NULL;  
      
    s = utf8_str;  
    e = unicode_str;  
      
    if ((utf8_str) && (unicode_str))  
    {  
        while (*s)  
        {  
            if ((n = UTF8ToUnicode (s, &unicode)) > 0)  
            {  
                if ((count + 1) >= unicode_str_size)  
                {  
                    return count;  
                }  
                else 
                {  
                #if 0//LE
                    *e = (unsigned short) unicode;  
                #else//BE
                    *e = ((unicode&0xFF)<<8) | ((unicode&0xFF00) >> 8);  
                #endif
                    e++;  
                    *e = 0;                     
                    s += n;  
                    count++;
                }  
            }  
            else 
            {
                return count;  
            }  
        }  
    }  
      
    return count;  
} 

static void ql_sms_cb_func( QL_SMS_MsgRef   msgRef,  
                            void*               contextPtr)
{
    int i;
    if(msgRef->e_storage != E_QL_SMS_STORAGE_TYPE_NONE)
    {
        char *msg_format[]  = {"CDMA",  "GW"};
        char *storage_type[]= {"UIM",   "NV"};
        LOG_ARG("###You've got one new %s message, stored to %s index=%d\n", 
                    msg_format[msgRef->e_mode & 1],
                    storage_type[msgRef->e_storage & 1],
                    msgRef->storage_index);  
    }
    else if(msgRef->format == E_QL_SMS_FORMAT_UCS2)
    {
        unsigned char* smsbuf = NULL;

        smsbuf = (char*)malloc(sizeof(char)*QL_SMS_MAX_MT_MSG_LENGTH);
        if(smsbuf == NULL)
        {
            LOG("Out of memory");
            return;
        }
        memset(smsbuf, 0, QL_SMS_MAX_MT_MSG_LENGTH);
        UnicodeStrToUTF8Str((unsigned short*)(&msgRef->sms_data[0]),  
                              smsbuf, 
                              QL_SMS_MAX_MT_MSG_LENGTH) ;
		if (msgRef->user_data_head_valid)
		{
			LOG_ARG("\n###You've got one new UCS2 msg from %s at %s, total_segments:%d, seg_number:%d, reference_number:%02x, len=%d, content=%s\n", 
							msgRef->src_addr,
							msgRef->timestamp,
							msgRef->user_data_head.total_segments,
							msgRef->user_data_head.seg_number,
							msgRef->user_data_head.reference_number,
							msgRef->sms_data_len,
							smsbuf); 
		}
		else
		{
	        LOG_ARG("\n###You've got one new UCS2 msg from %s at %s, len=%d, content=%s\n", 
		                    msgRef->src_addr,
		                    msgRef->timestamp,
		                    msgRef->sms_data_len,
		                    smsbuf); 
    	}
        LOG("Received UCS raw data:");
        for(i=0; i<msgRef->sms_data_len; i++)
        {
            LOG_ARG("%.2X ", msgRef->sms_data[i]);
        }
        
        LOG_ARG("\nAfter convert to UTF8, len=%d, data:", strlen(smsbuf));
        for(i=0; i<strlen(smsbuf); i++)
        {
            LOG_ARG("%.2X ", smsbuf[i]);
        }
        LOG("\n");
        free(smsbuf);
    }
    else if(msgRef->format == E_QL_SMS_FORMAT_BINARY_DATA)
    {
    	if (msgRef->user_data_head_valid)
    	{
        	LOG_ARG("###You've got one new BINARY msg from %s at %s , total_segments:%d, seg_number:%d, reference_number:%02x, len=%d, content=", 
							msgRef->src_addr,
							msgRef->timestamp,
							msgRef->user_data_head.total_segments,
							msgRef->user_data_head.seg_number,
							msgRef->user_data_head.reference_number,
							msgRef->sms_data_len); 
    	}
		else
		{
        	LOG_ARG("###You've got one new BINARY msg from %s at %s , len=%d, content=", 
							msgRef->src_addr,
							msgRef->timestamp,
							msgRef->sms_data_len); 
		}
        for(i=0; i<msgRef->sms_data_len; i++)
        {
            LOG_ARG("%.2X ", msgRef->sms_data[i]);
        }
        LOG("\n");
    }
    else //default is GSM-7
    {
    	if (msgRef->user_data_head_valid)
    	{
        	LOG_ARG("###You've got one new GSM-7 msg from %s at %s, total_segments:%d, seg_number:%d, reference_number:%02x, content=%s\n", 
		                    msgRef->src_addr,
		                    msgRef->timestamp,
							msgRef->user_data_head.total_segments,
		                    msgRef->user_data_head.seg_number,
					    	msgRef->user_data_head.reference_number,
		                    msgRef->sms_data);          
    	}
		else
		{
			LOG_ARG("###Joe lwm2mclient You've got one new GSM-7 msg from %s at %s, content=%s\n", 
		                    msgRef->src_addr,
		                    msgRef->timestamp,
		                    msgRef->sms_data);   
		}
        for(i=0; i<msgRef->sms_data_len; i++)
        {
            LOG_ARG("sms_data detail: %.2X ", msgRef->sms_data[i]);
        }
        
        LOG("\n\n");
    }
}

void quectel_handle_wap_push_sms(ql_wap_push_sms_info_t *wsp_info){

    lwm2m_client_handle_wap_sms(g_lwm2mH_ptr,wsp_info->push_content_ptr,wsp_info->push_content_len);

    //lwm2m_client_handle_wap_sms(lwm2m_ptr,wsp_info->push_content_ptr,wsp_info->push_content_len);

}

int quectel_sms_init(lwm2m_context_t * lwm2mH){
    int ret = E_QL_OK;
    lwm2m_ptr = lwm2mH;

//init client
    ret = QL_SMS_Client_Init(&h_sms);
    LOG_ARG("QL_SMS_Client_Init ret=%d with h_sms=0x%x\n", ret, h_sms);

    if(E_QL_OK != ret){
        LOG_ARG("sms client init failed, ret = %d\n", ret);
        return -1;// sms client init failed
    }

    QL_wap_push_SMS_AddRxMsgHandler(quectel_handle_wap_push_sms);

    #if 0
    ret = QL_SMS_AddRxMsgHandler(ql_sms_cb_func, (void*)h_sms);
    LOG_ARG("QL_SMS_AddRxMsgHandler ret=%d \n", ret);
    if(E_QL_OK != ret){
        LOG_ARG("sms client set ql_sms_cb_func failed, ret = %d\n", ret);
        return -1;// sms client init failed
    }
    #endif
}


int quectel_set_lwm2mcontxt_ptr(lwm2m_context_t * lwm2mH){
    lwm2m_ptr = lwm2mH;

}


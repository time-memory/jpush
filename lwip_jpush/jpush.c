#include <stdio.h>
#include "lwip/api.h"
#include "jpush.h"

extern int md5_32_encode(char* encrypt, char* out);

static int jpush_get_verification_code(JPUSH_MESSAGE_T* t_veri_info)
{
    char                to_code_str[STR_TO_MD5_MAX_LEN] = {0};
    char                c_reciever_type[RECVR_TYPE_MAX_LEN] = {0};

    sprintf(to_code_str, "%d", t_veri_info->sendno);
    sprintf(c_reciever_type, "%d", t_veri_info->receiver_type);
    strcat(to_code_str, c_reciever_type);

    strcat(to_code_str, t_veri_info->receiver_value);
    strcat(to_code_str, MASTER_SECRET);

    md5_32_encode(to_code_str,t_veri_info->verification_code);

    return RET_OK;
}

static int jpush_msg_content_process(MESSAGE_CONTENT_T* t_msg_content, char* s_content)
{
    char    n_builder_id[PARAM_NAME_MAX_LEN + 2] = {0};
    char    n_title[MSG_TITLE_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};

    if(NULL == t_msg_content || NULL == s_content)
    {
        printf("Error: param error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }

    memset(s_content, 0, MSG_CONTENT_MAX_LEN);
    sprintf(s_content,"msg_content={\"n_content\":\"%s\"",t_msg_content->n_content);

    if(PARAM_NOT_USE != t_msg_content->n_builder_id)
    {
        strcat(s_content,",");
        sprintf(n_builder_id,"\"n_builder_id\":\"%d\"",t_msg_content->n_builder_id);
        strcat(s_content,n_builder_id);
    }

    if(strcmp(t_msg_content->n_title,"\0") != 0)
    {
        strcat(s_content,",");
        sprintf(n_title,"\"n_title\":\"%s\"",t_msg_content->n_title);
        strcat(s_content,n_title);
    }

    if(NULL != t_msg_content->n_extras)
    {
        strcat(s_content,",");
        strcat(s_content,t_msg_content->n_extras);
    }

    strcat(s_content,"}");

    return RET_OK;
}

static int jpush_pack_post_data(JPUSH_MESSAGE_T*    t_jpush_msg, char* packed_msg)
{
    char                s_sendno[SEND_NO_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
    char                s_app_key[APP_KEY_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
    char                s_receiver_type[RECVR_TYPE_MAX_LEN + PARAM_NAME_MAX_LEN] ={0};
    char                s_verification_code[VERIFI_CODE_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
    char                s_msg_type[MSG_TYPE_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
    char                s_time_to_live[TIME_TO_LIVE_MAX_LEN + PARAM_NAME_MAX_LEN]= {0};
    char                s_override_msg_id[OVERRIDE_MSG_ID_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
    char                s_apns_production[APNS_PRODUCTION_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
    char                s_msg_content[MSG_CONTENT_MAX_LEN] = {0};
    int                 ret_val = RET_OK;

    if(NULL == t_jpush_msg || NULL == packed_msg)
    {
        printf("Error: param error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }

    memset(packed_msg,0,PACKED_MSG_MAX_LEN);

    /* sendno */
    sprintf(s_sendno,"sendno=%d",t_jpush_msg->sendno);
    strcat(packed_msg,s_sendno);

    /* app_key */
    sprintf(s_app_key,"&app_key=%s",t_jpush_msg->app_key);
    strcat(packed_msg,s_app_key);

    /* receiver_type */
    sprintf(s_receiver_type,"&receiver_type=%d",t_jpush_msg->receiver_type);
    strcat(packed_msg,s_receiver_type);

    /* receiver_value */
    if(strcmp(t_jpush_msg->receiver_value,"\0") != 0)
    {
        strcat(packed_msg,"&receiver_value=");
        strcat(packed_msg,t_jpush_msg->receiver_value);
    }

    /* rerification_code */
    jpush_get_verification_code(t_jpush_msg);
    sprintf(s_verification_code,"&verification_code=%s",t_jpush_msg->verification_code);
    strcat(packed_msg,s_verification_code);

    /* msg_type */
    sprintf(s_msg_type,"&msg_type=%d",t_jpush_msg->msg_type);
    strcat(packed_msg,s_msg_type);

    /* platform */
    strcat(packed_msg,"&platform=");
    strcat(packed_msg,t_jpush_msg->platform);

    /* apons_production */
    if (t_jpush_msg->apns_production > NOT_USE_APNS)
    {
        sprintf(s_apns_production,"&apns_production=%d",t_jpush_msg->apns_production - 1);
        strcat(packed_msg,s_apns_production);
    }

    /* time_to_live */
    if (TIME_TO_LIVE_DEFAULT != t_jpush_msg->time_to_live)
    {
        sprintf(s_time_to_live,"&time_to_live=%d",t_jpush_msg->time_to_live);
        strcat(packed_msg,s_time_to_live);
    }

    /* override_msg_id */
    if (strcmp(t_jpush_msg->override_msg_id,"\0") != 0)
    {
        sprintf(s_override_msg_id,"&override_msg_id=%s",t_jpush_msg->override_msg_id);
        strcat(packed_msg,s_override_msg_id);
    }

    /* msg_content */
    ret_val = jpush_msg_content_process(&t_jpush_msg->msg_content,s_msg_content);
    if(RET_OK != ret_val)
    {
        printf("msg content process failed! (%s, %d) \n",__FUNCTION__,__LINE__);
        return ret_val;
    }

    strcat(packed_msg,"&");
    strcat(packed_msg,s_msg_content);

    printf("%s\n",packed_msg);

    return RET_OK;
}

static int jpush_get_platform(int platform_val, char* s_platform)
{
    if(NULL == s_platform)
    {
        printf("Error: param error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }

    switch(platform_val)
    {
        case 0x0:
            memset(s_platform,0,sizeof(PLATFORM_MAX_LEN));
            break;
        case 0x1:
            strcpy(s_platform,"android");
            break;
        case 0x2:
            strcpy(s_platform,"ios");
            break;
        case 0x3:
            strcpy(s_platform,"android,ios");
            break;
        case 0x4:
            strcpy(s_platform,"winphone");
            break;
        case 0x5:
            strcpy(s_platform,"android,winphone");
            break;
        case 0x6:
            strcpy(s_platform,"ios,winphone");
            break;
        case 0x7:
            strcpy(s_platform,"android,ios,winphone");
            break;

        default:
            memset(s_platform,0,sizeof(PLATFORM_MAX_LEN));
            break;
    }

    return RET_OK;
}

static int jpush_user_param_process(JPUSH_PARAM_T* t_push_param,JPUSH_MESSAGE_T*    t_jpush_msg)
{
    if (NULL == t_push_param || NULL == t_jpush_msg)
    {
        printf("Error: param error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }

    /* send_no */
    if (t_push_param->user_send_no <= SEND_NO_VALUE_MIN || t_push_param->user_send_no > SEND_NO_VALUE_MAX )
    {
        printf("Error: send_no error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }
    t_jpush_msg->sendno = t_push_param->user_send_no;

    /* app_key */
    strcpy(t_jpush_msg->app_key, APP_KEY);

    /* receiver_type */
    if (t_push_param->user_recvr_type < TAG_SPECIFY || t_push_param->user_recvr_type > REGISTER_ID_BASED )
    {
        printf("Error: receiver_type error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }
    t_jpush_msg->receiver_type = t_push_param->user_recvr_type;

    /* receiver_value */
    if (t_push_param->user_recvr_type != BROADCAST && strcmp(t_push_param->user_recvr_value,"\0") == 0)
    {
        printf("Error: receiver_value error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }
    strcpy(t_jpush_msg->receiver_value, t_push_param->user_recvr_value);

    /* msg_type */
    if (t_push_param->user_msg_type < NOTICE || t_push_param->user_msg_type > CUSTOM)
    {
        printf("Error: msg_type error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }
    t_jpush_msg->msg_type = t_push_param->user_msg_type;

    /* platform */
    if (t_push_param->user_platform < 0x0 || t_push_param->user_platform > 0x7 )
    {
        printf("Error: platform error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }
    jpush_get_platform(t_push_param->user_platform, t_jpush_msg->platform);

    /* apns_production */
    if (t_push_param->user_apns_production < NOT_USE_APNS || t_push_param->user_apns_production > PRODUCTION)
    {
        printf("Error: apns error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }
    t_jpush_msg->apns_production = t_push_param->user_apns_production;

    /* time_to_live */
    if (t_push_param->user_time_to_live < TIME_TO_LIVE_DEFAULT || t_push_param->user_time_to_live > TIME_TO_LIVE_MAX)
    {
        printf("Error: time_to_live error! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }
    t_jpush_msg->time_to_live = t_push_param->user_time_to_live;

    /* override_msg_id */
    strcpy(t_jpush_msg->override_msg_id, t_push_param->user_override_msg_id);

    /* msg_content */
    t_jpush_msg->msg_content = t_push_param->user_msg_content;

    return RET_OK;
}

static int jpush_create_http_request(JPUSH_MESSAGE_T* t_jpush_msg, char* requst_string)
{
    char    post_param[PACKED_MSG_MAX_LEN] = {0};
    char    param_lenth[10] = {0};
    int     len = 0;
    int     ret_val = RET_OK;

    ret_val = jpush_pack_post_data(t_jpush_msg, post_param);
    if(RET_OK != ret_val)
    {
        printf("create http request failed! (%s, %d) \n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }

    memset(requst_string, 0, HTTP_REQUEST_STR_MAX_LEN);
    strcat(requst_string, "POST /v2/push HTTP/1.1\n");
    strcat(requst_string, "Host: api.jpush.cn\n");
    strcat(requst_string, "Content-Type: application/x-www-form-urlencoded\n");
    strcat(requst_string, "Content-Length: ");

    len = strlen(post_param);
    sprintf(param_lenth,"%d",len);

    strcat(requst_string, param_lenth);
    strcat(requst_string, "\n\n");
    strcat(requst_string, post_param);
    strcat(requst_string, "\r\n\r\n");

    return RET_OK;
}

static int jpush_send_http_request(struct netconn *p_connect, void* request_str, char* p_recv_data)
{
    err_t           err;
    struct          netbuf *recvbuf;
    struct          pbuf *q;
    int             data_len = 0;

    netconn_write(p_connect,request_str,sizeof(request_str),NETCONN_NOCOPY);

    err = netconn_recv(p_connect,&recvbuf);

    if (ERR_OK == err)
    {
        memset(p_recv_data,0,TCP_CLIENT_RX_BUFSIZE);

        for(q=recvbuf->p; q!=NULL; q=q->next)
        {
            if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len))
            {
                memcpy(p_recv_data+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));
            }
            else
            {
                memcpy(p_recv_data+data_len,q->payload,q->len);
            }

            data_len += q->len;
            if(data_len > TCP_CLIENT_RX_BUFSIZE)
            {
                break;
            }
        }

        data_len=0;
        netbuf_delete(recvbuf);
    }

    netconn_close(p_connect);
    netconn_delete(p_connect);

    return RET_OK;
}

static int jpush_process_return_data(char* p_recv_data)
{
    printf("%s \n",p_recv_data);

    return RET_OK;
}

int jpush_lwip_push_message(JPUSH_PARAM_T* t_push_param)
{
    struct netconn      *p_net_conn = NULL;
    struct ip_addr      server_ip;
    err_t               err_no;

    JPUSH_MESSAGE_T     t_jpush_msg;
    char                request_string[HTTP_REQUEST_STR_MAX_LEN] = {0};
    char                recv_data[TCP_CLIENT_RX_BUFSIZE];

    p_net_conn = netconn_new(NETCONN_TCP);
    if(NULL == p_net_conn)
    {
        printf("start new connect failed! (%s, %d)\n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }

    server_ip.addr = htonl(inet_addr(IPSTR));
    err_no = netconn_connect(p_net_conn, &server_ip, PORT);
    if(err_no != ERR_OK)
    {
        netconn_delete(p_net_conn);
        printf("netconn connect failed! (%s, %d)\n",__FUNCTION__,__LINE__);
        return RET_FAIL;
    }

    memset(&t_jpush_msg, 0, sizeof(JPUSH_MESSAGE_T));
    jpush_user_param_process(t_push_param,&t_jpush_msg);

    jpush_create_http_request(&t_jpush_msg,request_string);

    jpush_send_http_request(p_net_conn, (void*)request_string, recv_data);

    jpush_process_return_data(recv_data);

    return RET_OK;
}

int test_main(void)
{
    JPUSH_PARAM_T           t_push_param;
    MESSAGE_CONTENT_T       msg_content;

    memset(&t_push_param,0,sizeof(JPUSH_PARAM_T));
    memset(&msg_content,0,sizeof(MESSAGE_CONTENT_T));

    msg_content.n_builder_id = 2;
    strcpy(msg_content.n_content,"hello, world!");
    strcpy(msg_content.n_title,"test");
    msg_content.n_extras = NULL;

    t_push_param.user_send_no = 12345;
    t_push_param.user_msg_type = NOTICE;
    t_push_param.user_platform = PLATFORM_ANDROID | PLATFORM_IOS;
    t_push_param.user_recvr_type = BROADCAST;
    t_push_param.user_msg_content = msg_content;

    jpush_lwip_push_message(&t_push_param);

    return 0;
}

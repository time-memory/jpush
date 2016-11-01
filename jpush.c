#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "jpush.h"

#define IPSTR "121.46.20.48"
#define PORT 8800
#define BUFSIZE 1024

#define APP_KEY				"0b19ef9fac86ab297e8966db"
#define MASTER_SECRET		"2343b7ed439484be315e339f"

extern int md5_32_encode(char* encrypt, char* out);

static int jpush_get_verification_code(JPUSH_MESSAGE_T* t_veri_info)
{
	char	info_str[512] = {0};
	char	s_sendno[20] = {0};
	char	c_reciever_type[2] = {0};

	sprintf(info_str,"%d",t_veri_info->sendno);

	sprintf(c_reciever_type,"%d",t_veri_info->receiver_type);
	strcat(info_str,c_reciever_type);

	strcat(info_str,t_veri_info->receiver_value);

	strcat(info_str,MASTER_SECRET);

	printf("vertify_info: %s\n", info_str);

	md5_32_encode(info_str,t_veri_info->verification_code);

	printf("verify_code: %s\n",t_veri_info->verification_code);

	return 0;
}

int jpush_msg_content_process(MESSAGE_CONTENT_T*	t_msg_content, char* s_content)
{
	char	n_builder_id[PARAM_NAME_MAX_LEN + 2] = {0};
	char	n_title[MSG_TITLE_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};

	if(NULL == t_msg_content || NULL == s_content)
	{
		printf("Error: param error!\n");
		return FAIL;
	}

	memset(s_content,0,MSG_CONTENT_MAX_LEN);
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

	return OK;
}

int jpush_package_send_info(JPUSH_MESSAGE_T*	t_jpush_msg, char* packed_msg)
{
	char					s_sendno[SEND_NO_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
	char					s_app_key[APP_KEY_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
	char					s_receiver_type[RECVR_TYPE_MAX_LEN + PARAM_NAME_MAX_LEN] ={0};
	char					s_verification_code[VERIFI_CODE_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
	char					s_msg_type[MSG_TYPE_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
	char					s_time_to_live[TIME_TO_LIVE_MAX_LEN + PARAM_NAME_MAX_LEN]= {0};
	char					s_override_msg_id[OVERRIDE_MSG_ID_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
	char					s_apns_production[APNS_PRODUCTION_MAX_LEN + PARAM_NAME_MAX_LEN] = {0};
	char					s_msg_content[MSG_CONTENT_MAX_LEN] = {0};

	if(NULL == t_jpush_msg || NULL == packed_msg)
	{
		printf("Error: param error!\n");
		return FAIL;
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
	jpush_msg_content_process(&t_jpush_msg->msg_content,s_msg_content);
	strcat(packed_msg,"&");
	strcat(packed_msg,s_msg_content);

	printf("%s\n",packed_msg);

	return OK;
}

int jpush_get_platform(int	platform_val, char* s_platform)
{
	if(NULL == s_platform)
	{
		printf("Error: param error!\n");
		return FAIL;
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

	return OK;
}

static int jpush_user_param_process(JPUSH_PARAM_T* t_push_param,JPUSH_MESSAGE_T*	t_jpush_msg)
{
	if (NULL == t_push_param || NULL == t_jpush_msg)
	{
		printf("Error: param error!\n");
		return FAIL;
	}

	/* send_no */
	if (t_push_param->user_send_no <= SEND_NO_VALUE_MIN || t_push_param->user_send_no > SEND_NO_VALUE_MAX )
	{
		printf("Error: send_no error!\n");
		return FAIL;
	}
	t_jpush_msg->sendno = t_push_param->user_send_no;

	/* app_key */
	strcpy(t_jpush_msg->app_key, APP_KEY);

	/* receiver_type */
	if (t_push_param->user_recvr_type < TAG_SPECIFY || t_push_param->user_recvr_type > REGISTER_ID_BASED )
	{
		printf("Error: receiver_type error!\n");
		return FAIL;
	}
	t_jpush_msg->receiver_type = t_push_param->user_recvr_type;

	/* receiver_value */
	if (t_push_param->user_recvr_type != BROADCAST && strcmp(t_push_param->user_recvr_value,"\0") == 0)
	{
		printf("Error: receiver_value error!\n");
		return FAIL;
	}
	strcpy(t_jpush_msg->receiver_value, t_push_param->user_recvr_value);

	/* msg_type */
	if (t_push_param->user_msg_type < NOTICE || t_push_param->user_msg_type > CUSTOM)
	{
		printf("Error: msg_type error!\n");
		return FAIL;
	}
	t_jpush_msg->msg_type = t_push_param->user_msg_type;

	/* platform */
	if (t_push_param->user_platform < 0x0 || t_push_param->user_platform > 0x7 )
	{
		printf("Error: platform error!\n");
		return FAIL;
	}
	jpush_get_platform(t_push_param->user_platform, t_jpush_msg->platform);

	/* apns_production */
	if (t_push_param->user_apns_production < NOT_USE_APNS || t_push_param->user_apns_production > PRODUCTION)
	{
		printf("Error: apns error!\n");
		return FAIL;
	}
	t_jpush_msg->apns_production = t_push_param->user_apns_production;

	/* time_to_live */
	if (t_push_param->user_time_to_live < TIME_TO_LIVE_DEFAULT || t_push_param->user_time_to_live > TIME_TO_LIVE_MAX)
	{
		printf("Error: time_to_live error!\n");
		return FAIL;
	}
	t_jpush_msg->time_to_live = t_push_param->user_time_to_live;

	/* override_msg_id */
	strcpy(t_jpush_msg->override_msg_id, t_push_param->user_override_msg_id);

	/* msg_content */
	t_jpush_msg->msg_content = t_push_param->user_msg_content;

	return OK;
}

static int jpush_create_http_request(JPUSH_MESSAGE_T* t_jpush_msg, char* requst_string)
{
	char 	post_param[2048] = {0};
	char	param_lenth[10] = {0};
	int 	len;
	int 	reval = OK;

	reval = jpush_package_send_info(t_jpush_msg, post_param);

    memset(requst_string, 0, 4096);
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

	return OK;
}

int jpush_push_message(JPUSH_PARAM_T* t_push_param)
{
	int 				sockfd, ret, i, h;
	struct sockaddr_in 	servaddr;
	char 				buf[BUFSIZE];
	socklen_t 			len;
	fd_set	 			t_set1;
	struct timeval		tv;

	JPUSH_MESSAGE_T		t_jpush_msg;
	char				request_string[4096] = {0};

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
        printf("create socket failed ---socket error!\n");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IPSTR, &servaddr.sin_addr) <= 0 )
	{
        printf("create connect failed--inet_pton error!\n");
        exit(0);
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
        printf("connect failed,connect error!\n");
        exit(0);
    }

	memset(&t_jpush_msg,0,sizeof(JPUSH_MESSAGE_T));
	jpush_user_param_process(t_push_param,&t_jpush_msg);

	jpush_create_http_request(&t_jpush_msg,request_string);

	ret = write(sockfd,request_string,strlen(request_string));
	if (ret < 0) {
			printf("send failed ,%d, %s\n",errno, strerror(errno));
			exit(0);
	}else{
			printf("send succeed, %d world send!\n\n", ret);
	}

	FD_ZERO(&t_set1);
	FD_SET(sockfd, &t_set1);

	while(1){
			sleep(2);
			tv.tv_sec= 0;
			tv.tv_usec= 0;
			h= 0;
			printf("--------------->1");
			h= select(sockfd +1, &t_set1, NULL, NULL, &tv);
			printf("--------------->2");

			//if (h == 0) continue;
			if (h < 0) {
					close(sockfd);
					printf(" read failed \n");
					return -1;
			};

			if (h > 0){
					memset(buf, 0, 4096);
					i= read(sockfd, buf, 4095);
					if (i==0){
							close(sockfd);
							printf("remote closed \n");
							return -1;
					}

					printf("%s\n", buf);
			}
	}

	close(sockfd);

	return 0;
}


int main(int argc, char **argv)
{
	JPUSH_PARAM_T 			t_push_param;
	MESSAGE_CONTENT_T		msg_content;

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

	jpush_push_message(&t_push_param);

	return 0;
}

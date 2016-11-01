#ifndef _JPUSH_H_
#define _JPUSH_H_

#define SEND_NO_MAX_LEN				10
#define	APP_KEY_MAX_LEN				25
#define RECVR_VAL_MAX_LEN			40
#define VERIFI_CODE_MAX_LEN			33
#define RECVR_TYPE_MAX_LEN			2
#define MSG_TYPE_MAX_LEN			2
#define TIME_TO_LIVE_MAX_LEN		7
#define OVERRIDE_MSG_ID_MAX_LEN 	10
#define APNS_PRODUCTION_MAX_LEN		2
#define MSG_TITLE_MAX_LEN			20

#define	MSG_CONTENT_MAX_LEN			280
#define PLATFORM_MAX_LEN			30

#define	PARAM_NAME_MAX_LEN			20

#define PACKED_MSG_MAX_LEN			2048


#define PARAM_NOT_USE				-1

#define	PLATFORM_ANDROID			0x1
#define PLATFORM_IOS				0x1<<1
#define PLATFORM_WINDOWS_PHONE		0x1<<2


#define SEND_NO_VALUE_MIN			0
#define SEND_NO_VALUE_MAX			4294967295

#define TIME_TO_LIVE_MIN			0
#define TIME_TO_LIVE_MAX			864000
#define TIME_TO_LIVE_DEFAULT		-1

#define	OK							0
#define FAIL						-1


typedef enum _JPUSH_RECVR_TYPE_E
{
	TAG_SPECIFY = 2,
	ALIAS_SPECIFY,
	BROADCAST,
	REGISTER_ID_BASED
}JPUSH_RECVR_TYPE_E;

typedef enum _JPUSH_MSG_TYPE_E
{
	NOTICE = 1,
	CUSTOM
}JPUSH_MSG_TYPE_E;

typedef enum _JPUSH_APNS_ENV_TYPE_E
{
	NOT_USE_APNS = 0,
	DEVELOPING,
	PRODUCTION
}JPUSH_APNS_ENV_TYPE_E;

typedef struct _MESSAGE_CONTENT_T
{
	int						n_builder_id;
	char					n_title[MSG_TITLE_MAX_LEN];
	char					n_content[MSG_CONTENT_MAX_LEN];
	char*					n_extras;
}MESSAGE_CONTENT_T;

typedef struct _JPUSH_MESSAGE_T
{
	int 					sendno;
	char 					app_key[APP_KEY_MAX_LEN];
	JPUSH_RECVR_TYPE_E		receiver_type;
	char					receiver_value[RECVR_VAL_MAX_LEN];
	char					verification_code[VERIFI_CODE_MAX_LEN];
	JPUSH_MSG_TYPE_E		msg_type;
	MESSAGE_CONTENT_T		msg_content;
	char					platform[PLATFORM_MAX_LEN];
	JPUSH_APNS_ENV_TYPE_E	apns_production;
	int						time_to_live;
	char					override_msg_id[OVERRIDE_MSG_ID_MAX_LEN];
}JPUSH_MESSAGE_T;

typedef struct _JPUSH_PARAM_T
{
	int						user_send_no;
	JPUSH_RECVR_TYPE_E		user_recvr_type;
	char					user_recvr_value[RECVR_VAL_MAX_LEN];
	JPUSH_MSG_TYPE_E		user_msg_type;
	MESSAGE_CONTENT_T		user_msg_content;
	int						user_platform;
	JPUSH_APNS_ENV_TYPE_E	user_apns_production;
	int						user_time_to_live;
	char					user_override_msg_id[OVERRIDE_MSG_ID_MAX_LEN];
}JPUSH_PARAM_T;

#endif

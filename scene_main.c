#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include <sys/syscall.h>

#include <pthread.h>
#include "log_debug.h"
#include "scene_main.h"

#define gettid() syscall(SYS_gettid)

bool g_foreground = 0;

struct edge_usr_db *g_edge = NULL;

int g_icam = 0;
int g_tempValue = 0;
int g_relayAlarm = 0;

typedef struct
{
    char open[512];
    char close[512];
} RelayStatus;

typedef struct
{
    char devType[64];
    char alarmType[64];
    int alarmValue;
    int sceneId;
} SolinAlarm;

struct event_base *g_base = NULL;

struct edge_usr_db *usr_db_init(void)
{
    struct edge_usr_db *context = (struct edge_usr_db *)malloc(sizeof(struct edge_usr_db));
    if (context == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "Context Init");
        return (void *)0;
    }

    if (get_mac("eth1", context->localmac) < 0)
    {
        if (get_mac("eth0", context->localmac) < 0)
        {
            LOG_TH(LEVEL3_ERROR, "get eth0 mac addr failed");
            return (void *)0;
        }
    }

    context->cconline = 0;
    memset(context->platform_time, 0x00, sizeof(context->platform_time));
    macToName(context->localmac, context->localmacStr);

    return context;
}

/*****************************************************************
 * Function: Read the configuration file,
 *           save the scene in the configuration file,
 *           and write the scene configuration into an array
 */
int read_file_scene(char *name, char *buff, int i, struct edge_usr_db *thizz)
{
    char *pOption = NULL;
    char temp[64] = {0};
    char *pEnd = NULL;
    char sceneType[64] = {0};
    int cameraNum = 0, sceneId = 0, c = 0;

    int len = strlen(buff);

    if (!buff || !name)
        return -1;

    if (len <= 1 || strlen(name) <= 0)
        return -1;

    printf("%d  %s#######\n", i, name);

    if ((pOption = strstr(buff, "sceneId=")))
    {
        pOption += strlen("sceneId=");
        pEnd = strchr(pOption, '\n');
        if (pEnd)
        {
            sceneId = atoi(pOption);
        }
        thizz->scene[i].sceneId = sceneId;
        printf("sceneId=%d\n", thizz->scene[i].sceneId = sceneId);
    }

    if ((pOption = strstr(buff, "sceneType=")))
    {
        pOption += strlen("sceneType=");
        pEnd = strchr(pOption, '\n');
        if (pEnd)
            memcpy(sceneType, pOption, pEnd - pOption);
        printf("sceneType=%s\n", sceneType);
    }

    if (!strcmp(sceneType, "waterloggingMonit"))
    {
        if ((pOption = strstr(buff, "cameraNum=")))
        {
            pOption += strlen("cameraNum=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                cameraNum = atoi(pOption);
            g_icam = cameraNum;
            printf("cameraNum=%d\n", cameraNum);
        }

        if ((pOption = strstr(buff, "levelNumber=")))
        {
            pOption += strlen("levelNumber=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].levelNumber, pOption, pEnd - pOption);
            printf("levelNumber=%s\n", thizz->scene[i].levelNumber);
        }

        for (c = 0; c < cameraNum; c++)
        {
            sprintf(temp, "cameraIp%d=", c);
            if ((pOption = strstr(buff, temp)))
            {
                pOption += strlen(temp);
                pEnd = strchr(pOption, '\n');
                if (pEnd)
                    memcpy(thizz->scene[i].camera[c].cameraIp, pOption, pEnd - pOption);
                printf("cameraIp=%s\n", thizz->scene[i].camera[c].cameraIp);
            }

            sprintf(temp, "userName%d=", c);
            if ((pOption = strstr(buff, temp)))
            {
                pOption += strlen(temp);
                pEnd = strchr(pOption, '\n');
                if (pEnd)
                    memcpy(thizz->scene[i].camera[c].userName, pOption, pEnd - pOption);
                printf("userName=%s\n", thizz->scene[i].camera[c].userName);
            }

            sprintf(temp, "userPwd%d=", c);
            if ((pOption = strstr(buff, temp)))
            {
                pOption += strlen(temp);
                pEnd = strchr(pOption, '\n');
                if (pEnd)
                    memcpy(thizz->scene[i].camera[c].userPwd, pOption, pEnd - pOption);
                printf("userPwd=%s\n", thizz->scene[i].camera[c].userPwd);
            }

            sprintf(temp, "preset%d=", c);
            if ((pOption = strstr(buff, temp)))
            {
                pOption += strlen(temp);
                pEnd = strchr(pOption, '\n');
                if (pEnd)
                    thizz->scene[i].camera[c].preset = atoi(pOption);
                printf("preset=%d\n", thizz->scene[i].camera[c].preset);
            }
        }
    }

    else if (!strcmp(sceneType, "smartSecurity"))
    {
#if 0
        if ((pOption = strstr(buff, "cameraIp=")))
        {
            pOption += strlen("cameraIp=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].camera[0].cameraIp, pOption, pEnd - pOption);
            printf("cameraIp=%s\n", thizz->scene[i].camera[0].cameraIp);
        }

        if ((pOption = strstr(buff, "userName=")))
        {
            pOption += strlen("userName=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].camera[0].userName, pOption, pEnd - pOption);
            printf("userName=%s\n", thizz->scene[i].camera[0].userName);
        }

        if ((pOption = strstr(buff, "userPwd=")))
        {
            pOption += strlen("userPwd=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].camera[0].userPwd, pOption, pEnd - pOption);
            printf("userPwd=%s\n", thizz->scene[i].camera[0].userPwd);
        }
        
        if ((pOption = strstr(buff, "securityType=")))
        {
            pOption += strlen("securityType=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].camera[0].secutityType, pOption, pEnd - pOption);
            printf("securityType=%s\n", thizz->scene[i].camera[0].secutityType);
        }
#endif
        if ((pOption = strstr(buff, "ipAddr=")))
        {
            pOption += strlen("ipAddr=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].ipAddr, pOption, pEnd - pOption);
            printf("ipAddr=%s\n", thizz->scene[i].ipAddr);
        }

        if ((pOption = strstr(buff, "username=")))
        {
            pOption += strlen("username=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].username, pOption, pEnd - pOption);
            printf("username=%s\n", thizz->scene[i].username);
        }

        if ((pOption = strstr(buff, "userpwd=")))
        {
            pOption += strlen("userpwd=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].userPwd, pOption, pEnd - pOption);
            printf("userpwd=%s\n", thizz->scene[i].userPwd);
        }

        if ((pOption = strstr(buff, "smartSecurityType=")))
        {
            pOption += strlen("smartSecurityType=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].smartSecurityType, pOption, pEnd - pOption);
            printf("smartSecurityType=%s\n", thizz->scene[i].smartSecurityType);
        }

        if ((pOption = strstr(buff, "screenNumber=")))
        {
            pOption += strlen("screenNumber=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].screenNumber, pOption, pEnd - pOption);
            printf("screenNumber=%s\n", thizz->scene[i].screenNumber);
        }

        if ((pOption = strstr(buff, "relayId=")))
        {
            pOption += strlen("relayId=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                thizz->scene[i].relayId = atoi(pOption);
            printf("relayId=%d\n", thizz->scene[i].relayId);
        }

        if ((pOption = strstr(buff, "relayName=")))
        {
            pOption += strlen("relayName=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                memcpy(thizz->scene[i].relayName, pOption, pEnd - pOption);
            printf("relayName=%s\n", thizz->scene[i].relayName);
        }

        if ((pOption = strstr(buff, "channelNumber=")))
        {
            pOption += strlen("channelNumber=");
            pEnd = strchr(pOption, '\n');
            if (pEnd)
                thizz->scene[i].channelNumber = atoi(pOption);
            printf("channelNumber=%d\n", thizz->scene[i].channelNumber);
        }
    }

    return 0;
}

void load_file_scene(struct edge_usr_db *thizz)
{
    int i = 0;

    FILE *fp = fopen(DEV_SCENE_CONF, "r");
    if (NULL == fp)
    {
        LOG_TH(LEVEL3_ERROR, "config file %s fopen r failed\n", DEV_SCENE_CONF);
        return;
    }
    fseek(fp, 0, SEEK_END);

    int size = ftell(fp);

    printf("file(%s) size=%d\n", DEV_SCENE_CONF, size);
    rewind(fp);

    if (size <= 0)
    {
        LOG_TH(LEVEL3_ERROR, "file(%s) size=%d failed\n", DEV_SCENE_CONF, size);
        return;
    }

    char *ar = (char *)calloc(size + 1, sizeof(char));

    if (ar == NULL)
    {
        return;
    }

    fread(ar, 1, size, fp);

    char *pStart = ar;
    char *pEnd = pStart + size;
    char name[64] = {0};

    pStart = strchr(pStart, '[');
    if (NULL == pStart)
    {
        fclose(fp);
        free(ar);
        return;
    }

    while (1)
    {
        memset(name, 0x00, sizeof(name));

        char *pNameEnd = strchr(pStart, ']');
        if (NULL == pNameEnd)
        {
            break;
        }

        pStart++;
        memcpy(name, pStart, pNameEnd - pStart);
        pNameEnd++;

        char *pContainerEnd = strchr(pNameEnd, '[');
        if (NULL == pContainerEnd)
        {
            pContainerEnd = pEnd;
        }

        char buff[1024] = {0};

        memcpy(buff, pNameEnd, pContainerEnd - pNameEnd);
        // printf("container buff=%s\n", buff);
        read_file_scene(name, buff, i, thizz);
        i++;
        if (pContainerEnd == pEnd)
            break;
        pStart = pContainerEnd;
    }

    fclose(fp);
    free(ar);

    return;
}

/*****************************************************************
 * Delete scene from configuration file
 */
void del_scene(char *name)
{
    char token[256] = {0};
    printf("delete container(%s) from config file\n", name);

    FILE *fp = fopen(DEV_SCENE_CONF, "r");
    if (NULL == fp)
    {
        printf("config file %s fopen r failed\n", DEV_SCENE_CONF);
        return;
    }

    fseek(fp, 0, SEEK_END);

    int size = ftell(fp);

    printf("file(%s) size=%d\n", DEV_SCENE_CONF, size);
    rewind(fp);

    char *ar = (char *)calloc(size + 1, sizeof(char));
    if (ar == NULL)
    {
        printf("del scenen calloc failed\n");
        return;
    }

    fread(ar, 1, size, fp);

    snprintf(token, sizeof(token), "[%s]", name);

    char *pOption = strstr(ar, token);

    if (NULL == pOption)
    {
        fclose(fp);
        free(ar);
        return;
    }

    char *pEnd = strchr(pOption + 1, '[');
    if (pEnd)
    {
        while (*pEnd)
            *pOption++ = *pEnd++;
    }
    pOption[0] = 0;
    fclose(fp);

    fp = fopen(DEV_SCENE_CONF, "w");
    if (NULL == fp)
    {
        printf("config file %s fopen w failed\n", DEV_SCENE_CONF);
        return;
    }

    size = strlen(ar);
    if (size > 0)
        fwrite(ar, 1, size, fp);
    free(ar);
    fclose(fp);
    return;
}

/*****************************************************************
 * Function: Write scene configuration to file
 */
void add_scene_file(char *sceneType, int i, struct edge_usr_db *thizz)
{
    int c = 0;
    char buffer[1024] = {0};
    char *pData = buffer;

    if (NULL == thizz)
        return;

    FILE *fp = fopen(DEV_SCENE_CONF, "a"); /* created if it does not exist */
    if (NULL == fp)
    {
        LOG_TH(LEVEL3_ERROR, "config file %s fopen a failed\n", DEV_SCENE_CONF);
        return;
    }

    pData += sprintf(pData, "[%d]\n", thizz->scene[i].sceneId);
    pData += sprintf(pData, "sceneId=%d\n", thizz->scene[i].sceneId);

    if (!strcmp(sceneType, DEV_TYPE_LEVEL))
    {
        pData += sprintf(pData, "sceneType=%s\n", DEV_TYPE_LEVEL);
        pData += sprintf(pData, "cameraNum=%d\n", g_icam);
        pData += sprintf(pData, "levelNumber=%s\n", thizz->scene[i].levelNumber);
        for (c = 0; c < g_icam; c++)
        {
            pData += sprintf(pData, "cameraIp%d=%s\n", c, thizz->scene[i].camera[c].cameraIp);
            pData += sprintf(pData, "userName%d=%s\n", c, thizz->scene[i].camera[c].userName);
            pData += sprintf(pData, "userPwd%d=%s\n", c, thizz->scene[i].camera[c].userPwd);
            pData += sprintf(pData, "preset%d=%d\n", c, thizz->scene[i].camera[c].preset);
        }
    }
    else if (!strcmp(sceneType, DEV_TYPE_CAMERA))
    {
        pData += sprintf(pData, "sceneType=%s\n", DEV_TYPE_CAMERA);
        // pData += sprintf(pData, "cameraIp=%s\n", thizz->scene[i].camera[0].cameraIp);
        // pData += sprintf(pData, "userName=%s\n", thizz->scene[i].camera[0].userName);
        // pData += sprintf(pData, "userPwd=%s\n", thizz->scene[i].camera[0].userPwd);
        // pData += sprintf(pData, "securityType=%s\n", thizz->scene[i].camera[0].secutityType);
        pData += sprintf(pData, "ipAddr=%s\n", thizz->scene[i].ipAddr);
        pData += sprintf(pData, "username=%s\n", thizz->scene[i].username);
        pData += sprintf(pData, "userpwd=%s\n", thizz->scene[i].userPwd);
        pData += sprintf(pData, "smartSecurityType=%s\n", thizz->scene[i].smartSecurityType);
        pData += sprintf(pData, "screenNumber=%s\n", thizz->scene[i].screenNumber);
        pData += sprintf(pData, "relayId=%d\n", thizz->scene[i].relayId);
        pData += sprintf(pData, "relayName=%s\n", thizz->scene[i].relayName);
        pData += sprintf(pData, "channelNumber=%d\n", thizz->scene[i].channelNumber);
    }

    fprintf(fp, "%s", buffer);
    fflush(fp);
    fclose(fp);

    return;
}

/*****************************************************************
 * Function: Mqtt messages for the screen pub,
 *           Alarm is triggered when the alarmStatus value is 1,
 *           and disarmed when the value is 0
 */
void screen_json_msg_pub(int sId, char *devType, int alarmStatus, struct edge_usr_db *thizz)
{
    cJSON *root = NULL;
    char idStr[32] = {0};
    char sceneId[32] = {0};
    char *out;
    int iErr = -1, qos = 0;
    snprintf(idStr, sizeof(idStr), "%d", (int)time(NULL));
    snprintf(sceneId, sizeof(sceneId), "%d", thizz->scene[sId].sceneId);

    root = cJSON_CreateObject();
    if (root == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "screen cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON_AddStringToObject(root, "type", "setAlarmInfo");
    cJSON_AddStringToObject(root, "CCID", thizz->scene[sId].screenNumber);
    cJSON_AddStringToObject(root, "alarmScene", sceneId);
    cJSON_AddStringToObject(root, "concentratorSn", (const char *)thizz->localmacStr);
    cJSON_AddIntToObject(root, "alarmStatus", alarmStatus);
    cJSON_AddStringToObject(root, "ts", idStr);

    if (!strcmp(devType, "ipCamera"))
    {
        cJSON_AddIntToObject(root, "alarmType", 2);
    }
    else if (!strcmp(devType, "liquidlevelSensor"))
    {
        cJSON_AddIntToObject(root, "alarmType", 3);
    }

    out = cJSON_PrintUnformatted(root);
    snprintf(thizz->e_pub5_topic, sizeof(thizz->e_pub5_topic), "EDGE/DOWNLINK/ALARM/%s", thizz->scene[sId].screenNumber);

    if (g_foreground)
    {
        printf("pub_topic:%s\n", thizz->e_pub5_topic);
        printf("UpLinkMsg:%s\n", out);
    }

    iErr = mosq_usr_publish(thizz->mosq_edge_new, qos, thizz->e_pub5_topic, out, strlen(out));
    if (iErr != 0)
    {
        LOG_TH(LEVEL3_ERROR, "pub angle info  failed\n");
    }

    json_release(root);
    if (out)
    {
        free(out);
    }

    return;
}

void relay_ctrl_timer(int fd, short _event, void *arg)
{

    LOG_TH(LEVEL3_ERROR, "relay_ctrl_time 10 min end stop relay ctrl thread\n");
    g_relayAlarm = 0;
}

void *relay_ctrl_thread(void *arg)
{
    pthread_detach(pthread_self());
    int qos = 0;

    if (arg == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "relay thread arg is NULL fail\n");
        return (void *)0;
    }

    RelayStatus *p = (RelayStatus *)arg;

    LOG_TH(LEVEL3_ERROR, "Enter relay opera thread\n");
    printf("####open topic %s####\n", g_edge->e_pub6_topic);

    while (g_relayAlarm)
    {
        sleep(6);
        printf("open relay Tid %ld msg:%s\n", gettid(), p->open);
        printf("####open topic %s####\n", g_edge->e_pub6_topic);
        mosq_usr_publish(g_edge->mosq_edge_new, qos, g_edge->e_pub6_topic, p->open, strlen(p->open));
        sleep(6);
        printf("close relay Tid %ld msg:%s\n", gettid(), p->close);
        mosq_usr_publish(g_edge->mosq_edge_new, qos, g_edge->e_pub6_topic, p->close, strlen(p->close));
    }
    LOG_TH(LEVEL3_ERROR, "Exit relay opera thread\n");
    free(arg);

    return (void *)0;
}

/*****************************************************************
 * Function: Mqtt messages for the  relay pub
 */
void relay_json_msg_pub(int sId, struct edge_usr_db *thizz)
{
    cJSON *root = NULL, *out = NULL;
    char idStr[32] = {0};
    char *tempOpen;
    char *tempClose;
    int tidMark = -1;

    pthread_t tid1;
    printf("relay open#########\n");
    snprintf(idStr, sizeof(idStr), "%d", (int)time(NULL));

    root = cJSON_CreateObject();
    if (root == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "relay open cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON_AddStringToObject(root, "msgType", "relayCtrl");
    cJSON_AddStringToObject(root, "actionType", "open");
    cJSON_AddIntToObject(root, "termId", thizz->scene[sId].relayId);
    cJSON_AddStringToObject(root, "devType", thizz->scene[sId].relayName);
    cJSON_AddStringToObject(root, "ctrlType", "single");
    cJSON_AddIntToObject(root, "tunId", thizz->scene[sId].channelNumber);
    cJSON_AddStringToObject(root, "ts", idStr);
    tempOpen = cJSON_PrintUnformatted(root);

    printf("relay close##########\n");

    out = cJSON_CreateObject();
    if (out == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "relay close cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON_AddStringToObject(out, "msgType", "relayCtrl");
    cJSON_AddStringToObject(out, "actionType", "close");
    cJSON_AddIntToObject(out, "termId", thizz->scene[sId].relayId);
    cJSON_AddStringToObject(out, "devType", thizz->scene[sId].relayName);
    cJSON_AddStringToObject(out, "ctrlType", "single");
    cJSON_AddIntToObject(out, "tunId", thizz->scene[sId].channelNumber);
    cJSON_AddStringToObject(out, "ts", idStr);
    tempClose = cJSON_PrintUnformatted(out);

    RelayStatus *st = (RelayStatus *)malloc(sizeof(RelayStatus));
    strcpy(st->open, tempOpen);
    strcpy(st->close, tempClose);

    if (g_relayAlarm)
    {
        LOG_TH(LEVEL3_ERROR, "###recv alarm create relay thread and start relay timer###\n");
        tidMark = pthread_create(&tid1, NULL, relay_ctrl_thread, (void *)st);
        if (tidMark == 0)
        {
            LOG_TH(LEVEL3_ERROR, "relay thread create success\n");
            evtimer_set(&(g_edge->ev_ra), relay_ctrl_timer, (void *)g_edge);
            event_base_set(g_base, &(g_edge->ev_ra));
            struct timeval tvs;
            tvs.tv_sec = 600;
            tvs.tv_usec = 0;
            event_add(&(g_edge->ev_ra), &tvs);
        }
        else
        {
            LOG_TH(LEVEL3_ERROR, "relay thread create fail\n");
            json_release(root);
            if (tempOpen)
            {
                free(tempOpen);
            }

            json_release(out);
            if (tempClose)
            {
                free(tempClose);
            }
            return;
        }
    }

    if (g_foreground)
    {
        printf("pub_topic:%s\n", thizz->e_pub6_topic);
        printf("UpLinkMsgopen:%s\n", tempOpen);
        printf("UpLinkMsgopen:%s\n", tempClose);
    }

    json_release(root);
    if (tempOpen)
    {
        free(tempOpen);
    }

    json_release(out);
    if (tempClose)
    {
        free(tempClose);
    }

    return;
}

void *solin_alarm_thread(void *arg)
{
    pthread_detach(pthread_self());

    cJSON *root = NULL;
    char idStr[32] = {0};
    char *out;
    int qos = 0;

    if (arg == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "solin thread arg is NULL fail\n");
        return (void *)0;
    }

    SolinAlarm *p = (SolinAlarm *)arg;

    root = cJSON_CreateObject();
    if (root == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "solin thread cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return (void *)0;
    }

    cJSON_AddStringToObject(root, "msgType", "alarm");
    cJSON_AddIntToObject(root, "sceneId", g_edge->scene[p->sceneId].sceneId);
    cJSON_AddStringToObject(root, "concentratorSn", (const char *)g_edge->localmacStr);
    cJSON_AddIntToObject(root, "alarmStatus", p->alarmValue);
    cJSON_AddStringToObject(root, "ts", idStr);
    if (!strcmp(p->devType, "liquidlevelSensor"))
    {
        cJSON_AddStringToObject(root, "alarmCode", "140001");
    }
    else if (!strcmp(p->devType, "ipCamera"))
    {
        if (!strcmp(p->alarmType, "TamperDetector"))
        {
            cJSON_AddStringToObject(root, "alarmCode", "140004");
        }
        else if (!strcmp(p->alarmType, "MotionDetector"))
        {
            cJSON_AddStringToObject(root, "alarmCode", "140005");
        }
        else if (!strcmp(p->alarmType, "FieldDetector"))
        {
            cJSON_AddStringToObject(root, "alarmCode", "140002");
        }
        else if (!strcmp(p->alarmType, "LineDetector"))
        {
            cJSON_AddStringToObject(root, "alarmCode", "140003");
        }
    }

    out = cJSON_PrintUnformatted(root);
    printf("solin alarm Tid %ld msg:%s\n", gettid(), out);
    printf("####solin topic %s\n ####", g_edge->s_pub_topic);
    mosq_usr_publish(g_edge->mosq_solin_new, qos, g_edge->s_pub_topic, out, strlen(out));

    json_release(root);
    if (out)
    {
        free(out);
    }
    free(arg);

    return (void *)0;
}

/*****************************************************************
 * Function: Mqtt messages for the  solin pub
 */
void solin_json_msg_pub(char *devType, char *msgType, int alarmStatus, char *alarmType, char *sceneType, int sId, struct edge_usr_db *thizz)
{
    cJSON *root = NULL;
    char idStr[32];
    char *out;
    int iErr = -1, qos = 0;

    snprintf(idStr, sizeof(idStr), "%d", (int)time(NULL));

    root = cJSON_CreateObject();
    if (root == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "solin pud cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return;
    }

    if (!strcmp(msgType, "alarm"))
    {
        cJSON_AddStringToObject(root, "msgType", "alarm");
        // cJSON_AddStringToObject(root,"devType",devType);
        cJSON_AddIntToObject(root, "sceneId", thizz->scene[sId].sceneId);
        cJSON_AddStringToObject(root, "concentratorSn", (const char *)thizz->localmacStr);
        cJSON_AddIntToObject(root, "alarmStatus", alarmStatus);
        cJSON_AddStringToObject(root, "ts", idStr);
        if (!strcmp(devType, "liquidlevelSensor"))
        {
            cJSON_AddStringToObject(root, "alarmCode", "140001");
        }
        else if (!strcmp(devType, "ipCamera"))
        {
            if (!strcmp(alarmType, "TamperDetector"))
            {
                cJSON_AddStringToObject(root, "alarmCode", "140004");
            }
            else if (!strcmp(alarmType, "MotionDetector"))
            {
                cJSON_AddStringToObject(root, "alarmCode", "140005");
            }
            else if (!strcmp(alarmType, "FieldDetector"))
            {
                cJSON_AddStringToObject(root, "alarmCode", "140002");
            }
            else if (!strcmp(alarmType, "LineDetector"))
            {
                cJSON_AddStringToObject(root, "alarmCode", "140003");
            }
        }
    }
    else if (strcmp(msgType, "ccOnline") == 0)
    {
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "msgType", "ccOnline");
        cJSON_AddStringToObject(root, "concentratorSn", (const char *)thizz->localmacStr);
        cJSON_AddStringToObject(root, "errorCode", "0");
        cJSON_AddStringToObject(root, "ts", idStr);
    }
    else if (strcmp(msgType, "reportStatus") == 0)
    {
        printf("reportStatus#####\n");
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "msgType", "reportStatus");
        cJSON_AddStringToObject(root, "concentratorSn", (const char *)thizz->localmacStr);
        cJSON_AddStringToObject(root, "ts", idStr);
    }
    else if (strcmp(msgType, "sceneConfig") == 0)
    {
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "msgType", "sceneConfig");
        if (thizz->scene[sId].sceneId != 0)
        {
            cJSON_AddIntToObject(root, "sceneId", thizz->scene[sId].sceneId);
            cJSON_AddStringToObject(root, "errorCode", "0");
        }
        else
        {
            cJSON_AddStringToObject(root, "errorCode", "30020");
        }
        cJSON_AddStringToObject(root, "sceneType", sceneType);
        cJSON_AddStringToObject(root, "concentratorSn", (const char *)thizz->localmacStr);
        cJSON_AddStringToObject(root, "ts", idStr);
    }
    else if (strcmp(msgType, "sceneDel") == 0)
    {
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "msgType", "sceneDel");
        if (thizz->scene[sId].sceneId == 0)
        {
            cJSON_AddIntToObject(root, "sceneId", thizz->scene[sId].sceneId);
            cJSON_AddStringToObject(root, "errorCode", "0");
        }
        else
        {
            cJSON_AddStringToObject(root, "errorCode", "30020");
        }
        cJSON_AddStringToObject(root, "sceneType", sceneType);
        cJSON_AddStringToObject(root, "concentratorSn", (const char *)thizz->localmacStr);
        cJSON_AddStringToObject(root, "ts", idStr);
    }
    else if (strcmp(msgType, "sceneDebug") == 0)
    {
        root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "msgType", "sceneDebug");
        // cJSON_AddStringToObject(root,"sceneName",devType);
        cJSON_AddIntToObject(root, "sceneId", sId);
        cJSON_AddStringToObject(root, "sceneType", sceneType);
        cJSON_AddStringToObject(root, "errorCode", "0");
        cJSON_AddStringToObject(root, "concentratorSn", (const char *)thizz->localmacStr);
        cJSON_AddStringToObject(root, "ts", idStr);
    }

    out = cJSON_PrintUnformatted(root);

    if (g_foreground)
    {
        printf("s_pub_topic:%s\n", thizz->s_pub_topic);
        printf("UpLinkMsg:%s\n", out);
    }

    printf("####solin topic %s\n ####", thizz->s_pub_topic);

    iErr = mosq_usr_publish(thizz->mosq_solin_new, qos, thizz->s_pub_topic, out, strlen(out));
    if (iErr != 0)
    {
        LOG_TH(LEVEL3_ERROR, "pub angle info  failed\n");
    }

    json_release(root);
    if (out)
    {
        free(out);
    }

    return;
}

/*****************************************************************
 * Function: Level scene camera control MQTT message pub
 */
void level_json_msg_pub(int sId, struct edge_usr_db *thizz, int alarmValue)
{
    cJSON *root;
    int i = 0, preset = 0, iErr = -1, qos = 0;
    char *out;
    char idStr[32] = {0};
    char cameraIp[64] = {0}, userName[64] = {0}, userPwd[64] = {0};

    snprintf(idStr, sizeof(idStr), "%d", (int)time(NULL));

    for (i = 0; i < g_icam; i++)
    {
        root = cJSON_CreateObject();
        if (root == NULL)
        {
            LOG_TH(LEVEL3_ERROR, "level pub cjson parse failed[%s]\n", cJSON_GetErrorPtr());
            return;
        }

        strcpy(cameraIp, thizz->scene[sId].camera[i].cameraIp);
        strcpy(userName, thizz->scene[sId].camera[i].userName);
        strcpy(userPwd, thizz->scene[sId].camera[i].userPwd);
        preset = thizz->scene[sId].camera[i].preset;

        cJSON_AddStringToObject(root, "msgType", "cameraCtrl");
        cJSON_AddStringToObject(root, "sceneType", "liquidLevel");
        cJSON_AddStringToObject(root, "ipAddr", cameraIp);
        cJSON_AddStringToObject(root, "username", userName);
        cJSON_AddStringToObject(root, "userPwd", userPwd);

        if (alarmValue == 1)
        {
            cJSON_AddIntToObject(root, "preset", preset);
        }
        else
        {
            cJSON_AddIntToObject(root, "preset", 1);
        }
        cJSON_AddStringToObject(root, "ts", idStr);

        out = cJSON_PrintUnformatted(root);
        if (g_foreground)
        {
            printf("pub_topic:%s\n", thizz->e_pub4_topic);
            printf("UpLinkMsg:%s\n", out);
        }

        iErr = mosq_usr_publish(thizz->mosq_edge_new, qos, thizz->e_pub4_topic, out, strlen(out));
        if (iErr != 0)
        {
            LOG_TH(LEVEL3_ERROR, "pub angle info  failed\n");
        }

        json_release(root);
        if (out)
        {
            free(out);
        }
    }

    return;
}

/*****************************************************************
 * Function: Camera alarm, online message reply
 */
void secur_camera_json_pub(char *msgType, char *serialNumber, char *ipAddr, char *alarmType, int sId, struct edge_usr_db *thizz)
{
    cJSON *root;
    int iErr = -1, qos = 0;
    char *out;
    char idStr[32] = {0};
    snprintf(idStr, sizeof(idStr), "%d", (int)time(NULL));

    root = cJSON_CreateObject();
    if (root == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "secur camera pub cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return;
    }

    if (!strcmp(msgType, "cameraOline"))
    {
        cJSON_AddStringToObject(root, "msgType", "cameraOline");
        cJSON_AddStringToObject(root, "serialNumber", serialNumber);
        cJSON_AddStringToObject(root, "ipAddr", ipAddr);
        // cJSON_AddStringToObject(root, "username", thizz->scene[sId].camera[cId].userName);
        cJSON_AddStringToObject(root, "username", thizz->scene[sId].username);
        // cJSON_AddStringToObject(root, "userPwd", thizz->scene[sId].camera[cId].userPwd);
        cJSON_AddStringToObject(root, "userPwd", thizz->scene[sId].userPwd);
        cJSON_AddStringToObject(root, "ts", idStr);
        cJSON_AddStringToObject(root, "errorCode", "0");
    }
    else if (!strcmp(msgType, "cameraAlarm"))
    {
        cJSON_AddStringToObject(root, "msgType", "cameraAlarm");
        cJSON_AddStringToObject(root, "alarmType", alarmType);
        cJSON_AddStringToObject(root, "serialNumber", serialNumber);
        cJSON_AddStringToObject(root, "ipAddr", ipAddr);
        cJSON_AddStringToObject(root, "ts", idStr);
        cJSON_AddStringToObject(root, "errorCode", "0");
    }

    out = cJSON_PrintUnformatted(root);
    if (g_foreground)
    {
        printf("pub_topic:%s\n", thizz->e_pub3_topic);
        printf("UpLinkMsg:%s\n", out);
    }

    iErr = mosq_usr_publish(thizz->mosq_edge_new, qos, thizz->e_pub3_topic, out, strlen(out));
    if (iErr != 0)
    {
        LOG_TH(LEVEL3_ERROR, "pub angle info  failed\n");
    }

    json_release(root);
    if (out)
    {
        free(out);
    }

    return;
}

/*****************************************************************
 * Function: Solin MQTT message parsing, including scene config,
 *           scenario delete, and scene debug
 */
int solin_cjson_msg_parse(struct edge_usr_db *thizz, char *msg)
{
    cJSON *root = NULL, *device = NULL, *level = NULL, *screen = NULL;
    cJSON *arrayLevel = NULL, *cameraObject = NULL, *arrayScreen = NULL;
    cJSON *arrayCamera = NULL, *ipCamera = NULL, *relay = NULL, *arrayRelay = NULL;

    char msgType[32] = {0}, sceneName[32] = {0}, ccId[32] = {0};
    char sceneType[32] = {0}, relayName[32] = {0}, levelNumber[32], screenNumber[32];
    char cameraIp[5][32] = {0}, userName[5][32] = {0}, userPwd[5][32] = {0}, secutityType[5][32] = {0};
    char ipAddr[32] = {0}, userNameCamera[32] = {0}, userPwdCamera[32] = {0}, smartSecurityType[32] = {0};

    int preset[5] = {0};
    int sceneId = 0, relayId = 0, channelNumber = 0, sId = 0;
    int iSize = -1, iCnt = 0, i = 0, c = 0, alarmStatus = 1, alarmStatusDebug = 0;

    root = cJSON_Parse(msg);
    if (!root)
    {
        LOG_TH(LEVEL3_ERROR, "solin cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    else
    {
        if (json_get_string_object(root, "concentratorSn", ccId, sizeof(ccId)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "concentratorSn parse failed\n");
            json_release(root);
            return -1;
        } // 边缘网关序列号

        if (strncmp(thizz->localmacStr, ccId, strlen(thizz->localmacStr)))
        {
            LOG_TH(LEVEL3_ERROR, "LocalMac:%s & ccId:%s not match\n", thizz->localmacStr, ccId);
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "msgType", msgType, sizeof(msgType)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "msgType parse failed\n");
            json_release(root);
            return -1;
        } // 消息类型

        if (json_get_string_object(root, "ts", thizz->platform_time, sizeof(thizz->platform_time)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "ts parse failed\n");
            json_release(root);
            return -1;
        } // 时间戳

        if (!strcmp(msgType, "ccOnline"))
        {
            uint8_t flag = 1;
            if (flag)
            {
                thizz->cconline = 1;
                LOG_TH(LEVEL3_ERROR, "ccOline success \n");
            }
            json_release(root);
            return 0;
        }
#if 1
        if (!strcmp(msgType, "sceneDebug"))
        {
            if (json_get_string_object(root, "sceneType", sceneType, sizeof(sceneType)) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "sceneType parse failed\n");
                json_release(root);
                return -1;
            }

            /*
            if(json_get_string_object(root,"sceneName",sceneName,sizeof(sceneType))!=0)
            {
                LOG_TH(LEVEL3_ERROR, "sceneName parse failed\n");
                json_release(root);
                return -1;
             }
             */

            if (json_get_int_object(root, "sceneId", &sceneId) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "sceneId parse failed\n");
                json_release(root);
                return -1;
            }

            if (json_get_int_object(root, "alarmStatus", &alarmStatusDebug) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "alarmStatus parse failed\n");
                json_release(root);
                return -1;
            }

            for (i = 0; i < SCENE_MAX; i++)
            {
                if (thizz->scene[i].sceneId == sceneId)
                {
                    sId = i;
                    break;
                }
            }

            if (!strcmp(sceneType, DEV_TYPE_LEVEL))
            {
                if (strlen(thizz->scene[sId].levelNumber))
                {
                    if (alarmStatusDebug == 1)
                    {
                        level_json_msg_pub(sId, thizz, alarmStatusDebug);
                    }
                    else
                    {
                        level_json_msg_pub(sId, thizz, alarmStatusDebug);
                    }
                }
                else
                {
                    printf("Scenario operation error this scene not levelNumber\n");
                    return -1;
                }
                printf("%d####\n", thizz->scene[sId].sceneId);
                solin_json_msg_pub("liquidlevelSensor", "alarm", alarmStatusDebug, NULL, NULL, sId, thizz);
            }
            if (!strcmp(sceneType, DEV_TYPE_CAMERA))
            {
                if (strlen(thizz->scene[sId].ipAddr))
                {
                    if (alarmStatusDebug != g_relayAlarm)
                    {
                        g_relayAlarm = alarmStatusDebug;
                        if (alarmStatusDebug == 1)
                        {
                            if (strlen(thizz->scene[sId].relayName))
                            {
                                relay_json_msg_pub(sId, thizz);
                            }
                            else
                            {
                                printf("relay not present\n");
                            }
                        }
                    }

                    if (strlen(thizz->scene[sId].screenNumber))
                    {
                        screen_json_msg_pub(sId, "ipCamera", alarmStatusDebug, thizz);
                    }
                    else
                    {
                        printf("screen not present\n");
                    }
                }
                solin_json_msg_pub("ipCamera", "alarm", alarmStatusDebug, thizz->scene[sId].smartSecurityType, NULL, sId, thizz);
            }
            solin_json_msg_pub(NULL, "sceneDebug", 0, NULL, sceneType, sceneId, thizz);
        }
#endif
        else if (!strcmp(msgType, "sceneConfig") || !strcmp(msgType, "sceneDel"))
        {
            if (json_get_string_object(root, "sceneType", sceneType, sizeof(sceneType)) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "sceneType parse failed\n");
                json_release(root);
                return -1;
            }

            if (json_get_int_object(root, "sceneId", &sceneId) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "sceneId parse failed\n");
                json_release(root);
                return -1;
            }
#if 0
            if(json_get_string_object(root,"sceneName",sceneName,sizeof(sceneName))!=0)
           {
                LOG_TH(LEVEL3_ERROR, "sceneName parse failed\n");
                json_release(root);          
                return -1;
           }
#endif
            if (!strcmp(msgType, "sceneConfig"))
            {
                if ((!strcmp(sceneType, DEV_TYPE_LEVEL)) || (!strcmp(sceneType, DEV_TYPE_CAMERA)))
                {
                    if (1 == cJSON_HasObjectItem(root, "deviceGroup"))
                    {
                        device = cJSON_GetObjectItem(root, "deviceGroup");
                    }

                    if (1 == cJSON_HasObjectItem(device, "liquidlevelSensor"))
                    {
                        level = cJSON_GetObjectItem(device, "liquidlevelSensor");

                        if (level->type != cJSON_Array)
                        {
                            LOG_TH(LEVEL3_ERROR, "liquidlevelSensor type is not arry\n");
                            json_release(level);
                            json_release(device);
                            goto JSONDELETE;
                        }

                        iSize = cJSON_GetArraySize(level);
                        for (iCnt = 0; iCnt < iSize; iCnt++)
                        {
                            arrayLevel = cJSON_GetArrayItem(level, iCnt);
                            if (arrayLevel)
                            {
                                if (json_get_string_object(arrayLevel, "serialNumber", levelNumber, sizeof(levelNumber)) != 0)
                                {
                                    LOG_TH(LEVEL3_ERROR, "serialNumber parse failed\n");
                                    json_release(arrayLevel);
                                    json_release(level);
                                    json_release(device);
                                    goto JSONDELETE;
                                }
                            }
                        }
                    }
                    if (!strcmp(sceneType, DEV_TYPE_CAMERA))
                    {
                        if (1 == cJSON_HasObjectItem(device, "ipCamera"))
                        {
                            ipCamera = cJSON_GetObjectItem(device, "ipCamera");

                            if (json_get_string_object(ipCamera, "ipAddr", ipAddr, sizeof(ipAddr)) != 0)
                            {
                                LOG_TH(LEVEL3_ERROR, "ipAddr parse failed\n");
                                json_release(ipCamera);
                                json_release(device);
                                goto JSONDELETE;
                            }

                            if (json_get_string_object(ipCamera, "username", userNameCamera, sizeof(userNameCamera)) != 0)
                            {
                                LOG_TH(LEVEL3_ERROR, "username parse failed\n");
                                json_release(ipCamera);
                                json_release(device);
                                goto JSONDELETE;
                            }

                            if (json_get_string_object(ipCamera, "userPwd", userPwdCamera, sizeof(userPwdCamera)) != 0)
                            {
                                LOG_TH(LEVEL3_ERROR, "userPwd parse failed\n");
                                json_release(ipCamera);
                                json_release(device);
                                goto JSONDELETE;
                            }

                            if (json_get_string_object(ipCamera, "smartSecurityType", smartSecurityType, sizeof(smartSecurityType)) != 0)
                            {
                                LOG_TH(LEVEL3_ERROR, "smartSecurityType parse failed\n");
                                json_release(ipCamera);
                                json_release(device);
                                goto JSONDELETE;
                            }
                        }
                    }
                    if (!strcmp(sceneType, DEV_TYPE_LEVEL))
                    {
                        if (1 == cJSON_HasObjectItem(device, "ipCamera"))
                        {
                            cameraObject = cJSON_GetObjectItem(device, "ipCamera");

                            if (cameraObject->type != cJSON_Array)
                            {
                                LOG_TH(LEVEL3_ERROR, "ipCamera type is not arry\n");
                                json_release(cameraObject);
                                json_release(device);
                                goto JSONDELETE;
                            }

                            g_icam = cJSON_GetArraySize(cameraObject);
                            for (iCnt = 0; iCnt < g_icam; iCnt++)
                            {
                                arrayCamera = cJSON_GetArrayItem(cameraObject, iCnt);
                                if (arrayCamera)
                                {
                                    if (json_get_string_object(arrayCamera, "ipAddr", cameraIp[iCnt], sizeof(cameraIp[iCnt])) != 0)
                                    {
                                        LOG_TH(LEVEL3_ERROR, "ipAddr parse failed\n");
                                        json_release(arrayCamera);
                                        json_release(cameraObject);
                                        json_release(device);
                                        goto JSONDELETE;
                                    }

                                    if (json_get_string_object(arrayCamera, "username", userName[iCnt], sizeof(userName[iCnt])) != 0)
                                    {
                                        LOG_TH(LEVEL3_ERROR, "userName parse failed\n");
                                        json_release(arrayCamera);
                                        json_release(arrayCamera);
                                        json_release(cameraObject);
                                        json_release(device);
                                        goto JSONDELETE;
                                    }

                                    if (json_get_string_object(arrayCamera, "userPwd", userPwd[iCnt], sizeof(userPwd[iCnt])) != 0)
                                    {
                                        LOG_TH(LEVEL3_ERROR, "userPwd parse failed\n");
                                        json_release(arrayCamera);
                                        json_release(arrayCamera);
                                        json_release(cameraObject);
                                        json_release(device);
                                        goto JSONDELETE;
                                    }

                                    if (!strcmp(sceneType, DEV_TYPE_LEVEL))
                                    {
                                        if (json_get_int_object(arrayCamera, "preset", &preset[iCnt]) != 0)
                                        {
                                            LOG_TH(LEVEL3_ERROR, "preset parse failed\n");
                                            json_release(arrayCamera);
                                            json_release(cameraObject);
                                            json_release(device);
                                            goto JSONDELETE;
                                        }
                                    }
                                    if (!strcmp(sceneType, DEV_TYPE_CAMERA))
                                    {
                                        if (json_get_string_object(arrayCamera, "smartSecurityType", secutityType[iCnt], sizeof(secutityType[iCnt])) != 0)
                                        {
                                            LOG_TH(LEVEL3_ERROR, "smartSecutityType parse failed\n");
                                            json_release(arrayCamera);
                                            json_release(cameraObject);
                                            json_release(device);
                                            goto JSONDELETE;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if ((1 == cJSON_HasObjectItem(device, "screen")) || (1 == cJSON_HasObjectItem(device, "relay")))
                    {

                        if (1 == cJSON_HasObjectItem(device, "screen"))
                        {
                            screen = cJSON_GetObjectItem(device, "screen");

                            if (screen->type != cJSON_Array)
                            {
                                LOG_TH(LEVEL3_ERROR, "screen type is not arry\n");
                                json_release(screen);
                                json_release(device);
                                goto JSONDELETE;
                            }
                            iSize = cJSON_GetArraySize(screen);

                            for (iCnt = 0; iCnt < iSize; iCnt++)
                            {
                                arrayScreen = cJSON_GetArrayItem(screen, iCnt);
                                if (arrayScreen)
                                {
                                    if (json_get_string_object(arrayScreen, "serialNumber", screenNumber, sizeof(screenNumber)) != 0)
                                    {
                                        LOG_TH(LEVEL3_ERROR, "serialNumber parse failed\n");
                                        json_release(arrayScreen);
                                        json_release(screen);
                                        json_release(device);
                                        goto JSONDELETE;
                                    }
                                }
                            }
                        }

                        if (1 == cJSON_HasObjectItem(device, "relay"))
                        {
                            relay = cJSON_GetObjectItem(device, "relay");

                            if (relay->type != cJSON_Array)
                            {
                                LOG_TH(LEVEL3_ERROR, "relay type is not arry\n");
                                json_release(relay);
                                json_release(device);
                                goto JSONDELETE;
                            }

                            iSize = cJSON_GetArraySize(relay);
                            for (iCnt = 0; iCnt < iSize; iCnt++)
                            {
                                arrayRelay = cJSON_GetArrayItem(relay, iCnt);

                                if (arrayRelay)
                                {
                                    if (json_get_int_object(arrayRelay, "termId", &relayId) != 0)
                                    {
                                        LOG_TH(LEVEL3_ERROR, "relay termId parse failed\n");
                                        json_release(arrayRelay);
                                        json_release(relay);
                                        json_release(device);
                                        goto JSONDELETE;
                                    }

                                    if (json_get_string_object(arrayRelay, "modelName", relayName, sizeof(relayName)) != 0)
                                    {
                                        LOG_TH(LEVEL3_ERROR, "relay name parse failed\n");
                                        json_release(arrayRelay);
                                        json_release(relay);
                                        json_release(device);
                                        goto JSONDELETE;
                                    }

                                    if (json_get_int_object(arrayRelay, "channelNumber", &channelNumber) != 0)
                                    {
                                        LOG_TH(LEVEL3_ERROR, "channelNumber parse failed\n");
                                        json_release(arrayRelay);
                                        json_release(relay);
                                        json_release(device);
                                        goto JSONDELETE;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        LOG_TH(LEVEL3_ERROR, "Lack of scene linkage equipment");
                        json_release(device);
                        goto JSONDELETE;
                    }
                }
            }
        }

        if (!strcmp(sceneType, DEV_TYPE_LEVEL) && !strcmp(msgType, "sceneConfig"))
        {
            printf("add sceneId %d#############\n", sceneId);
            for (i = 0; i <= SCENE_MAX; i++)
            {
                if (thizz->scene[i].sceneId == sceneId)
                {
                    sId = i;
                    sprintf(sceneName, "%d", sceneId);
                    memset(&(thizz->scene[sId]), 0x00, sizeof(thizz->scene[sId]));
                    del_scene(sceneName);
                    break;
                }
                else if (thizz->scene[i].sceneId == 0)
                {
                    sId = i;
                    break;
                }
            }

            printf("add sceneID2 %d %d#############\n", sceneId, sId);
            thizz->scene[sId].sceneId = sceneId;
            // memcpy(thizz->scene[sId].sceneName,sceneName,strlen(sceneName));
            memcpy(thizz->scene[sId].levelNumber, levelNumber, strlen(levelNumber));

            for (c = 0; c < g_icam; c++)
            {
                memcpy(thizz->scene[sId].camera[c].cameraIp, cameraIp[c], strlen(cameraIp[c]));
                memcpy(thizz->scene[sId].camera[c].userName, userName[c], strlen(userName[c]));
                memcpy(thizz->scene[sId].camera[c].userPwd, userPwd[c], strlen(userPwd[c]));
                thizz->scene[sId].camera[c].preset = preset[c];
            }

            add_scene_file(sceneType, sId, thizz);
            solin_json_msg_pub(NULL, "sceneConfig", 0, NULL, "waterloggingMonit", sId, thizz);
        }
        else if (!strcmp(sceneType, DEV_TYPE_CAMERA) && !strcmp(msgType, "sceneConfig"))
        {
            printf("add sceneId %d#############\n", sceneId);
            for (i = 0; i <= SCENE_MAX; i++)
            {
                if (thizz->scene[i].sceneId == sceneId)
                {
                    sId = i;
                    sprintf(sceneName, "%d", sceneId);
                    memset(&(thizz->scene[sId]), 0x00, sizeof(thizz->scene[sId]));
                    del_scene(sceneName);
                    break;
                }
                else if (thizz->scene[i].sceneId == 0)
                {
                    sId = i;
                    printf("add sceneId  %d sid %d#############\n", sceneId, sId);
                    break;
                }
            }

            memcpy(thizz->scene[sId].ipAddr, ipAddr, strlen(ipAddr));
            memcpy(thizz->scene[sId].username, userNameCamera, strlen(userNameCamera));
            memcpy(thizz->scene[sId].userPwd, userPwdCamera, strlen(userPwdCamera));
            memcpy(thizz->scene[sId].smartSecurityType, smartSecurityType, strlen(smartSecurityType));

            // memcpy(thizz->scene[sId].camera[0].cameraIp, ipAddr, strlen(ipAddr));
            // memcpy(thizz->scene[sId].camera[0].userName, userNameCamera, strlen(userNameCamera));
            // memcpy(thizz->scene[sId].camera[0].userPwd, userPwdCamera, strlen(userPwdCamera));
            // memcpy(thizz->scene[sId].camera[0].secutityType, smartSecurityType, strlen(smartSecurityType));

            memcpy(thizz->scene[sId].screenNumber, screenNumber, strlen(screenNumber));
            memset(screenNumber, 0x00, sizeof(screenNumber));

            thizz->scene[sId].sceneId = sceneId;
            thizz->scene[sId].relayId = relayId;
            relayId = 0;
            memcpy(thizz->scene[sId].relayName, relayName, strlen(relayName));
            memset(relayName, 0X00, sizeof(relayName));
            thizz->scene[sId].channelNumber = channelNumber;
            channelNumber = 0;

            add_scene_file(sceneType, sId, thizz);
            solin_json_msg_pub(NULL, "sceneConfig", 0, NULL, "smartSecutity", sId, thizz);
            LOG_TH(LEVEL3_ERROR, "add security success, sceneId %d ,cameraIpAddr %s \n", thizz->scene[sId].sceneId, thizz->scene[sId].ipAddr);
        }

        else if (!strcmp(sceneType, DEV_TYPE_LEVEL) && !strcmp(msgType, "sceneDel"))
        {
            printf("sceneDel########\n");
            for (i = 0; i <= SCENE_MAX; i++)
            {
                if (thizz->scene[i].sceneId == sceneId)
                {
                    sId = i;
                    break;
                }
            }

            if (i >= SCENE_MAX)
            {
                LOG_TH(LEVEL3_ERROR, "sceneId %d not exist\n", sceneId);
            }

            thizz->scene[sId].sceneId = 0;
            memset(thizz->scene[sId].levelNumber, 0x00, sizeof(thizz->scene[sId].levelNumber));

            for (c = 0; c < 5; c++)
            {
                memset(thizz->scene[sId].camera[c].cameraIp, 0x00, sizeof(cameraIp[c]));
                memset(thizz->scene[sId].camera[c].userName, 0x00, sizeof(userName[c]));
                memset(thizz->scene[sId].camera[c].userPwd, 0x00, sizeof(userPwd[c]));
                thizz->scene[sId].camera[c].preset = 0;
            }

            sprintf(sceneName, "%d", sceneId);
            del_scene(sceneName);
            solin_json_msg_pub(NULL, "sceneDel", 0, NULL, "waterloggingMonit", sId, thizz);
        }
        else if (!strcmp(sceneType, DEV_TYPE_CAMERA) && !strcmp(msgType, "sceneDel"))
        {
            for (i = 0; i <= SCENE_MAX; i++)
            {
                if (thizz->scene[i].sceneId == sceneId)
                {
                    sId = i;
                    break;
                }
            }

            if (i >= SCENE_MAX)
            {
                LOG_TH(LEVEL3_ERROR, "sceneId %d not exist\n", sceneId);
            }

            thizz->scene[sId].sceneId = 0;
            thizz->scene[sId].relayId = 0;
            thizz->scene[sId].channelNumber = 0;
            memset(thizz->scene[sId].relayName, 0x00, sizeof(thizz->scene[sceneId].relayName));
            memset(thizz->scene[sId].screenNumber, 0x00, sizeof(thizz->scene[sceneId].screenNumber));

            memset(thizz->scene[sId].ipAddr, 0x00, sizeof(ipAddr));
            memset(thizz->scene[sId].username, 0x00, sizeof(userNameCamera));
            memset(thizz->scene[sId].userPwd, 0x00, sizeof(userPwd));
            memset(thizz->scene[sId].smartSecurityType, 0x00, sizeof(smartSecurityType));

            // memset(thizz->scene[sId].camera[0].cameraIp, 0x00, sizeof(cameraIp[0]));
            // memset(thizz->scene[sId].camera[0].userName, 0x00, sizeof(userName[0]));
            // memset(thizz->scene[sId].camera[0].userPwd, 0x00, sizeof(userPwd[0]));
            // memset(thizz->scene[sId].camera[0].secutityType, 0x00, sizeof(secutityType[0]));
            sprintf(sceneName, "%d", sceneId);
            del_scene(sceneName);
            solin_json_msg_pub(NULL, "sceneDel", 0, NULL, "smartSecutity", sId, thizz);
        }

        if (!strcmp(msgType, "alarm"))
        {
            if (json_get_int_object(root, "alarmStatus", &alarmStatus) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "alarmStatus parse failed\n");
                json_release(root);
                return -1;
            }

            if (json_get_int_object(root, "sceneId", &sceneId) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "sceneId parse failed\n");
                json_release(root);
                return -1;
            }
        }
    }
JSONDELETE:
    json_release(root);

    return 0;
}

int power_cmd(unsigned char p)
{
    return (p & 0x80) >> 7;
}

uint8_t char_to_code(char x)
{
    if ((x >= 'A') && (x <= 'Z'))
    {
        return (uint8_t)x - (uint8_t)'A';
    }
    else if ((x >= 'a') && (x <= 'z'))
    {
        return (uint8_t)x - (uint8_t)'a' + 26;
    }
    else if ((x >= '0') && (x <= '9'))
    {
        return (uint8_t)x - (uint8_t)'0' + 52;
    }
    else if (x == '+')
    {
        return 62;
    }
    else if (x == '/')
    {
        return 63;
    }
    else
    {
        LOG_TH(LEVEL3_ERROR, "%c (0x%x) IS INVALID CHARACTER FOR BASE64 DECODING\n", x, x);
        return -1;
    }

    return 0;
}

int b64_to_bin_nopad(const char *in, int size, uint8_t *out, int max_len)
{
    int i;
    int result_len;  /* size of the result */
    int full_blocks; /* number of 3 unsigned chars / 4 characters blocks */
    int last_chars;  /* number of characters <4 in the last block */
    int last_bytes;  /* number of unsigned chars <3 in the last block */
    uint32_t b;
    ;

    /* check input values */
    if ((out == NULL) || (in == NULL))
    {
        LOG_TH(LEVEL3_ERROR, "NULL POINTER AS OUTPUT OR INPUT IN B64_TO_BIN\n");
        return -1;
    }

    if (size == 0)
    {
        return 0;
    }

    /* calculate the number of base64 'blocks' */
    full_blocks = size / 4;
    last_chars = size % 4;
    switch (last_chars)
    {
    case 0: /* no char left to decode */
        last_bytes = 0;
        break;
    case 1: /* only 1 char left is an error */
        LOG_TH(LEVEL3_ERROR, "ONLY ONE CHAR LEFT IN B64_TO_BIN\n");
        return -1;
    case 2: /* 2 chars left to decode -> +1 byte */
        last_bytes = 1;
        break;
    case 3: /* 3 chars left to decode -> +2 bytes */
        last_bytes = 2;
        break;
    default:
        printf("switch default that should not be possible\n");
    }

    /* check if output buffer is big enough */
    result_len = (3 * full_blocks) + last_bytes;
    if (max_len < result_len)
    {
        LOG_TH(LEVEL3_ERROR, "OUTPUT BUFFER TOO SMALL IN B64_TO_BIN\n");
        return -1;
    }

    /* process all the full blocks */
    for (i = 0; i < full_blocks; ++i)
    {
        b = (0x3F & char_to_code(in[4 * i])) << 18;
        b |= (0x3F & char_to_code(in[4 * i + 1])) << 12;
        b |= (0x3F & char_to_code(in[4 * i + 2])) << 6;
        b |= 0x3F & char_to_code(in[4 * i + 3]);
        out[3 * i + 0] = (b >> 16) & 0xFF;
        out[3 * i + 1] = (b >> 8) & 0xFF;
        out[3 * i + 2] = b & 0xFF;
    }

    /* process the last 'partial' block */
    i = full_blocks;

    if (last_bytes == 1)
    {
        b = (0x3F & char_to_code(in[4 * i])) << 18;
        b |= (0x3F & char_to_code(in[4 * i + 1])) << 12;
        out[3 * i + 0] = (b >> 16) & 0xFF;
        if (((b >> 12) & 0x0F) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "last character contains unusable bits\n");
        }
    }
    else if (last_bytes == 2)
    {
        b = (0x3F & char_to_code(in[4 * i])) << 18;
        b |= (0x3F & char_to_code(in[4 * i + 1])) << 12;
        b |= (0x3F & char_to_code(in[4 * i + 2])) << 6;
        out[3 * i + 0] = (b >> 16) & 0xFF;
        out[3 * i + 1] = (b >> 8) & 0xFF;
        if (((b >> 6) & 0x03) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "last character contains unusable bits\n");
        }
    }

    return result_len;
}

int b64_to_bin(const char *in, int size, uint8_t *out, int max_len)
{
    if (in == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "NULL POINTER AS OUTPUT OR INPUT IN B64_TO_BIN\n");
        return -1;
    }

    if ((size % 4 == 0) && (size >= 4))
    { /* potentially padded Base64 */
        if (in[size - 2] == '=')
        { /* 2 padding char to ignore */
            return b64_to_bin_nopad(in, size - 2, out, max_len);
        }
        else if (in[size - 1] == '=')
        { /* 1 padding char to ignore */
            return b64_to_bin_nopad(in, size - 1, out, max_len);
        }
        else
        { /* no padding to ignore */
            return b64_to_bin_nopad(in, size, out, max_len);
        }
    }
    else
    { /* treat as unpadded Base64 */
        return b64_to_bin_nopad(in, size, out, max_len);
    }
}

/*****************************************************************
 * Function: Level mqtt messages analysis,
 *           When the alarmValue value is 1,
 *           an alarm is triggered
 */
int level_cjson_msg_parse(struct edge_usr_db *thizz, char *msg)
{
    cJSON *root = NULL;
    int a = 0, alarmValue = 0, i = 0, sId = 0, tidMark = -1;
    char applicationId[64] = {0}, profileName[64] = {0}, profileId[64] = {0};
    char devEui[64] = {0}, temp[128] = {0}, devType[64] = {"liquidlevelSensor"};
    // char alarmType[32] = {0}, msgType[32] = {"alarm"};
    uint8_t data[128] = {0};
    pthread_t tid2;

    root = cJSON_Parse(msg);
    if (!root)
    {
        LOG_TH(LEVEL3_ERROR, "level cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    else
    {
        if (json_get_string_object(root, "applicationID", applicationId, sizeof(applicationId)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "applicationId parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "deviceProfileName", profileName, sizeof(profileName)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "deviceProfileName parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "deviceProfileID", profileId, sizeof(profileId)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "deviceProfileID parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "devEui", devEui, sizeof(devEui)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "devEui parse failed\n");
            json_release(root);
            return -1;
        }

        /*if(json_get_string_object(root,"gatewayID",gatewayId,sizeof(gatewayId))!=0)
        {
            LOG_TH(LEVEL3_ERROR, "gatewayID parse failed\n");
            json_release(root);
            return -1;
        }*/

        if (json_get_string_object(root, "data", temp, sizeof(temp)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "data parse failed\n");
            json_release(root);
            return -1;
        }
    }

    a = b64_to_bin(temp, strlen(temp), data, sizeof(data));
    printf("b64_to_bin #####\n");

    if (a <= 0)
    {
        printf("data parse faild\n");
        return -1;
    }

    alarmValue = power_cmd(data[4]);
    printf("waterloggingMonit alarm status %d  %02x\n", alarmValue, data[4]);

    for (i = 0; i < SCENE_MAX; i++)
    {
        if (!strcmp(devEui, thizz->scene[i].levelNumber))
        {
            sId = i;
            printf("###waterSecne match success###\n");
            printf("g_index %d#####\n", sId);
            printf("%d   %d\n", g_tempValue, alarmValue);

            if (alarmValue == 1)
            {
                printf("waterSecne ctrl camera\n");
                level_json_msg_pub(sId, thizz, alarmValue);
            }

            if (alarmValue != g_tempValue)
            {
                g_tempValue = alarmValue;
                if (alarmValue == 0)
                {
                    printf("waterSecne ctrl camera alarm release\n");
                    level_json_msg_pub(sId, thizz, alarmValue);
                }
            }
            // solin_json_msg_pub(devType, msgType, alarmValue, alarmType, NULL, sId, thizz);

            LOG_TH(LEVEL3_ERROR, "###level alarm thread starten###\n");
            SolinAlarm *levelAlarmMsg = (SolinAlarm *)malloc(sizeof(SolinAlarm));
            strcpy(levelAlarmMsg->devType, devType);
            levelAlarmMsg->alarmValue = alarmValue;
            levelAlarmMsg->sceneId = sId;

            tidMark = pthread_create(&tid2, NULL, solin_alarm_thread, (void *)levelAlarmMsg);
            if (tidMark == 0)
            {
                LOG_TH(LEVEL3_ERROR, "levle SOLIN thread create success\n");
            }
            else
            {
                LOG_TH(LEVEL3_ERROR, "levle SOLIN thread create fail\n");
                free(levelAlarmMsg);
            }
            break;
        }
    }

    if (i >= SCENE_MAX)
    {
        LOG_TH(LEVEL3_ERROR, "waterSecne no match ,devEui %s\n", devEui);
    }

    return 0;
}

/*****************************************************************
 * Function: secur Camera mqtt messages analysis,
 *           When the alarmStatus value is 1,
 *           an alarm is triggered
 */
int secur_camera_cjson_msg_parse(struct edge_usr_db *thizz, char *msg)
{
    cJSON *root = NULL;
    int alarmStatus = 1, i = 0, sId = 0, tidMark = -1;
    char msgType[64] = {0}, alarmType[64] = {0}, alarmTime[64] = {0};
    char ipAddr[64] = {0}, ts[64] = {0}, devType[64] = {"ipCamera"};
    char serialNumber[64] = {0};
    pthread_t tid3;

    root = cJSON_Parse(msg);
    if (!root)
    {
        LOG_TH(LEVEL3_ERROR, "secur camera cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    else
    {
        if (json_get_string_object(root, "msgType", msgType, sizeof(msgType)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "msgType parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "serialNumber", serialNumber, sizeof(serialNumber)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "serialNumber parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "ipAddr", ipAddr, sizeof(ipAddr)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "ipAddr parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "ts", ts, sizeof(ts)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "ts parse failed\n");
            json_release(root);
            return -1;
        }

        if (!strcmp(msgType, "cameraAlarm"))
        {
            if (json_get_string_object(root, "alarmType", alarmType, sizeof(alarmType)) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "alarmType parse failed\n");
                json_release(root);
                return -1;
            }

            if (json_get_int_object(root, "alarmStatus", &alarmStatus) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "alarmStatus parse failed\n"); // 发给平台
                json_release(root);
                return -1;
            }

            if (json_get_string_object(root, "alarmTime", alarmTime, sizeof(alarmTime)) != 0)
            {
                LOG_TH(LEVEL3_ERROR, "alarmTime parse failed\n");
                json_release(root);
                return -1;
            }
        }
    }

    if (!strcmp(msgType, "cameraAlarm"))
    {
        for (i = 0; i < SCENE_MAX; i++)
        {
            if (!strcmp(ipAddr, thizz->scene[i].ipAddr))
            {
                sId = i;
                if (!strcmp(thizz->scene[i].smartSecurityType, alarmType))
                {
                    if (alarmStatus != g_relayAlarm)
                    {
                        g_relayAlarm = alarmStatus;
                        if (alarmStatus == 1)
                        {
                            if (strlen(thizz->scene[sId].relayName))
                            {
                                relay_json_msg_pub(sId, thizz);
                            }
                            else
                            {
                                LOG_TH(LEVEL3_ERROR, "recv camera %s Alarm,scene match,but relay not set params,ceneId:%d\n", thizz->scene[i].ipAddr, thizz->scene[i].sceneId);
                            }
                        }
                    }

                    // secur_camera_json_pub(msgType, serialNumber, ipAddr, alarmType, sId, 0, thizz);
                    secur_camera_json_pub(msgType, serialNumber, ipAddr, alarmType, sId, thizz);
                    // solin_json_msg_pub(devType, "alarm", alarmStatus, alarmType, NULL, sId, thizz);
                    if (strlen(thizz->scene[sId].screenNumber))
                    {
                        screen_json_msg_pub(sId, devType, alarmStatus, thizz);
                    }
                    else
                    {
                        LOG_TH(LEVEL3_ERROR, "recv camera %s Alarm,scene match,but screen not set param,sceneId:%d\n", thizz->scene[i].ipAddr, thizz->scene[i].sceneId);
                    }

                    LOG_TH(LEVEL3_ERROR, "###secur camera alarm thread starten###\n");
                    SolinAlarm *cameraAlarmMsg = (SolinAlarm *)malloc(sizeof(SolinAlarm));

                    strcpy(cameraAlarmMsg->devType, devType);
                    strcpy(cameraAlarmMsg->alarmType, alarmType);
                    cameraAlarmMsg->alarmValue = alarmStatus;
                    cameraAlarmMsg->sceneId = sId;
                    printf("secur camera####2\n");

                    tidMark = pthread_create(&tid3, NULL, solin_alarm_thread, (void *)cameraAlarmMsg);
                    if (tidMark == 0)
                    {
                        LOG_TH(LEVEL3_ERROR, "camera SOLIN thread create success\n");
                    }
                    else
                    {
                        LOG_TH(LEVEL3_ERROR, "camera SOLIN thread create fail\n");
                        free(cameraAlarmMsg);
                    }
                }
                else
                {
                    LOG_TH(LEVEL3_ERROR, "cameraIP %s not smartSecurityType\n", thizz->scene[i].ipAddr);
                }
                break;
            }
        }

        if (i >= SCENE_MAX)
        {
            LOG_TH(LEVEL3_ERROR, "scameraIP %s not find match scene\n", thizz->scene[i].ipAddr);
        }
    }

    if (!strcmp(msgType, "cameraOline"))
    {
        for (i = 0; i < SCENE_MAX; i++)
        {
            if (!strcmp(ipAddr, thizz->scene[i].ipAddr))
            {
                LOG_TH(LEVEL3_ERROR, "recv %s cameraOline,secutity scene match success,sceneId %d\n", thizz->scene[i].ipAddr, thizz->scene[i].sceneId);
                sId = i;
                // cId = c;
                secur_camera_json_pub(msgType, serialNumber, ipAddr, alarmType, sId, thizz);
                break;
            }
        }

        if (i >= SCENE_MAX)
        {
            LOG_TH(LEVEL3_ERROR, "recv cameraOline,but not secutity scene matchx \n");
        }
    }

    json_release(root);
    return 0;
}

/*****************************************************************
 * Function: camera mqtt message parsing
 */
int camera_cjson_msg_parse(struct edge_usr_db *thizz, char *msg)
{
    cJSON *root = NULL;
    char msgType[32] = {0}, sceneType[32] = {0}, ipAddr[32] = {0}, ts[32] = {0}, errorCode[32] = {0};
    root = cJSON_Parse(msg);
    if (!root)
    {
        LOG_TH(LEVEL3_ERROR, "camera cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    else
    {
        if (json_get_string_object(root, "msgType", msgType, sizeof(msgType)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "msgType parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "sceneType", sceneType, sizeof(sceneType)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "sceneType parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "ipAddr", msgType, sizeof(ipAddr)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "ipAddr parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "ts", ts, sizeof(ts)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "ts parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "errorCode", errorCode, sizeof(errorCode)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "errorCode parse failed\n");
            json_release(root);
            return -1;
        }

        printf("camera errorCode %d\n", atoi(errorCode));
        if (0 == atoi(errorCode))
        {
            LOG_TH(LEVEL3_ERROR, "camera return errorCode normal\n");
        }
        else
        {
            LOG_TH(LEVEL3_ERROR, "camera return errorCode error\n");
        }
    }

    json_release(root);
    return 0;
}

/*****************************************************************
 * Function: relay mqtt message parsing
 */
int secur_relay_cjson_msg_parse(struct edge_usr_db *thizz, char *msg)
{
    cJSON *root = NULL;
    char msgType[32] = {0}, devType[32] = {0}, ctrlType[32] = {0}, ts[32] = {0}, errorCode[32] = {0};
    int termId = 0, tunId = 0;
    root = cJSON_Parse(msg);
    if (!root)
    {
        LOG_TH(LEVEL3_ERROR, "secur relay cjson parse failed[%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    else
    {
        if (json_get_string_object(root, "msgType", msgType, sizeof(msgType)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "msgType parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_int_object(root, "termId", &termId) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "termId parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "devType", devType, sizeof(devType)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "ipAddr parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "ctrlType", ctrlType, sizeof(ctrlType)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "ctrlType parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_int_object(root, "tunId", &tunId) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "tunId parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "ts", ts, sizeof(ts)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "ts parse failed\n");
            json_release(root);
            return -1;
        }

        if (json_get_string_object(root, "errorCode", errorCode, sizeof(errorCode)) != 0)
        {
            LOG_TH(LEVEL3_ERROR, "errorCode parse failed\n");
            json_release(root);
            return -1;
        }

        printf("camera errorCode %d\n", atoi(errorCode));
        if (0 == atoi(errorCode))
        {
            LOG_TH(LEVEL3_ERROR, "relay return errorCode normal\n");
        }
        else
        {
            LOG_TH(LEVEL3_ERROR, "relay return errorCode error\n");
        }
    }

    json_release(root);
    return 0;
}

void solin_sub_msg_callback(char *topic, void *msg, int msgLen)
{
    printf("topic:%s\n", (char *)topic);
    printf("msg:%s\n", (char *)msg);
    solin_cjson_msg_parse(g_edge, msg);
}

void edge_apply_sub_msg_callback(char *topic, void *msg, int msgLen)
{
    // char loratopic[]={"application/+/device/+/event/up"};
    char cameraTopic[] = {"EDGE/UPLINK/LIQUIDLEVEL/CAMERA/"};
    char s_cameratopic[] = {"EDGE/UPLINK/SECURITY/CAMERA"};
    char s_relaytopic[] = {"EDGE/UPLINK/SECURITY/RELAY"};
    // char s_screentopic[]={"EDGE/UPLINK/SECURITY/SCREEN"};
    // printf("topic msg:%s\n",topic);

    printf("edg submsg:%s\n", (char *)msg);

    if (strstr(topic, "/event/up"))
    {
        printf("level_liquidlevel####\n");
        level_cjson_msg_parse(g_edge, msg);
    }
    else if (!strcmp(topic, s_cameratopic))
    {
        secur_camera_cjson_msg_parse(g_edge, msg);
    }
    else if (!strcmp(topic, cameraTopic))
    {
        camera_cjson_msg_parse(g_edge, msg);
    }
    else if (!strcmp(topic, s_relaytopic))
    {
        secur_relay_cjson_msg_parse(g_edge, msg);
    }

    return;
}

int get_edge_config(struct edge_usr_db *thizz)
{
    char retBuff[128] = {0x00};
    char tempBuff[128] = {0x00};

    memset(thizz->edge_hostIp, 0x00, sizeof(thizz->edge_hostIp));
    memset(thizz->edge_username, 0x00, sizeof(thizz->edge_username));
    memset(thizz->edge_password, 0x00, sizeof(thizz->edge_password));

    cmd_popen_execute("sed -n '/ip/p' /tmp/edgelocalfoward | sed 's/ip://g'", retBuff);
    memcpy(thizz->edge_hostIp, retBuff, strlen(retBuff) - 1);

    cmd_popen_execute("grep 'username' /tmp/edgelocalfoward", tempBuff); // Check if username exists
    if (strlen(tempBuff))
    {
        cmd_popen_execute("sed -n '/username/p' /tmp/edgelocalfoward | sed 's/username://g'", retBuff); // get username
        memcpy(thizz->edge_username, retBuff, strlen(retBuff) - 1);
    }
    else
    {
        memcpy(thizz->edge_username, "seb_scene", strlen("seb_scene"));
    }

    cmd_popen_execute("grep 'password' /tmp/edgelocalfoward", tempBuff); // Check if password exists
    if (strlen(tempBuff))
    {
        cmd_popen_execute("sed -n '/password/p' /tmp/edgelocalfoward | sed 's/password://g'", retBuff); // get password
        memcpy(thizz->edge_password, retBuff, strlen(retBuff) - 1);
    }
    else
    {
        memcpy(thizz->edge_password, "seb+123456", strlen("seb+123456"));
    }

    if (g_foreground)
    {
        printf("###edg ip = %s###\n", thizz->edge_hostIp);
        printf("###edg username = %s###\n", thizz->edge_username);
        printf("###edg password = %s###\n", thizz->edge_password);
    }

    return 0;
}

void solin_time_check_cb(int fd, short _event, void *arg)
{
    struct edge_usr_db *thizz = (struct edge_usr_db *)arg;

    if (!(thizz->cconline))
    {
        solin_json_msg_pub(NULL, "ccOnline", 0, NULL, NULL, 0, thizz);
    }

    struct timeval tv;
    tv.tv_sec = 6;
    tv.tv_usec = 0;
    event_add((struct event *)&(thizz->ev_ls), &tv);
}

int cmdline_config_load(struct edge_usr_db *thizz, int argc, char *argv[])
{
    int i;
    char *vesion = "app_scene.1.0.1";

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h"))
        {
            if (i == argc - 1)
            {
                LOG_TH(LEVEL3_ERROR, "-h argument given but no specified hostip.\n");
                exit(0);
            }
            else
            {
                memcpy(thizz->mosq_hostIp, argv[i + 1], sizeof(thizz->mosq_hostIp));
            }
            i++;
        }
        else if (!strcmp(argv[i], "-e"))
        {
            if (i == argc - 1)
            {
                LOG_TH(LEVEL3_ERROR, "-e argument given but no specified egdeip.\n");
                exit(0);
            }
            else
            {
                memcpy(thizz->edge_hostIp, argv[i + 1], sizeof(thizz->edge_hostIp));
            }
            i++;
        }
        else if (!strcmp(argv[i], "-m"))
        {
            if (i == argc - 1)
            {
                LOG_TH(LEVEL3_ERROR, "-m argument given but no specified Mac\n");
                exit(0);
            }
            else
            {
                memcpy(thizz->localmacStr, argv[i + 1], sizeof(thizz->localmacStr));
            }
            i++;
        }
        else if (!strcmp(argv[i], "-d"))
        {
            g_foreground = 1;
        }
        else if (!strcmp(argv[i], "-v"))
        {
            printf("verion## %s ## \n", vesion);
        }
        else
        {
            printf("######### help info####\n");
            printf("-h  $(ipaddr)   eg: -h  192.168.10.211  \n");
            printf("-e  $(ipaddr)   eg: -e  192.168.20.22  \n");
            printf("-m $(mac)       eg: -m  C0F636001D39  \n");
            printf("-v $(vesion)    eg: -v  \n");
            printf("-d  is debug    eg: -d    \n");
            exit(0);
        }
    }

    printf("mosq_hostIp:%s  \n", thizz->mosq_hostIp);
    printf("egde_hostIp:%s  \n", thizz->edge_hostIp);

    if (!g_foreground)
    {
        daemon2(0, 0);
    }

    return 0;
}

int init_egde_status_data(struct edge_usr_db *thizz)
{
    int i = 0, c = 0;
    memset(thizz->scene, 0x00, sizeof(thizz->scene));

    for (i = 0; i < SCENE_MAX; i++)
    {
        thizz->scene[i].sceneId = 0;
        thizz->scene[i].relayId = 0;
        thizz->scene[i].channelNumber = 0;
        memset(thizz->scene[i].levelNumber, 0x00, sizeof(thizz->scene[i].levelNumber));
        memset(thizz->scene[i].username, 0x00, sizeof(thizz->scene[i].username));
        memset(thizz->scene[i].userPwd, 0x00, sizeof(thizz->scene[i].userPwd));
        memset(thizz->scene[i].smartSecurityType, 0x00, sizeof(thizz->scene[i].smartSecurityType));
        memset(thizz->scene[i].screenNumber, 0x00, sizeof(thizz->scene[i].screenNumber));
        memset(thizz->scene[i].relayName, 0x00, sizeof(thizz->scene[i].relayName));

        for (c = 0; c < 5; c++)
        {
            thizz->scene[i].camera[c].preset = 0;
            memset(thizz->scene[i].camera[c].cameraIp, 0x00, sizeof(thizz->scene[i].camera[c].cameraIp));
            memset(thizz->scene[i].camera[c].userName, 0x00, sizeof(thizz->scene[i].camera[c].userName));
            memset(thizz->scene[i].camera[c].userPwd, 0x00, sizeof(thizz->scene[i].camera[c].userPwd));
            memset(thizz->scene[i].camera[c].secutityType, 0x00, sizeof(thizz->scene[i].camera[c].secutityType));
        }
    }

    return 0;
}

int solin_sub_topic_init(struct edge_usr_db *thizz)
{
    char sub_topic1[128] = {0};
    snprintf(sub_topic1, sizeof(sub_topic1), "OEM/485/DOWNLINK/SCENE/%s", thizz->localmacStr);
    mosq_usr_sub_topic_add(thizz->mosq_solin_new, sub_topic1);

    return 0;
}

int solin_pub_topic_init(struct edge_usr_db *thizz)
{
    snprintf(thizz->s_pub_topic, sizeof(thizz->s_pub_topic), "OEM/485/UPLINK/SCENE/%s", thizz->localmacStr);

    return 0;
}

int edg_sub_topic_init(struct edge_usr_db *thizz)
{
    char sub_topic1[128] = {0};
    snprintf(sub_topic1, sizeof(sub_topic1), "EDGE/UPLINK/LIQUIDLEVEL/CAMERA");
    mosq_usr_sub_topic_add(thizz->mosq_edge_new, sub_topic1);

    char sub_topic2[128] = {0};
    snprintf(sub_topic2, sizeof(sub_topic2), "EDGE/UPLINK/SECURITY/SCREEN");
    mosq_usr_sub_topic_add(thizz->mosq_edge_new, sub_topic2);

    char sub_topic3[128] = {0};
    snprintf(sub_topic3, sizeof(sub_topic3), "EDGE/UPLINK/SECURITY/RELAY");
    mosq_usr_sub_topic_add(thizz->mosq_edge_new, sub_topic3);

    char sub_topic4[128] = {0};
    snprintf(sub_topic4, sizeof(sub_topic4), "application/+/device/+/event/up");
    mosq_usr_sub_topic_add(thizz->mosq_edge_new, sub_topic4);

    char sub_topic5[128] = {0};
    snprintf(sub_topic5, sizeof(sub_topic5), "EDGE/UPLINK/SECURITY/CAMERA");
    mosq_usr_sub_topic_add(thizz->mosq_edge_new, sub_topic5);

    return 0;
}

int edg_pub_topic_init(struct edge_usr_db *thizz)
{
    snprintf(thizz->e_pub2_topic, sizeof(thizz->e_pub2_topic), "application/+/device/+/event/down");
    snprintf(thizz->e_pub3_topic, sizeof(thizz->e_pub3_topic), "EDGE/DOWNLINK/SECURITY/CAMERA");
    snprintf(thizz->e_pub4_topic, sizeof(thizz->e_pub4_topic), "EDGE/DOWNLINK/LIQUIDLEVEL/CAMERA");
    // snprintf(thizz->e_pub5_topic,sizeof(thizz->e_pub5_topic),"EDGE/DOWNLINK/SECURITY/SCREEN");
    snprintf(thizz->e_pub6_topic, sizeof(thizz->e_pub6_topic), "EDGE/DOWNLINK/SECURITY/RELAY");

    return 0;
}

int will_init(struct edge_usr_db *thizz)
{
    char will_topic[128];
    char will_paload[256];
    snprintf(will_paload, sizeof(will_paload), "{ \"msgType\":\"ccOffline\",\"concentratorSn\":\"%s\",\"ts\":\"%lu\"}", thizz->localmacStr, time(NULL));
    snprintf(will_topic, sizeof(will_topic), "OEM/485/UPLINK/SCENE/%s", thizz->localmacStr);
    mosq_usr_will_set(thizz->mosq_solin_new, will_topic, strlen(will_paload), will_paload);

    return 0;
}

int solin_mosq(struct edge_usr_db *thizz)
{
    mosq_usr_ip_interface_set(thizz->mosq_solin_new, thizz->mosq_hostIp, "eth0");
    mosq_usr_sub_message_callback_set(thizz->mosq_solin_new, solin_sub_msg_callback);
    mosq_usr_pub_start(thizz->mosq_solin_new);
    mosq_usr_sub_start(thizz->mosq_solin_new);

    return 0;
}

int edg_mosq(struct edge_usr_db *thizz)
{
    if (!access("/tmp/edgelocalfoward", F_OK))
    {
        get_edge_config(thizz);
        mosq_usr_name_password_set(thizz->mosq_edge_new, thizz->edge_username, thizz->edge_password);
        mosq_usr_port_set(thizz->mosq_edge_new, 1883);
        mosq_usr_ip_interface_set(thizz->mosq_edge_new, thizz->edge_hostIp, "eth0");
        mosq_usr_sub_message_callback_set(thizz->mosq_edge_new, edge_apply_sub_msg_callback);
        mosq_usr_pub_start(thizz->mosq_edge_new);
        mosq_usr_sub_start(thizz->mosq_edge_new);
    }

    return 0;
}

int main(int argc, char *argv[])
{

    openlog("edge", LOG_NDELAY | LOG_PID, LOG_DAEMON);

    g_edge = usr_db_init();

    cmdline_config_load(g_edge, argc, argv);

    g_edge->mosq_solin_new = mosq_usr_new(PUB_MODE_SHORT_CONNECT, 0);
    g_edge->mosq_edge_new = mosq_usr_new(PUB_MODE_SHORT_CONNECT, 0);

    edg_sub_topic_init(g_edge);
    edg_pub_topic_init(g_edge);

    solin_sub_topic_init(g_edge);
    solin_pub_topic_init(g_edge);

    solin_mosq(g_edge);
    edg_mosq(g_edge);

    will_init(g_edge);

    g_base = event_init();
    if (g_base == NULL)
    {
        LOG_TH(LEVEL3_ERROR, "event_init failed");
        return -1;
    }

    init_egde_status_data(g_edge);

    if (!access(DEV_SCENE_CONF, F_OK))
    {
        load_file_scene(g_edge);
    }
    else
    {
        printf("file %s does not exist\n", DEV_SCENE_CONF);
    }

    evtimer_set(&(g_edge->ev_ls), solin_time_check_cb, (void *)g_edge);
    event_base_set(g_base, &(g_edge->ev_ls));
    struct timeval tvi;
    tvi.tv_sec = 25;
    tvi.tv_usec = 0;
    event_add(&(g_edge->ev_ls), &tvi);

#if 0
    evtimer_set(&(g_edge->ev_ra),relay_ctrl_timer,(void *)g_edge);
    event_base_set(g_base, &(g_edge->ev_ra));
    struct timeval tvs;
    tvs.tv_sec=600; 
    tvs.tv_usec=0;
    event_add(&(g_edge->ev_ra),&tvs);
#endif

    event_base_loop(g_base, 0);

    closelog();

    return 0;
}

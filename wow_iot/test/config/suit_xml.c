#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "config/wow_xml.h"
#include "file/wow_file.h"
#include "file/wow_hfile.h"
#include "plugin/wow_debug.h"
#include "system/wow_time.h"
#include "utils/wow_type.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"


#define MOD_TAG "xml"
#define XML_FILE_TEST     "config/test_xml.xml"
#define XML_FILE_SAVE     "config/save_xml.xml"
#define XML_FILE_DEMO 	  "config/demo.xml"
#define XML_FILE_PROJECT  "config/project.xml"
#define XML_FILE_ALERT    "config/alert.xml"


#define TEST_STR "<formula1><team name=\"McLaren\"><driver><name>Kimi Raikkonen</name><points>112</points></driver><driver><name>Juan Pablo Montoya</name><points>70</points></driver></team></formula1>"



TEST test_xml_create(void)
{
	ModDebug_I(MOD_TAG,"suit_xml.........test_xml_create\n");
	
	File_T* hfile = wow_hfile_open(FILE_FIO,WOW_TEST_FILE_STRING(XML_FILE_TEST),FILE_RDONLY|FILE_CREATE);
	GREATEST_ASSERT(hfile);

	int ret = wow_hfile_write(hfile, TEST_STR, strlen(TEST_STR));
	GREATEST_ASSERT(ret  == strlen(TEST_STR));

	wow_hfile_close(hfile);

	PASS();
}

#if SUIT_RUN_ERROR_TEST
TEST test_xml_error(void)
{
	ModDebug_I(MOD_TAG,"suit_xml.........test_xml_error\n");

	int ret = 0;
	const char* value = NULL;
	Xml_PT root_xml = NULL;
	Xml_PT new_xml = NULL;
	
	root_xml = wow_xml_init_by_file(NULL);
	GREATEST_ASSERT(root_xml == NULL);
	root_xml = wow_xml_init_by_file("AAA.xml");
	GREATEST_ASSERT(root_xml == NULL);
	root_xml = wow_xml_init_by_str(NULL);
	GREATEST_ASSERT(root_xml == NULL);

	wow_xml_exit(root_xml);

	ret = wow_xml_set_txt(NULL,"AAA");
	GREATEST_ASSERT(ret != 0);
	ret = wow_xml_set_attr(NULL,"AAA","BBB");
	GREATEST_ASSERT(ret != 0);
	ret = wow_xml_add_item_entry(NULL,root_xml);
	GREATEST_ASSERT(ret != 0);
	value = wow_xml_get_txt(NULL);
	GREATEST_ASSERT(value == NULL);
	value = wow_xml_get_attr(NULL,"AAA");
	GREATEST_ASSERT(value == NULL);
	value = wow_xml_get_name(NULL);
	GREATEST_ASSERT(value == NULL);

	
	root_xml = wow_xml_init_by_file(WOW_TEST_FILE_STRING(XML_FILE_TEST));
	GREATEST_ASSERT(root_xml);
	
	new_xml = wow_xml_new_item(NULL);
	GREATEST_ASSERT(new_xml == NULL);

	ret = wow_xml_set_txt(root_xml,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_xml_set_attr(root_xml,NULL,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_xml_add_item_entry(root_xml,NULL);
	GREATEST_ASSERT(ret != 0);
	
	new_xml = wow_xml_add_item_info(root_xml,NULL,"AAA","BBBB","CCCCCC");
	GREATEST_ASSERT(new_xml == NULL);
	new_xml = wow_xml_add_item_info(root_xml,"DDD",NULL,NULL,NULL);
	GREATEST_ASSERT(new_xml);
	new_xml = wow_xml_add_item_info(root_xml,"DDD",NULL,"BBBBB","CCCCCC");
	GREATEST_ASSERT(new_xml);

	value = wow_xml_get_txt(root_xml);
	//GREATEST_ASSERT(value == NULL);
	value = wow_xml_get_attr(root_xml,"AAA");
	GREATEST_ASSERT(value == NULL);
	value = wow_xml_get_name(root_xml);
	GREATEST_ASSERT(value);

	ret = wow_xml_save(root_xml,NULL);
	GREATEST_ASSERT(ret != 0);
	
	wow_xml_exit(&root_xml);

	PASS();
}
#endif

TEST test_xml_read_str(void)
{
	ModDebug_I(MOD_TAG,"suit_xml..........test_xml_read_str\n");
	
	Xml_PT xml = NULL;
	char* test_str = NULL;

	///!!!!!段错误
	//xml = wow_xml_init_by_str(TEST_STR);

	test_str = malloc(strlen(TEST_STR));
	strcpy(test_str,TEST_STR);
	xml = wow_xml_init_by_str(test_str);
	GREATEST_ASSERT(xml);

	Xml_PT team    = NULL;
	Xml_PT driver  = NULL;
	const char *teamname = NULL;

	///<<查询模式1
	for (team = wow_xml_findName(xml, "team"); team; team = wow_xml_nextSibling(team)) {
		teamname = wow_xml_get_attr(team, "name");
		for (driver = wow_xml_findName(team, "driver"); driver; driver = wow_xml_nextSibling(driver)) {
			Xml_PT tmp = wow_xml_findName(driver, "poind");
			GREATEST_ASSERT(tmp == NULL);	
			printf("[%s] %s %s\n",teamname, wow_xml_get_txt(wow_xml_findName(driver, "name")), 
				   wow_xml_get_txt(wow_xml_findName(driver, "points")));
		}
	}

	///<<查询模式2
	Xml_PT dname1  = wow_xml_findPath(xml,"sis", "team", 0, "name");
	Xml_PT points1 = wow_xml_findPath(xml,"sis",  "team", 0, "points");
	GREATEST_ASSERT(dname1 && points1);
	GREATEST_ASSERT(!strcmp(wow_xml_get_txt(dname1),"Kimi Raikkonen"));
	GREATEST_ASSERT(atoi(wow_xml_get_txt(points1)) == 112);


	Xml_PT dname2  = wow_xml_findPath(xml,"sis",  "team", 1, "name");
	Xml_PT points2 = wow_xml_findPath(xml,"sis",  "team", 1, "points");
	GREATEST_ASSERT(dname2 && points2);
	GREATEST_ASSERT(!strcmp(wow_xml_get_txt(dname2),"Juan Pablo Montoya"));
	GREATEST_ASSERT(atoi(wow_xml_get_txt(points2)) == 70);

	wow_xml_exit(&xml);	
	free(test_str);
	PASS();
}

TEST test_xml_read_file(void)
{
	ModDebug_I(MOD_TAG,"suit_xml..........test_xml_read_file\n");
	
	Xml_PT xml = NULL;

	xml = wow_xml_init_by_file(WOW_TEST_FILE_STRING(XML_FILE_TEST));
	GREATEST_ASSERT(xml);

	Xml_PT team    = NULL;
	Xml_PT driver  = NULL;
	const char *teamname = NULL;
	
	///<<查询模式1
	for (team = wow_xml_findName(xml, "team"); team; team = wow_xml_nextSibling(team)) {
		teamname = wow_xml_get_attr(team, "name");
		for (driver = wow_xml_findName(team, "driver"); driver; driver = wow_xml_nextSibling(driver)) {
			Xml_PT tmp = wow_xml_findName(driver, "poind");
			GREATEST_ASSERT(tmp == NULL);	
			printf("[%s] %s %s\n",teamname, wow_xml_get_txt(wow_xml_findName(driver, "name")), 
				   wow_xml_get_txt(wow_xml_findName(driver, "points")));
		}
	}

	///<<查询模式2
	Xml_PT dname1  = wow_xml_findPath(xml,"sis", "team", 0, "name");
	Xml_PT points1 = wow_xml_findPath(xml,"sis", "team", 0, "points");
	GREATEST_ASSERT(dname1 && points1);
	GREATEST_ASSERT(!strcmp(wow_xml_get_txt(dname1),"Kimi Raikkonen"));
	GREATEST_ASSERT(atoi(wow_xml_get_txt(points1)) == 112);

	Xml_PT dname2  = wow_xml_findPath(xml,"sis", "team", 1, "name");
	Xml_PT points2 = wow_xml_findPath(xml,"sis", "team", 1, "points");
	GREATEST_ASSERT(dname2 && points2);
	GREATEST_ASSERT(!strcmp(wow_xml_get_txt(dname2),"Juan Pablo Montoya"));
	GREATEST_ASSERT(atoi(wow_xml_get_txt(points2)) == 70);
	
	wow_xml_exit(&xml);	
	PASS();
}

TEST test_xml_write_fun(void)
{
	ModDebug_I(MOD_TAG,"suit_xml..........test_xml_write_fun1\n");
	
	int ret = 0;
	Xml_PT xml = NULL;

	xml = wow_xml_init_by_file(WOW_TEST_FILE_STRING(XML_FILE_TEST));
	GREATEST_ASSERT(xml);

	///<<插入模式1
	Xml_PT txml01 = wow_xml_new_item("team");
	GREATEST_ASSERT(txml01);
	ret = wow_xml_set_attr(txml01,"name", "Wow");
	GREATEST_ASSERT(ret == 0);

	Xml_PT dxml;Xml_PT nxml;Xml_PT pxml;

	dxml = wow_xml_new_item("driver");
	GREATEST_ASSERT(dxml);
	nxml = wow_xml_new_item("name");
	wow_xml_set_txt(nxml,"john");
	pxml = wow_xml_new_item("points");
	wow_xml_set_txt(pxml,"150");
	wow_xml_add_item_entry(dxml,nxml);
	wow_xml_add_item_entry(dxml,pxml);
	wow_xml_add_item_entry(txml01,dxml);

	dxml = wow_xml_new_item("driver");
	GREATEST_ASSERT(dxml);
	nxml = wow_xml_new_item("name");
	wow_xml_set_txt(nxml,"holly");
	pxml = wow_xml_new_item("points");
	wow_xml_set_txt(pxml,"112");
	wow_xml_add_item_entry(dxml,nxml);
	wow_xml_add_item_entry(dxml,pxml);
	wow_xml_add_item_entry(txml01,dxml);

	wow_xml_add_item_entry(xml,txml01);

	///<<插入模式2
	Xml_PT txml02 = wow_xml_add_item_info(xml,"team",NULL,"name", "iot");
	GREATEST_ASSERT(txml02);
	
	Xml_PT dxml1 = wow_xml_add_item_info(txml02,"driver",NULL,NULL, NULL);;
	GREATEST_ASSERT(dxml1);
	Xml_PT dxml2 = wow_xml_add_item_info(txml02,"driver",NULL,NULL, NULL);;
	GREATEST_ASSERT(dxml2);

	
	Xml_PT nxml1 = wow_xml_add_item_info(dxml1,"name","bob",NULL, NULL);
	GREATEST_ASSERT(nxml1);
	Xml_PT pxml1 = wow_xml_add_item_info(dxml1,"points","101",NULL, NULL);
	GREATEST_ASSERT(pxml1);
	
	Xml_PT nxml2 = wow_xml_add_item_info(dxml2,"name","lee",NULL, NULL);
	GREATEST_ASSERT(nxml2 );
	Xml_PT pxml2 = wow_xml_add_item_info(dxml2,"points","99",NULL, NULL);
	GREATEST_ASSERT(pxml2);
	
	ret = wow_xml_save(xml,WOW_TEST_FILE_STRING(XML_FILE_SAVE));
	GREATEST_ASSERT(ret == 0);
	printf("------------------------------------------------------------\n");
	printf("tesxml02:%s\n",wow_xml_print(txml02));
	printf("------------------------------------------------------------\n");
	wow_xml_exit(&xml);	
	PASS();	
}


TEST test_xml_normal(void)
{
	ModDebug_I(MOD_TAG,"suit_xml..........test_xml_normal\n");
	
	Xml_PT xml = NULL;
	Xml_PT admin = NULL;
	Xml_PT updatesector = NULL;
	
	xml = wow_xml_init_by_file(WOW_TEST_FILE_STRING(XML_FILE_DEMO));
	GREATEST_ASSERT(xml);

	admin = wow_xml_findName(xml, "admin");
	GREATEST_ASSERT(admin);
	GREATEST_ASSERT(atoi(wow_xml_get_attr(admin, "id")) == 1);
	printf("admin username:%s\n", wow_xml_get_txt(wow_xml_findName(admin, "username")));
	printf("admin password:%s\n", wow_xml_get_txt(wow_xml_findName(admin, "password")));
	
	updatesector  = wow_xml_findPath(xml,"ss", "burn","updatesector");
	GREATEST_ASSERT(updatesector);
	GREATEST_ASSERT(atoi(wow_xml_get_attr(updatesector, "enable")) == 1);

	Xml_PT nap = wow_xml_findPath(updatesector,"sss","updatebtaddr","curr_bt_addr_gen","NAP");
	Xml_PT uap = wow_xml_findPath(updatesector,"sss","updatebtaddr","curr_bt_addr_gen","UAP");
	Xml_PT lap = wow_xml_findPath(updatesector,"sss","updatebtaddr","curr_bt_addr_gen","LAP");
	printf("updatesector nap:%s\n", wow_xml_get_txt(nap));
	printf("updatesector uap:%s\n", wow_xml_get_txt(uap));
	printf("updatesector lap:%s\n", wow_xml_get_txt(lap));
	
	wow_xml_exit(&xml);	
	PASS(); 
}

#define PROJECT_STR_SZIE  (2048)
#define PROJECT_STR_HEAD   "<main_config version=\"ver 1.0.0\">"
#define PROJECT_STR_CHANS  "<channels>%d</channels>"
#define PROJECT_STR_CHAN_S "<channel%d num=\"%d\">"
#define PROJECT_STR_CHAN_I "<item>%04x</item>"
#define PROJECT_STR_CHAN_E "</channel%d>"
#define PROJECT_STR_TAIL   "</main_config>"

typedef struct{
    int num;
    uint16_t addr[12];
}ChannelInfo_T;

typedef struct{
    uint16_t num;
    uint16_t back;
    ChannelInfo_T chan[8];
}ChannelAllInfo_T;

static ChannelAllInfo_T gt_channel_info={
	.num = 4,
	.back=0,
	.chan = {
        [0] = {.num = 7,.addr={0x1000,0x1A01,0x1B01,0x1C01,0x1A02,0x1B02,0x1C02}},
     	[1] = {.num = 6,.addr={0x2A01,0x2B01,0x2C01,0x2A02,0x2B02,0x2C02}},
     	[2] = {.num = 8,.addr={0x3A01,0x3B01,0x3C01,0x3F01,0x3A02,0x3B02,0x3C02,0x3F02}},
     	[3] = {.num = 9,.addr={0x4000,0x4A01,0x4B01,0x4C01,0x4F01,0x4A02,0x4B02,0x4C02,0x4F02}},
    }
};

TEST test_xml_project(void)
{
	ModDebug_I(MOD_TAG,"suit_xml.........test_xml_project\n");

	int i = 0;
	int j = 0;
	int dlen = 0;
	char data[PROJECT_STR_SZIE] = {0};
	ChannelAllInfo_T info ={0};
	
	if(wow_file_check_exist(XML_FILE_PROJECT) == 1){
		wow_file_remove(XML_FILE_PROJECT);
	}

	File_T* hfile = wow_hfile_open(FILE_FIO,WOW_TEST_FILE_STRING(XML_FILE_PROJECT),FILE_RDWR|FILE_CREATE);
	GREATEST_ASSERT(hfile);


	dlen += snprintf(data,PROJECT_STR_SZIE,PROJECT_STR_HEAD);
	dlen += snprintf(data+dlen,PROJECT_STR_SZIE-dlen,PROJECT_STR_CHANS,gt_channel_info.num);
	for(i = 0; i < gt_channel_info.num; i++){
		dlen += snprintf(data+dlen,PROJECT_STR_SZIE-dlen,PROJECT_STR_CHAN_S,i+1,gt_channel_info.chan[i].num);
		for(j = 0; j < gt_channel_info.chan[i].num; j++){
			dlen += snprintf(data+dlen,PROJECT_STR_SZIE-dlen,PROJECT_STR_CHAN_I,gt_channel_info.chan[i].addr[j]);
		}
		dlen += snprintf(data+dlen,PROJECT_STR_SZIE-dlen,PROJECT_STR_CHAN_E,i+1);
	}
	dlen += snprintf(data+dlen,PROJECT_STR_SZIE-dlen,PROJECT_STR_TAIL);

	
	int ret = wow_hfile_write(hfile, data, dlen);
	GREATEST_ASSERT(ret  == dlen);
	
	wow_hfile_close(hfile);

	Xml_PT xml = wow_xml_init_by_file(WOW_TEST_FILE_STRING(XML_FILE_PROJECT));
	GREATEST_ASSERT(xml);

	Xml_PT channels = wow_xml_findName(xml, "channels");
	GREATEST_ASSERT(channels);

	char name[9] = {0};
	Xml_PT item;
	info.num = atoi(wow_xml_get_txt(channels));
	for(i = 0; i < info.num; i++){
		snprintf(name,9,"channel%d",i+1);
		Xml_PT channel = wow_xml_findName(xml, name);
		GREATEST_ASSERT(channel);
		
		info.chan[i].num = atoi(wow_xml_get_attr(channel,"num"));
		for(item = wow_xml_findName(channel, "item"),j = 0; item; item = wow_xml_nextSibling(item),j++){
			S16toU16(wow_xml_get_txt(item),&info.chan[i].addr[j]);	
		}
	}

	printf("project info channels:%d\n",info.num);
	for(i = 0; i < info.num; i++){
		printf("chan%d info num:%d addr:\n",i,info.chan[i].num);
		for(j = 0; j < info.chan[i].num; j++){
			printf("%04x ",info.chan[i].addr[j]);
		}
		printf("\r\n");
	}
	wow_xml_exit(&xml);	



	PASS();
}


#define ALTER_STR_SZIE  (4096)
#define ALTER_STR_HEAD   "<main_config version=\"ver 1.0.0\">"
#define ALTER_STR_ITEM   "<item id=\"%08x\">%ld</item>"
#define ALTER_STR_TAIL   "</main_config>"

#define ALT_IDX_BIT      (16)
#define ALT_IDX_MASK     (0xFFFF)
#define ALT_TYPE_BIT     (0)
#define ALT_TYPE_MASK    (0xFFFF)

#define BUILD_ALERT_ID(idx,type) \
    (((((uint16_t)idx) & ALT_IDX_MASK) << ALT_IDX_BIT) | \
     ((((uint16_t)type) & ALT_TYPE_MASK) << ALT_TYPE_BIT))

TEST test_xml_alert(void)
{
	ModDebug_I(MOD_TAG,"suit_xml.........test_xml_alert\n");

	int j = 0;
	int dlen = 0;
	char data[ALTER_STR_SZIE] = {0};
	
	if(wow_file_check_exist(XML_FILE_ALERT) == 1){
		wow_file_remove(XML_FILE_ALERT);
	}

	File_T* hfile = wow_hfile_open(FILE_FIO,WOW_TEST_FILE_STRING(XML_FILE_ALERT),FILE_RDONLY|FILE_CREATE);
	GREATEST_ASSERT(hfile);

	dlen += snprintf(data,ALTER_STR_SZIE,ALTER_STR_HEAD);
	dlen += snprintf(data+dlen,ALTER_STR_SZIE-dlen,ALTER_STR_ITEM,BUILD_ALERT_ID(32,1),wow_time_get_msec());
	dlen += snprintf(data+dlen,ALTER_STR_SZIE-dlen,ALTER_STR_ITEM,BUILD_ALERT_ID(45,2),wow_time_get_msec());
	dlen += snprintf(data+dlen,ALTER_STR_SZIE-dlen,ALTER_STR_ITEM,BUILD_ALERT_ID(58,2),wow_time_get_msec());
	dlen += snprintf(data+dlen,ALTER_STR_SZIE-dlen,ALTER_STR_ITEM,BUILD_ALERT_ID(88,1),wow_time_get_msec());
	dlen += snprintf(data+dlen,ALTER_STR_SZIE-dlen,ALTER_STR_TAIL);

	
	int ret = wow_hfile_write(hfile, data, dlen);
	GREATEST_ASSERT(ret  == dlen);
	
	wow_hfile_close(hfile);

	Xml_PT xml = wow_xml_init_by_file(WOW_TEST_FILE_STRING(XML_FILE_ALERT));
	GREATEST_ASSERT(xml);

	Xml_PT item;
	for(item = wow_xml_findName(xml, "item"),j = 0; item; item = wow_xml_nextSibling(item),j++){
		 uint64_t id,ms;
	     S16toU64(wow_xml_get_attr(item,"id"),&id);
		 S10toU64(wow_xml_get_txt(item),&ms);

		 printf("---item  id:%08lx ms:%ld\n",id,ms);
	}


	
	item = wow_xml_findName(xml, "item");
	wow_xml_remove(item);
	printf("-------------------------------------------------\n");
	for(item = wow_xml_findName(xml, "item"),j = 0; item; item = wow_xml_nextSibling(item),j++){
		 uint64_t id,ms;
	     S16toU64(wow_xml_get_attr(item,"id"),&id);
		 S10toU64(wow_xml_get_txt(item),&ms);

		 printf("---item  id:%08lx ms:%ld\n",id,ms);
	}	


	
	wow_xml_add_item_info(xml,"item","2044512641","id", "11001001");
	printf("-------------------------------------------------\n");
	for(item = wow_xml_findName(xml, "item"),j = 0; item; item = wow_xml_nextSibling(item),j++){
		 uint64_t id,ms;
	     S16toU64(wow_xml_get_attr(item,"id"),&id);
		 S10toU64(wow_xml_get_txt(item),&ms);

		 printf("---item  id:%08lx ms:%ld\n",id,ms);
	}	

	wow_xml_save(xml,WOW_TEST_FILE_STRING(XML_FILE_ALERT));
	
	wow_xml_exit(&xml);	

	PASS();
}



SUITE(suit_xml)
{	

	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_xml_error);
#endif

	RUN_TEST(test_xml_create);
	RUN_TEST(test_xml_read_str);
	RUN_TEST(test_xml_read_file);
	RUN_TEST(test_xml_write_fun);
	RUN_TEST(test_xml_normal);
	RUN_TEST(test_xml_project);
	RUN_TEST(test_xml_alert);
}


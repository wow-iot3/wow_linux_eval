#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>



#include "prefix/wow_common.h"
#include "plugin/wow_debug.h"
#include "system/wow_time.h"
#include "config/wow_json.h"

#include "greatest/greatest.h"
#include "wow_test_config.h"

static Json_PT object  = NULL;
static Json_PT object1 = NULL;
static Json_PT object2 = NULL;
static Json_PT arrary1 = NULL;
static Json_PT arrary2 = NULL;
static Json_PT root = NULL;




#define MOD_TAG "json"
#define JSON_FILE_TEST "config/test_json.json"
#define JSON_FILE_SAVE "config/save_json.json"
#define JSON_FILE_DEMO "config/demo.json"



static const char test[1024] =  "\
{ \"version\":1.0, \
  \"init\":true, \
  \"profile\":{ \
		   \"productID\":\"C0NEMO9UO0\" \
	   }, \
   \"properties\":[   \
    { \
        \"id\":1, \
        \"name\":\"AAA\", \
        \"define\":{  \
            \"type\":\"timestamp\" \
        } \
    }, \
    { \
        \"id\":2, \
        \"name\":\"BBB\", \
        \"define\":{  \
            \"type\":\"float\", \
            \"min\":10 \
        } \
    } \
    ]\
} \
";

#if SUIT_RUN_ERROR_TEST
TEST test_json_error(void)
{
	int ret = 0;
	Json_PT val = NULL;
	
	ModDebug_I(MOD_TAG,"suit_json------test_json_error\n");
	
	ret = wow_json_object_add(NULL,"AAA",JSON_TYPE_INT32,"BBBBB");
	GREATEST_ASSERT(ret != 0);
	ret = wow_json_object_del(NULL,"AAA");
	GREATEST_ASSERT(ret != 0);
	ret = wow_json_array_add(NULL,JSON_TYPE_INT32,"CCCCC");
	GREATEST_ASSERT(ret != 0);
	ret = wow_json_array_size(NULL);
	GREATEST_ASSERT(ret != 0);


	object1 = wow_json_object_new();
	GREATEST_ASSERT(object1);
	
	GREATEST_ASSERT(ret != 0);
	ret = wow_json_object_add(object1,NULL,JSON_TYPE_INT32,"BBBBB");
	GREATEST_ASSERT(ret != 0);
	ret = wow_json_object_add(object1,"AAAA",255,"BBBBB");
	GREATEST_ASSERT(ret != 0);
	ret = wow_json_object_add(object1,"AAAA",255,NULL);
	GREATEST_ASSERT(ret != 0);
	ret = wow_json_object_del(object1,"AAA");
	GREATEST_ASSERT(ret == 0);

	wow_json_destory(&object1);
	
	arrary1 = wow_json_arrary_new();
	GREATEST_ASSERT(arrary1);
	ret = wow_json_array_add(arrary1,JSON_TYPE_INT32,NULL);
	GREATEST_ASSERT(ret != 0);

	wow_json_destory(&arrary1);

	val = wow_json_init_by_file(NULL);
	GREATEST_ASSERT(val == NULL);
	val = wow_json_init_by_str(NULL);
	GREATEST_ASSERT(val == NULL);
	val = wow_json_object_value(NULL,"AAA");
	GREATEST_ASSERT(val == NULL);


	object1 = wow_json_init_by_str(test);
	GREATEST_ASSERT(object1);
	val = wow_json_object_value(object1,NULL);
	GREATEST_ASSERT(val == NULL);
	wow_json_destory(&object1);

	PASS();
}
#endif


TEST test_json_object_write(void)
{
	int ret = 0;
	ModDebug_I(MOD_TAG,"suit_json------test_json_object_write\n");
	
	object1 = wow_json_object_new();
	GREATEST_ASSERT(object1);

	char sdata[16] = "hello world!";
	char ldata[16] = "are you ok!";
	double ddata = 12.123;
	int idata = 137;
	long long lldata = wow_time_get_usec();
	bool bdata = true;
	
	ret = wow_json_object_add(object1,"string", JSON_TYPE_STRING,sdata);
	GREATEST_ASSERT(ret == 0);

	ret = wow_json_object_add(object1,"double", JSON_TYPE_DOUBLE,&ddata);
	GREATEST_ASSERT(ret == 0);

	ret = wow_json_object_add(object1,"int", JSON_TYPE_INT32,&idata);
	GREATEST_ASSERT(ret == 0);

	ret = wow_json_object_add(object1,"long", JSON_TYPE_INT64,&lldata);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_json_object_add(object1,"bool", JSON_TYPE_BOOL,&bdata);
	GREATEST_ASSERT(ret == 0);

	object2 = wow_json_object_new();
	GREATEST_ASSERT(object2);
	
	ret = wow_json_object_add(object2,"name", JSON_TYPE_STRING,ldata);
	GREATEST_ASSERT(ret == 0);

	PASS();
}

TEST test_json_arrary_write(void)
{
	int ret = 0;
	int i = 0;
	
	ModDebug_I(MOD_TAG,"suit_json------test_json_arrary_write\n");
	
	arrary1 = wow_json_arrary_new();
	GREATEST_ASSERT(arrary1);
	
    char a1data[3][8] = {"abc","def","lmnopq"};
	for(i = 0; i < 3; i++){
		ret = wow_json_array_add(arrary1,JSON_TYPE_STRING,&a1data[i][0]);
		GREATEST_ASSERT(ret == 0);
	}


	arrary2 = wow_json_arrary_new();
	GREATEST_ASSERT(arrary2);
	int a2data[5] = {110,120,122,119,10086};
	for(i = 0; i < 5; i++){
		ret = wow_json_array_add(arrary2,JSON_TYPE_INT32,&a2data[i]);
		GREATEST_ASSERT(ret == 0);
	}

	PASS();
}

TEST test_json_save_file(void)
{
	int ret = 0;
	
	ModDebug_I(MOD_TAG,"suit_json------test_json_save_file\n");	
	
	object = wow_json_object_new();
	GREATEST_ASSERT(object);

	ret = wow_json_object_add(object,"object1", JSON_TYPE_OBJECT,object1);
	GREATEST_ASSERT(ret == 0);

	ret = wow_json_object_add(object,"object2", JSON_TYPE_OBJECT,object2);
	GREATEST_ASSERT(ret == 0);

	ret = wow_json_object_add(object,"arrary1", JSON_TYPE_ARRARY,arrary1);
	GREATEST_ASSERT(ret == 0);

	ret = wow_json_object_add(object,"arrary2", JSON_TYPE_ARRARY,arrary2);
	GREATEST_ASSERT(ret == 0);
	
	ret = wow_json_save_file(object,WOW_TEST_FILE_STRING(JSON_FILE_SAVE));
	GREATEST_ASSERT(ret == 0);


	PASS();
}

TEST test_json_laod_file(void)
{
	ModDebug_I(MOD_TAG,"suit_json------test_json_laod_file\n");	
	
	root = wow_json_init_by_file(WOW_TEST_FILE_STRING(JSON_FILE_SAVE));
	GREATEST_ASSERT(root);
	
	char* data = wow_json_dump_new(root);
	printf("-----------------------------------------------------\n");
	printf("%s\n",data);
	printf("-----------------------------------------------------\n");
	wow_json_dump_free(data);

	wow_json_destory(&root);
	PASS();
}


TEST test_json_read(void)
{
	int i = 0;
	void* value = NULL;
	
	ModDebug_I(MOD_TAG,"suit_json------test_json_read\n");	
	
	root = wow_json_init_by_str(test);
	GREATEST_ASSERT(root);

	///<获取方式1
	value = wow_json_object_value(root,"init");
	GREATEST_ASSERT(value && (*(double*)value) == 1);

	value = wow_json_object_value(root,"version");
	GREATEST_ASSERT(value && (*(double*)value) == 1.0);

	Json_PT prof_val;
	prof_val = wow_json_object_value(root,"profile");
	GREATEST_ASSERT(prof_val);

	value = wow_json_object_value(prof_val,"productID");
	GREATEST_ASSERT(value && !strcmp("C0NEMO9UO0",(char*)value));

	Json_PT prop_val;
	prop_val = wow_json_object_value(root,"properties");
	GREATEST_ASSERT(prop_val);


	for(i = 0 ; i < wow_json_array_size(prop_val);i++){
		Json_PT arr_val = wow_json_array_value(prop_val,i);
		GREATEST_ASSERT(arr_val);

		value = wow_json_object_value(arr_val,"id");
		GREATEST_ASSERT(value);
		ModDebug_D(MOD_TAG,"properties-id:%d\n",(int)*(double*)value);

		value = wow_json_object_value(arr_val,"name");
		GREATEST_ASSERT(value);
		ModDebug_D(MOD_TAG,"properties-name:%s\n",(char*)value);

		Json_PT def_val = wow_json_object_value(arr_val,"define");
		GREATEST_ASSERT(def_val);

		value = wow_json_object_value(def_val,"type");
		GREATEST_ASSERT(value);
		ModDebug_D(MOD_TAG,"properties-type:%s\n",(char*)value);
	}


	///<获取方式2
	value = wow_json_object_value_ex(root,"s","version");
	GREATEST_ASSERT(value && (*(double*)value) == 1.0);

	value = wow_json_object_value_ex(root,"ss","profile","productID");
	GREATEST_ASSERT(value && !strcmp("C0NEMO9UO0",(char*)value));


	value = wow_json_object_value_ex(root,"sis","properties",0,"id");
	GREATEST_ASSERT(value && (int)*(double*)value == 1);

	value = wow_json_object_value_ex(root,"sis","properties",0,"name");
	GREATEST_ASSERT(value && !strcmp("AAA",(char*)value));

	value = wow_json_object_value_ex(root,"siss","properties",0,"define","type");
	GREATEST_ASSERT(value && !strcmp("timestamp",(char*)value));

	wow_json_destory(&root);

	PASS();
}



typedef struct {
    char name[16];
	int  score;
} Hometown_T;

typedef struct {
    uint8_t id;
    double weight;
    uint8_t score[8];
    char name[10];
    Hometown_T hometown;
} Student_T;

typedef struct {
	char code[256];
	int  size;
    Student_T* student;
}Class_T;


static Student_T student_01 = {
	.id = 24,
	.weight = 71.2,
	.score = {1, 2, 3, 4, 5, 6, 7, 8},
	.name = "armink",
	.hometown.name	 = "China",
	.hometown.score  = 86,
};
	
static Student_T student_02 = {
	.id = 25,
	.weight = 55.2,
	.score = {1, 2, 3, 4, 5, 6, 7, 8},
	.name = "jdsl",
	.hometown.name	 = "China",
	.hometown.score  = 86,
};
	

static Json_PT test_student_struct_to_json(Student_T* student)
{
	Json_PT hometown_object = wow_json_object_new();
	wow_json_object_add(hometown_object,"name",JSON_TYPE_STRING,student->hometown.name);
	wow_json_object_add(hometown_object,"score",JSON_TYPE_INT32,&student->hometown.score);

	Json_PT student_object = wow_json_object_new();
	wow_json_object_add(student_object,"id",JSON_TYPE_INT8,&student->id);
	wow_json_object_add(student_object,"weight",JSON_TYPE_DOUBLE,&student->weight);

	wow_json_object_add_array(student_object,score,JSON_TYPE_INT8,&student->score,8);
	wow_json_object_add(student_object,"name",JSON_TYPE_STRING,student->name);
	wow_json_object_add(student_object,"hometown",JSON_TYPE_OBJECT,hometown_object);

	return student_object;
}

TEST test_struct_to_json_01(void) 
{
	ModDebug_D(MOD_TAG,"suit_json------test_struct_to_json_01\n");

	Json_PT student_object = test_student_struct_to_json(&student_01);

	char* data = wow_json_dump_new(student_object);
	printf("------------------------------------------------------------\n");
	printf(":%s\n",data);
	printf("------------------------------------------------------------\n");
	wow_json_dump_free(data);

	wow_json_destory(&student_object);

	PASS();
}

TEST test_struct_to_json_02(void) 
{
	ModDebug_D(MOD_TAG,"suit_json------test_struct_to_json_02\n");

	char code[9] = {0};
	
	Class_T* class = CALLOC(1, Class_T);
	class->student = CALLOC(2, Student_T);
	
	class->size = 2;
	memset(code,0,sizeof(code));
	memcpy(code,"AAAAAAA",8);
	memcpy(class->code,code,8);
	memcpy(&class->student[0],&student_01,sizeof(Student_T));
	memcpy(&class->student[1],&student_02,sizeof(Student_T));

	Json_PT* student_object = CALLOC(2, Json_PT);
	student_object[0] = test_student_struct_to_json(&class->student[0]);
	student_object[1] = test_student_struct_to_json(&class->student[1]);
	
	Json_PT class_object = wow_json_object_new();
	wow_json_object_add(class_object,"code",JSON_TYPE_STRING,class->code);
	wow_json_object_add(class_object,"size",JSON_TYPE_INT32,&class->size);

	wow_json_object_add_array(class_object,student,JSON_TYPE_OBJECT,student_object,2);

	char* data = wow_json_dump_new(class_object);
	printf("------------------------------------------------------------\n");
	printf(":%s\n",data);
	printf("------------------------------------------------------------\n");
	wow_json_dump_free(data);

	wow_json_destory(&class_object);

	FREE(student_object);
	FREE(class->student);
	FREE(class);
	
	PASS();
}

TEST test_json_normal(void)
{
	void* value = NULL;
	
	ModDebug_I(MOD_TAG,"suit_json------test_json_normal\n");	
	
	root = wow_json_init_by_file(WOW_TEST_FILE_STRING(JSON_FILE_DEMO));
	GREATEST_ASSERT(root);
	
	value = wow_json_object_value(root,"name");
	GREATEST_ASSERT(value && !strcmp("haas200",(char*)value));

	value = wow_json_object_value(root,"version");
	GREATEST_ASSERT(value && !strcmp("1.0.0",(char*)value));

	Json_PT io_val;
	io_val = wow_json_object_value(root,"io");
	GREATEST_ASSERT(io_val);

	value = wow_json_object_value_ex(root,"sss","io","ADC0","type");
	GREATEST_ASSERT(value && !strcmp("ADC",(char*)value));
	value = wow_json_object_value_ex(root,"sss","io","ADC0","port",NULL);
	GREATEST_ASSERT(value && (int)*(double*)value == 0);
	value = wow_json_object_value_ex(root,"sss","io","ADC0","sampling",NULL);
	GREATEST_ASSERT(value && (int)*(double*)value == 12000000);

	wow_json_destory(&root);
	
	PASS();
}

	

SUITE(suit_json)
{
	wow_debug_setStatus(MOD_TAG,SUIT_RUN_DEBUG_FLAG);
#if SUIT_RUN_ERROR_TEST
	RUN_TEST(test_json_error);
#endif
	RUN_TEST(test_json_object_write);
	RUN_TEST(test_json_arrary_write);
	RUN_TEST(test_json_save_file);
	RUN_TEST(test_json_laod_file);
	RUN_TEST(test_json_read);
	RUN_TEST(test_struct_to_json_01);
	RUN_TEST(test_struct_to_json_02);
	RUN_TEST(test_json_normal);
}



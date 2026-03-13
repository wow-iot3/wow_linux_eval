#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <math.h>


#include "greatest/greatest.h"

#include "prefix/prefix.h"
#include "utils/wow_math.h"
#include "algorithm/quadsort.h"


#define MOD_TAG "[quadsort]"


int cmp_int(const void * a, const void * b)
{
	return *(int *)a - *(int *)b;
}

// uncomment for fast primitive comparisons
// #define cmp(a,b) (*(a) > *(b))

int cmp_str(const void * a, const void * b)
{
	return strcmp(*(const char **) a, *(const char **) b);
}

int cmp_float(const void * a, const void * b)
{
	return *(float *) a - *(float *) b;
}

int cmp_double(const void * a, const void * b)
{
	return *(double *) a - *(double *) b;
}


#define SORT_TEST_SIZE 100000
TEST test_sort_int(void)
{
	printf(MOD_TAG"suit_quadsort------test_sort_uint32\n");
	
	int i = 0;
	int a_array[SORT_TEST_SIZE]={0};
	int v_array[SORT_TEST_SIZE]={0};
	
	for (i = 0 ; i < SORT_TEST_SIZE ; i++)
	{
		a_array[i] = wow_random_value();
	}

	memcpy(v_array,a_array,SORT_TEST_SIZE*sizeof(int));
	quadsort(a_array, SORT_TEST_SIZE, sizeof(int), cmp_int);
	qsort(v_array, SORT_TEST_SIZE, sizeof(int), cmp_int);

	for (i = 1 ; i < SORT_TEST_SIZE ; i++)
	{
		GREATEST_ASSERT(v_array[i - 1] <= v_array[i]);
		GREATEST_ASSERT(a_array[i] == v_array[i]);
	}

	PASS();
}


TEST test_sort_float(void)
{
	printf(MOD_TAG"suit_quadsort------test_sort_float\n");

	int i = 0;
	float a_array[SORT_TEST_SIZE]={0};
	float v_array[SORT_TEST_SIZE]={0};
	
	for (i = 0 ; i < SORT_TEST_SIZE ; i++)
	{
		a_array[i] =(float)(wow_random_value()/MAXS32);
	}

	memcpy(v_array,a_array,SORT_TEST_SIZE*sizeof(float));
	quadsort(a_array, SORT_TEST_SIZE, sizeof(float), cmp_float);
	qsort(v_array, SORT_TEST_SIZE, sizeof(float), cmp_float);

	for (i = 1 ; i < SORT_TEST_SIZE ; i++)
	{
		GREATEST_ASSERT(v_array[i - 1] <= v_array[i]);
		GREATEST_ASSERT(a_array[i] == v_array[i]);
	}

	PASS();
}

TEST test_sort_double(void)
{
	printf(MOD_TAG"suit_quadsort------test_sort_double\n");
	int i = 0;
	double a_array[SORT_TEST_SIZE]={0};
	double v_array[SORT_TEST_SIZE]={0};
	
	for (i = 0 ; i < SORT_TEST_SIZE ; i++)
	{
		a_array[i] =(float)(wow_random_value()/MAXS32);
	}

	memcpy(v_array,a_array,SORT_TEST_SIZE*sizeof(double));
	quadsort(a_array, SORT_TEST_SIZE, sizeof(double), cmp_double);
	qsort(v_array, SORT_TEST_SIZE, sizeof(double), cmp_double);

	for (i = 1 ; i < SORT_TEST_SIZE ; i++)
	{
		GREATEST_ASSERT(v_array[i - 1] <= v_array[i]);
		GREATEST_ASSERT(a_array[i] == v_array[i]);
	}

	PASS();

}

TEST test_sort_string(void)
{
	printf(MOD_TAG"suit_quadsort------test_sort_string\n");

	int i=  0;
	char **sa_array = (char **) malloc(SORT_TEST_SIZE * sizeof(char *));
	char **sr_array = (char **) malloc(SORT_TEST_SIZE * sizeof(char *));
	char *buffer = (char *) malloc(SORT_TEST_SIZE * 16);

	for (i = 0 ; i < SORT_TEST_SIZE ; i++)
	{
		sprintf(buffer + i * 16, "%X", (int)(wow_random_value() % 1000000));
		sa_array[i] = buffer + i * 16;
	}

	//memcpy(sa_array, sr_array, SORT_TEST_SIZE * sizeof(char *));
	quadsort(sa_array, SORT_TEST_SIZE, sizeof(char *), cmp_str);
	//qsort(sr_array, SORT_TEST_SIZE, sizeof(char *), cmp_str);
	
	for (i = 1 ; i < SORT_TEST_SIZE ; i++)
	{
		//printf("%s\n",sa_array[i]);
	}

	free(sa_array);
	free(sr_array);
	free(buffer);
		
	PASS();
}

SUITE(suit_quadsort)
{
    RUN_TEST(test_sort_int);
	RUN_TEST(test_sort_float);
	RUN_TEST(test_sort_double);
	RUN_TEST(test_sort_string);
}



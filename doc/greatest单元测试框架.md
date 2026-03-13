# greatest单元测试框架

源码地址：https://github.com/silentbicycle/greatest
Greatest是一个极简的C语言单元测试框架，具有小巧、易用、可移植的特点，非常适合于嵌入式系统和资源受限环境下的单元测试。

## 特点：

- 轻量级：Greatest的核心代码非常小巧，只包含了必要的功能，因此它非常适合在资源受限的环境中使用，比如嵌入式系统或者具有有限内存和处理能力的设备。
- 简单易用：Greatest提供了一个简洁明了的API，使得编写和运行单元测试变得非常简单。它的API设计遵循了KISS（保持简单原则），让开发者可以专注于测试代码的编写而不是框架本身的学习和配置。
- 可移植性：Greatest不依赖于任何特定的平台或者库，因此可以在各种C语言环境中使用，无论是嵌入式系统还是桌面应用程序。这种可移植性使得开发者可以在不同的项目中方便地重复使用单元测试代码。
- 无依赖性：Greatest框架本身不依赖于任何其他外部库，这意味着在使用Greatest进行单元测试时，不需要引入额外的依赖关系，从而减少了项目的复杂性和维护成本。

## 使用方法：

1. 包含头文件：在测试文件中包含greatest.h头文件，这样就可以使用Greatest提供的API。
2. 定义测试用例：使用TEST宏定义测试用例，并在其中编写测试代码。每个测试用例都应该是一个独立的函数，用于测试被测代码的特定功能或行为。
3. 运行测试：编写RUN_TEST宏来运行测试用例。这样可以确保所有的测试用例都会被执行，并且可以收集和报告测试结果。
4. 断言：使用Greatest提供的断言宏来验证测试结果。断言宏通常用于检查被测代码的输出是否符合预期，例如ASSERT_EQ用于比较两个值是否相等，ASSERT_TRUE用于验证表达式是否为真等等。

## 简单单元测试实例：

	#include <greatest.h>

	TEST test_example() {
		int result = 1 + 1;
		ASSERT_EQ(result, 2);
		PASS();
	}

	int main(int argc, char **argv) {
		GREATEST_MAIN_BEGIN();
		RUN_TEST(test_example);
		GREATEST_MAIN_END();
	}
	
## 多模块测试红实例：
#### main文件代码

	#include "greatest/greatest.h"

	/*file*/
	SUITE_EXTERN(suit_dir);
	SUITE_EXTERN(suit_file);
	/*utils*/
	SUITE_EXTERN(suit_hex);
	SUITE_EXTERN(suit_ring_buff);
	SUITE_EXTERN(suit_str);
	SUITE_EXTERN(suit_type);

	GREATEST_MAIN_DEFS(); 
	int main(int argc,char* argv[])
	{
		GREATEST_MAIN_BEGIN();	

		/*utils*/	
		RUN_SUITE(suit_dir);
		RUN_SUITE(suit_file);

		/*utils*/	
		RUN_SUITE(suit_hex);
		RUN_SUITE(suit_ring_buff);
		RUN_SUITE(suit_str);
		RUN_SUITE(suit_type);

		GREATEST_MAIN_END();
	}

#### 模块文件代码

	#include "greatest/greatest.h"

	#define TEST_DIR_PATH_01 "file/1/1/1/"
	#define TEST_DIR_PATH_02 "file/1/1/2/"

	TEST test_dir_create(void)
	{
		int  ret = -1;
		printf(MOD_TAG"suit_dir----test_dir_create\n");

		ret = wow_dir_create(WOW_TEST_FILE_STRING(TEST_DIR_PATH_01));
		GREATEST_ASSERT(ret == 0);

		ret = wow_dir_create(WOW_TEST_FILE_STRING(TEST_DIR_PATH_02));
		GREATEST_ASSERT(ret == 0);

		PASS();
	}

	TEST test_dir_remove(void)
	{
		int  ret = -1;
		printf(MOD_TAG"suit_dir----test_dir_remove\n");

		ret = wow_dir_remove(WOW_TEST_FILE_STRING(TEST_DIR_PATH_02));
		GREATEST_ASSERT(ret == 0);

		PASS();
	}

	SUITE(suit_dir)
	{
		RUN_TEST(test_dir_create);
		RUN_TEST(test_dir_remove);
	}

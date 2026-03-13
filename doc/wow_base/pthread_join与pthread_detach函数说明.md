# pthread_join与pthread_detach函数说明
pthread_join 和 pthread_detach 都是用来管理线程的函数，主要用于控制线程的结束和资源释放。

## pthread_join函数
pthread_join 函数用于等待一个线程的结束，并获取其返回值（如果有）。当一个线程调用 pthread_join 来等待另一个线程时，调用线程会被阻塞，直到目标线程结束为止。函数原型：

	int pthread_join(pthread_t thread, void **retval);
	thread：要等待的线程的标识符。
	retval：用于存储目标线程的返回值，可以传入 NULL。
### 实例
	#include <stdio.h>
	#include <pthread.h>
	#include <unistd.h>

	void *thread_function(void *arg) {
		int *value = (int *)arg;
		printf("Thread started with argument: %d\n", *value);
		sleep(3); // 模拟线程执行一段时间
		*value = 100; // 修改传入的值
		pthread_exit(NULL);
	}

	int main() {
		pthread_t thread;
		int value = 42;

		pthread_create(&thread, NULL, thread_function, &value);

		// 等待线程结束
		pthread_join(thread, NULL);

		printf("Thread finished with result: %d\n", value);

		return 0;
	}
	
## pthread_detach函数
pthread_detach 函数用于将一个线程标记为“分离状态”，这样当线程结束时，系统会自动回收其资源，无需调用 pthread_join。函数原型：

	int pthread_detach(pthread_t thread);
	thread：要标记为分离状态的线程标识符。
	
### 实例
	#include <stdio.h>
	#include <pthread.h>
	#include <unistd.h>

	void *thread_function(void *arg) {
		printf("Thread started\n");
		sleep(3); // 模拟线程执行一段时间
		printf("Thread finished\n");
		pthread_exit(NULL);
	}

	int main() {
		pthread_t thread;

		pthread_create(&thread, NULL, thread_function, NULL);

		// 将线程标记为分离状态
		pthread_detach(thread);

		// 主线程继续执行其他操作
		sleep(5);

		return 0;
	}
	
## 总结
- 使用 pthread_join 当需要等待线程终止并获取其返回值时。
- 使用 pthread_detach 当不需要等待线程终止或获取其返回值时，并且希望操作系统在该线程终止后自动释放其资源。
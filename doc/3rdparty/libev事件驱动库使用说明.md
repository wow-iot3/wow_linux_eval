# libev事件驱动库使用说明

libev 是一个轻量级、跨平台的事件驱动编程的高性能库，提供了事件循环、定时器、信号处理等功能，类似于其他事件驱动库如libevent和libuv。libev 的设计目标是简单、高效，尽量减少不必要的内存分配和拷贝操作，适用于高性能的网络服务器、实时数据处理等场景。

## 使用方法：
要使用 libev，请执行以下步骤：
(1) 创建事件循环：创建一个 ev_loop 结构，它代表事件循环。
(2) 创建事件观察器：对于每个要监视的事件源（如文件描述符、定时器等），创建一个 ev_io、ev_timer 或 ev_signal 结构，并将其添加到事件循环中。
(3)设置回调函数：为每个事件观察器设置一个回调函数，当相应的事件发生时调用。
(4)启动事件循环：调用 ev_run 函数来启动事件循环。它将持续运行，直到没有更多事件需要处理或调用 ev_break 函数。

##  libev 创建定时器：

	#include <ev.h>
	#include <stdio.h>

	// 定时器回调函数
	static void timer_cb(EV_P_ ev_timer *timer, int revents) {
		printf("Timer event\n");
		// 停止事件循环
		ev_break(EV_A_ EVBREAK_ALL);
	}

	int main() {
		// 创建一个事件循环
		struct ev_loop *loop = EV_DEFAULT;
		// 创建一个定时器对象，并设置定时器回调函数
		ev_timer timer;
		// 每隔2秒触发一次定时器
		ev_timer_init(&timer, timer_cb, 2.0, 0.0); 
		ev_timer_start(loop, &timer);
		// 开始事件循环
		ev_run(loop, 0);

		return 0;
	}

##  libev 创建IO事件：

	#include <ev.h>
	#include <stdio.h>

	// IO事件回调函数
	static void io_cb(EV_P_ ev_io *w, int revents) {
		printf("IO event occurred\n");
	}

	int main() {
		// 创建事件循环实例
		struct ev_loop *loop = ev_default_loop(0);
		// 创建IO事件监控器
		ev_io io_watcher;
		ev_io_init(&io_watcher, io_cb, STDIN_FILENO, EV_READ);
		ev_io_start(loop, &io_watcher);
		// 启动事件循环
		ev_run(loop, 0);

		return 0;
	}


## 应用：
- 网络服务器和客户端：处理来自多个客户端的并发连接。
- 事件驱动的应用程序：如 GUI 应用程序和消息传递系统。
- 可扩展后端服务：处理大量并发请求。
- 数据处理管道：连接多个处理组件，以高效地处理数据流。
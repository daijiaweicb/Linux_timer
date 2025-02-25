#include "Timer.h"

void HighPrecisionTimer::start(int millisecs, Callback cb) {
    if (running.exchange(true)) return;

    // 配置定时参数
    struct itimerspec its;
    const int sec = millisecs / 1000;
    const int nsec = (millisecs % 1000) * 1000000;

    its.it_value.tv_sec = sec;
    its.it_value.tv_nsec = nsec;
    its.it_interval.tv_sec = sec;    // 循环间隔
    its.it_interval.tv_nsec = nsec;

    if (timerfd_settime(fd, 0, &its, nullptr) == -1) {
        throw std::runtime_error("timerfd_settime failed: " + std::string(strerror(errno)));
    }

    // 启动工作线程
    worker = std::thread([this, cb]() {
        setThreadPriority();
        eventLoop(cb);
    });
}

void HighPrecisionTimer::stop() 
{
    if (!running.exchange(false)) return;
    
    // 停止定时器
    struct itimerspec its{};
    timerfd_settime(fd, 0, &its, nullptr);
    
    if (worker.joinable()) {
        worker.join();
    }
}

void HighPrecisionTimer:: eventLoop(const Callback& cb) {
    constexpr size_t buf_size = sizeof(uint64_t);
    uint64_t exp;

    while (running) {
        ssize_t bytes = read(fd, &exp, buf_size);
        
        // 处理信号中断
        if (bytes == -1 && errno == EINTR) {
            continue;
        }

        // 错误处理
        if (bytes != buf_size) {
            std::cerr << "Timer read error: " << strerror(errno) << std::endl;
            break;
        }

        // 触发回调
        if (exp > 0 && cb) {
            try {
                cb();
            } catch (const std::exception& e) {
                std::cerr << "Timer callback error: " << e.what() << std::endl;
            }
        }
    }
}

void HighPrecisionTimer:: setThreadPriority() {
    // 设置实时线程优先级
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        std::cerr << "Warning: Failed to set realtime priority (" 
                  << strerror(errno) << ")" << std::endl;
    }
}
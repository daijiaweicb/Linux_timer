#include <iostream>
#include "Timer.h"

using namespace std;

int main() {
    try {
        HighPrecisionTimer timer;
        int counter = 0;

        // 启动100Hz定时器（10ms间隔）
        timer.start(1000, [&]() {
            std::cout << "Timer event " << ++counter 
                      << " at " << std::chrono::high_resolution_clock::now().time_since_epoch().count()
                      << " ns\n";
        });

        // 运行5秒
        std::this_thread::sleep_for(std::chrono::seconds(5));
        timer.stop();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
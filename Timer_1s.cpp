#include <iostream>
#include "Timer.h"

int main()
{
    HighPrecisionTimer timer_1s;
    int count = 0;

    timer_1s.start(1000,[&]()
    {
        std :: cout << "Time event: " << ++count<< std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    timer_1s.stop();

    return 0;
}
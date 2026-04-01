#include <string>

#include "demo/httpserver/bootstrap/HttpServerDemoLauncher.h"
#include "demo/modbusmaster/bootstrap/ModbusMasterDemoLauncher.h"
#include "demo/timer_test/TimerDemoLauncher.h"

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        const std::string mode = argv[1];

        if (mode == "modbus")
        {
            return run_modbusmaster_demo();
        }

        if (mode == "http")
        {
            return run_httpserver_demo();
        }

        if (mode == "timer")
        {
            return run_timer_demo();
        }
    }

    return run_httpserver_demo();
}

#pragma once

#include "ssd1306Display.h"

class DisplayCtl{
public:

    static DisplayCtl& GetInstance() {
        return instance;
    }

    Ssd1306Display* GetDisplay() {
        static Ssd1306Display display;
        return &display;
    }

private:
    static DisplayCtl instance;

};

DisplayCtl DisplayCtl::instance;



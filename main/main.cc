#include <stdio.h>
#include "application.hpp"
#include "buttonCtl.hpp"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "esp_idf_version.h"
#include "sdCard.hpp"
#include "musicCtl.hpp"
#include "displayCtl.hpp"
#include "lrcFileCtl.hpp"

extern "C" void app_main(void)
{
    // 挂载 SD 卡
    SdCard::GetInstance().Init();
    //  初始化 button
    ButtonCtl::GetInstance().InitButtons();
    // 读取音乐列表
    MusicCtl::GetInstance().LoadFileNames();
    // 显示
    DisplayCtl::GetInstance().GetDisplay()->SetIcon("pause");
    std::string name = MusicCtl::GetInstance().GetName(0);
    DisplayCtl::GetInstance().GetDisplay()->SetName(name);
    // 读取歌词
    LrcFileCtl::GetInstance().Init(0);
    LrcFileCtl::GetInstance().PrintLrcs();
    // 启动 Application
    Application::GetInstance().AppStart();
}

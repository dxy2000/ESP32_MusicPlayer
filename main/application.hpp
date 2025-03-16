#pragma once

#include <stdio.h>
#include <memory>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wavFile.hpp"
#include "audioCtl.hpp"

class Application{
    public:
    
        static Application& GetInstance() {
            return instance;
        }
    
        void AppStart(){
            xTaskCreate(PlayMusic, "main_loop", 4096 * 2, this, 2, nullptr);
        }
    
        static void PlayMusic(void* arg){
           
            // 读取、解析 wav 文件
            WavFile::Ptr wavPtr = std::make_shared<WavFile>();
            if(!wavPtr->OpenWav("/sdcard/music/0.wav")){
                return;
            }
            
            // 初始化 Output
            AudioCtl::GetInstance().InitOutput(wavPtr);
            
            while(true){
                AudioCtl::GetInstance().Playing();
            }
            
            vTaskDelay(pdMS_TO_TICKS(800));
        }
    
    private:
        static Application instance;
};
Application Application::instance;
#pragma once

#include "wavFile.hpp"
#include "musicCtl.hpp"
#include "i2sOutput.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2s_std.h"
#include "math.h"
#include "displayCtl.hpp"
#include "lrcFileCtl.hpp"

static const char *AudioTAG = "AudioCtl";

// 计时器相关
long long _timer = 0;
TimerHandle_t _xTimer;

// 每 10ms 调用一次
void timerCallback(TimerHandle_t xTimer) {
    _timer += 10;
}

enum PlayStatus{play, tpause};

class AudioCtl{
public:

    static AudioCtl& GetInstance(){
        return instance;
    }

    AudioCtl(){
        playStatus_ = tpause;
        volume_ = 70;
        output_ = std::make_shared<I2sOutput>();

        // 创建定时器
        _xTimer = xTimerCreate("MyTimer", pdMS_TO_TICKS(10), pdTRUE, NULL, timerCallback);
        if (_xTimer == NULL) {
            // 定时器创建失败的处理代码
            printf("Failed to create timer\n");
        }
    }

    void InitOutput(WavFile::Ptr wavFile){
        wavFile_ = wavFile;
        output_->Init(wavFile_->GetnNumChannels(),
                            wavFile_->GetSampleRate(),
                            wavFile_->GetBitsPerSanple());
    }

    void ChangeStatus(){
        if(playStatus_ == tpause){  
            bool res = output_->EnableChannel();
            if(res){    
                // 切换显示的播放状态图标
                DisplayCtl::GetInstance().GetDisplay()->SetIcon("play");
                vTaskDelay(pdMS_TO_TICKS(10));  
                playStatus_ = play;     
                // 检查歌词是否是第一行
                bool first = LrcFileCtl::GetInstance().CheckFirst();
                if(first){
                    std::string res = LrcFileCtl::GetInstance().GetCurLrc();
                    DisplayCtl::GetInstance().GetDisplay()->SetLrc(res);
                    // std::cout << res << std::endl;
                }     
                StartTimer();  
                ESP_LOGI(AudioTAG, "\tChange status to play");
            } 
        }else if(playStatus_ == play){
            bool res = output_->DisableChannel();
            if(res){ 
                StopTimer();     
                playStatus_ = tpause;    
                // 切换显示的播放状态图标
                DisplayCtl::GetInstance().GetDisplay()->SetIcon("pause");
                ESP_LOGI(AudioTAG, "\tChange status to pause");    
            } 
        }
    }

    void VolUp(){
        volume_ += 10;
        if(volume_ >= 100){
            volume_ = 100;
        }
    }

    void VolDown(){
        volume_ -= 10;
        if(volume_ <= 0){
            volume_ = 0;
        }
    }

    // 在主循环中调用
    void Playing(){
        if(playStatus_ == play){  // 播放状态
            CheckLrc();
            FILE* fp = wavFile_->GetFp();
            int16_t buffer[1024];
            int len = fread(buffer, sizeof(int16_t), 1024, fp);
            if(len > 0){      // 歌曲没有结束,且在播放中
                double volumeFactor = pow(10, (-50 + volume_ / 2) / 20.0);
                for(int i = 0; i < len; i++){
                    buffer[i] *= volumeFactor;
                }
                if(playStatus_ == play){
                    output_->Write(buffer, len);   
                }       
            }else{                      // 歌曲结束
                NextMusic();
            }
        }else if(playStatus_ == tpause){  // 暂停状态
            ESP_LOGI(AudioTAG, "\tPause...");
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
    }

    void NextMusic(){
        ESP_LOGI(AudioTAG, "\tChange to Next Music");
        if(playStatus_ == play){   // 处于播放状态
            // 播放状态改为暂停
            ChangeStatus();
            // 关闭音乐通道
            output_->DelChannel();
            // 音乐列表跳转到下一曲
            MusicCtl::GetInstance().GotoNext();
            int next = MusicCtl::GetInstance().GetCurIndex();
            // 切换歌曲
            ChangeWaveTo(next);
            // 播放状态改为播放
            ChangeStatus();
        }else{                    // 处于暂停状态
            output_->DelChannel();
            MusicCtl::GetInstance().GotoNext();
            int next = MusicCtl::GetInstance().GetCurIndex();
            ChangeWaveTo(next);
        } 
    }

    void PreviousMusic(){
        ESP_LOGI(AudioTAG, "\tChange to Previous Music");
        if(playStatus_ == play){   // 处于播放状态
            // 播放状态改为暂停
            ChangeStatus();
            // 关闭音乐通道
            output_->DelChannel();
            // 音乐列表跳转到下一曲
            MusicCtl::GetInstance().GotoPrevious();
            int pre = MusicCtl::GetInstance().GetCurIndex();
            // 切换歌曲
            if(ChangeWaveTo(pre)){
                // 播放状态改为播放
                ChangeStatus();
            } 
        }else{                    // 处于暂停状态
            output_->DelChannel();
            MusicCtl::GetInstance().GotoPrevious();
            int pre = MusicCtl::GetInstance().GetCurIndex();
            ChangeWaveTo(pre);
        } 
    }

    // 切换歌曲
    bool ChangeWaveTo(int index){
        // 重置计时器
        _timer = 0;
        // 打开 lrc 文件
        LrcFileCtl::GetInstance().Init(index);
        vTaskDelay(pdMS_TO_TICKS(100));    // 等待读取 lrc 文件
        // 打开 wav 文件
        std::string path = "/sdcard/music/";
        path += std::to_string(index);
        path +=  ".wav";
        wavFile_->CloseWav();
        bool res = wavFile_->OpenWav(path);
        vTaskDelay(pdMS_TO_TICKS(100));    // 等待读取 wav 文件
        if(res){
            // 重新初始化 i2s 发送通道
            InitOutput(wavFile_);
            // 切换显示的歌名
            std::string name = MusicCtl::GetInstance().GetName(index);
            DisplayCtl::GetInstance().GetDisplay()->SetName(name);
        } 
        vTaskDelay(pdMS_TO_TICKS(100));    // 等待
        return true;
    }

    static void StartTimer(){
        if (xTimerStart(_xTimer, 0) != pdPASS) {
            printf("Failed to start timer\n");
        } else {
            printf("Timer Start\n");
        }
    }

    static void StopTimer(){
        std::cout << "Timer Stop!\n";
        xTimerStop(_xTimer,0);
    }

    // 检查歌词
    void CheckLrc(){
        bool last = LrcFileCtl::GetInstance().CheckLast();
        if(!last){
            long long nextTime = LrcFileCtl::GetInstance().GetNextLrcTime();
            if(_timer >= nextTime){
                LrcFileCtl::GetInstance().MoveToNextLine();
                std::string res = LrcFileCtl::GetInstance().GetCurLrc();
                DisplayCtl::GetInstance().GetDisplay()->SetLrc(res);
                // std::cout << res << std::endl;
            }
        }
    }

private:
    I2sOutput::Ptr output_;
    WavFile::Ptr wavFile_;
    int volume_;

    PlayStatus playStatus_;
    
    static AudioCtl instance;
};
AudioCtl AudioCtl::instance;
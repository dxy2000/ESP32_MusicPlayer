#pragma once

#include <iostream>
#include <stdio.h>
#include <memory>
#include <string.h>
#include <esp_log.h>

static const char *WavTAG = "WavFile";

class WavFile{
public:
    typedef std::shared_ptr<WavFile> Ptr;

    ~WavFile(){
        fclose(fp_);
    }

    void CloseWav(){
        fclose(fp_);
    }
    
    bool OpenWav(const std::string& path){
        // 以二进制格式打开 wav 音频文件
        FILE* fp = fopen(path.c_str(), "rb"); 
        if (fp == NULL) {
            ESP_LOGE(WavTAG, "\tFailed to open wav file:%s", path.c_str());
            return false;
        }
        
        // wav 文件的前 44 个字节存储到 header[] 中
        // 文件指针 fp 后移 44 个字节后，指向了音频数据
        uint8_t header[44];
        size_t bytesRead = fread(&header, sizeof(uint8_t), 44, fp);
        if(bytesRead <= 0){
            ESP_LOGE(WavTAG, "\tFail to parse wav file:%s", path.c_str());
            fclose(fp);
            return false;
        }else{
            ESP_LOGI(WavTAG, "\tSucceed to parse wav file:%s", path.c_str());     
        }

        fp_ = fp;
        sampleRate_ = *(uint32_t *)(header + 24);            // 采样率
        numChannels_ = *(uint16_t *)(header + 22);           // 声道数
        bitsPerSanple_ = *(uint32_t *)(header + 34);         // 位深度

        return true;
    }

    FILE* GetFp(){
        return fp_;
    }

    uint16_t GetnNumChannels(){
        return numChannels_;
    }

    uint32_t GetSampleRate(){
        return sampleRate_;
    }

    uint16_t GetBitsPerSanple(){
        return bitsPerSanple_;
    }

private:
    FILE *fp_;
    uint16_t numChannels_;          
    uint32_t sampleRate_;           
    uint16_t bitsPerSanple_;        
};
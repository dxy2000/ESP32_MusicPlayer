#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "driver/i2s_std.h"
#include <esp_log.h>
#include "esp_check.h"
#include "config.h"

static i2s_chan_handle_t _i2s_ch_tx; // I2S 发送通道

static const char *OutputTAG = "Output";

class I2sOutput{
public:

    typedef std::shared_ptr<I2sOutput> Ptr;

	// 初始化 I2S 发送通道，并使能（不使能不能播放）
    void Init(const uint16_t& numChannels = I2S_SLOT_MODE_STEREO, 
                const uint32_t& sampleRate = 16000, 
                const uint16_t& bitsPerSanple = I2S_DATA_BIT_WIDTH_16BIT){

        ESP_LOGI(OutputTAG, "\tInit output channel..."); 
        
         // 1、创建通道
        chcfg_ = I2S_CHANNEL_DEFAULT_CONFIG(i2s_port_t(0), I2S_ROLE_MASTER);
        ESP_ERROR_CHECK(i2s_new_channel(&chcfg_, &_i2s_ch_tx, NULL));

        // 2、配置通道
        // 用到之前从 wav 文件头中解析出来的三个属性
        i2s_data_bit_width_t i2s_data_bit_width = I2S_DATA_BIT_WIDTH_16BIT;    // 默认
        i2s_slot_mode_t i2s_slot_mode = I2S_SLOT_MODE_STEREO;                  // 默认 
        // 根据实际的位深度进行修改
        switch(bitsPerSanple){
            case 16:
                i2s_data_bit_width = I2S_DATA_BIT_WIDTH_16BIT;
                break;
            case 24:
                i2s_data_bit_width = I2S_DATA_BIT_WIDTH_24BIT;
                break;
            case 32:
                i2s_data_bit_width = I2S_DATA_BIT_WIDTH_32BIT;
                break;                
        }
        // 根据实际的声道数进行修改
        switch(numChannels){
            case 1:
                i2s_slot_mode = I2S_SLOT_MODE_MONO;
                break;
            case 2:
                i2s_slot_mode = I2S_SLOT_MODE_STEREO;
                break;              
        }
        spkr_config_ = {
            .clk_cfg = {
                .sample_rate_hz = sampleRate, 
                .clk_src = I2S_CLK_SRC_DEFAULT,
                .mclk_multiple = I2S_MCLK_MULTIPLE_384,    
            },
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(i2s_data_bit_width, i2s_slot_mode),
            .gpio_cfg ={
                .mclk = I2S_GPIO_UNUSED,     
                .bclk = AUDIO_I2S_SPK_GPIO_BCLK,     // config.h 中定义好的
                .ws = AUDIO_I2S_SPK_GPIO_WS,        // config.h 中定义好的
                .dout = AUDIO_I2S_SPK_GPIO_DOUT,    // config.h 中定义好的
                .din = GPIO_NUM_NC,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv = false
                }
            }
        };
        ESP_ERROR_CHECK(i2s_channel_init_std_mode(_i2s_ch_tx, &spkr_config_));

        // 3、使能通道
        //ESP_ERROR_CHECK(i2s_channel_enable(_i2s_ch_tx));
    }

    // 将长度为 len 的 data 数据写入到 i2s 通道中，即播放
    void Write(int16_t* data, size_t len){
        size_t bytes_written = 0;
        ESP_ERROR_CHECK(i2s_channel_write(_i2s_ch_tx, data, len * sizeof(int16_t), &bytes_written, 1000));
    }

    bool DisableChannel(){
        ESP_LOGI(OutputTAG, "\tDisable output channel..."); 
        ESP_ERROR_CHECK(i2s_channel_disable(_i2s_ch_tx));
        return true;
    }

    bool EnableChannel(){
        ESP_LOGI(OutputTAG, "\tEnable output channel..."); 
        ESP_ERROR_CHECK(i2s_channel_enable(_i2s_ch_tx));
        return true;
    }

    bool DelChannel(){
        ESP_LOGI(OutputTAG, "\tDelete output channel..."); 
        ESP_ERROR_CHECK(i2s_del_channel(_i2s_ch_tx));
        return true;
    }

private:
    i2s_chan_config_t chcfg_;        
    i2s_std_config_t spkr_config_;

};
#pragma once

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "config.h"

#define MOUNT_POINT "/sdcard"
static const char *SDTAG = "sdCard";

class SdCard{
public:

    static SdCard& GetInstance(){
        return instance;
    }

	void Init(){
        sdmmc_host_t host = SDSPI_HOST_DEFAULT();
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = SD_PIN_MOSI,     // config.h 中定义好的
            .miso_io_num = SD_PIN_MISO,     // config.h 中定义好的
            .sclk_io_num = SD_PIN_CLK,      // config.h 中定义好的
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 400000,
        };
        esp_err_t ret = spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK) {
            ESP_LOGE(SDTAG, "Failed to initialize bus.");
            return;
        }else{
            ESP_LOGI(SDTAG, "Succeeded to initialize bus.");
        }

		const char mount_point[] = MOUNT_POINT;
        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.gpio_cs = SD_PIN_CS;  // CS 引脚，在 config.h 中已定义对应为 GPIO9
        slot_config.host_id = (spi_host_device_t)host.slot;
        sdmmc_card_t *card;
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = true,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024
        };
        ESP_LOGI(SDTAG, "Mounting filesystem");
        ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
        if (ret != ESP_OK) {
            if (ret == ESP_FAIL) {
                ESP_LOGE(SDTAG, "Failed to mount filesystem. "
                        "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
            } else {
                ESP_LOGE(SDTAG, "Failed to initialize the card (%s). "
                        "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
            }
            return;
        }
        ESP_LOGI(SDTAG, "Filesystem mounted");

        // Card has been initialized, print its properties
        sdmmc_card_print_info(stdout, card);
    }

    void GetAllFiles(const char *path){

        // 打开目录
        DIR* dir = opendir(path);
        if (dir == NULL)
        {
            std::cout << "Failed to open dir:" << path << std::endl;
            return;
        }

        // 遍历目录下的文件
        struct dirent *entry;
        struct stat statbuf;
        while ((entry = readdir(dir)) != NULL) 
        {
            // 获取 entry 的完整路径
            std::string file_path = path;
            file_path.append("/");
            file_path.append(entry->d_name);
            // 输出路径
            std::cout << file_path << std::endl;

            // 获取 entry 的属性
            if (stat(file_path.c_str(), &statbuf) == -1)
            {
                continue;
            }
            // entry 是文件夹       
            if (S_ISDIR(statbuf.st_mode)) {
                // 递归遍历子文件夹
                GetAllFiles(file_path.c_str()); 
            }
        }
        // 关闭目录
        closedir(dir);
    }

private:
    static SdCard instance;
};

SdCard SdCard::instance;
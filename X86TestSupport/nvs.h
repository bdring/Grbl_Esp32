#pragma once

#include "WString.h"
#include "esperror.h"

struct nvs_handle
{};

struct nvs_stats_t 
{
    int used_entries;
    int free_entries;
    int total_entries;
};

esp_err_t nvs_get_stats(const char* part_name, nvs_stats_t* nvs_stats);
esp_err_t nvs_erase_all(nvs_handle& handle);


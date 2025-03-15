#ifndef HEADER_FILE_ARITHE_H
#define HEADER_FILE_ARITHE_H

// 头文件内容
#include "common.h"
// like-lru
uint32_t like_lru_arithem(uint32_t addr_set, uint32_t way_num);
void like_lru_arithem_update(uint32_t set_addr, uint32_t way_addr, uint32_t way_num);
void cache_tag_like_lru_arithem_init(Cache * cache);

// lru
void cache_tag_lru_arithem_init(Cache * cache);
uint32_t lru_arithem(Cache_way* cache_way, uint32_t way_num);
void lru_arithem_update(Cache_way* cache_way, uint32_t way_addr, uint32_t way_num);

// random
uint32_t random_arithem(uint32_t way_num);
void random_arithem_update();
void cache_tag_random_arithem_init();

#endif // HEADER_FILE_ARITHE_H

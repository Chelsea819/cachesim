#include "common.h"
// 使用伪lru replace算法的辅助数组
uint32_t * lru_array_2;
uint32_t ** lru_array_4;

uint32_t like_lru_arithem(uint32_t addr_set, uint32_t way_num){
  if(way_num == 2){
    return lru_array_2[addr_set];
  }else if(way_num == 4){
    uint32_t small_set = lru_array_4[addr_set][way_num]; // 0 or 1
    uint32_t way_addr = lru_array_4[addr_set][small_set];
    return small_set * 2 + way_addr;  // 即，辅助数组里面记录的way的地址，是要淘汰的way
  }
  return 0;
}
// update like-lru arithem
void like_lru_arithem_update(uint32_t set_addr, uint32_t way_addr, uint32_t way_num){
  if(way_num == 2){
    lru_array_2[set_addr] = 1 - way_addr;
  } else if(way_num == 4){
    // way_addr = 3 -> big:0 small:
    uint32_t small_set_addr = way_addr / 2;
    uint32_t inner_way_addr = way_addr % 2;
    lru_array_4[set_addr][way_num] = 1 - small_set_addr;
    lru_array_4[set_addr][small_set_addr] = 1 - inner_way_addr;
  }
}


// 使用伪lru replace算法的支持 
void cache_tag_like_lru_arithem_init(Cache * cache){
  if(cache->config->cache_way == 2) {
    lru_array_2 = (uint32_t *)calloc(cache->config->cache_set, sizeof(uint32_t));
    assert(lru_array_2 != NULL);
    // 4-way cache teg: 1->1+1
  } else if(cache->config->cache_way == 4) {
    lru_array_4 = (uint32_t **)calloc(cache->config->cache_set, sizeof(uint32_t*));
    assert(lru_array_4 != NULL);
    for(int i = 0; i < cache->config->cache_set; i ++) {
      // big and two little
      lru_array_4[i] = calloc(cache->config->cache_way + 1, sizeof(uint32_t));
      assert(lru_array_4[i] != NULL);
    }
  }
}

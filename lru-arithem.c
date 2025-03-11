#include "common.h"

// 使用lru replace算法的支持 
void cache_tag_lru_arithem_init(Cache * cache){
  for(int i = 0; i < cache->config->cache_set; i ++){
    for(int j = 0; j < cache->config->cache_way; j ++) {
      if(j == cache->config->cache_way - 1) cache->area->cache_sets[i].cache_ways[j].other_0 = NULL;
      else cache->area->cache_sets[i].cache_ways[j].other_0 = &cache->area->cache_sets[i].cache_ways[j+1];

      if(j == 0) cache->area->cache_sets[i].cache_ways[j].other_1 = NULL;
      else cache->area->cache_sets[i].cache_ways[j].other_1 = &cache->area->cache_sets[i].cache_ways[j-1];
    }
  }
}

Cache_way *find_header_ptr(Cache_way* cache_way){
  while(cache_way->other_1 != NULL){
    cache_way = cache_way->other_1;
    Assert(cache_way != NULL, "cache_way = NULL");
  }
  // printf("cache_way: %p\n", cache_way);
  return cache_way;
}



// lru
// search for way to store data
uint32_t get_index(Cache_way* way, Cache_way* ways, uint32_t way_num){
  for(size_t i = 0; i < way_num; i ++){
    // printf("way: %p\n", way);
    // printf("ways[%ld]: %p\n", i, &ways[i]);
    if(&ways[i] == way) return i;
  }
  assert(0);
}

Cache_way *find_tail_ptr(Cache_way* cache_way){
  while(cache_way->other_0 != NULL){
    cache_way = cache_way->other_0;
    Assert(cache_way != NULL, "cache_way = NULL");
  }
  return cache_way;
}

Cache_way *find_header_ptr(Cache_way* cache_way);

uint32_t lru_arithem(Cache_way* cache_way, uint32_t way_num){
  return get_index(find_header_ptr(cache_way), cache_way, way_num);
}

void lru_arithem_update(Cache_way* cache_way, uint32_t way_addr, uint32_t way_num){
  Cache_way *tail = find_tail_ptr(cache_way);
  // 本身就是最后一个
  if(&cache_way[way_addr] == tail) return;

  tail->other_0 = &cache_way[way_addr];
  ((Cache_way *)(cache_way[way_addr].other_0))->other_1 = (Cache_way *)(cache_way[way_addr].other_1);

  // 非第一个
  if (cache_way[way_addr].other_1 != NULL) {
    ((Cache_way *)(cache_way[way_addr].other_1))->other_0 = (Cache_way *)(cache_way[way_addr].other_0);

  }
  cache_way[way_addr].other_0 = NULL;
  cache_way[way_addr].other_1 = tail;
  
}
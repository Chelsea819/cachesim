#include "common.h"
#include "arithe.h"

extern size_t cache_set;
extern size_t cache_way;
extern size_t cache_line;
extern bool batch_flag;
extern Cache cache_batch[DEFAUT_CACHE_MAX_NUM];
extern Cache *cache_ptr;
extern Cache cache_spec;

// 操作集中在一个cacheset中，传递cacheset即可和way数，返回cacheset中的way号
uint32_t choose_eliminate(Cache* cache, uint32_t way_num, uint32_t addr_set){
  Cache_way* cache_way = cache->area->cache_sets[addr_set].cache_ways;
  switch (cache->config->arithem_idx) {
  case LRU_ARITHEME:
    /* code */
    return lru_arithem(cache_way, way_num);
  case LIKE_LRU_ARITHEME:
    /* code */
    return like_lru_arithem(addr_set, way_num);

  case RANDOM_ARITHEME:
    /* code */
    return random_arithem(way_num);
  default:
    break;
  }
  return 0;
}
  

void update_tag(Cache* cache, uint32_t set_addr, uint32_t way_addr, uint32_t way_num){  
  Cache_way* cache_way = cache->area->cache_sets[set_addr].cache_ways;
  switch (cache->config->arithem_idx) {
  case LRU_ARITHEME:
    /* code */
    lru_arithem_update(cache_way, way_addr, way_num);
    break;
  case LIKE_LRU_ARITHEME:
    /* code */
    like_lru_arithem_update(set_addr, way_addr, way_num);
    break;
  case RANDOM_ARITHEME:
    /* code */
    random_arithem_update();
    break;
  default:
    break;
  }
}

// 操作集中在一个cacheset中
uint32_t load_miss_to_cache(Cache *cache_ptr, uint32_t addr_set, uint32_t addr_index){
  // choose a worst way to store data
  uint32_t way_addr = choose_eliminate(cache_ptr, cache_ptr->config->cache_way, addr_set);

  // load data
  cache_ptr->area->cache_sets[addr_set].cache_ways[way_addr].tag = addr_index;
  return way_addr;

}

// pc addr: log2(set addr)  log2(offset)
void cache_func(Cache *cache_ptr, uint32_t pc){
  bool hit_flag = false;
  uint32_t way_addr = 0;
  uint32_t a_off_w = (uint32_t)log2(cache_ptr->config->cache_line);
  uint32_t a_set_w = (uint32_t)log2(cache_ptr->config->cache_set);
  // check if hit
  // uint32_t addr_off = BITS(pc, a_off_w - 1, 0);
  uint32_t addr_set = BITS(pc, a_off_w + a_set_w - 1, a_off_w);
  uint32_t addr_index = BITS(pc, 31, a_off_w + a_set_w);

  for(int i = 0; i < cache_ptr->config->cache_way; i ++){
    if(cache_ptr->area->cache_sets[addr_set].cache_ways[i].tag == addr_index){
      hit_flag = true;  // hit
      way_addr = i;
    }
  }
  // hit
  if(hit_flag){
    cache_ptr->log->hit_time ++;
    // update hit data
    // update_hit(cache_sets[addr_set].cache_ways, hit_way_addr, cache_ptr->config->cache_way);

  } 
  // miss
  else{
    // if miss, load
    way_addr = load_miss_to_cache(cache_ptr, addr_set, addr_index);
  }
  update_tag(cache_ptr, addr_set, way_addr, cache_ptr->config->cache_way);
  
}

void cache_tag_arithem_init(Cache * cache){
  switch (cache->config->arithem_idx)
  {
  case LRU_ARITHEME:
    /* code */
    cache_tag_lru_arithem_init(cache);
    break;
  case LIKE_LRU_ARITHEME:
    /* code */
    cache_tag_like_lru_arithem_init(cache);
    break;
  case RANDOM_ARITHEME:
    /* code */
    cache_tag_random_arithem_init();
    break;
  default:
    break;
  }
  
}

void cache_area_init(Cache * cache){
  Cache_set *sets = (Cache_set *)malloc(sizeof(Cache_set)*cache->config->cache_set);
  Assert(sets != NULL, "Failed to allocate memory for Cache_set array");

  for(int i = 0; i < cache->config->cache_set; i ++){
    sets[i].cache_ways = (Cache_way *)malloc(sizeof(Cache_way) * cache->config->cache_way);
    Assert(sets[i].cache_ways != NULL, "Failed to allocate memory for Cache_way array");
  }

  cache->area = (Cache_area *)malloc(sizeof(Cache_area));
  Assert(cache->area != NULL, "Failed to allocate memory for Cache_area");

  cache->area->cache_sets = sets;
}
uint32_t default_cache_num = 0;
void cache_init(){
  // default batch cache flag test
  // Cache_config *config = NULL;
  if(batch_flag){
    for(int i = 0; i < DEFAUT_CACHE_MAX_NUM; i ++){
      cache_batch[i].config = (Cache_config *)malloc(sizeof(Cache_config));
    }
    int idx = 0;
    *(cache_batch[idx++].config) = (Cache_config){4,2,16, LIKE_LRU_ARITHEME};
    *(cache_batch[idx++].config) = (Cache_config){4,2,16, RANDOM_ARITHEME};
    *(cache_batch[idx++].config) = (Cache_config){8,1,16, NON_ARITHEME};
    *(cache_batch[idx++].config) = (Cache_config){4,4,8, LIKE_LRU_ARITHEME};
    *(cache_batch[idx++].config) = (Cache_config){4,4,8, RANDOM_ARITHEME};
    *(cache_batch[idx++].config) = (Cache_config){8,1,16, NON_ARITHEME};

    default_cache_num = idx;
    for(int i = 0; i < default_cache_num; i ++){
      cache_area_init(&cache_batch[i]);
      cache_tag_arithem_init(&cache_batch[i]);
      cache_batch[i].log = (Report *)malloc(sizeof(Report));
      if(i < default_cache_num-1)
        cache_batch[i].next = &cache_batch[i+1];
      else 
        cache_batch[i].next = NULL;
    }
    cache_ptr = cache_batch;
  // specific cache test
  } else{
    cache_ptr = &cache_spec;
    cache_spec.config = (Cache_config *)malloc(sizeof(Cache_config));
    *(cache_spec.config) = (Cache_config){cache_set, cache_way, cache_line};
    cache_spec.next = NULL;
    cache_ptr->log = (Report *)malloc(sizeof(Report));
    // init cache area
    cache_area_init(&cache_spec);
    cache_tag_arithem_init(&cache_spec);
  }
}

void cache_destroy(){
  while (cache_ptr != NULL)
  {
    Assert(cache_ptr->area != NULL, "Cache->area is NULL!");
    Assert(cache_ptr->config != NULL, "Cache->config is NULL!");
    Assert(cache_ptr->area->cache_sets != NULL, "Cache->area->sets is NULL!");

    // 释放每个Cache_set中的cache_ways
    for (int i = 0; i < cache_ptr->config->cache_set; i++) {
      Assert(cache_ptr->area->cache_sets[i].cache_ways != NULL, "Cache->area->sets->ways is NULL!");
      free(cache_ptr->area->cache_sets[i].cache_ways);
      cache_ptr->area->cache_sets[i].cache_ways = NULL;
    }
    
    free(cache_ptr->area->cache_sets);
    cache_ptr->area->cache_sets = NULL;
    free(cache_ptr->area);
    cache_ptr->area = NULL;

    
    free(cache_ptr->config);

    cache_ptr = cache_ptr->next;
  }
  
}
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <regex.h>
#include <math.h>
#include <debug.h>

//位抽取
#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
#define CACHE_SIZE 128
#define READ_FILE_BUFFER 16
#define DEFAUT_CACHE_NUM 5

size_t cache_set = 0;
size_t cache_way = 0;
size_t cache_line = 0;
FILE *log_fp = NULL;
static char *log_file = NULL;
bool batch_flag = false;

typedef struct cache_config{
  size_t cache_set;
  size_t cache_way;
  size_t cache_line;
} Cache_config;

typedef struct cache_way{
  uint32_t tag;
  // bool valid;
  void *other_0;
  void *other_1;
} Cache_way;

typedef struct cache_set{
  Cache_way* cache_ways;
}Cache_set;

typedef struct cache_area{
  Cache_set* cache_sets;
} Cache_area;

typedef struct report{
  uint64_t hit_time;
  uint64_t total_time;
  float hit_percent;
} Report;


typedef struct cache{
  Cache_config *config;
  Cache_area *area;
  struct cache *next;
  Report *log;
} Cache;


Cache *cache_ptr;
Cache cache_batch[DEFAUT_CACHE_NUM];
Cache cache_spec;

uint32_t string_to_uint(const char * str){
  char *endPtr; // 用于检查转换是否成功
  errno = 0;  // 清除errno 以检查错误
  uint32_t num = (uint32_t)strtoul(str, &endPtr, 16);
  //check if translate successfully
  Assert(errno != 0 || *endPtr != '\0', "Conversion failed");
  return num;
}

void open_log(){
  Assert(log_file, "Log file is NULL!");
  log_fp = fopen(log_file, "r+");
  Assert(log_fp, "Can not open '%s'", log_file);
  printf("Open log file %s\n", log_file);
}

void cache_tag_lru_arithem_init(Cache * cache);
void cache_tag_arithem_init(Cache * cache){
  cache_tag_lru_arithem_init(cache);
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

void cache_init(){
  // default batch cache flag test
  // Cache_config *config = NULL;
  if(batch_flag){
    for(int i = 0; i < DEFAUT_CACHE_NUM; i ++){
      cache_batch[i].config = (Cache_config *)malloc(sizeof(Cache_config));
    }
    *(cache_batch[0].config) = (Cache_config){16,1,16};
    *(cache_batch[1].config) = (Cache_config){8,1,16};
    *(cache_batch[2].config) = (Cache_config){64,1,4};
    *(cache_batch[3].config) = (Cache_config){16,1,8};
    *(cache_batch[4].config) = (Cache_config){32,1,8};
    // *(cache_batch[5].config) = (Cache_config);
    for(int i = 0; i < DEFAUT_CACHE_NUM; i ++){
      cache_area_init(&cache_batch[i]);
      cache_tag_arithem_init(&cache_batch[i]);
      cache_batch[i].log = (Report *)malloc(sizeof(Report));
      if(i < DEFAUT_CACHE_NUM-1)
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

//解析命令行参数
// nr_way -w nr_set -s line_sz -l
static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"set"      , required_argument, NULL, 's'},
    {"way"     , required_argument, NULL, 'w'},
    {"line"     , required_argument, NULL, 'l'},
    {"log"     , required_argument, NULL, 'f'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bs:w:l:f:", table, NULL)) != -1) {
    //参数个数 参数数组 短选项列表 长选项列表 处理长选项时返回选项的索引
    switch (o) {
      case 'b': batch_flag = true; break;
      case 's': cache_set = atoi(optarg); break;
      case 'w': cache_way = atoi(optarg); break;
      case 'l': cache_line = atoi(optarg); break;
      case 'f': log_file = optarg; break;
      case 1: return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

// lru
// search for way to store data

uint32_t get_index(Cache_way* way, Cache_way* ways, uint32_t way_num){
  for(size_t i = 0; i < way_num; i ++){
    if(&ways[i] == way) return i;
  }
  assert(0);
}

Cache_way *find_header_ptr(Cache_way* cache_way);
uint32_t lru_arithem(Cache_way* cache_way, uint32_t way_num){
  return get_index(find_header_ptr(cache_way), cache_way, way_num);
}

Cache_way *find_header_ptr(Cache_way* cache_way){
  while(cache_way->other_1 != NULL){
    cache_way = cache_way->other_1;
    Assert(cache_way != NULL, "cache_way = NULL");
  }
  return cache_way;
}

Cache_way *find_tail_ptr(Cache_way* cache_way){
  while(cache_way->other_0 != NULL){
    cache_way = cache_way->other_0;
    Assert(cache_way != NULL, "cache_way = NULL");
  }
  return cache_way;
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
// 使用伪lru replace算法的支持 

void cache_tag_lru_arithem_init(Cache * cacn,he){
  for(int i = 0; i < cache->config->cache_set; i ++){
    for(int j = 0; j < cache->config->cache_way; j ++) {
      if(j == cache->config->cache_way - 1) cache->area->cache_sets[i].cache_ways[j].other_0 = NULL;
      else cache->area->cache_sets[i].cache_ways[j].other_0 = &cache->area->cache_sets[i].cache_ways[j+1];

      if(j == 0) cache->area->cache_sets[i].cache_ways[j].other_1 = NULL;
      else cache->area->cache_sets[i].cache_ways[j].other_1 = &cache->area->cache_sets[i].cache_ways[j-1];
    }
  }
}


// 使用lru replace算法的支持 
void cache_tag_lru_arithem_init(Cache * cacn,he){
  for(int i = 0; i < cache->config->cache_set; i ++){
    for(int j = 0; j < cache->config->cache_way; j ++) {
      if(j == cache->config->cache_way - 1) cache->area->cache_sets[i].cache_ways[j].other_0 = NULL;
      else cache->area->cache_sets[i].cache_ways[j].other_0 = &cache->area->cache_sets[i].cache_ways[j+1];

      if(j == 0) cache->area->cache_sets[i].cache_ways[j].other_1 = NULL;
      else cache->area->cache_sets[i].cache_ways[j].other_1 = &cache->area->cache_sets[i].cache_ways[j-1];
    }
  }
}

// 操作集中在一个cacheset中，传递cacheset即可和way数，返回cacheset中的way号
uint32_t choose_eliminate(Cache_way* cache_way, uint32_t way_num){
  return lru_arithem(cache_way, way_num);
}

void update_tag(Cache_way* cache_way, uint32_t way_addr, uint32_t way_num){
  lru_arithem_update(cache_way, way_addr, way_num);
}

// 操作集中在一个cacheset中
uint32_t load_miss_to_cache(Cache *cache_ptr, uint32_t addr_set, uint32_t addr_index){
  // choose a worst way to store data
  uint32_t way_addr = choose_eliminate(cache_ptr->area->cache_sets[addr_set].cache_ways, cache_ptr->config->cache_way);

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
  update_tag(cache_ptr->area->cache_sets[addr_set].cache_ways, way_addr, cache_ptr->config->cache_way);

  
}

int main(int argc, char *argv[]){
    // 命令行参数处理
    parse_args(argc,argv);

    // cache config setting
    cache_init();

    // 文件打开
    open_log();

    // 不同cache配置，最外层循环
    printf("Cache\tset_num\tway_num\tcache_line\tcache_size\thit_percent\t\n");
    int index = 0;
    Cache * cache_con = cache_ptr;
    while(cache_con != NULL){
      // 文件读取，这是内层循环，读取文件数据，模拟cache的行为
      char buffer[READ_FILE_BUFFER] = {0};
      // 读取文件，拿到当前pc值
      fseek(log_fp, 0, SEEK_SET); // 移动到文件开头
      while(fgets(buffer, sizeof(buffer), log_fp)){
        cache_con->log->total_time ++;
        uint32_t pc = string_to_uint(&buffer[2]);
        cache_func(cache_con, pc);
      }
      cache_con->log->hit_percent = (float)cache_con->log->hit_time / cache_con->log->total_time * 100;
      printf("%d\t%ld\t%ld\t%ld\t%ldB\t%f%%\n", index++, cache_con->config->cache_set, cache_con->config->cache_way, cache_con->config->cache_line, cache_con->config->cache_line*cache_con->config->cache_way*cache_con->config->cache_set, cache_con->log->hit_percent);
      cache_con = cache_con->next;
    }

    // while(cache_con != NULL){
    //   printf("%d\t%ld\t%ld\t%ld\t\n", index++, cache_con->config->cache_set, cache_con->config->cache_way, cache_con->config->cache_line);
    //   cache_con->log->hit_percent = (float)cache_con->log->hit_time / cache_con->log->total_time;
    //   cache_con = cache_con->next;
    // }

    // free cache area
    cache_destroy();

    // close log file 
    fclose(log_fp);

    // 性能统计输出

    return 0;  
}

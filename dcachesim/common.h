#ifndef HEADER_FILE_COMMON_H
#define HEADER_FILE_COMMON_H

// 头文件内容
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
#define DEFAUT_CACHE_MAX_NUM 20


#define LRU_ARITHEME      1
#define LIKE_LRU_ARITHEME 2
#define RANDOM_ARITHEME   3
#define NON_ARITHEME      4

typedef struct cache_config{
  size_t cache_set;
  size_t cache_way;
  size_t cache_line;
  uint32_t arithem_idx;
} Cache_config;

typedef struct cache_way{
  uint32_t tag;
  // bool valid;
  void *other_0;
  void *other_1;
} Cache_way;

typedef struct cache_set{
  Cache_way* cache_ways;
  uint32_t * lru_array_2;
  uint32_t ** lru_array_4;
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

#endif // HEADER_FILE_COMMON_H


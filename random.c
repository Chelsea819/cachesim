#include "common.h"
#include <time.h>

uint32_t random_num(uint32_t way_num) {
    // 使用当前时间作为随机数生成器的种子
    srand((unsigned int)time(NULL));

    // 生成一个特定范围内的随机数
    int random_number_in_range = rand() % way_num; // 取模运算得到0到way_num的随机数

    return random_number_in_range;
}

uint32_t random_arithem(uint32_t way_num){
  return random_num(way_num);
}
// update like-lru arithem
void random_arithem_update(){
  return;
}


// 使用伪lru replace算法的支持 
void cache_tag_random_arithem_init(){
  return;
}

#include "common.h"
#include <time.h>
#include <string.h>
void cache_func(Cache *cache_ptr, uint32_t pc);
void cache_init();
void cache_destroy();

size_t cache_set = 0;
size_t cache_way = 0;
size_t cache_line = 0;
FILE *log_fp = NULL;
static char *log_file = NULL;
bool batch_flag = false;

Cache *cache_ptr;
Cache cache_batch[DEFAUT_CACHE_MAX_NUM];
Cache cache_spec;
 
uint32_t string_to_uint(const char * str){
  // printf("string: [%s]\n", str);
  char *endPtr; // 用于检查转换是否成功
  errno = 0;  // 清除errno 以检查错误
  uint32_t num = (uint32_t)strtoul(str, &endPtr, 16);
  // printf("num: [%d]\n", num);
  //check if translate successfully
  // Assert(errno != 0 || *endPtr != '\0', "Conversion failed");
  return num;
}

const char* get_algorithm_name(int var) {
    switch (var) {
        case LRU_ARITHEME:
            return "LRU";
        case LIKE_LRU_ARITHEME:
            return "LIKE_LRU";
        case RANDOM_ARITHEME:
            return "RANDOM";
        case NON_ARITHEME:
            return "NONE";
        default:
            return "UNKNOWN";
    }
}

void open_log(){
  Assert(log_file, "Log file is NULL!");
  log_fp = fopen(log_file, "r+");
  Assert(log_fp, "Can not open '%s'", log_file);
  printf("Open log file %s\n", log_file);
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




int main(int argc, char *argv[]){
    srand((unsigned int)time(NULL));
    // 命令行参数处理
    parse_args(argc,argv);

    // cache config setting
    cache_init();

    // 文件打开
    open_log();

    // 不同cache配置，最外层循环
    printf("Cache\tset_num\tway_num\tcache_line\tcache_size\treplace-arithem\thit_percent\t\n");
    int index = 0;
    Cache * cache_con = cache_ptr;
    while(cache_con != NULL){
      // 文件读取，这是内层循环，读取文件数据，模拟cache的行为
      char buffer[READ_FILE_BUFFER] = {0};
      // 读取文件，拿到当前pc值
      fseek(log_fp, 0, SEEK_SET); // 移动到文件开头
      while(fgets(buffer, sizeof(buffer), log_fp)){
        cache_con->log->total_time ++;
        uint32_t addr = string_to_uint(&buffer[2]);
        cache_func(cache_con, addr);
      }
      cache_con->log->hit_percent = (float)cache_con->log->hit_time / cache_con->log->total_time * 100;
      printf("%d\t%ld\t%ld\t%ld\t\t%ldB\t\t%s\t\t%f%%\n", index++, cache_con->config->cache_set, cache_con->config->cache_way, cache_con->config->cache_line, cache_con->config->cache_line*cache_con->config->cache_way*cache_con->config->cache_set, get_algorithm_name(cache_con->config->arithem_idx), cache_con->log->hit_percent);
      // printf("cache_con->log->total_time=%lx\n",cache_con->log->total_time);
      cache_con = cache_con->next;
    }

    // free cache area
    cache_destroy();

    // close log file 
    fclose(log_fp);

    // 性能统计输出

    return 0;  
}

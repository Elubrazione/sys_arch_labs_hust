/*
 *csim.c-使用C编写一个Cache模拟器，它可以处理来自Valgrind的跟踪和输出统计
 *息，如命中、未命中和逐出的次数。更换政策是LRU。
 * 设计和假设:
 *  1. 每个加载/存储最多可导致一个缓存未命中。（最大请求是8个字节。）
 *  2. 忽略指令负载（I），因为我们有兴趣评估trace.c内容中数据存储性能。
 *  3. 数据修改（M）被视为加载，然后存储到同一地址。因此，M操作可能导致两次缓存命中，或者一次未命中和一次命中，外加一次可能的逐出。
 * 使用函数printSummary() 打印输出，输出hits, misses and evictions 的数，这对结果评估很重要
 */
#include "cachelab.h"
 //                    请在此处添加代码  
//****************************Begin*********************
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

//#define DEBUG_ON
#define ADDRESS_LENGTH 64

/* 类型：内存地址 */
typedef unsigned long long int mem_addr_t;

/* 类型：Cache行
LRU是用于实现LRU替换策略的计数器*/
typedef struct cache_line {
  char valid;   //有效位
  mem_addr_t tag;    //标记位
  unsigned long long int lru;   //LRU计数器
}
cache_line_t;

typedef cache_line_t * cache_set_t;
typedef cache_set_t * cache_t;

/* 全局变量由命令行参数设置 */
int verbosity = 0; /* 如果设置，则打印跟踪 */
int s = 0; /* 组索引位数 */
int b = 0; /* 块偏移位数 */
int E = 0; /* 相联度 */
char * trace_file = NULL;

/* 从命令行参数派生 */
int S; /* 组数 */
int B; /* 块大小（字节） */

/* 用于记录缓存统计信息的计数器 */
int miss_count = 0; //缺失计数器
int hit_count = 0;   //命中计数器
int eviction_count = 0; //逐出计数器
unsigned long long int lru_counter = 1;  //LRU计数器

/* cache */
cache_t cache;
mem_addr_t set_index_mask;

/* 
 * initCache - 分配内存，将valid、tag和LRU写入0，
 * 同时计算set_index_mask
 */
void initCache() {
  if (s < 0) {
    printf("set number error!\n");
    exit(0);
  }
  cache = (cache_set_t * ) malloc(S * sizeof(cache_set_t));
  if (cache == NULL) {
    printf("No set memory!\n");
    exit(0);
  }
  for (int i = 0; i < S; ++i) {
    cache[i] = (cache_line_t * ) malloc(E * sizeof(cache_line_t)); //为行申请空间
    if (!cache[i]) {
      printf("No line memory!\n");
      exit(0);
    }
    for (int j = 0; j < E; ++j) {
      cache[i][j].lru = 0;
      cache[i][j].tag = 0;
      cache[i][j].valid = 0;
    }
  }
}

/* 
 * 释放已分配的内存
 */
void freeCache() {
  for (int i = 0; i < S; ++i)
    free(cache[i]);
  free(cache);
}

/* 
 * 访问地址为 addr 的数据.
 * 如果该数据已在缓存中，增加 hit_count
 * 如果该数据不在缓存中，将其载入缓存，增加 miss_count.
 * 如果有一行被替换，增加 eviction_count.
 */
void accessData(mem_addr_t addr) {
  int flag = 0, goal = 0;
  mem_addr_t tag_now = (addr >> b) >> s;
  set_index_mask = (addr >> b) & ((1 << s) - 1);
  for (int i = 0; i < E; ++i) {
    if (cache[set_index_mask][i].valid == 1 && cache[set_index_mask][i].tag == tag_now) {
      hit_count++;
      lru_counter++;
      cache[set_index_mask][i].lru = lru_counter;
      return;
    }
  }
  miss_count++;
  for (int i = 0; i < E; ++i) {
    if (cache[set_index_mask][i].valid == 0) {
      flag = 1;
      goal = i;
      break;
    }
  }
  if (flag == 1) {
    cache[set_index_mask][goal].valid = 1;
    cache[set_index_mask][goal].tag = tag_now;
    cache[set_index_mask][goal].lru = (++lru_counter);
  } else {
    eviction_count++;
    flag = 0;
    int minlru = cache[set_index_mask][0].lru;
    for (int i = 0; i < E; i++) {
      if (minlru > cache[set_index_mask][i].lru) {
        minlru = cache[set_index_mask][i].lru;
        flag = i;
      }
    }
    cache[set_index_mask][flag].valid = 1;
    cache[set_index_mask][flag].tag = tag_now;
    cache[set_index_mask][flag].lru = (++lru_counter);
  }
}

/*
 * 对给定的跟踪文件进行回放
 */
void replayTrace(char * trace_fn) {
  char buf[1000];
  mem_addr_t addr = 0;
  unsigned int len = 0;
  FILE * trace_fp = fopen(trace_fn, "r");
  if (!trace_fp) {
    fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
    exit(1);
  }
  while (fgets(buf, 1000, trace_fp) != NULL) {
    if (buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M') {
      sscanf(buf + 3, "%llx,%u", & addr, & len);
      if (verbosity)
        printf("%c %llx,%u ", buf[1], addr, len);
      accessData(addr);
      /* 如果指令为读写，则再次访问 */
      if (buf[1] == 'M')
        accessData(addr);
      if (verbosity)
        printf("\n");
    }
  }
  fclose(trace_fp);
}

/*
 * 打印使用信息
 */
void printUsage(char * argv[]) {
  printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace file.\n");
  printf("\nExamples:\n");
  printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
  printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
  exit(0);
}


int main(int argc, char * argv[]) {
  char c;
  while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
    switch (c) {
      case 's':
        s = atoi(optarg);
        break;
      case 'E':
        E = atoi(optarg);
        break;
      case 'b':
        b = atoi(optarg);
        break;
      case 't':
        trace_file = optarg;
        break;
      case 'v':
        verbosity = 1;
        break;
      case 'h':
        printUsage(argv);
        exit(0);
      default:
        printUsage(argv);
        exit(1);
    }
  }
  /* 所有必需的命令行参数都已指定 */
  if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
    printf("%s: Missing required command line argument\n", argv[0]);
    printUsage(argv);
    exit(1);
  }
  /* 从命令行参数计算S、E和B */
  S = 1 << s;
  B = 1 << b;
  E = E;
  /* 初始化缓存 */
  initCache();
  replayTrace(trace_file);
  /* 释放分配的内存 */
  freeCache();
  /* 输出自动测试程序的命中和未命中统计信息 */
  printSummary(hit_count, miss_count, eviction_count);
  return 0;
}
//****************************End**********************#

/* 
 * trans.c - 矩阵转置B=A^T
 *每个转置函数都必须具有以下形式的原型：
 *void trans（int M，int N，int a[N][M]，int B[M][N]）；
 *通过计算，块大小为32字节的1KB直接映射缓存上的未命中数来计算转置函数。
 */ 
#include <stdio.h>
#include "cachelab.h"
int is_transpose(int M, int N, int A[N][M], int B[M][N]);
char transpose_submit_desc[] = "Transpose submission";  //请不要修改“Transpose_submission”


void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
//                          请在此处添加代码
//*************************************Begin********************************************************
  int i, j, k,tmp0, tmp1, tmp2, tmp3, tmp4, tmp5,tmp6,tmp7;
  if (M == 64 && N == 64) {
    for(i = 0; i < M / 8; i++) {
      for(j = 0; j < N / 8; j++) {
        for(k = 0; k < 4; k++) {
          tmp0 = A[j*8+k][i*8+0];
          tmp1 = A[j*8+k][i*8+1];
          tmp2 = A[j*8+k][i*8+2];
          tmp3 = A[j*8+k][i*8+3];
          B[i*8+0][j*8+k] = tmp0;
          B[i*8+1][j*8+k] = tmp1;
          B[i*8+2][j*8+k] = tmp2;
          B[i*8+3][j*8+k] = tmp3;
        }
        for(k = 0; k < 4; k++) {
          tmp0 = A[j*8+k][i*8+4+0];
          tmp1 = A[j*8+k][i*8+4+1];
          tmp2 = A[j*8+k][i*8+4+2];
          tmp3 = A[j*8+k][i*8+4+3];
          B[i*8+0][j*8+4+k] = tmp0;
          B[i*8+1][j*8+4+k] = tmp1;
          B[i*8+2][j*8+4+k] = tmp2;
          B[i*8+3][j*8+4+k] = tmp3;
        }
        for(k = 0; k < 4; k++) {
          tmp0 = B[i*8+k][j*8+4+0];
          tmp1 = B[i*8+k][j*8+4+1];
          tmp2 = B[i*8+k][j*8+4+2];
          tmp3 = B[i*8+k][j*8+4+3];
          tmp4 = A[j*8+4+0][i*8+k];
          tmp5 = A[j*8+4+1][i*8+k];
          tmp6 = A[j*8+4+2][i*8+k];
          tmp7 = A[j*8+4+3][i*8+k];
          B[i*8+k][j*8+4+0] = tmp4;
          B[i*8+k][j*8+4+1] = tmp5;
          B[i*8+k][j*8+4+2] = tmp6;
          B[i*8+k][j*8+4+3] = tmp7;
          B[i*8+4+k][j*8+0] = tmp0;
          B[i*8+4+k][j*8+1] = tmp1;
          B[i*8+4+k][j*8+2] = tmp2;
          B[i*8+4+k][j*8+3] = tmp3;
        }
        for(k = 0; k < 4; k++) {
          tmp0 = A[j*8+4+k][i*8+4+0];
          tmp1 = A[j*8+4+k][i*8+4+1];
          tmp2 = A[j*8+4+k][i*8+4+2];
          tmp3 = A[j*8+4+k][i*8+4+3];
          B[i*8+4+0][j*8+4+k] = tmp0;
          B[i*8+4+1][j*8+4+k] = tmp1;
          B[i*8+4+2][j*8+4+k] = tmp2;
          B[i*8+4+3][j*8+4+k] = tmp3;
        }
      }
    }
    for(i = 0; i < N ; i++) {
      for(j = 8 * (M / 8); j < M; j++) {
        tmp0 = A[i][j];
        B[j][i] = tmp0;
      }
    }
    for(i = 8 * (N / 8); i < N; i++) {
      for(j = 0; j < 8*(M/8); j++) {
        tmp0 = A[i][j];
        B[j][i] = tmp0;
      }
    }
  } else {
    for(i = 0; i < M/8; i++) {
      for(j = 0; j < N/8; j++) {
        for(k = 0; k < 8; k++) {
          tmp0 = A[j*8+k][i*8+0];
          tmp1 = A[j*8+k][i*8+1];
          tmp2 = A[j*8+k][i*8+2];
          tmp3 = A[j*8+k][i*8+3];
          tmp4 = A[j*8+k][i*8+4];
          tmp5 = A[j*8+k][i*8+5];
          tmp6 = A[j*8+k][i*8+6];
          tmp7 = A[j*8+k][i*8+7];
          B[i*8+0][j*8+k] = tmp0;
          B[i*8+1][j*8+k] = tmp1;
          B[i*8+2][j*8+k] = tmp2;
          B[i*8+3][j*8+k] = tmp3;
          B[i*8+4][j*8+k] = tmp4;
          B[i*8+5][j*8+k] = tmp5;
          B[i*8+6][j*8+k] = tmp6;
          B[i*8+7][j*8+k] = tmp7;
        }
      }
    }
    for(i = 0; i < N ; i++) {
      for(j = 8*(M/8); j < M; j++) {
        tmp0 = A[i][j];
        B[j][i]=tmp0;
      }
    }
    for(i = 8*(N/8); i < N; i++) {
      for(j = 0; j < 8*(M/8); j++) {
        tmp0 = A[i][j];
        B[j][i]=tmp0;
      }
    }
  }
//**************************************End**********************************************************
}

/* 
 * 我们在下面定义了一个简单的方法来帮助您开始，您可以根据下面的例子把上面值置补充完整。
 */ 

/* 
 * 简单的基线转置功能，未针对缓存进行优化。
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions-此函数向驱动程序注册转置函数。
 *在运行时，驱动程序将评估每个注册的函数并总结它们的性能。这是一种试验不同转置策略的简便方法。
 */
void registerFunctions()
{
    /* 注册解决方案函数  */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* 注册任何附加转置函数 */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - 函数检查B是否是A的转置。在从转置函数返回之前，可以通过调用它来检查转置的正确性。
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
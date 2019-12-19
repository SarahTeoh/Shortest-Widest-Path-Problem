#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NODE_NUM 10 /* 総ノード数 */
#define MAX 9999 /* 無限大に相当する数 */
#define INITIAL_CAPACITY 1 /* 可変長配列の初期容量 */

#define FLAG 0 /* Dijkstraのテストの場合は0に、シミュレーション評価を行う場合は1にする */

/* 全てのリンクのノードと容量を格納する構造体 */
struct edge{
  int nodeA;
  int nodeB;
  int link;
};

/* 容量を大きい順に並べる関数 */
/* 例: 容量の種類が1, 4, 3, 5の場合、5, 4, 3, 1に並べる */
int compareLink(const void *a, const void *b){
  int *a1 = (int *)a; 
  int *a2 = (int *)b;
  if ((*a1)> (*a2))
      return -1;
  else if ((*a1) < (*a2))
      return 1;
  else
      return 0;
}

/* 可変長配列に要素を入れる関数 */
void push(int *arr, int index, int value, int *size, int *capacity){
     if(*size > *capacity){
          arr = realloc(arr, sizeof(arr)+ sizeof(int));
          *capacity = sizeof(arr) + sizeof(int);
     }
     arr[index] = value;
     *size = *size + 1;
}

/* 配列に要素valが存在するかどうかをチェックする関数 */
int findValueInArray(int val, int arr[], int size)
{
    int i;
    for(i = 0; i < size; i++)
    {
        if(arr[i] == val)
            return 1;
    }
    return 0;
}

int main()
{
/* 最大路アルゴリズム部分で必要な変数 */
  int tmp_node;  /*注目しているノード*/ 
  int src, dest; /* 始点・終点ノード */
  int a, b, c, d, i, j, k=0;
  FILE *fp; /* 距離の入ったファイルを示すポインタ */
  int num_of_edges = 0; /* リンクの数 */
  int new_link[NODE_NUM][NODE_NUM]; /* G'のリンクに対応する配列(順次に追加、まだ経路が存在しない場合は-1) */
  int inserted_to_edge[NODE_NUM][NODE_NUM]; /* 重複を避けるために各リンクを配列に入れたかどうかのフラグ配列 */
  int connected[NODE_NUM][NODE_NUM]; /* ノードとノードの間に経路があるかどうかの配列 */
  int* all_link = malloc(INITIAL_CAPACITY * sizeof(int)); /* 全ての容量の種類の配列 */
  int size = 0; /* 可変長配列の初期長さ */
  int index = 0; /* 可変長配列にpushする時のインデックス */
  int capacity = INITIAL_CAPACITY; /* 可変長配列の初期容量 */
  int which_link; /* G'に追加するリンクの容量 */
  int fin_widest; /* srcからdestまでの経路があるかどうかのフラグ */
  int fin_find_path; /* 最大経路を辿り着いてdestまで行ったかどうかのフラグ */
  int next[NODE_NUM]; /*後ノード表*/
  int chk_widest[NODE_NUM]; /*最大路確定のフラグ*/
  int current_node; /* 最大経路を辿る時にどのノードに辿りついたか */

  /* シミュレーション評価の部分で必要な変数 */
  int link[NODE_NUM][NODE_NUM]; /* リンク容量 */
  int bandwidth[NODE_NUM][NODE_NUM]; /* リンクの空き容量 */
  int miss; /* 呼損を表すフラグ */
  int success; /* 確立できた通信回数 */
  int sum_success; /* 確立できた通信回数の合計 */
  int sim_time; /* 評価の回数をカウント */
  int failed; /* 呼損が発生したかどうかのフラグ */

  /*
  容量行列の作成
  */

  for(i=0; i<NODE_NUM; i++){
    for(j=0; j<NODE_NUM; j++){
      //graph[i][j] = MAX; /* 接続されていないノード間の距離をMAXにする */ 
      link[i][j] = -1; /* 接続されていないノード間のリンク容量を-1にする */
      inserted_to_edge[i][j] = 0; /* まだedge構造体に入れてないので0にする */
      if(i==j){ link[i][j] = -1;}/* graph[i][j] = 0; そのノード自身への距離は0とし、リンク容量は-1とする */
    }
  }

  fp=fopen("./distance.txt", "r");

  while(fscanf(fp, "%d %d %d %d", &a, &b, &c, &d) != EOF) /* EOFまで4つ組を読み込む */
    {
      link[a][b]=d; /* 接続されているノード間のリンクを設定 */
      link[b][a]=d; /* 逆方向も同じ容量と仮定 */
      num_of_edges ++; /* 読み込んだリンクの数 */

      if(!findValueInArray(d, all_link, size)){ /* リンク容量がall_link配列に入ってるかどうかをチェック */
        push(all_link, index, d, &size, &capacity); /* 配列に入ってなければ入れる */
        index ++; /* 次に要素を入れる時のインデックス */
      }
    }

  fclose(fp);

  /* 全てのリンクをedge構造体の配列 */
  struct edge edges[num_of_edges]; /* 全てのリンクをedge構造体の配列 */

  /* 全てのリンクを構造体の配列に重複なしに入れる */
  for(i=0; i<NODE_NUM; i++){
    for(j=0; j<NODE_NUM; j++){
      if(link[i][j] != -1){
        if(inserted_to_edge[j][i]==0){
          inserted_to_edge[i][j] = 1;
          inserted_to_edge[j][i] = 1;
          edges[k].nodeA = i;
          edges[k].nodeB = j;
          edges[k].link = link[i][j];
          k++;
        }
      }
    }
  }
  

  qsort(all_link, size, sizeof(all_link[0]), compareLink); /*  */

/* 
始点・終点ノードを標準入力から得る (評価の場合は、実行しない)
*/
  if (FLAG == 0){
    printf("Source Node?(0-%d)", NODE_NUM-1);
    fscanf(stdin, "%d", &src);
    printf("Destination Node?(0-%d)", NODE_NUM-1);
    fscanf(stdin, "%d", &dest);
  }

  if (FLAG == 1) srand((unsigned)time(NULL)); /* 乱数の初期化, これ以降、rand()で乱数を得ることができる */

/****************************/
/* シミュレーション評価開始 */
/****************************/

  success = 0; sum_success = 0; /* 評価指標を初期化 */
  for (sim_time=0; sim_time<1000; sim_time++){
    miss = 0; /* 空きリンクが存在しない場合のフラグをOFFにする */
    success = 0; /* 確立できた通信回数を初期化する */

    for (i=0; i<NODE_NUM; i++){  /* 全リンクの空き容量を初期状態に戻す */
      for (j=0; j<NODE_NUM; j++){
        bandwidth[i][j] = link[i][j];
        bandwidth[j][i] = link[i][j];
      }
    }


    while(miss == 0){ /* 呼損が発生するまで繰り返す */
      /* 評価の場合、送受信ノードをランダムに決定 */
      if (FLAG == 1){
        /* ランダムに送受信ノードを決定 */
        src = rand() % 10;
        dest = rand() % 10;

        printf("src=%d, dest=%d\n", src, dest); /* 送受信ノードを表示 */
        if (src==dest) printf("送受信ノードが一致している\n");
      }

      /****************************************/
      /* ここからは最大経路アルゴリズム */
      /****************************************/
      
      /* 初期化 */
      for(i=0; i<NODE_NUM; i++){
        for(j=0; j<NODE_NUM; j++){
          new_link[i][j] = -1; /* G'のリンクを空にする　*/
          connected[i][j] = 0; /* 全てのノードが繋がってない初期状態にする */
          if(i==j){new_link[i][j] = 0; connected[i][j] = 1;} /* ノード自分自身へ容量0の経路が存在する状態にする */
          }
        }

      for(i=0; i<NODE_NUM; i++){
        chk_widest[i] = 0; /* 最大路確定のフラグを初期状態にする */
        next[i] = NODE_NUM; /* 後ノード表 */
      }  
      
      chk_widest[src] = 1; /* srcから最大路を探索 */
      current_node = src; /* srcから最大路を探索 */
      fin_widest = 0; /* フラグの初期化 */
      fin_find_path = 0; /* フラグの初期化 */
      which_link = 0; /*　all_linkのインデックス。最初は最大容量all_link[0]からG'に追加する　*/

      /*最大路(ボトルネックリンク)を見つける*/
      while(fin_widest==0){   
        for(i=0; i<num_of_edges; i++){
          if(edges[i].link == all_link[which_link]){
            new_link[edges[i].nodeA][edges[i].nodeB] = edges[i].link;  /* リンクをG'に追加する */
            new_link[edges[i].nodeB][edges[i].nodeA] = edges[i].link;
            connected[edges[i].nodeA][edges[i].nodeB] = 1; /* 直接繋がっているノードの間に経路が存在するので1にする */
            connected[edges[i].nodeB][edges[i].nodeA] = 1;
          }
        }

        /*直接繋がってないノードnodeAからノードkを経由してノードjへ経路が存在すればconnected[i][j]を1にする */
        for(k=0; k<num_of_edges; k++){
          for(i=0; i<NODE_NUM; i++){
            if(connected[edges[k].nodeA][i] == 1){
              for(j=0; j<NODE_NUM; j++){
                  connected[edges[k].nodeA][j] = (connected[edges[k].nodeA][j] || connected[i][j]);
              } 
            }
          }
        }

        /* srcからdestへ経路を見つけた場合、あるいは、全てのリンクをチェックした場合にloopをbreakする */
        if(which_link > size || (connected[src][dest]==1)){
          fin_widest = 1;
          break;
        }else{
          which_link ++;
          continue;
          }
       } 
       /*ここまで来ると最大路はすでにall_link[which_link]に格納されているが、最大経路まだ確定してない*/

      /*ここから最大路を用いた固定経路を辿る*/
      while(fin_find_path==0){
        /* 初期化 */
        for(i=0; i<NODE_NUM; i++){ 
          chk_widest[i] = 0;
        }

        loop_again:
        chk_widest[current_node] = 1; /* 現在注目しているノードのチェックフラグを1にする */
        if(current_node==dest){ /* 現在注目しているノードは終点ノードであればloopをbreakする */
            fin_find_path = 1;
            break;
        }
        for(i=0; i<NODE_NUM; i++){ /* 現在注目しているノードと隣接しているノードについて */
          if(!chk_widest[i]){ /* まだチェックされていない、かつ、G'上にボトルネックより大きい容量を持つ経路がある場合 */
            if(new_link[current_node][i]>=all_link[which_link]){
              if(i==dest){ /* 現在注目しているノードと隣接しているノードが終点ノードであればwhile loopをbreakして */
                next[current_node] = i; /* 後ノード表に経路を記録 */
                current_node = i;
                fin_find_path = 1;
                goto done; 
              }
              /* iが終点ノードでなければ最大経路上のノードにするのが適切かどうかをチェックする */
              /* 詳しくはレポートの1.3.1章を参照してください */
              tmp_node = i; /* まず、現在注目しているノードと隣接しているノードのうちの一つ */
              chk_widest[tmp_node] = 1; /* 一時的にチェックフラグを1にする */
              for(j=0; j<NODE_NUM; j++){
                if(!chk_widest[j]){
                  if(new_link[i][j]>=all_link[which_link]){
                    if(j==dest){ /* 終点ノードであれば最大経路確定。whileループをbreak */
                      next[current_node] = i;
                      next[i] = j;
                      current_node = j;
                      fin_find_path = 1;
                      goto done;
                    }
                    chk_widest[j] = 1; /* 一時的にチェックフラグを1にする */
                    for(k=0; k<NODE_NUM; k++){  
                      if(!chk_widest[k]){
                        if(new_link[j][k] >= all_link[which_link]){
                          if(k==dest){ /* 終点ノードであれば最大経路確定。whileループをbreak */
                            next[current_node] = i;
                            next[i] = j;
                            next[j] = dest;
                            fin_find_path = 1;
                            goto done;
                          }
                        next[current_node] = tmp_node;
                        current_node = tmp_node;
                        chk_widest[j] = 0;
                        goto loop_again;
                        }else if(k==NODE_NUM-1){
                          chk_widest[j] = 0;
                        }
                      }
                    }
                  }
                }else{chk_widest[tmp_node] = 0;}
              }
            }
          }
        }
        if(fin_find_path==1) break;
      }

      done:
      /*最大路出力*/
      printf("Bottleneck link from node %d to node %d is %d\n",src, dest, all_link[which_link]);
      printf("Widest Path from node %d to node %d is ",src, dest);
      for(i=src; i!=dest; i=next[i]){
        printf("%d", i );
        printf("->");
      }
      printf("%d\n",dest );
   
      return 0;

      /**********************************************************************/
      /* アルゴリズムを評価するためのプログラム */
      /**********************************************************************/

      /* その経路上の全てのリンクに空き容量があるかどうかをチェック */      
      failed = 0;
      for(i=src; i!=dest; i=next[i]){ 
        if(bandwidth[i][next[i]]<1){
          failed = 1;
        }
      }

/*      2-(a) その経路上の全てのリンクに空き容量がある場合、それらを1Mbpsだけ
      減少させ、「確立できた通信回数」を1増やす
*/
      if(failed==0){
        for(i=src; i!=dest ;i=next[i]){
          bandwidth[i][next[i]] --;
          bandwidth[next[i]][i] --;
        }
        success++;
/*
      2-(b) その経路上に空き容量のないリンクが存在する場合、呼損とし、
      その時点までの「確立できた通信回数」をsum_successに足して、while文をbreakする
*/        
      }else if(failed==1){
        miss = 1;
        sum_success += success;
        break;
      }


    } /* while(1) */
  } /* 1～3の手続きを1000回行うためのfor文 */


  /*
  シミュレーション評価の結果出力
  */
  printf("\naverage = %f\n", sum_success / 1000.0); /* 初めて呼損が起こるまでに確立できた通信回数の平均を表示 */
  return 0;
}
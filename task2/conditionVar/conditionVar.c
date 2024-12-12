/* boundedbuffer.c */
/* bounded buffer example */
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 20
#define NUMITEMS 30

/*생산자-소비자(Producer-Consumer) 문제를 해결하기 위한 bounded buffer(제한된 버퍼) 구현
생산자 쓰레드가 데이터를 생성해 버퍼에 추가하고, 
소비자 쓰레드는 버퍼에서 데이터를 가져와 처리함.
이 문제는 멀티쓰레드 프로그래밍에서 흔히 나타나는 동기화 문제를 보여줌. */
typedef struct {
    int item[BUFFER_SIZE]; //버퍼에 저장되는 항목들
    int totalitems; //현재 버퍼에 저장된 항목 수.
    int in, out; 
    //in: 다음에 저장할 위치의 인덱스.
    //out: 다음에 제거할 위치의 인덱스.
    pthread_mutex_t mutex; //버퍼 접근을 위한 뮤텍스(동기화 도구).
    pthread_cond_t full; //조건 변수로, 버퍼 상태(가득 참 또는 비어 있음)에 따라 대기 및 신호 전달
    pthread_cond_t empty;
} buffer_t;

buffer_t bb = { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, //초기화된 버퍼로, 생산자와 소비자가 공유.
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER
};

int produce_item()//랜덤하게 항목을 생성하고, 생성 시간을 시뮬레이션(일정 시간 대기). 생성된 항목을 반환.
{
    int item = (int) (100.0 * rand() / (RAND_MAX + 1.0));
    sleep((unsigned long) (5.0 * rand() / (RAND_MAX + 1.0)));
    printf("produce_item: item=%d\n", item);
    return item;
}

int insert_item(int item) //생산자가 항목을 버퍼에 삽입.
{/*
뮤텍스 잠금:
1. 버퍼가 가득 차면 pthread_cond_wait를 호출해 empty 조건이 충족될 때까지 대기.
2. 항목을 item 배열의 in 인덱스에 삽입, in 및 totalitems 업데이트.
3. 삽입 후 pthread_cond_signal(&bb.full)로 소비자에게 신호.
 */
    int status;
    status = pthread_mutex_lock(&bb.mutex);
    if (status != 0)
        return status;

    while (bb.totalitems >= BUFFER_SIZE && status == NULL)
        status = pthread_cond_wait(&bb.empty, &bb.mutex);

    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    bb.item[bb.in] = item;
    bb.in = (bb.in + 1) % BUFFER_SIZE;
    bb.totalitems++;

    if (status = pthread_cond_signal(&bb.full)) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    return pthread_mutex_unlock(&bb.mutex);
}

void consume_item(int item)
//소비자가 항목을 처리하는 함수.
//처리 시간을 시뮬레이션(일정 시간 대기).
{
    sleep((unsigned long) (5.0 * rand() / (RAND_MAX + 1.0)));
    printf("\t\tconsume_item: item=%d\n", item);
}

int remove_item(int *temp)//소비자가 버퍼에서 항목을 제거.
/*뮤텍스 잠금:
1. 버퍼가 비어 있으면 pthread_cond_wait를 호출해 full 조건이 충족될 때까지 대기.
2. 항목을 out 인덱스에서 제거해 temp에 저장, out 및 totalitems 업데이트.
3. 제거 후 pthread_cond_signal(&bb.empty)로 생산자에게 신호*/
{
    int status;
    status = pthread_mutex_lock(&bb.mutex);
    if (status != 0)
        return status;

    while (bb.totalitems <= 0 && status == NULL)
        status = pthread_cond_wait(&bb.full, &bb.mutex);

    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    *temp = bb.item[bb.out];
    bb.out = (bb.out + 1) % BUFFER_SIZE;
    bb.totalitems--;

    if (status = pthread_cond_signal(&bb.empty)) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    return pthread_mutex_unlock(&bb.mutex);
}

void *producer(void *arg) // producer() 쓰레드 함수, 무한 루프에서 항목을 생성(produce_item)하고 버퍼에 삽입(insert_item).
{
    int item;
    while (1) {
        item = produce_item();
        insert_item(item);
    }
}

void *consumer(void *arg) // consumer() 쓰레드 함수, 무한 루프에서 버퍼에서 항목을 제거(remove_item)하고 처리(consume_item).
{
    int item;
    while (1) {
        remove_item(&item);
        consume_item(item);
    }
}

int main()
/*
1. 생산자(producer)와 소비자(consumer) 쓰레드를 생성.
2. 쓰레드가 종료되기를 기다림(pthread_join).
3. 쓰레드 생성 또는 종료 중 오류가 발생하면 perror로 오류 출력.
*/
{
    int status;
    void *result;
    pthread_t producer_tid, consumer_tid;

    /* 쓰레드 생성 */
    status = pthread_create(&producer_tid, NULL, producer, NULL);
    if (status != 0)
        perror("Create producer thread");

    status = pthread_create(&consumer_tid, NULL, consumer, NULL);
    if (status != 0)
        perror("Create consumer thread");

    status = pthread_join(producer_tid, NULL);
    if (status != 0)
        perror("Join producer thread");

    status = pthread_join(consumer_tid, NULL);
    if (status != 0)
        perror("Join consumer thread");

    return 0;
}

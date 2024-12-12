/*
클라이언트(자식) 쓰레드들로부터 메시지 전송 요청을 받으면 서버(부모) 쓰레
드는 모든 클라이언트 쓰레드에게 메시지를 방송하는 프로그램을 구현하시오.
(힌트: 소켓은 사용하지 말고 데이터 전송을 위한 동기화를 위해 뮤텍스와 조건변수를 사용한다.)
*/

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 20
#define NUMITEMS 30

typedef struct {
    char message[100];
    char user[50];
} chat_message_t;

typedef struct {
    chat_message_t item[BUFFER_SIZE]; 
    int totalitems; 
    int in, out; 
    pthread_mutex_t mutex; 
    pthread_cond_t full; 
    pthread_cond_t empty;
} buffer_t;

buffer_t bb = { {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER
};


chat_message_t produce_item() //부모 프로세스가 자식프로세스에게 메시지를 넘겨줌
{
    chat_message_t msg;
    printf("Enter your message: ");
    fgets(msg.message, sizeof(msg.message), stdin);
    msg.message[strcspn(msg.message, "\n")] = '\0'; 

    snprintf(msg.user, sizeof(msg.user), "Producer %ld", pthread_self());
    printf("produce_msg: user=%s, message=%s\n", msg.user, msg.message);
    return msg;
}

int insert_item(chat_message_t item)
{
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

void consume_item(chat_message_t item)
{
    sleep((unsigned long) (5.0 * rand() / (RAND_MAX + 1.0)));
    printf("\t\tconsume_item: user=%s, message=%s\n", item.user, item.message);
}

int remove_item(chat_message_t *temp)
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

void *producer(void *arg)
{
    chat_message_t item;
    while (1) {
        item = produce_item();
        insert_item(item);
    }
}

void *consumer(void *arg) 
{
    chat_message_t item;
    while (1) {
        remove_item(&item);
        consume_item(item);
    }
}

int main(){
    int status;
    void *result;
    pthread_t producer_tid, consumer_tid;
    pid_t pid;
    

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

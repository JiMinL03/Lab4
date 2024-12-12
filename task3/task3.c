/*
쓰레드를 사용하여 생산자 소비자 문제를 해결하는 제한 버퍼(Bounded
Buffer)를 생성하고 활용하는 프로그램을 구현하시오. 
단, 생산자와 소비자 쓰레드는 각각 둘 이상 가능해야 한다
*/

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h> // for sleep

#define BUFFER_SIZE 20
#define NUMITEMS 30
#define NUM_PRODUCERS 2 // 생성할 생산자 쓰레드 수
#define NUM_CONSUMERS 2 // 생성할 소비자 쓰레드 수

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

buffer_t bb = { {{"", ""}}, 0, 0, 0,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    PTHREAD_COND_INITIALIZER
};

chat_message_t produce_item() {
    chat_message_t msg;
    // 사용자로부터 메시지를 입력받음
    printf("Enter your message: ");
    fgets(msg.message, sizeof(msg.message), stdin);
    msg.message[strcspn(msg.message, "\n")] = '\0'; // 개행 문자 제거

    snprintf(msg.user, sizeof(msg.user), "Producer %ld", pthread_self());
    printf("produce_item: user=%s, message=%s\n", msg.user, msg.message);
    return msg;
}

int insert_item(chat_message_t item) {
    int status;
    status = pthread_mutex_lock(&bb.mutex);
    if (status != 0)
        return status;

    while (bb.totalitems >= BUFFER_SIZE && status == 0)
        status = pthread_cond_wait(&bb.empty, &bb.mutex);

    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    bb.item[bb.in] = item;
    bb.in = (bb.in + 1) % BUFFER_SIZE;
    bb.totalitems++;

    pthread_cond_signal(&bb.full);
    return pthread_mutex_unlock(&bb.mutex);
}

void consume_item(chat_message_t item) {
    sleep((unsigned long)(2.0 * rand() / (RAND_MAX + 1.0))); // 소비 지연
    printf("\t\tconsume_item: user=%s, message=%s\n", item.user, item.message);
}

int remove_item(chat_message_t *temp) {
    int status;
    status = pthread_mutex_lock(&bb.mutex);
    if (status != 0)
        return status;

    while (bb.totalitems <= 0 && status == 0)
        status = pthread_cond_wait(&bb.full, &bb.mutex);

    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    *temp = bb.item[bb.out];
    bb.out = (bb.out + 1) % BUFFER_SIZE;
    bb.totalitems--;

    pthread_cond_signal(&bb.empty);
    return pthread_mutex_unlock(&bb.mutex);
}

void *producer(void *arg) {
    chat_message_t item;
    while (1) {
        item = produce_item();
        insert_item(item);
    }
}

void *consumer(void *arg) {
    chat_message_t item;
    while (1) {
        remove_item(&item);
        consume_item(item);
    }
}

int main() {
    int status;
    pthread_t producer_tid[NUM_PRODUCERS], consumer_tid[NUM_CONSUMERS];

    // 생산자 쓰레드 생성
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        status = pthread_create(&producer_tid[i], NULL, producer, NULL);
        if (status != 0) {
            perror("Create producer thread");
            exit(1);
        }
    }

    // 소비자 쓰레드 생성
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        status = pthread_create(&consumer_tid[i], NULL, consumer, NULL);
        if (status != 0) {
            perror("Create consumer thread");
            exit(1);
        }
    }

    // 생성된 쓰레드의 종료 대기
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        status = pthread_join(producer_tid[i], NULL);
        if (status != 0) {
            perror("Join producer thread");
        }
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        status = pthread_join(consumer_tid[i], NULL);
        if (status != 0) {
            perror("Join consumer thread");
        }
    }

    return 0;
}

/* mutexthread.c */
/* mutex example */
#include <stdio.h>
#include <pthread.h>  // pthread 라이브러리 포함
#define NUM_THREADS 3 // 생성할 쓰레드 수 정의

pthread_mutex_t mutex; // 뮤텍스 객체 선언
int sum;               // 공유 자원 변수

/* 쓰레드 함수 */
void *mutex_thread(void *arg)
{
    pthread_mutex_lock(&mutex); // 뮤텍스 잠금
    sum += (int)arg;            // 공유 자원 sum에 전달받은 값을 더함
    pthread_mutex_unlock(&mutex); // 뮤텍스 잠금 해제
    return arg; // 쓰레드 종료 시 전달받은 값을 반환
}

int main(int argc, char *argv[])
{
    pthread_t tid[NUM_THREADS]; // 쓰레드 ID 배열
    int arg[NUM_THREADS], i;    // 입력값 배열과 반복 변수
    void *result;               // 쓰레드 반환값 저장
    int status;                 // 함수 호출 결과 상태 저장

    /* 사용자 입력 확인 */
    if (argc < 4) {
        fprintf(stderr, "Usage: mutexthread number1 number2 number3\n"); // 사용법 출력
        exit(1); // 프로그램 종료
    }

    /* 입력값 배열에 저장 */
    for (i = 0; i < NUM_THREADS; i++)
        arg[i] = atoi(argv[i + 1]); // argv에서 정수 값으로 변환해 arg에 저장

    pthread_mutex_init(&mutex, NULL); // 뮤텍스 초기화

    /* 쓰레드 생성 */
    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_create(&tid[i], NULL, mutex_thread, (void *)arg[i]); // 쓰레드 생성
        if (status != 0) { // 쓰레드 생성 실패 시 오류 출력
            fprintf(stderr, "Create thread %d: %d", i, status);
            exit(1); // 프로그램 종료
        }
    }

    /* 쓰레드 종료 대기 */
    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_join(tid[i], &result); // 쓰레드가 종료되기를 기다림
        if (status != 0) { // join 실패 시 오류 출력
            fprintf(stderr, "Join thread %d: %d", i, status);
            exit(1); // 프로그램 종료
        }
    }

    status = pthread_mutex_destroy(&mutex); // 뮤텍스 소멸
    if (status != 0) // 소멸 실패 시 오류 출력
        perror("Destroy mutex");

    printf("sum is %d\n", sum); // 최종 합계 출력
    pthread_exit(result); // 메인 쓰레드 종료
}

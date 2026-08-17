#include <pthread.h>
#include <stdio.h>

typedef struct Counter {
    int count;
    pthread_mutex_t lock;
} Counter;

Counter init(int count) {
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    Counter c = { count, mutex };
    return c;
}

int increment(Counter* c) {
    pthread_mutex_lock(&c->lock);
    c->count += 1;
    pthread_mutex_unlock(&c->lock);
}

int decrement(Counter* c) {
    pthread_mutex_lock(&c->lock);
    c->count -= 1;
    pthread_mutex_unlock(&c->lock);
}

int get(Counter* c) {
    pthread_mutex_lock(&c->lock);
    const int val = c->count;
    pthread_mutex_unlock(&c->lock);
    return val;
}

int getprint(Counter* c) {
    pthread_mutex_lock(&c->lock);
    printf("Counter value: %d", c->count);
    pthread_mutex_unlock(&c->lock);
}

typedef struct ThreadArgs {
    pthread_t* p;
    Counter* c;
    int n;
} ThreadArgs;

/** Adds to the counter until it equals 10 */
void add_till_ten(void* args) {
    ThreadArgs* targs = (ThreadArgs*) args;
    while (get(targs->c) < targs->n) {
        increment(targs->c);
    }
}

int main() {
    pthread_t p1;
    pthread_t p2;

    Counter c = init(0);
    int N = 100;

    ThreadArgs args1 = { &p1, &c, N };
    ThreadArgs args2 = { &p2, &c, N  };

    pthread_create(&p1, NULL, (void*) add_till_ten, &args1);
    
    pthread_create(&p2, NULL, (void*) add_till_ten, &args2);

    pthread_join(p1, NULL);
    printf("Thread 1 Done.\n");

    pthread_join(p2, NULL);
    printf("Thread 2 Done.\n");

    printf("Final count: %d", c.count);

    return 0;
}
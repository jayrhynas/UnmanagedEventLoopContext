#include "EventLoop.h"

#include <pthread.h>
#include <dispatch/dispatch.h>

typedef struct Task {
    void (*work)(void *);
    void *context;
    struct Task *next;
} Task;

typedef struct EventLoop_t {
    pthread_t thread;
    pthread_mutex_t mutex;
    dispatch_semaphore_t signal;
    Task *tasks;
    int isCancelled;
} EventLoop_t;

Task *EventLoopNextTask(EventLoop_t *eventLoop);

void *mainLoop(void *ctx) {
    EventLoop_t *eventLoop = (EventLoop_t *)ctx;
    
    while (true) {
        dispatch_semaphore_wait(eventLoop->signal, DISPATCH_TIME_FOREVER);
        
        Task *task;
        if ((task = EventLoopNextTask(eventLoop))) {
            task->work(task->context);
            free(task);
        }
    }
    
    return NULL;
}

int EventLoopIsCancelled(EventLoop eventLoop) {
    int isCancelled = 0;
    pthread_mutex_lock(&eventLoop->mutex);
    isCancelled = eventLoop->isCancelled;
    pthread_mutex_unlock(&eventLoop->mutex);
    return isCancelled;
}
    
void EventLoopCancel(EventLoop eventLoop) {
    pthread_mutex_lock(&eventLoop->mutex);
    eventLoop->isCancelled = 1;
    pthread_mutex_unlock(&eventLoop->mutex);
}

void EventLoopAddTask(EventLoop eventLoop, void *context, void (*work)(void *)) {
    Task *task = (Task *)calloc(1, sizeof(Task));
    task->work = work;
    task->context = context;
    task->next = NULL;
    
    pthread_mutex_lock(&eventLoop->mutex);
    if (!eventLoop->tasks) {
        eventLoop->tasks = task;
    } else {
        Task *last = eventLoop->tasks;
        while (last->next) {
            last = last->next;
        }
        
        last->next = task;
    }
    pthread_mutex_unlock(&eventLoop->mutex);
    
    dispatch_semaphore_signal(eventLoop->signal);
}

Task *EventLoopNextTask(EventLoop_t *eventLoop) {
    Task *task;
    pthread_mutex_lock(&eventLoop->mutex);
    task = eventLoop->tasks;
    
    if (task) {
        eventLoop->tasks = task->next;
        task->next = NULL;
    }
    pthread_mutex_unlock(&eventLoop->mutex);
    
    return task;
}

EventLoop EventLoopCreate(void) {
    EventLoop_t* eventLoop = (EventLoop_t *)calloc(1, sizeof(EventLoop_t));
    
    if (pthread_mutex_init(&eventLoop->mutex, NULL) != 0) {
        free(eventLoop);
        return NULL;
    }
    
    if (!(eventLoop->signal = dispatch_semaphore_create(0))) {
        pthread_mutex_destroy(&eventLoop->mutex);
        free(eventLoop);
        return NULL;
    }
    
    eventLoop->tasks = NULL;
    
    if (pthread_create(&eventLoop->thread, NULL, mainLoop, eventLoop) != 0) {
        free(eventLoop->signal);
        pthread_mutex_destroy(&eventLoop->mutex);
        free(eventLoop);
        return NULL;
    }
    
    return eventLoop;
}

void EventLoopDestroy(EventLoop eventLoop) {
    EventLoopCancel(eventLoop);
    pthread_join(eventLoop->thread, NULL);
    
    free(eventLoop->signal);
    pthread_mutex_destroy(&eventLoop->mutex);
    free(eventLoop);
}

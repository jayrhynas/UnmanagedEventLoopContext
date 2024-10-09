#ifndef EventLoop_h
#define EventLoop_h

#include <stdlib.h>
#include <stdio.h>

typedef struct EventLoop_t *EventLoop;

EventLoop EventLoopCreate(void);
void EventLoopDestroy(EventLoop eventLoop);
void EventLoopAddTask(EventLoop eventLoop, void *context, void (*work)(void *));

int EventLoopIsCancelled(EventLoop eventLoop);
void EventLoopCancel(EventLoop eventLoop);
    
#endif /* EventLoop_h */

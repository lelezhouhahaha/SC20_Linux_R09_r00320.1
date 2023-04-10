/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "osi/include/allocator.h"
#include "osi/include/fixed_queue.h"
#include "osi/include/list.h"
#include "osi/include/osi.h"
#include "osi/include/semaphore.h"
#include "osi/include/reactor.h"
#include "osi/include/log.h"

typedef struct fixed_queue_t {
  list_t *list;
  semaphore_t *enqueue_sem;
  semaphore_t *dequeue_sem;
  pthread_mutex_t lock;
  size_t capacity;

  reactor_object_t *dequeue_object;
  fixed_queue_cb dequeue_ready;
  void *dequeue_context;
} fixed_queue_t;

static void internal_dequeue_ready(void *context);

fixed_queue_t *btapp_fixed_queue_new(size_t capacity) {
	//LOG_ERROR("%s: from btapp", __func__);
  fixed_queue_t *ret = osi_calloc(sizeof(fixed_queue_t));
  if (!ret) {
     LOG_ERROR("%s:Failed to allocate memory", __func__);
    goto error;
  }

  pthread_mutex_init(&ret->lock, NULL);
  ret->capacity = capacity;

  ret->list = btapp_list_new(NULL);
  if (!ret->list) {
     LOG_ERROR("%s:List is NULL", __func__);
    goto error;
  }

  ret->enqueue_sem = semaphore_new(capacity);
  if (!ret->enqueue_sem) {
     LOG_ERROR("%s:enqueue_sem is NULL", __func__);
    goto error;
  }

  ret->dequeue_sem = semaphore_new(0);
  if (!ret->dequeue_sem) {
     LOG_ERROR("%s:dequeue_sem is NULL", __func__);
    goto error;
  }

  return ret;

error:;
  btapp_fixed_queue_free(ret, NULL);
  return NULL;
}

void btapp_fixed_queue_free(fixed_queue_t *queue, fixed_queue_free_cb free_cb) {
  if (!queue)
    return;

  btapp_fixed_queue_unregister_dequeue(queue);

  if (free_cb)
    for (const list_node_t *node = btapp_list_begin(queue->list); node != btapp_list_end(queue->list); node = btapp_list_next(node))
      free_cb(btapp_list_node(node));

  btapp_list_free(queue->list);
  semaphore_free(queue->enqueue_sem);
  semaphore_free(queue->dequeue_sem);
  pthread_mutex_destroy(&queue->lock);
  osi_free(queue);
}

bool btapp_fixed_queue_is_empty(fixed_queue_t *queue) {
  printf("################# calling from btapp ###################");
  //assert(queue != NULL);
	if(queue==NULL)
		return false;
  pthread_mutex_lock(&queue->lock);
  bool is_empty = btapp_list_is_empty(queue->list);
  pthread_mutex_unlock(&queue->lock);

  return is_empty;
}

size_t btapp_fixed_queue_capacity(fixed_queue_t *queue) {
  assert(queue != NULL);

  return queue->capacity;
}

void btapp_fixed_queue_enqueue(fixed_queue_t *queue, void *data) {
  assert(queue != NULL);
  assert(data != NULL);

  semaphore_wait(queue->enqueue_sem);

  pthread_mutex_lock(&queue->lock);
  btapp_list_append(queue->list, data);
  pthread_mutex_unlock(&queue->lock);

  semaphore_post(queue->dequeue_sem);
}

void *btapp_fixed_queue_dequeue(fixed_queue_t *queue) {
  assert(queue != NULL);

  semaphore_wait(queue->dequeue_sem);

  pthread_mutex_lock(&queue->lock);
  void *ret = btapp_list_front(queue->list);
  btapp_list_remove(queue->list, ret);
  pthread_mutex_unlock(&queue->lock);

  semaphore_post(queue->enqueue_sem);

  return ret;
}

bool btapp_fixed_queue_try_enqueue(fixed_queue_t *queue, void *data) {
  assert(queue != NULL);
  assert(data != NULL);

  if (!semaphore_try_wait(queue->enqueue_sem))
    return false;

  pthread_mutex_lock(&queue->lock);
  btapp_list_append(queue->list, data);
  pthread_mutex_unlock(&queue->lock);

  semaphore_post(queue->dequeue_sem);
  return true;
}

void *btapp_fixed_queue_try_dequeue(fixed_queue_t *queue) {
  assert(queue != NULL);

  if (!semaphore_try_wait(queue->dequeue_sem)) {
      LOG_ERROR("%s:Failed to dequeue_sem", __func__);
      return NULL;
  }

  pthread_mutex_lock(&queue->lock);
  void *ret = btapp_list_front(queue->list);
  btapp_list_remove(queue->list, ret);
  pthread_mutex_unlock(&queue->lock);

  semaphore_post(queue->enqueue_sem);

  return ret;
}

void *btapp_fixed_queue_try_peek(fixed_queue_t *queue) {
  assert(queue != NULL);

  pthread_mutex_lock(&queue->lock);
  // Because protected by the lock, the empty and front calls are atomic and not a race condition
  void *ret = btapp_list_is_empty(queue->list) ? NULL : btapp_list_front(queue->list);
  pthread_mutex_unlock(&queue->lock);

  return ret;
}

int btapp_fixed_queue_get_dequeue_fd(const fixed_queue_t *queue) {
  assert(queue != NULL);
  return semaphore_get_fd(queue->dequeue_sem);
}

int btapp_fixed_queue_get_enqueue_fd(const fixed_queue_t *queue) {
  assert(queue != NULL);
  return semaphore_get_fd(queue->enqueue_sem);
}

void btapp_fixed_queue_register_dequeue(fixed_queue_t *queue, reactor_t *reactor, fixed_queue_cb ready_cb, void *context) {
  assert(queue != NULL);
  assert(reactor != NULL);
  assert(ready_cb != NULL);

  // Make sure we're not already registered
  btapp_fixed_queue_unregister_dequeue(queue);

  queue->dequeue_ready = ready_cb;
  queue->dequeue_context = context;
  queue->dequeue_object = reactor_register(
    reactor,
    btapp_fixed_queue_get_dequeue_fd(queue),
    queue,
    internal_dequeue_ready,
    NULL
  );
}

void btapp_fixed_queue_unregister_dequeue(fixed_queue_t *queue) {
  assert(queue != NULL);

  if (queue->dequeue_object) {
    reactor_unregister(queue->dequeue_object);
    queue->dequeue_object = NULL;
  }
}

static void internal_dequeue_ready(void *context) {
  assert(context != NULL);

  fixed_queue_t *queue = context;
  queue->dequeue_ready(queue, queue->dequeue_context);
}

/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2021 Parai Wang <parai@foxmail.com>
 */
#ifndef ISOTP_TYPES_H
#define ISOTP_TYPES_H
/* ================================ [ INCLUDES  ] ============================================== */
#include "isotp.h"
#include <pthread.h>
#include <semaphore.h>
#include "Std_Types.h"
#include "Std_Timer.h"
/* ================================ [ MACROS    ] ============================================== */
/* ================================ [ TYPES     ] ============================================== */
struct isotp_s {
  uint8_t Channel;
  Std_TimerType timerErrorNotify;
  volatile uint32_t errorTimeout;
  pthread_t serverThread;
  uint8_t *data;
  size_t length;
  size_t index;

  pthread_mutex_t mutex;
  sem_t sem;

  volatile boolean running;
  volatile int result;

  isotp_parameter_t *params;
};
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */

#endif /* ISOTP_TYPES_H */

/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2021 Parai Wang <parai@foxmail.com>
 */
#ifndef STD_DEBUG_H
#define STD_DEBUG_H
/* ================================ [ INCLUDES  ] ============================================== */
#if !defined(USE_STD_PRINTF) || defined(linux) || defined(_WIN32)
#include <stdio.h>
#endif
#if defined(linux) || defined(_WIN32)
#include <stdarg.h>
#endif
#include <inttypes.h>
#ifdef __cplusplus
extern "C" {
#endif
/* ================================ [ MACROS    ] ============================================== */
#if defined(linux) || defined(_WIN32)
#ifndef USE_STD_DEBUG
#define USE_STD_DEBUG
#endif
#ifndef AS_LOG_DEFAULT
#define AS_LOG_DEFAULT std_get_log_level()
#endif
#define AS_LOG_LEVEL(lvl) std_get_as_log_level(#lvl)
#else
#define AS_LOG_LEVEL(lvl) AS_LOG_##lvl
#endif

#ifndef AS_LOG_DEFAULT
#define AS_LOG_DEFAULT 1
#endif
#define AS_LOG_DEBUG 0
#define AS_LOG_INFO 1
#define AS_LOG_WARN 2
#define AS_LOG_ERROR 3

#ifdef USE_STD_PRINTF
#define PRINTF std_printf
#endif

#ifndef PRINTF
#if defined(linux) || defined(_WIN32)
#define PRINTF printf
#else
#define PRINTF(fmt, ...)
#endif
#endif

#ifdef USE_STD_DEBUG
#define ASLOG(level, msg)                                                                          \
  do {                                                                                             \
    if ((AS_LOG_LEVEL(level)) >= AS_LOG_DEFAULT) {                                                 \
      PRINTF("%-8s:", #level);                                                                     \
      PRINTF msg;                                                                                  \
    }                                                                                              \
  } while (0)

#define ASPRINT(level, msg)                                                                        \
  do {                                                                                             \
    if ((AS_LOG_LEVEL(level)) >= AS_LOG_DEFAULT) {                                                 \
      PRINTF msg;                                                                                  \
    }                                                                                              \
  } while (0)

#define ASHEXDUMP(level, msg, data, size)                                                          \
  do {                                                                                             \
    if ((AS_LOG_LEVEL(level)) >= AS_LOG_DEFAULT) {                                                 \
      uint8_t *__pData = (uint8_t *)(data);                                                        \
      uint32_t __index;                                                                            \
      PRINTF("%-8s:", #level);                                                                     \
      PRINTF msg;                                                                                  \
      for (__index = 0; __index < (size); __index++) {                                             \
        if (0 == (__index & 0x1F)) {                                                               \
          PRINTF("\n  %08X ", __index);                                                            \
        }                                                                                          \
        PRINTF("%02X ", __pData[__index]);                                                         \
      }                                                                                            \
      PRINTF("\n");                                                                                \
    }                                                                                              \
  } while (0)

#define asAssert(e)                                                                                \
  do {                                                                                             \
    if (!(e)) {                                                                                    \
      ASLOG(ERROR,                                                                                 \
            ("assert error on condition(%s) at line %d of %s\n", #e, (int)__LINE__, __FILE__));    \
      while (1)                                                                                    \
        ;                                                                                          \
    }                                                                                              \
  } while (0)
#else
#define ASLOG(level, msg)
#define ASPRINT(level, msg)
#define ASHEXDUMP(level, msg, data, size)
#define asAssert(e)
#endif
/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
#ifdef USE_STD_PRINTF
int std_printf(const char *fmt, ...);
#endif
#if defined(linux) || defined(_WIN32)
int std_get_log_level(void);
void std_set_log_level(int level);
int std_get_as_log_level(const char *name);
void std_set_log_name(const char *name);
#endif
/* ================================ [ DATAS     ] ============================================== */
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ FUNCTIONS ] ============================================== */
#ifdef __cplusplus
}
#endif
#endif /* STD_DEBUG_H */

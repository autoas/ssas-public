/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2022 Parai Wang <parai@foxmail.com>
 */
/* ================================ [ INCLUDES  ] ============================================== */
#include <mutex>
#include <thread>
#include <sys/queue.h>

#include "canlib.h"
#include "canlib_types.hpp"
#include "TcpIp.h"

using namespace std::literals::chrono_literals;
/* ================================ [ MACROS    ] ============================================== */
#define CAN_PORT_MIN 9000

#define CAN_MAX_DLEN 64 /* 64 for CANFD */
#define CAN_MTU (CAN_MAX_DLEN + 5)

#define mCANID(frame)                                                                              \
  (((uint32_t)frame.data[CAN_MAX_DLEN + 0] << 24) +                                                \
   ((uint32_t)frame.data[CAN_MAX_DLEN + 1] << 16) +                                                \
   ((uint32_t)frame.data[CAN_MAX_DLEN + 2] << 8) + ((uint32_t)frame.data[CAN_MAX_DLEN + 3]))

#define mSetCANID(frame, canid)                                                                    \
  do {                                                                                             \
    frame.data[CAN_MAX_DLEN + 0] = (uint8_t)(canid >> 24);                                         \
    frame.data[CAN_MAX_DLEN + 1] = (uint8_t)(canid >> 16);                                         \
    frame.data[CAN_MAX_DLEN + 2] = (uint8_t)(canid >> 8);                                          \
    frame.data[CAN_MAX_DLEN + 3] = (uint8_t)(canid);                                               \
  } while (0)

#define mCANDLC(frame) ((uint8_t)frame.data[CAN_MAX_DLEN + 4])
#define mSetCANDLC(frame, dlc)                                                                     \
  do {                                                                                             \
    frame.data[CAN_MAX_DLEN + 4] = dlc;                                                            \
  } while (0)
/* ================================ [ TYPES     ] ============================================== */
struct can_frame {
  uint8_t data[CAN_MTU];
};

struct Can_SerialHandle_s {
  uint32_t busid;
  uint32_t port;
  int online;
  TcpIp_SocketIdType sock;
  can_device_rx_notification_t rx_notification;
  struct can_frame frame;
  int index;
  STAILQ_ENTRY(Can_SerialHandle_s) entry;
};
struct Can_SerialHandleList_s {
  bool initialized;
  std::thread rx_thread;
  volatile boolean terminated;
  STAILQ_HEAD(, Can_SerialHandle_s) head;
  std::mutex mutex;
};
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
static bool qs_probe(int busid, uint32_t port, uint32_t baudrate,
                     can_device_rx_notification_t rx_notification);
static bool qs_write(uint32_t port, uint32_t canid, uint8_t dlc, const uint8_t *data,
                     uint64_t timestamp);
static void qs_close(uint32_t port);
static void rx_daemon(void *);
/* ================================ [ DATAS     ] ============================================== */
extern "C" const Can_DeviceOpsType can_qs_ops = {
  .name = "qemu",
  .probe = qs_probe,
  .close = qs_close,
  .write = qs_write,
};

static struct Can_SerialHandleList_s serialH = {
  .initialized = false,
  .terminated = false,
};
/* ================================ [ LOCALS    ] ============================================== */
static struct Can_SerialHandle_s *getHandle(uint32_t port) {
  struct Can_SerialHandle_s *handle, *h;
  handle = NULL;

  std::lock_guard<std::mutex>(serialH.mutex);
  STAILQ_FOREACH(h, &serialH.head, entry) {
    if (h->port == port) {
      handle = h;
      break;
    }
  }

  return handle;
}

static bool qs_probe(int busid, uint32_t port, uint32_t baudrate,
                     can_device_rx_notification_t rx_notification) {
  bool rv = true;
  struct Can_SerialHandle_s *handle;
  TcpIp_SocketIdType sock;
  TcpIp_SockAddrType RemoteAddr;
  Std_ReturnType ret;

  if (false == serialH.initialized) {
    STAILQ_INIT(&serialH.head);
    TcpIp_Init(NULL);
    serialH.initialized = true;
    serialH.terminated = true;
  }

  handle = getHandle(port);

  if (handle) {
    ASLOG(WARN, ("CAN qemu port=%d is already on-line, no need to probe it again!\n", port));
    rv = false;
  } else {
    sock = TcpIp_Create(TCPIP_IPPROTO_TCP);
    if (sock >= 0) {
      TcpIp_SetupAddrFrom(&RemoteAddr, TCPIP_IPV4_ADDR(127, 0, 0, 1), CAN_PORT_MIN + port);
      ret = TcpIp_TcpConnect(sock, &RemoteAddr);
      if (E_OK != ret) {
        TcpIp_Close(sock, TRUE);
        rv = FALSE;
        ASLOG(ERROR, ("CAN qemu connect to 127.0.0.1:%d failed\n", CAN_PORT_MIN + port));
      }
    } else {
      rv = FALSE;
      ASLOG(ERROR, ("CAN qemu create read sock failed\n"));
    }

    if (rv) { /* open port OK */
      handle = new struct Can_SerialHandle_s;
      assert(handle);
      handle->busid = busid;
      handle->port = port;
      handle->rx_notification = rx_notification;
      handle->sock = sock;
      handle->index = 0;
      std::lock_guard<std::mutex>(serialH.mutex);
      STAILQ_INSERT_TAIL(&serialH.head, handle, entry);
    } else {
      rv = false;
    }
  }

  std::lock_guard<std::mutex>(serialH.mutex);
  if ((true == serialH.terminated) && (false == STAILQ_EMPTY(&serialH.head))) {
    serialH.terminated = false;
    serialH.rx_thread = std::thread(rx_daemon, nullptr);
  }

  return rv;
}
static bool qs_write(uint32_t port, uint32_t canid, uint8_t dlc, const uint8_t *data,
                     uint64_t timestamp) {
  bool rv = true;
  struct can_frame frame;
  Std_ReturnType ret;
  struct Can_SerialHandle_s *handle = getHandle(port);
  (void)timestamp;
  if (handle != NULL) {
    mSetCANID(frame, canid);
    mSetCANDLC(frame, dlc);
    assert(dlc <= CAN_MAX_DLEN);
    memcpy(frame.data, data, dlc);
    std::lock_guard<std::mutex>(serialH.mutex);
    ret = TcpIp_Send(handle->sock, (const uint8_t *)&frame, CAN_MTU);
    if (E_OK != ret) {
      ASLOG(WARN, ("CAN qemu port=%d send message failed!\n", port));
      rv = false;
    }
  } else {
    rv = false;
    ASLOG(WARN, ("CAN qemu port=%d is not on-line, not able to send message!\n", port));
  }

  return rv;
}

static void qs_close(uint32_t port) {
  struct Can_SerialHandle_s *handle = getHandle(port);

  if (NULL != handle) {
    std::lock_guard<std::mutex>(serialH.mutex);
    STAILQ_REMOVE(&serialH.head, handle, Can_SerialHandle_s, entry);
    TcpIp_Close(handle->sock, TRUE);
    delete handle;

    if (true == STAILQ_EMPTY(&serialH.head)) {
      serialH.terminated = true;
      if (serialH.rx_thread.joinable()) {
        serialH.rx_thread.join();
      }
    }
  }
}

static void rx_notifiy(struct Can_SerialHandle_s *handle) {
  uint32_t len;
  uint8_t *data;
  Std_ReturnType ret;
  do {
    len = sizeof(handle->frame) - handle->index;
    data = &handle->frame.data[handle->index];
    ret = TcpIp_Recv(handle->sock, data, &len);
    if ((E_OK == ret) && (len > 0)) {
      handle->index += len;
    }
    if ((E_OK == ret) && (handle->index == sizeof(handle->frame))) {
      handle->index = 0;
      handle->rx_notification(handle->busid, mCANID(handle->frame), mCANDLC(handle->frame),
                              handle->frame.data, 0);
    } else {
      ret = E_NOT_OK;
    }
  } while (E_OK == ret);
}

static void rx_daemon(void *param) {
  (void)param;
  struct Can_SerialHandle_s *handle;
  while (false == serialH.terminated) {
    std::lock_guard<std::mutex>(serialH.mutex);
    STAILQ_FOREACH(handle, &serialH.head, entry) {
      rx_notifiy(handle);
    }
    std::this_thread::sleep_for(1ms);
  }
}
/* ================================ [ FUNCTIONS ] ============================================== */

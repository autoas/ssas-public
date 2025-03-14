/**
 * SSAS - Simple Smart Automotive Software
 * Copyright (C) 2021 Parai Wang <parai@foxmail.com>
 */
#ifndef LINTP_PRIV_H
#define LINTP_PRIV_H
/* ================================ [ INCLUDES  ] ============================================== */
/* ================================ [ MACROS    ] ============================================== */
#define DET_THIS_MODULE_ID MODULE_ID_LINTP

#define CanTp_AddressingFormatType LinTp_AddressingFormatType
#define CanTp_ChannelConfigType LinTp_ChannelConfigType
#define CanTp_ChannelContextType LinTp_ChannelContextType

#define CanTp_Config LinTp_Config

#define CanIf_Transmit LinIf_TpTransmit

#ifndef PduR_CanTpCopyTxData
#define PduR_CanTpCopyTxData PduR_LinTpCopyTxData
#endif
#ifndef PduR_CanTpRxIndication
#define PduR_CanTpRxIndication PduR_LinTpRxIndication
#endif
#ifndef PduR_CanTpTxConfirmation
#define PduR_CanTpTxConfirmation PduR_LinTpTxConfirmation
#endif
#ifndef PduR_CanTpStartOfReception
#define PduR_CanTpStartOfReception PduR_LinTpStartOfReception
#endif
#ifndef PduR_CanTpCopyRxData
#define PduR_CanTpCopyRxData PduR_LinTpCopyRxData
#endif

#define LINTP_PHYSICAL CANTP_PHYSICAL

#define CANTP_MAIN_FUNCTION_PERIOD LINTP_MAIN_FUNCTION_PERIOD
/* ================================ [ TYPES     ] ============================================== */
/* ================================ [ DECLARES  ] ============================================== */
/* ================================ [ DATAS     ] ============================================== */
/* ================================ [ LOCALS    ] ============================================== */
/* ================================ [ ALIAS     ] ============================================== */
#include "CanTp_Priv.h"
/* ================================ [ FUNCTIONS ] ============================================== */

#endif /* LINTP_PRIV_H */

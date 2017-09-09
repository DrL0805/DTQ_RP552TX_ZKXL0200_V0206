/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include <string.h>
#include "app_error.h"
#include "app_scheduler.h"
#include "ser_config.h"
#include "ser_conn_handlers.h"
#include "ser_conn_event_encoder.h"
#include "ser_conn_pkt_decoder.h"
#include "ser_conn_dtm_cmd_decoder.h"


/** @file
 *
 * @defgroup ser_conn_handlers Events handlers for the Connectivity Chip.
 * @{
 * @ingroup sdk_lib_serialization
 *
 * @brief   A module to handle the Connectivity application events.
 *
 * @details There are two types of events in the Connectivity application: BLE events generated by
 *          the SoftDevice and events generated by the HAL Transport layer.
 */

/** Parameters of a received packet. */
static ser_hal_transport_evt_rx_pkt_received_params_t m_rx_pkt_received_params;

/** Indicator of received packet that should be process. */
static bool m_rx_pkt_to_process = false;


void ser_conn_hal_transport_event_handle(ser_hal_transport_evt_t event)
{
    switch (event.evt_type)
    {
        case SER_HAL_TRANSP_EVT_TX_PKT_SENT:
        {
            /* SoftDevice event or response to received packet was sent, so unblock the application
             * scheduler to process a next event. */
            app_sched_resume();

            /* Check if chip is ready to enter DTM mode. */
            ser_conn_is_ready_to_enter_dtm();

            break;
        }

        case SER_HAL_TRANSP_EVT_RX_PKT_RECEIVING:
        {
            /* The connectivity side has started receiving a packet. Temporary block processing
             * SoftDevice events. It is going to be unblocked when a response for the packet will
             * be sent. This prevents communication block. */
            app_sched_pause();
            break;
        }

        case SER_HAL_TRANSP_EVT_RX_PKT_RECEIVED:
        {
            /* We can NOT add received packets as events to the application scheduler queue because
             * received packets have to be processed before SoftDevice events but the scheduler
             * queue do not have priorities. */
            memcpy(&m_rx_pkt_received_params, &event.evt_params.rx_pkt_received,
                   sizeof (ser_hal_transport_evt_rx_pkt_received_params_t));
            m_rx_pkt_to_process = true;
            break;
        }

        case SER_HAL_TRANSP_EVT_RX_PKT_DROPPED:
        {
            APP_ERROR_CHECK(SER_WARNING_CODE);
            break;
        }

        case SER_HAL_TRANSP_EVT_PHY_ERROR:
        {
            APP_ERROR_CHECK(NRF_ERROR_FORBIDDEN);
            break;
        }

        default:
        {
            /* do nothing */
            break;
        }
    }
}


uint32_t ser_conn_rx_process(void)
{
    uint32_t err_code = NRF_SUCCESS;

    if (m_rx_pkt_to_process)
    {
        /* No critical section needed on m_rx_pkt_to_process parameter because it is not possible
         * to get next packet before sending a response. */
        m_rx_pkt_to_process = false;
        err_code            = ser_conn_received_pkt_process(&m_rx_pkt_received_params);
    }

    return err_code;
}


void ser_conn_ble_event_handle(ble_evt_t * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    /* We can NOT encode and send BLE events here. SoftDevice handler implemented in
     * softdevice_handler.c pull all available BLE events at once but we need to reschedule between
     * encoding and sending every BLE event because sending a response on received packet has higher
     * priority than sending a BLE event. Solution for that is to put BLE events into application
     * scheduler queue to be processed at a later time. */
    err_code = app_sched_event_put(p_ble_evt, sizeof (ble_evt_hdr_t) + p_ble_evt->header.evt_len,
                                   ser_conn_ble_event_encoder);
    APP_ERROR_CHECK(err_code);
}

/** @} */

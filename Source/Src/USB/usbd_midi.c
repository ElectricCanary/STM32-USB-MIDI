/**
  ******************************************************************************
  * @file    usbd_cdc.c
  * @author  MCD Application Team
  * @brief   This file provides the high layer firmware functions to manage the
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  *
  *           These aspects may be enriched or modified for a specific user application.
  *
  *            This driver doesn't implement the following aspects of the specification
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include <USB/usbd_ctlreq.h>
#include <USB/usbd_midi.h>


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_CDC
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CDC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CDC_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CDC_Private_FunctionPrototypes
  * @{
  */

static uint8_t  USBD_MIDI_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  *USBD_MIDI_GetCfgDesc (uint16_t *length);
//uint8_t  *USBD_MIDI_GetDeviceQualifierDescriptor (uint16_t *length);
USBD_HandleTypeDef *pInstance = NULL;

uint32_t APP_Rx_ptr_in  = 0;
uint32_t APP_Rx_ptr_out = 0;
uint32_t APP_Rx_length  = 0;
uint8_t  USB_Tx_State = USB_TX_READY;

__ALIGN_BEGIN uint8_t USB_Rx_Buffer[MIDI_DATA_OUT_PACKET_SIZE] __ALIGN_END ;
__ALIGN_BEGIN uint8_t APP_Rx_Buffer[APP_RX_DATA_SIZE] __ALIGN_END ;


/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Variables
  * @{
  */


/* USB MIDI interface class callbacks structure */
USBD_ClassTypeDef  USBD_MIDI =
{
  USBD_MIDI_Init,
  USBD_MIDI_DeInit,
  NULL,
  NULL,
  NULL,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,
  NULL,
  NULL,
  NULL,
  NULL,// HS
  USBD_MIDI_GetCfgDesc,// FS
  NULL,// OTHER SPEED
  NULL,// DEVICE_QUALIFIER
};

/* USB MIDI device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_MIDI_CfgDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END =
{
/*
  // configuration descriptor for two midi cables
  0x09, 0x02, LOBYTE(USB_MIDI_CONFIG_DESC_SIZ), HIBYTE(USB_MIDI_CONFIG_DESC_SIZ),
  0x02, 0x01, 0x00, 0x80, 0x31,
  // The Audio Interface Collection
  0x09, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x02, // Standard AC Interface Descriptor /2
  0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x01, // Class-specific AC Interface Descriptor
  0x09, 0x04, 0x01, 0x00, 0x02, 0x01, 0x03, 0x00, 0x02, // MIDIStreaming Interface Descriptors /2
  0x07, 0x24, 0x01, 0x00, 0x01, 0x41, 0x00,             // Class-Specific MS Interface Header Descriptor
  // MIDI IN JACKS
  0x06, 0x24, 0x02, 0x01, 0x01, 0x00,
  0x06, 0x24, 0x02, 0x02, 0x02, 0x00,
  // MIDI OUT JACKS
  0x09, 0x24, 0x03, 0x01, 0x03, 0x01, 0x02, 0x01, 0x00,
  0x09, 0x24, 0x03, 0x02, 0x04, 0x01, 0x01, 0x01, 0x00,
  // MIDI IN JACKS
  0x06, 0x24, 0x02, 0x01, 0x05, 0x00,
  0x06, 0x24, 0x02, 0x02, 0x06, 0x00,
  // MIDI OUT JACKS
  0x09, 0x24, 0x03, 0x01, 0x07, 0x01, 0x06, 0x01, 0x00,
  0x09, 0x24, 0x03, 0x02, 0x08, 0x01, 0x05, 0x01, 0x00,
  // OUT endpoint descriptor
  0x09, 0x05, MIDI_OUT_EP, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00,
  0x06, 0x25, 0x01, 0x02, 0x01, 0x05,
  // IN endpoint descriptor
  0x09, 0x05, MIDI_IN_EP, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00,
  0x06, 0x25, 0x01, 0x02, 0x03, 0x07,
*/

  // configuration descriptor for single midi cable
  0x09, 0x02, LOBYTE(USB_MIDI_CONFIG_DESC_SIZ), HIBYTE(USB_MIDI_CONFIG_DESC_SIZ), 0x02, 0x01, 0x00, 0x80, 0x31,
  // The Audio Interface Collection
  0x09, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, // Standard AC Interface Descriptor
  0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x01, // Class-specific AC Interface Descriptor
  0x09, 0x04, 0x01, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00, // MIDIStreaming Interface Descriptors
  0x07, 0x24, 0x01, 0x00, 0x01, 0x41, 0x00,             // Class-Specific MS Interface Header Descriptor
  // MIDI IN JACKS
  0x06, 0x24, 0x02, 0x01, 0x01, 0x00,
  0x06, 0x24, 0x02, 0x02, 0x02, 0x00,
  // MIDI OUT JACKS
  0x09, 0x24, 0x03, 0x01, 0x03, 0x01, 0x02, 0x01, 0x00,
  0x09, 0x24, 0x03, 0x02, 0x04, 0x01, 0x01, 0x01, 0x00,
  // OUT endpoint descriptor
  0x09, 0x05, MIDI_OUT_EP, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00,
  0x05, 0x25, 0x01, 0x01, 0x01,
  // IN endpoint descriptor
  0x09, 0x05, MIDI_IN_EP, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00,
  0x05, 0x25, 0x01, 0x01, 0x03
};

/**
  * @}
  */

/** @defgroup USBD_CDC_Private_Functions
  * @{
  */

/**
  * @brief  USBD_CDC_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx){
  pInstance = pdev;
  USBD_LL_OpenEP(pdev,MIDI_IN_EP,USBD_EP_TYPE_BULK,MIDI_DATA_IN_PACKET_SIZE);
  USBD_LL_OpenEP(pdev,MIDI_OUT_EP,USBD_EP_TYPE_BULK,MIDI_DATA_OUT_PACKET_SIZE);
  USBD_LL_PrepareReceive(pdev,MIDI_OUT_EP,(uint8_t*)(USB_Rx_Buffer),MIDI_DATA_OUT_PACKET_SIZE);
  return 0;
}

/**
  * @brief  USBD_CDC_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx){
  pInstance = NULL;
  USBD_LL_CloseEP(pdev,MIDI_IN_EP);
  USBD_LL_CloseEP(pdev,MIDI_OUT_EP);
  return 0;
}

static uint8_t USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  if (APP_Rx_length != 0)
   {
    USB_Tx_State = USB_TX_CONTINUE;
   }
  else
   {
    APP_Rx_ptr_out = 0;
    APP_Rx_ptr_in = 0;
    USB_Tx_State = USB_TX_READY;
   }
  //!!!!!!!!!!! �*�� �**��� ������� !!!!!!!!!!!!!!!1111
  //USBD_MIDI_DataOut (pInstance, 1);
  return USBD_OK;
}

static uint8_t USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
 {
  uint16_t USB_Rx_Cnt;
  USBD_MIDI_ItfTypeDef *pmidi;

  USB_Rx_Cnt = ((PCD_HandleTypeDef*)pdev->pData)->OUT_ep[epnum].xfer_count;
  if (USB_Rx_Cnt > 0) //��������� ������� - ���� ������� �������, � ������ ���, �� ������ ����� �������
  {
    pmidi = (USBD_MIDI_ItfTypeDef *)(pdev->pUserData);
    pmidi->pIf_MidiRx((uint8_t *)&USB_Rx_Buffer, USB_Rx_Cnt);
  }
  USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, (uint8_t*)(USB_Rx_Buffer), MIDI_DATA_OUT_PACKET_SIZE);
  return USBD_OK;
 }

static uint8_t *USBD_MIDI_GetCfgDesc (uint16_t *length)
 {
  *length = sizeof (USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
 }

//uint8_t *USBD_MIDI_GetDeviceQualifierDescriptor (uint16_t *length){
//  *length = sizeof (USBD_MIDI_DeviceQualifierDesc);
//  return USBD_MIDI_DeviceQualifierDesc;
//}

uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI_ItfTypeDef *fops)
{
  uint8_t ret = USBD_FAIL;

  if(fops != NULL)
   {
    pdev->pUserData= fops;
    ret = USBD_OK;
   }

  return ret;
}


//MIDI TX START FUNCTION
void USBD_MIDI_SendPacket()
{
 uint16_t USB_Tx_ptr;
 uint16_t USB_Tx_length;
 if (APP_Rx_ptr_out == APP_RX_DATA_SIZE)
  {
   APP_Rx_ptr_out = 0;
  }

 if(APP_Rx_ptr_out == APP_Rx_ptr_in)
  {
   USB_Tx_State = USB_TX_READY;
   return;
  }

 if(APP_Rx_ptr_out > APP_Rx_ptr_in)
  {
   APP_Rx_length = APP_RX_DATA_SIZE - APP_Rx_ptr_out;
  }
 else
  {
   APP_Rx_length = APP_Rx_ptr_in - APP_Rx_ptr_out;
  }

 if (APP_Rx_length > MIDI_DATA_IN_PACKET_SIZE)
  {
   USB_Tx_ptr = APP_Rx_ptr_out;
   USB_Tx_length   = MIDI_DATA_IN_PACKET_SIZE;
   APP_Rx_ptr_out += MIDI_DATA_IN_PACKET_SIZE;
   APP_Rx_length  -= MIDI_DATA_IN_PACKET_SIZE;
  }
 else
  {
   USB_Tx_ptr      = APP_Rx_ptr_out;
   USB_Tx_length   = APP_Rx_length;
   APP_Rx_ptr_out += APP_Rx_length;
   APP_Rx_length   = 0;
  }

 USB_Tx_State = USB_TX_BUSY;
 USBD_LL_Transmit(pInstance, MIDI_IN_EP, &APP_Rx_Buffer[USB_Tx_ptr], USB_Tx_length);
 //USB_Tx_State = USB_TX_READY;
}

/*void resetep(void)
{
  USBD_LL_PrepareReceive(pInstance,MIDI_OUT_EP,(uint8_t*)(USB_Rx_Buffer),MIDI_DATA_OUT_PACKET_SIZE);
}*/

/**
  ******************************************************************************
  * @file           : usbd_midi_if.h
  * @brief          : Header for usbd_midi_if file.
  ******************************************************************************
*/

#ifndef __USBD_MIDI_IF_H
#define __USBD_MIDI_IF_H

#ifdef __cplusplus
 extern "C" {
#endif

#define USBMIDIBUFSIZE 2048 //must be power of 2
#define USBMIDIMASK (USBMIDIBUFSIZE-1)

#include <USB/usbd_midi.h>
#include <USB/usbd_desc.h>

//circuit buffer for midi rx data
typedef struct
{
  uint16_t curidx; //current pointer position
  uint16_t rdidx;  //reading pointer position
  uint8_t buf[USBMIDIBUFSIZE];
} tUsbMidiCable;

extern tUsbMidiCable usbmidicable1; //rx buf for virtual midi cable 1
extern tUsbMidiCable usbmidicable2; //rx buf for vortual midi cable 2

extern uint8_t MIDI_CHANNEL;
extern uint8_t MIDI_CABLE;

extern USBD_MIDI_ItfTypeDef  USBD_Interface_fops_FS;

//Create SysEx buffer
void USBD_AddSysExMessage(uint8_t cable, uint8_t *msg, uint8_t length);

//Create MIDI Message buffer
void USBD_MidiMessage(uint8_t cable, uint8_t ch, uint8_t byte1, uint8_t byte2, uint8_t byte3);

//Start transfer buffer
void USBD_SendMidiMessages(void);


//void OTG_FS_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif /* __USBD_MIDI_IF_H */

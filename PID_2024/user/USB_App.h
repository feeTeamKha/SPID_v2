#ifndef _USB_APP_H_
#define _USB_APP_H_

#include "usbd_customhid.h"
#include <PID.h>

#define DATA_SIZE 64
#define TIME_OUT_SEND 20 //ms

typedef enum 
{
    NoneCMD ,Save_config = 1, Get_infor,
}CMD_Request;
typedef struct _data_HID
{
  uint16_t timeOut;
  uint8_t isConnected;
  uint8_t header[2];
  uint8_t requestCMD;
  uint8_t  isUSBControl;
  uint8_t* buffer[DATA_SIZE];
  uint8_t dataIn[DATA_SIZE];
  uint8_t dataOut[DATA_SIZE];
  void(*f_param_init)(struct _data_HID *hid);
  void(*f_data_write)(struct _data_HID *hid);
  void(*f_data_read)(struct _data_HID *hid);
  void(*f_process_data)(struct _data_HID *hid);
  void(*f_request_func)(struct _data_HID *hid);
}dataHID_typedef;
extern dataHID_typedef usbPID;
void HID_Data_Init(dataHID_typedef *hid);
//extern USBD_HandleTypeDef 				  *pdev1;
//extern USBD_CUSTOM_HID_HandleTypeDef      *hhid1;

#endif /* _USB_APP_H_ */
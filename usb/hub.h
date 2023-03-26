#ifndef HUB_H
#define HUB_H

#include <stdbool.h>
#include <inttypes.h>

typedef struct {
  uint8_t  bNbrPorts;	    // number of ports
  uint32_t qLastPollTime;   // last poll time
  bool	   bPollEnable;	    // poll enable flag
  ep_t ep;	            // interrupt endpoint info structure
} usb_hub_info_t;

// Hub Requests
#define USB_HUB_REQ_CLEAR_HUB_FEATURE		USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_DEVICE
#define USB_HUB_REQ_CLEAR_PORT_FEATURE	USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER
#define USB_HUB_REQ_CLEAR_TT_BUFFER		USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER
#define USB_HUB_REQ_GET_HUB_DESCRIPTOR	USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_DEVICE
#define USB_HUB_REQ_GET_HUB_STATUS		USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_DEVICE
#define USB_HUB_REQ_GET_PORT_STATUS		USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER
#define USB_HUB_REQ_RESET_TT			USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER
#define USB_HUB_REQ_SET_HUB_DESCRIPTOR	USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_DEVICE
#define USB_HUB_REQ_SET_HUB_FEATURE		USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_DEVICE
#define USB_HUB_REQ_SET_PORT_FEATURE		USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER
#define USB_HUB_REQ_GET_TT_STATE		USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER
#define USB_HUB_REQ_STOP_TT			USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_OTHER

// Hub Features
#define HUB_FEATURE_C_HUB_LOCAL_POWER	0
#define HUB_FEATURE_C_HUB_OVER_CURRENT	1

#define HUB_FEATURE_PORT_CONNECTION	0
#define HUB_FEATURE_PORT_ENABLE		1
#define HUB_FEATURE_PORT_SUSPEND	2
#define HUB_FEATURE_PORT_OVER_CURRENT	3
#define HUB_FEATURE_PORT_RESET		4
#define HUB_FEATURE_PORT_POWER		8
#define HUB_FEATURE_PORT_LOW_SPEED	9
#define HUB_FEATURE_C_PORT_CONNECTION	16
#define HUB_FEATURE_C_PORT_ENABLE	17
#define HUB_FEATURE_C_PORT_SUSPEND	18
#define HUB_FEATURE_C_PORT_OVER_CURRENT	19
#define HUB_FEATURE_C_PORT_RESET	20
#define HUB_FEATURE_PORT_TEST		21
#define HUB_FEATURE_PORT_INDICATOR	22

// Additional Error Codes
#define HUB_ERROR_PORT_HAS_BEEN_RESET			0xb1

// Hub Port Status Bitmasks 
#define USB_HUB_PORT_STATUS_PORT_CONNECTION		0x0001
#define USB_HUB_PORT_STATUS_PORT_ENABLE			0x0002
#define USB_HUB_PORT_STATUS_PORT_SUSPEND			0x0004
#define USB_HUB_PORT_STATUS_PORT_OVER_CURRENT		0x0008
#define USB_HUB_PORT_STATUS_PORT_RESET			0x0010
#define USB_HUB_PORT_STATUS_PORT_POWER			0x0100
#define USB_HUB_PORT_STATUS_PORT_LOW_SPEED		0x0200
#define USB_HUB_PORT_STATUS_PORT_HIGH_SPEED		0x0400
#define USB_HUB_PORT_STATUS_PORT_TEST			0x0800
#define USB_HUB_PORT_STATUS_PORT_INDICATOR		0x1000

// Bit mask to check for DISABLED state in HubEvent::bmStatus field 
#define USB_HUB_PORT_STATE_CHECK_DISABLED	(USB_HUB_PORT_STATUS_PORT_POWER | USB_HUB_PORT_STATUS_PORT_ENABLE | USB_HUB_PORT_STATUS_PORT_CONNECTION | USB_HUB_PORT_STATUS_PORT_SUSPEND)

// Hub Port States
#define USB_HUB_PORT_STATE_DISABLED		(USB_HUB_PORT_STATUS_PORT_POWER | USB_HUB_PORT_STATUS_PORT_CONNECTION)

// Hub Port Events
#define USB_HUB_PORT_EVENT_CONNECT		(((0UL | USB_HUB_PORT_STATUS_PORT_CONNECTION)	<< 16) | USB_HUB_PORT_STATUS_PORT_POWER | USB_HUB_PORT_STATUS_PORT_CONNECTION)
#define USB_HUB_PORT_EVENT_DISCONNECT		(((0UL | USB_HUB_PORT_STATUS_PORT_CONNECTION)	<< 16) | USB_HUB_PORT_STATUS_PORT_POWER)
#define USB_HUB_PORT_EVENT_RESET_COMPLETE	(((0UL | USB_HUB_PORT_STATUS_PORT_RESET) << 16) | USB_HUB_PORT_STATUS_PORT_POWER | USB_HUB_PORT_STATUS_PORT_ENABLE | USB_HUB_PORT_STATUS_PORT_CONNECTION)

#define USB_HUB_PORT_EVENT_LS_CONNECT		(((0UL | USB_HUB_PORT_STATUS_PORT_CONNECTION)	<< 16) | USB_HUB_PORT_STATUS_PORT_POWER | USB_HUB_PORT_STATUS_PORT_CONNECTION | USB_HUB_PORT_STATUS_PORT_LOW_SPEED)
#define USB_HUB_PORT_EVENT_LS_RESET_COMPLETE	(((0UL | USB_HUB_PORT_STATUS_PORT_RESET)		<< 16) | USB_HUB_PORT_STATUS_PORT_POWER | USB_HUB_PORT_STATUS_PORT_ENABLE | USB_HUB_PORT_STATUS_PORT_CONNECTION | USB_HUB_PORT_STATUS_PORT_LOW_SPEED)
#define USB_HUB_PORT_EVENT_LS_PORT_ENABLED	(((0UL | USB_HUB_PORT_STATUS_PORT_CONNECTION | USB_HUB_PORT_STATUS_PORT_ENABLE) << 16) | USB_HUB_PORT_STATUS_PORT_POWER | USB_HUB_PORT_STATUS_PORT_ENABLE | USB_HUB_PORT_STATUS_PORT_CONNECTION | USB_HUB_PORT_STATUS_PORT_LOW_SPEED)

typedef struct {
  uint8_t bDescLength;		// descriptor length
  uint8_t bDescriptorType;	// descriptor type
  uint8_t bNbrPorts;		// number of ports a hub equipped with
  
  struct {
    uint16_t LogPwrSwitchMode	     : 2;
    uint16_t CompoundDevice	     : 1;
    uint16_t OverCurrentProtectMode  : 2;
    uint16_t TTThinkTime	     : 2;
    uint16_t PortIndicatorsSupported : 1;
    uint16_t Reserved		     : 8;
  } __attribute__((packed));
  
  uint8_t bPwrOn2PwrGood;
  uint8_t bHubContrCurrent;
} __attribute__((packed)) usb_hub_descriptor_t;

typedef struct {
  union {
    struct {
      uint16_t bmStatus;	// port status bits
      uint16_t bmChange;	// port status change bits
    } __attribute__((packed));
    uint32_t bmEvent;
    uint8_t  evtBuff[4];
  } __attribute__((packed));
}__attribute__((packed)) hub_event_t;

// interface to usb core
extern const usb_device_class_config_t usb_hub_class;

#endif // HUB_H

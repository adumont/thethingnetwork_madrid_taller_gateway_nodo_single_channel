/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

#include "TtnOtaa.h"
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <U8x8lib.h>



#if 1
// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
// Change this address for every node!
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// This should also be in little endian format, see above.
// Change this address for every node!
static const u1_t PROGMEM DEVEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
// Change this address for every node!
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#else
#include "config.h"
#endif

static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

//#define OLED

#ifdef OLED
// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8 (/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
#endif //OLED

void onLmicEvent (ev_t ev) {
#ifdef OLED
	u8x8.setCursor (0, 3);
	u8x8.printf ("%0x", LMIC.devaddr);
	u8x8.setCursor (0, 5);
	u8x8.printf ("RSSI %d SNR: %d", LMIC.rssi, LMIC.snr);
#endif //OLED
    switch(ev) {
        case EV_SCAN_TIMEOUT:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_SCAN_TIMEOUT");
#endif //OLED
			break;
        case EV_BEACON_FOUND:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_BEACON_FOUND");
#endif //OLED
			break;
        case EV_BEACON_MISSED:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_BEACON_MISSED");
#endif //OLED
			break;
        case EV_BEACON_TRACKED:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_BEACON_TRACKED");
#endif //OLED
			break;
        case EV_JOINING:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_JOINING    ");
#endif //OLED
			break;
        case EV_JOINED:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_JOINED     ");
#endif //OLED
            break;
        case EV_RFU1:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_RFUI");
#endif //OLED
			break;
        case EV_JOIN_FAILED:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_JOIN_FAILED");
#endif //OLED
			break;
        case EV_REJOIN_FAILED:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_REJOIN_FAILED");
#endif //OLED
			break;
        case EV_TXCOMPLETE:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_TXCOMPLETE");
#endif //OLED
            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_LOST_TSYNC");
#endif //OLED
			break;
        case EV_RESET:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_RESET");
#endif //OLED
			break;
        case EV_RXCOMPLETE:
            // data received in ping slot
#ifdef OLED
			u8x8.drawString (0, 7, "EV_RXCOMPLETE");
#endif //OLED
			break;
        case EV_LINK_DEAD:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_LINK_DEAD");
#endif //OLED
			break;
        case EV_LINK_ALIVE:
#ifdef OLED
			u8x8.drawString (0, 7, "EV_LINK_ALIVE");
#endif //OLED
			break;
		case EV_TXSTART:
#ifdef OLED
			//u8x8.drawString (0, 7, "EV_TXSTART   ");
#endif //OLED
			break;
         default:
#ifdef OLED
			u8x8.printf ("UNKNOWN EVENT %d", ev);
#endif //OLED
			break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
	} else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
	}
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("Starting"));

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

	TtnOtaa.setEventHandler (onLmicEvent);
	TtnOtaa.begin ();

	// Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}

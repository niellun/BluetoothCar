#include "mbed.h"
#include "ble/BLE.h"


#define BLE_UUID_TXRX_SERVICE            0x0000 /**< The UUID of the Nordic UART Service. */
#define BLE_UUID_TX_CHARACTERISTIC       0x0002 /**< The UUID of the TX Characteristic. */
#define BLE_UUIDS_RX_CHARACTERISTIC      0x0003 /**< The UUID of the RX Characteristic. */

#define TXRX_BUF_LEN                     20

#define NAME                            "Biscuit"

BLE  ble;

//Serial pc(USBTX, USBRX);

// The Nordic UART Service
static const uint8_t uart_base_uuid[] = {0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_tx_uuid[]   = {0x71, 0x3D, 0, 3, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_rx_uuid[]   = {0x71, 0x3D, 0, 2, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static const uint8_t uart_base_uuid_rev[] = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0, 0, 0x3D, 0x71};

uint8_t txPayload[TXRX_BUF_LEN] = {0,};
uint8_t rxPayload[TXRX_BUF_LEN] = {0,};

GattCharacteristic  txCharacteristic (uart_tx_uuid, txPayload, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
                                      
GattCharacteristic  rxCharacteristic (uart_rx_uuid, rxPayload, 1, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
                                      
GattCharacteristic *uartChars[] = {&txCharacteristic, &rxCharacteristic};

GattService         uartService(uart_base_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));


DigitalOut Motor1(P0_10, 1);
DigitalOut Motor2(P0_9, 1);
DigitalOut Led1(D0, 0);
DigitalOut Led2(D3, 0);

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    ble.startAdvertising();
}

void WrittenHandler(const GattWriteCallbackParams *Handler)
{   
    uint8_t buf[TXRX_BUF_LEN];
    uint16_t bytesRead;
    
    if (Handler->handle == txCharacteristic.getValueAttribute().getHandle()) 
    {
        ble.readCharacteristicValue(txCharacteristic.getValueAttribute().getHandle(), buf, &bytesRead);
        memset(txPayload, 0, TXRX_BUF_LEN);
        memcpy(txPayload, buf, TXRX_BUF_LEN);       
        if(bytesRead>=3)
        {
            Motor1.write(txPayload[0]>100?0:1);  
            Motor2.write(txPayload[1]>100?0:1); 
            Led1.write(txPayload[2]>100?0:1); 
            Led2.write(txPayload[2]>100?0:1);
        }
    }
}



int main(void)
{

    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(WrittenHandler);  

   // setup advertising 
    ble.accumulateAdvertisingPayload(GapAdvertisingData::GENERIC_PHONE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME, (const uint8_t *)NAME, sizeof(NAME) - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid));

    ble.setAdvertisingPayload();
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.setActiveScan(false);
    ble.setAdvertisingInterval(100);

    ble.addService(uartService);
    ble.startAdvertising(); 

    while(1)
    {
        ble.waitForEvent(); 
    }
}

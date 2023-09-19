# Description
Anemometer and rain detector with Wemos D1 Mini (ESP8266). Powered by a 18650 battery and solar panels. The values are sent to a MQTT server every minute. The wemos d1 mini turns to deep sleep to save power.

# Bill of materials
- Wemos d1 mini: https://fr.aliexpress.com/item/32831353752.html?spm=a2g0o.order_list.order_list_main.5.535d5e5bnZzy1w&gatewayAdapt=glo2fra
- MT3608 DC-DC Step Up: https://fr.aliexpress.com/item/1005001622010062.html?spm=a2g0o.order_list.order_list_main.11.535d5e5bnZzy1w&gatewayAdapt=glo2fra
- Rain detector: https://fr.aliexpress.com/item/1005002224429237.html?spm=a2g0o.order_list.order_list_main.10.535d5e5bnZzy1w&gatewayAdapt=glo2fra
- Anemometer NPN DC5-30V: https://fr.aliexpress.com/item/4000590278227.html?spm=a2g0o.order_list.order_list_main.23.535d5e5bnZzy1w&gatewayAdapt=glo2fra
- Solar panels 2*1W 5V: https://fr.aliexpress.com/item/32844672794.html?spm=a2g0o.order_list.order_list_main.29.535d5e5bnZzy1w&gatewayAdapt=glo2fra
- Battery 18650 3500mAh: https://fr.aliexpress.com/item/32810252344.html?spm=a2g0o.order_list.order_list_main.72.535d5e5bnZzy1w&gatewayAdapt=glo2fra
- TP4056 Li-lon Battery Charger: https://fr.aliexpress.com/item/1005005341202402.html?spm=a2g0o.productlist.main.21.7f9d69d3lzxJqs&algo_pvid=6c66791c-1d5e-44fd-bc36-4e7d3ac250f0&algo_exp_id=6c66791c-1d5e-44fd-bc36-4e7d3ac250f0-10&pdp_npi=4%40dis%21EUR%214.53%212.45%21%21%214.76%21%21%40211b88f116946321726576023efcfb%2112000032671731628%21sea%21FR%213296225565%21S&curPageLogUid=nqsTiVLaFvhV
- 100 k ohms resistor
- Jumper wires

# Wiring

<img src="docs/schema.jpg"/>
<img src="docs/20230908_173628.jpg"/>


**Battery monitoring**: the voltage of an 18650 battery is 3.7V and can go up to 4.2V when it is fully charged. 
The Wemos D1 mini already has an internal voltage divider that connects the A0 pin to the ADC of the ESP8266 chip. This is a 220 k resistor over 100 k resistor. By adding a 100k resistor, it will in fact be a total of 420 k. So the ADC of the ESP8266 would get 4.2 * 100 / 420 = 1V with a fully charged 18650. 1V is the max input to the ADC and will give a raw reading of 1023. We consider that the battery is 0% when the voltage 3.3V. With the resistors the ESP8266 will get about 0.8V (3.3 * 100 / 420), which correspond to a raw reading of 820 (0.8 * 1 023).

**Anemometer**: it's powered by the D8 pin of the Wemos D1 Mini only during the measurement. As the output voltage of the Wemos D1 Mini is 3.3V and the anemometer needs at least 5V, an MT3608 is used to step up the voltage to 5V.

**Rain detector**: it's powered by the D6 pin of the Wemos D1 Mini only during the measurement. The potentiometer is calibrated by turning the screw. Counterclockwise for increased sensitivity.


# Sources
https://arduinodiy.wordpress.com/2016/12/25/monitoring-lipo-battery-voltage-with-wemos-d1-minibattery-shield-and-thingspeak/
https://github.com/StationMeteoDIY/anemometre/tree/main
https://how2electronics.com/interfacing-anemometer-npn-pulse-output-with-arduino/
https://lastminuteengineers.com/rain-sensor-arduino-tutorial/


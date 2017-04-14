# Enviro-Monitor-Station

 How to use in home-assistant?
 
 You can add the following sensor topic in your configuration.yaml
 
 sensor:
 
  - platform: mqtt
  
    name: light
    
    state_topic: "sensor/luminosity"
    
    unit_of_measurement: "%"
    
  - platform: mqtt
  
    name: noise
    
    state_topic: "sensor/noise"
    
    unit_of_measurement: "dB"
    
  - platform: mqtt
   
    name: temperature
    
    state_topic: "sensor/temperature"
    
    unit_of_measurement: "°C"
    
  - platform: mqtt
  
    name: humidity
    
    state_topic: "sensor/humidity"
    
    unit_of_measurement: "%"
 
  - platform: mqtt
 
    name: pm1
    
    state_topic: "sensor/pm1"
    
    unit_of_measurement: "ug/m³"
    
  - platform: mqtt
  
    name: pm25
    
    state_topic: "sensor/pm25"
    
    unit_of_measurement: "ug/m³"
    
  - platform: mqtt
  
    name: pm10
    
    state_topic: "sensor/pm10"
    
    unit_of_measurement: "ug/m³"    
      
  - platform: mqtt
    
    name: Ch2o
    
    state_topic: "sensor/ch2o"
    
    unit_of_measurement: "mg/m³"
    

Develop board:

  - [NodeMCU V1.0 ESP-12E](https://detail.tmall.com/item.htm?id=535588732894&spm=a1z09.2.0.0.kPM6Dz&_u=cktg8o8364)
  
The following sensors:

  - [LDR sensor](https://item.taobao.com/item.htm?spm=2013.1.20141002.5.duusCn&scm=1007.10009.70205.100200300000001&id=531468405248&pvid=dccb63ed-4bc3-4cb6-bc94-2e1cae9f4917)
  - [24bit WS2812 5050 RGB LED](https://item.taobao.com/item.htm?spm=a1z09.2.0.0.kPM6Dz&id=540785401008&_u=cktg8o42f1)
  - [DHT22 sensor AM2302](https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-14162336577.22.MHbiq5&id=543447398813)
  - [MQ-2 sensor](https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-14162336577.18.9clTO2&id=531451462058)
  - [Ch2O sensor](https://item.taobao.com/item.htm?spm=a1z09.2.0.0.kPM6Dz&id=526919367835&_u=cktg8oc609)
  - [Air sensor PM1/PM2.5/PM10](https://item.taobao.com/item.htm?spm=a1z09.2.0.0.kPM6Dz&id=526939702749&_u=cktg8o8b1e)
  


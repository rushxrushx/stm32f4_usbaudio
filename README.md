## CHEAPEST USB ASYNC AUDIO1 USING ST32F401RB

###### knows bugs:
- 1,if USB cable/plug is poor quality,audio may become noise when playing,and must stop or re-plug to fix it.  
Now usb rx endpoint CRC is disabled,it works much better.beacuse CRC only detect error but no re-transmission.    
also add Orange color LED indicate for lost packet warning.    

###### 2023.10.3
- add UART log ring buffer ,get out of 'while' when print
- hw ad1955 is ok    
- work for H81 motherboard usb compbility
- change panel LED display  

###### 2022.5.28
- HW seprate folder,(only ak4396 is work done now,other working in process).
- remove LICENSE,it's only for fun.

###### 2022.5.27
- improve usb setup--audio class,fix mute/vol,

###### 2020to2021
- add many HW board configs

###### 2019.12 
- add 24bit, add 48/88/96k audio streaming formats.
- fix audio buffer overflow 2words

###### initial release
- support 32bit.44k format
- async using osc input to stm32f4 i2s_ckin pin,cpld device not required anymore.
(please see the schemaitcs for more infomation).
- test with "xmos stereo driver",win10,linux.


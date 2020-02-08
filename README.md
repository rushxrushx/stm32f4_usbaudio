# 仅供个人使用，严禁商用Any Commercial use prohibited !!!

bugs now:

A.When some PC restart,hibrate,sleep,the card not found and must re-pluged.（某些电脑重启休眠后需要重新拔插才能识别

B.USB3.1 host through external usb3.0 hub,async feedback not successful tx to PC. direct connect to usb3.1 is ok.（USB3.1
主机接上外部usb3.0HUB不能发回反馈，直接连接可以正常用。


# 2019年12月最新F4 usb 异步声卡程序

1，外部双时钟，无需任何CPLD

2，支持 24bit 44.1/48/88.2/96k多种格式

3，支持XMOS驱动，win7自带驱动，各种linux



# 2019-12 updated stm32f4 usb async audio

1,async clock with external clock input to stm32f4 series i2s_ckin pin,cpld device not required anymore.(see the schemaitcs for more infomation).

2,NEW support 24bit 44.1/48/88.2/96k audio streaming formats.

3,working with "xmos stereo driver",win build-in driver,linux os.


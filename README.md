# ValidSoundDetector
자세한 사항은 Report-Microprocessor.pdf를 읽어주시기 바랍니다.   
## Source Code List
- Main/src/batterydisplay.c
- Main/src/batterydisplay.h
- Main/src/button.c
- Main/src/button.h
- Main/src/co2.c
- Main/src/co2.h
- Main/src/led.c
- Main/src/led.h
- Main/src/main.c
- Main/src/pir.h
- Main/src/value.h

## How to Build
1. nRF Connect로 해당 프로젝트를 Open한다.
2. Add Build Configuration를 눌러 Build환경을 설정한다.   
2-1. Board: nrf52840dk_nrf52840   
2-2. configuration: prj.conf   
2-3. DeviceTree Overlay: nrf52840_nrf52840.overlay   
2-4. Press Build Configuration   

## About
#### Developer
>하정원   
>&emsp;한동대학교 전산전자공학부 hajeongwon77@gmail.com   
>한나린   
>&emsp;한동대학교 전산전자공학부 lynnie21@handong.ac.kr   
#### Open Source Code
https://github.com/UmileVX/IoT-Development-with-Nordic-Zephyr
Sensor에 관한 대부분의 코드는 위 깃허브의 예제 코드를 참고했습니다.
#### Project Workspace
.   
├── CMakeLists.txt   
├── LICENSE   
├── nrf52840_nrf52840.overlay   
├── prj.conf   
├── sample.yaml   
└── src   
&emsp;&emsp;&emsp;├── batterydisplay.c   
&emsp;&emsp;&emsp;├── batterydisplay.h   
&emsp;&emsp;&emsp;├── button.c   
&emsp;&emsp;&emsp;├── button.h   
&emsp;&emsp;&emsp;├── co2.c   
&emsp;&emsp;&emsp;├── co2.h   
&emsp;&emsp;&emsp;├── led.c   
&emsp;&emsp;&emsp;├── led.h   
&emsp;&emsp;&emsp;├── main.c   
&emsp;&emsp;&emsp;├── pir.h   
&emsp;&emsp;&emsp;└── value.h   

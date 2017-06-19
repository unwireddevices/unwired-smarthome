Для сборки нужен arm-none-eabi-gcc(GNU ARM Embedded Toolchain), набор утилит srecord, и lua для некоторых вспомогательных действий.

##Быстрый старт
Быстрый старт(и проверка, все ли нормально) выглядит так:
```bash
git clone git@github.com:unwireddevices/unwired-smarthome.git
cd unwired-smarthome
git checkout develop
cd examples/unwired_smarthome
make all
```
Сборка должна завершиться без ошибок, а команда ls *.hex должна показать .hex-файлы:
```
button.hex
relay.hex
root.hex
```

button - это прошивка для выключателей, работающих от батарей

relay - прошивка для реле, управляющего светом

root - прошивка координатора сети, который подключается к роутеру и общается с ним по UART, используя [UDCP](2. Unwired Devices Coordinator Protocol).


##Параметры сборки
По умолчанию, прошивка собирается под контроллер СС2650 и платы от UnwiredDevices. Для изменения этого поведения служат переменные CPU и BOARD. 

CPU может принимать значения cc26xx или cc13xx. Для сборки прошивок под процессор СС1310, цель для make должна выглядеть следующим образом:

`make CPU=cc13xx all`

BOARD может принимать значения udboards(Unwired Devices Boards) или srf06(SmartRF06 Board). По умолчанию сборка происходит под udboards. Для сборки прошивок под плату SmartRF06 Board, цель для make должна выглядеть следующим образом:

`make BOARD=cc13xx all`

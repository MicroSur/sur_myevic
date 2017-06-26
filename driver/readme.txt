
usbser.sys is required for USB Virtual COM device.
It is included by windows operation system and could be found at \WINDOWS\system32\drivers
or \i386\driver.cab

To install the VCOM driver, user must find usbser.sys on their OS first and copy it to the 
same directory with Nuvoton CDC INF file. Finally install the driver manually when windows 
request to install the VCOM device driver.

************

Для вытаскивания LDRom (загрузчик) можно воспользоваться таким способом.
Скомпилировать исходный код специальной прошивки для этого, лежит тут.
https://gist.github.com/ReservedField/6125e9c7d2b9f92536ee
Готовая прошивка ldrom_dump.bin прилагается.
Залить эту прошивку в мод, в сейф-режиме.
На экране мода будет надпись: включите терминальную программу и нажмите Пуск.

При этом винда найдет устройство виртуального com-порта, потребуется драйвер.
Посмотреть сведения и запомнить VID и PID найденного устройства.
Проверить, что в файле NuvotonCDC.inf прописаны эти же VID и PID, если нет - изменить.
Найти в системе usbser.sys, скопировать его в папку к файлу NuvotonCDC.inf, 
указать вручную (выберу сам) на NuvotonCDC.inf файл при установке драйвера.
 
В системе появится новый com-порт, открыть терминальную программу (например putty),
настроить и подключить к этому номеру порта. Остальные настройки по умолчанию.

Нажать Пуск на моде - в окне терминальной программы появиться вытащенный ldrom, 
скопировать его с помощью хекс-редактора в бинарник.
Пример вытащенного из RX23 файла rx23_ldrom_data.bin прилагается.
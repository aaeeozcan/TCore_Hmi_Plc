# TCore_Hmi_Plc

STM32 tabanli HMI/PLC firmware projesi. Bu repo, Keil tabanli `BaseCodes` projesinin PlatformIO ortamina tasinmis halini icerir.

## Hedef Donanim

- MCU: `STM32F103RCT6`
- Framework: `STM32Cube`
- Build ortami: `PlatformIO`
- CubeMX proje dosyasi: `HMI_PLC.ioc`

## Proje Yapisi

- `src/`: uygulama kaynak dosyalari
- `include/`: baslik dosyalari
- `lib/`: harici veya yerel kutuphaneler icin ayrilan alan
- `test/`: test klasoru
- `docs/`: teknik notlar, I/O ozeti ve UART komut dokumani
- `HMI_PLC.ioc`: CubeMX konfigrasyonu
- `platformio.ini`: PlatformIO hedef ve derleme ayarlari

## Derleme

```powershell
pio run -e genericSTM32F103RC
```

## Yukleme

Programlayici bagliysa:

```powershell
pio run -t upload -e genericSTM32F103RC
```

## Donanim Ozeti

- 4 adet optokupler izoleli dijital giris
- 8 adet role cikisi
- 4 adet analog giris
- 2 adet PT100 sicaklik kanali (`MAX31865`)
- 1 adet `DS18B20` sicaklik sensori
- `SST25VF016B` SPI flash + FFS + YMODEM yukleme akisi
- `74HC595` ile surulen LED ve kontrol cikislari

## Aktif Cevre Birimleri

- `ADC1`: analog giris okumalari
- `CAN`: saha haberlesmesi
- `I2C1`: yardimci cihaz haberlesmesi
- `SPI1`: flash ve sicaklik sensor altyapisi
- `TIM3`: zamanlama isleri
- `USART1`: Modbus/test hatti
- `USART3`: debug/komut hatti
- `UART4`: ek seri haberlesme hatti

## Hizli Pin Ozeti

- `PA3-PA6`: dijital girisler
- `PB1-PB8`: relay cikislari
- `PB0`: durum LED'i
- `PC0-PC7`: relay durum LED'leri
- `PB14-PB15`: node ID switch girisleri
- `PA4`: `DS18B20`
- `PA5/PA6/PA7`: `SPI1` (`SCLK/MISO/MOSI`)

Detayli I/O ve komut dokumani:

- [I/O kilavuzu](d:/Projects/TCore/CelalBey/HMI_PLC/StmCode/STM32_HMI_PLC_PIO/docs/IO_KILAVUZU.md)
- [UART komutlari](d:/Projects/TCore/CelalBey/HMI_PLC/StmCode/STM32_HMI_PLC_PIO/docs/UART_KOMUTLARI.md)

## Notlar

- Keil'e ozel `Retarget.c` uygulanimi GCC/PlatformIO ile uyumlu hale getirildi.
- `main.c` icindeki `ffs` degiskeni, GCC standart kutuphanesi ile cakismayi onlemek icin `flash_fs` olarak duzenlendi.
- STM32Cube metadata dosyalari (`HMI_PLC.ioc`, `.mxproject`) repo icinde tutuluyor.

## Git

Ilk tasima commit'i:

- `0052bf2` `Initial PlatformIO migration from Keil base project`

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

## Notlar

- Keil'e ozel `Retarget.c` uygulanimi GCC/PlatformIO ile uyumlu hale getirildi.
- `main.c` icindeki `ffs` degiskeni, GCC standart kutuphanesi ile cakismayi onlemek icin `flash_fs` olarak duzenlendi.
- STM32Cube metadata dosyalari (`HMI_PLC.ioc`, `.mxproject`) repo icinde tutuluyor.

## Git

Ilk tasima commit'i:

- `0052bf2` `Initial PlatformIO migration from Keil base project`

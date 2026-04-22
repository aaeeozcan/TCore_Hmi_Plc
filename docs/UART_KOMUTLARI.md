# UART Komutlari

Bu komutlar `USART3` debug terminali uzerinden islenir. Komut parser'i `src/main.c` icinde yer alir.

## Temel Komutlar

| Komut | Aciklama |
|---|---|
| `S` | PLC'yi durdur |
| `R` | PLC'yi calistir |
| `D` | debug modunu ac/kapat |
| `T` | role testini baslat |
| `I` | dijital ve analog girisleri yazdir |
| `O` | role ve HC595 durumunu yazdir |
| `J` | HMI durum paketi gonder |
| `?` | yardim menusu |

## Sensor ve Donanim Komutlari

| Komut | Aciklama |
|---|---|
| `P` | PT100 sensorlerini oku |
| `W` | DS18B20 degerini oku |
| `U` | `USART1` test mesaji gonder |
| `F` | SPI flash test menusu |
| `Y` | flash yazma testini onayla |
| `Z` | FFS formatla |

## Dosya ve Flash Akisi

| Komut | Aciklama |
|---|---|
| `X` | YMODEM ile dosya yukleme akisini baslat |
| `G` | FFS icindeki dosyalari listele |
| `B` | `plc.bin` okuma/test akisini calistir |

## LED ve Gosterge Testleri

| Komut | Aciklama |
|---|---|
| `H` | 32 LED sirasiyla test |
| `L` | tum LED'leri toplu ac/kapat |

## Devre Disi Birakilan Komutlar

| Komut | Aciklama |
|---|---|
| `C` | HSC modulu kaldirildigi icin pasif |
| `1`, `2`, `0` | HSC ile ilgili eski akisin kalintilari |
| `M` | Modbus modulu kaldirildi bildirimi verir |

## Kullanima Not

- Terminal icin `115200 8N1` seri ayar kullaniliyorsa debug akislarinin okunmasi kolaylasir.
- `X` komutundan sonra terminal yaziliminin YMODEM gonderim ekranindan dosya secilmelidir.
- `Z` tum flash dosyalarini siler; geri alinmaz.

## Kod Referansi

- Komut parser'i: [src/main.c](d:/Projects/TCore/CelalBey/HMI_PLC/StmCode/STM32_HMI_PLC_PIO/src/main.c)

# I/O Kilavuzu

Bu dokuman, `src/main.c` ve eski `BaseCodes/DOC` notlarina gore projenin temel I/O dagilimini ozetler.

## Dijital Girisler

| Sinyal | Port/Pin | Not |
|---|---|---|
| `DIN_1` | `PA3` | Optokupler izoleli giris |
| `DIN_2` | `PA4` | Optokupler izoleli giris |
| `DIN_3` | `PA5` | Optokupler izoleli giris |
| `DIN_4` | `PA6` | Optokupler izoleli giris |

Not: Kodda dijital girisler aktif-low mantikla okunuyor.

## Relay Cikislari

| Cikis | Port/Pin |
|---|---|
| `RELAY_1` | `PB1` |
| `RELAY_2` | `PB2` |
| `RELAY_3` | `PB3` |
| `RELAY_4` | `PB4` |
| `RELAY_5` | `PB5` |
| `RELAY_6` | `PB6` |
| `RELAY_7` | `PB7` |
| `RELAY_8` | `PB8` |

## Durum LED'leri

| LED | Port/Pin |
|---|---|
| `STATUS_LED` | `PB0` |
| `RELAY_LED1` | `PC0` |
| `RELAY_LED2` | `PC1` |
| `RELAY_LED3` | `PC2` |
| `RELAY_LED4` | `PC3` |
| `RELAY_LED5` | `PC4` |
| `RELAY_LED6` | `PC5` |
| `RELAY_LED7` | `PC6` |
| `RELAY_LED8` | `PC7` |

## Node ID Switch

| Sinyal | Port/Pin |
|---|---|
| `ID_SW_1` | `PB14` |
| `ID_SW_2` | `PB15` |

## Sicaklik ve SPI

| Fonksiyon | Port/Pin | Not |
|---|---|---|
| `DS18B20` | `PA4` | 1-wire sicaklik sensori |
| `SPI1_SCLK` | `PA5` | Flash/PT100 altyapisi |
| `SPI1_MISO` | `PA6` | Flash/PT100 altyapisi |
| `SPI1_MOSI` | `PA7` | Flash/PT100 altyapisi |

## Cevre Birimleri

| Peripheral | Kullanim |
|---|---|
| `ADC1` | 4 analog giris |
| `CAN` | haberlesme |
| `I2C1` | yardimci cihaz/EEPROM benzeri akislar |
| `IWDG` | watchdog |
| `SPI1` | SPI flash ve sensor arayuzu |
| `TIM3` | zamanlama |
| `USART1` | Modbus/test |
| `USART3` | debug terminal / komut hatti |
| `UART4` | ek seri kanal |

## Kod Referansi

- Ana pin tanimlari: [src/main.c](d:/Projects/TCore/CelalBey/HMI_PLC/StmCode/STM32_HMI_PLC_PIO/src/main.c)
- HAL konfigrasyonu: [include/stm32f1xx_hal_conf.h](d:/Projects/TCore/CelalBey/HMI_PLC/StmCode/STM32_HMI_PLC_PIO/include/stm32f1xx_hal_conf.h)

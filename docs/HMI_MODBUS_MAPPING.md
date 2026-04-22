# HMI Modbus Mapping Draft

## Durum Ozeti

Bu dokuman HMI ile PLC arasindaki ilk Modbus veri modelini tanimlar. Tavsiye edilen mimari:

- HMI = `Modbus Master`
- PLC = `Modbus Slave`

HMI sadece komut istegi yazar; PLC karar verir ve durum/feedback uretir.

## Adresleme Kurali

- `Coils (0xxxx)` = yazilabilir komut istekleri
- `Discrete Inputs (1xxxx)` = yalniz okunur durum bitleri
- `Input Registers (3xxxx)` = analog/deger/olcumler
- `Holding Registers (4xxxx)` = mod, set, sayac, alarm sayisi, parametre

## Coil Haritasi

### Global

| Adres | Variable | Aciklama |
|---|---|---|
| `00001` | `PV_CMD_GLOBAL_ALARM_ACK_REQ` | tum aktif alarm ack istegi |
| `00002` | `PV_CMD_GLOBAL_ALARM_RESET_REQ` | resetlenebilir alarm reset istegi |
| `00003` | `PV_CMD_GLOBAL_HORN_SILENCE_REQ` | horn/sound silence |

### Ventilation

| Adres | Variable |
|---|---|
| `00011` | `PV_CMD_JF_01_START_REQ` |
| `00012` | `PV_CMD_JF_01_STOP_REQ` |
| `00013` | `PV_CMD_JF_01_RESET_REQ` |
| `00014` | `PV_CMD_JF_02_START_REQ` |
| `00015` | `PV_CMD_JF_02_STOP_REQ` |
| `00016` | `PV_CMD_JF_02_RESET_REQ` |

### Lighting

| Adres | Variable |
|---|---|
| `00031` | `PV_CMD_LGT_01_ON_REQ` |
| `00032` | `PV_CMD_LGT_01_OFF_REQ` |
| `00033` | `PV_CMD_LGT_02_ON_REQ` |
| `00034` | `PV_CMD_LGT_02_OFF_REQ` |
| `00035` | `PV_CMD_LGT_03_ON_REQ` |
| `00036` | `PV_CMD_LGT_03_OFF_REQ` |
| `00037` | `PV_CMD_LGT_04_ON_REQ` |
| `00038` | `PV_CMD_LGT_04_OFF_REQ` |

### Energy / Auxiliary

| Adres | Variable |
|---|---|
| `00051` | `PV_CMD_PMP_01_START_REQ` |
| `00052` | `PV_CMD_PMP_01_STOP_REQ` |
| `00053` | `PV_CMD_PMP_01_RESET_REQ` |
| `00054` | `PV_CMD_BAR_01_OPEN_REQ` |
| `00055` | `PV_CMD_BAR_01_CLOSE_REQ` |
| `00056` | `PV_CMD_BAR_02_OPEN_REQ` |
| `00057` | `PV_CMD_BAR_02_CLOSE_REQ` |
| `00058` | `PV_CMD_VMS_01_ENABLE_REQ` |
| `00059` | `PV_CMD_LCS_01_ENABLE_REQ` |

## Discrete Input Haritasi

### Sistem

| Adres | Variable |
|---|---|
| `10001` | `PV_SYS_PLC_COMM_OK` |
| `10002` | `PV_SYS_MODE_AUTO` |
| `10003` | `PV_SYS_MODE_MANUAL` |
| `10004` | `PV_SYS_FIRE_ACTIVE` |
| `10005` | `PV_SYS_ESTOP_ACTIVE` |
| `10006` | `PV_ALM_SUM_CRITICAL` |
| `10007` | `PV_ALM_SUM_MAJOR` |
| `10008` | `PV_ALM_SUM_MINOR` |

### JF-01

| Adres | Variable |
|---|---|
| `10101` | `PV_STS_JF_01_RUN` |
| `10102` | `PV_STS_JF_01_FAULT` |
| `10103` | `PV_STS_JF_01_LOCAL` |
| `10104` | `PV_STS_JF_01_AUTO` |
| `10105` | `PV_STS_JF_01_READY` |
| `10106` | `PV_STS_JF_01_REMOTE` |

### JF-02

| Adres | Variable |
|---|---|
| `10111` | `PV_STS_JF_02_RUN` |
| `10112` | `PV_STS_JF_02_FAULT` |
| `10113` | `PV_STS_JF_02_LOCAL` |
| `10114` | `PV_STS_JF_02_AUTO` |
| `10115` | `PV_STS_JF_02_READY` |
| `10116` | `PV_STS_JF_02_REMOTE` |

### Lighting

| Adres | Variable |
|---|---|
| `10201` | `PV_STS_LGT_01_FB_ON` |
| `10202` | `PV_STS_LGT_01_FAULT` |
| `10203` | `PV_STS_LGT_02_FB_ON` |
| `10204` | `PV_STS_LGT_02_FAULT` |
| `10205` | `PV_STS_LGT_03_FB_ON` |
| `10206` | `PV_STS_LGT_03_FAULT` |
| `10207` | `PV_STS_LGT_04_FB_ON` |
| `10208` | `PV_STS_LGT_04_FAULT` |

### Energy / Auxiliary

| Adres | Variable |
|---|---|
| `10301` | `PV_STS_PWR_MAIN_AVAILABLE` |
| `10302` | `PV_STS_PWR_UPS_AVAILABLE` |
| `10303` | `PV_STS_PWR_GEN_AVAILABLE` |
| `10304` | `PV_STS_PWR_MCC_AVAILABLE` |
| `10305` | `PV_STS_PMP_01_RUN` |
| `10306` | `PV_STS_PMP_01_FAULT` |
| `10307` | `PV_STS_VMS_01_RUN` |
| `10308` | `PV_STS_LCS_01_RUN` |
| `10309` | `PV_STS_BAR_01_FB_OPEN` |
| `10310` | `PV_STS_BAR_01_FB_CLOSED` |
| `10311` | `PV_STS_BAR_02_FB_OPEN` |
| `10312` | `PV_STS_BAR_02_FB_CLOSED` |

## Holding Register Haritasi

### Mod / Set

| Adres | Variable | Aciklama |
|---|---|---|
| `40001` | `PV_SET_VENT_GLOBAL_MODE` | 0=off,1=manual,2=auto |
| `40002` | `PV_SET_LIGHTING_GLOBAL_MODE` | 0=off,1=manual,2=auto |
| `40003` | `PV_SET_JF_01_MODE` | 0=off,1=manual,2=auto |
| `40004` | `PV_SET_JF_02_MODE` | 0=off,1=manual,2=auto |
| `40005` | `PV_SET_PMP_01_MODE` | 0=off,1=manual,2=auto |

### Sayaçlar

| Adres | Variable |
|---|---|
| `40101` | `PV_STS_JF_01_RUNTIME_H` |
| `40102` | `PV_STS_JF_01_STARTS_CNT` |
| `40103` | `PV_STS_JF_02_RUNTIME_H` |
| `40104` | `PV_STS_JF_02_STARTS_CNT` |
| `40105` | `PV_STS_PMP_01_RUNTIME_H` |
| `40106` | `PV_STS_PMP_01_STARTS_CNT` |

### Alarm Ozetleri

| Adres | Variable |
|---|---|
| `40201` | `PV_ALM_ACTIVE_CNT` |
| `40202` | `PV_ALM_UNACK_CNT` |
| `40203` | `PV_ALM_LAST_CODE` |
| `40204` | `PV_ALM_LAST_SEV` |

### Alarm Bitmap

| Adres | Variable | Kapsam |
|---|---|---|
| `40221` | `PV_ALM_BITMAP_W1` | alarm 1-16 |
| `40222` | `PV_ALM_BITMAP_W2` | alarm 17-32 |
| `40223` | `PV_ALM_BITMAP_W3` | alarm 33-48 |
| `40224` | `PV_ALM_BITMAP_W4` | alarm 49-64 |

## Input Register Haritasi

| Adres | Variable | Birim |
|---|---|---|
| `30001` | `PV_STS_TUNNEL_TEMP_C` | 0.1 C |
| `30002` | `PV_STS_TUNNEL_HUMIDITY_PCT` | 0.1 % |
| `30003` | `PV_STS_TUNNEL_CO_PPM` | ppm |
| `30004` | `PV_STS_TUNNEL_VISIBILITY_M` | metre |
| `30005` | `PV_STS_PWR_MAIN_VOLT` | V |
| `30006` | `PV_STS_UPS_LOAD_PCT` | % |

## Alarm Code Standardi

Alarm kodlari 4 haneli grup yapisinda onerilir:

- `1xxx` = ventilation
- `2xxx` = lighting
- `3xxx` = energy
- `4xxx` = pump/drain
- `5xxx` = fire & safety
- `6xxx` = comm/system

Ornek:

- `1001` = `JF_01_FAIL_TO_START`
- `1002` = `JF_01_TRIP`
- `5001` = `FIRE_Z1_ACTIVE`
- `5002` = `FIRE_Z2_ACTIVE`
- `5003` = `ESTOP_ACTIVE`
- `6001` = `PLC_COMM_LOSS`

## Riskler / Acik Noktalar

- Gercek PLC memory map ile cakisabilir, bu taslak hizalanmalidir
- 32-bit sayaç alanlari icin register ciftleme ihtiyaci dogabilir
- Runtime birimleri HMI editorunun destekledigi formatla uyumlandirilmalidir

## Sonraki Adim

Bu mapping, alarm/cause-effect ve ekran object listeleriyle baglanacak.

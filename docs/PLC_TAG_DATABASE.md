# PLC Tag Database v1

## Durum Ozeti

Bu dokuman, base tunnel sistemin PLC-HMI ortak veri sozlugudur. Amaç, fiziksel I/O'dan bagimsiz, ekipman-tabanli ve urunlesebilir bir tag katalogu olusturmaktir.

## Tag Aileleri

| Aile | Prefix | Amac |
|---|---|---|
| System status | `PV_SYS_` | sistem genel durumlari |
| Commands | `PV_CMD_` | HMI'den PLC'ye komut istekleri |
| Status | `PV_STS_` | PLC'den HMI'ya ekipman durumlari |
| Settings | `PV_SET_` | modlar ve set degerleri |
| Alarms | `PV_ALM_` | alarm, ozet ve alarm meta bilgileri |
| Navigation/UI | `PV_NAV_`, `IV_UI_` | HMI ekran mantigi |

## System Tags

| Tag | Tip | Aciklama |
|---|---|---|
| `PV_SYS_PLC_COMM_OK` | bool | PLC genel comm healthy |
| `PV_SYS_ETH_COMM_OK` | bool | ethernet modbus healthy |
| `PV_SYS_RS485_COMM_OK` | bool | rs485 field comm healthy |
| `PV_SYS_CAN_COMM_OK` | bool | can aux comm healthy |
| `PV_SYS_RTC_SYNC_OK` | bool | RTC senkron durumu |
| `PV_SYS_MODE_AUTO` | bool | sistem auto mode |
| `PV_SYS_MODE_MANUAL` | bool | sistem manual mode |
| `PV_SYS_FIRE_ACTIVE` | bool | herhangi bir fire zone aktif |
| `PV_SYS_ESTOP_ACTIVE` | bool | e-stop aktif |
| `PV_SYS_REMOTE_MODE` | bool | sistem remote command kabul ediyor |
| `PV_SYS_MOBILE_AQS_ONLINE` | bool | mobil hava kalite istasyonu online |
| `PV_SYS_MOBILE_AQS_NEW_DATA` | bool | yeni paket geldi |

## Global Command Tags

| Tag | Tip | Aciklama |
|---|---|---|
| `PV_CMD_GLOBAL_ALARM_ACK_REQ` | bool | alarm ack |
| `PV_CMD_GLOBAL_ALARM_RESET_REQ` | bool | alarm reset |
| `PV_CMD_GLOBAL_HORN_SILENCE_REQ` | bool | horn mute |
| `PV_CMD_GLOBAL_LAMP_TEST_REQ` | bool | lamba test |
| `PV_CMD_GLOBAL_MODE_AUTO_REQ` | bool | auto mode istegi |
| `PV_CMD_GLOBAL_MODE_MAN_REQ` | bool | manual mode istegi |

## Jet Fan Tags

### JF_01

| Tag | Tip |
|---|---|
| `PV_CMD_JF_01_START_REQ` | bool |
| `PV_CMD_JF_01_STOP_REQ` | bool |
| `PV_CMD_JF_01_RESET_REQ` | bool |
| `PV_STS_JF_01_RUN` | bool |
| `PV_STS_JF_01_FAULT` | bool |
| `PV_STS_JF_01_LOCAL` | bool |
| `PV_STS_JF_01_AUTO` | bool |
| `PV_STS_JF_01_READY` | bool |
| `PV_STS_JF_01_REMOTE` | bool |
| `PV_STS_JF_01_RUNTIME_H` | uint32 |
| `PV_STS_JF_01_STARTS_CNT` | uint32 |
| `PV_SET_JF_01_MODE` | uint16 |
| `PV_SET_JF_01_SPEED_REF` | uint16 |

### JF_02

| Tag | Tip |
|---|---|
| `PV_CMD_JF_02_START_REQ` | bool |
| `PV_CMD_JF_02_STOP_REQ` | bool |
| `PV_CMD_JF_02_RESET_REQ` | bool |
| `PV_STS_JF_02_RUN` | bool |
| `PV_STS_JF_02_FAULT` | bool |
| `PV_STS_JF_02_LOCAL` | bool |
| `PV_STS_JF_02_AUTO` | bool |
| `PV_STS_JF_02_READY` | bool |
| `PV_STS_JF_02_REMOTE` | bool |
| `PV_STS_JF_02_RUNTIME_H` | uint32 |
| `PV_STS_JF_02_STARTS_CNT` | uint32 |
| `PV_SET_JF_02_MODE` | uint16 |
| `PV_SET_JF_02_SPEED_REF` | uint16 |

## Lighting Tags

Her grup icin ayni model uygulanir:

- `PV_CMD_LGT_0X_ON_REQ`
- `PV_CMD_LGT_0X_OFF_REQ`
- `PV_STS_LGT_0X_FB_ON`
- `PV_STS_LGT_0X_FAULT`

Global:

- `PV_SET_LIGHTING_GLOBAL_MODE`
- `PV_SET_LIGHT_DIM_REF`
- `PV_SET_AO_02_RAW`
- `PV_STS_AO_02_FB_RAW`

## Pump Tags

| Tag | Tip |
|---|---|
| `PV_CMD_PMP_01_START_REQ` | bool |
| `PV_CMD_PMP_01_STOP_REQ` | bool |
| `PV_CMD_PMP_01_RESET_REQ` | bool |
| `PV_STS_PMP_01_RUN` | bool |
| `PV_STS_PMP_01_FAULT` | bool |
| `PV_STS_PMP_01_RUNTIME_H` | uint32 |
| `PV_STS_PMP_01_STARTS_CNT` | uint32 |
| `PV_SET_PMP_01_MODE` | uint16 |

## Energy Tags

| Tag | Tip |
|---|---|
| `PV_STS_PWR_MAIN_AVAILABLE` | bool |
| `PV_STS_PWR_UPS_AVAILABLE` | bool |
| `PV_STS_PWR_GEN_AVAILABLE` | bool |
| `PV_STS_PWR_MCC_AVAILABLE` | bool |
| `PV_STS_PWR_MAIN_VOLT` | uint16 |
| `PV_STS_PWR_MAIN_CURR` | uint16 |
| `PV_STS_PWR_MAIN_PWR_KW` | uint16 |
| `PV_STS_UPS_LOAD_PCT` | uint16 |
| `PV_STS_UPS_BATT_PCT` | uint16 |
| `PV_STS_GEN_FUEL_PCT` | uint16 |

## Process / Sensor Tags

| Tag | Tip |
|---|---|
| `PV_STS_TUNNEL_TEMP_C` | int16 |
| `PV_STS_MCC_ROOM_TEMP_C` | int16 |
| `PV_STS_CABINET_TEMP_C` | int16 |
| `PV_STS_TUNNEL_CO_PPM` | uint16 |
| `PV_STS_TUNNEL_NOX_PPM` | uint16 |
| `PV_STS_SUMP_LEVEL_PCT` | uint16 |
| `PV_STS_POWER_DEMAND_PCT` | uint16 |
| `PV_STS_AO_01_FB_RAW` | uint16 |
| `PV_STS_AO_02_FB_RAW` | uint16 |
| `PV_STS_CO_SENSOR_OK` | bool |
| `PV_STS_VIS_SENSOR_OK` | bool |
| `PV_STS_SUMP_LEVEL_SENSOR_OK` | bool |
| `PV_STS_PT100_01_OK` | bool |
| `PV_STS_PT100_02_OK` | bool |
| `PV_STS_DS18B20_01_OK` | bool |

## Mobile Air Quality Station Tags

| Tag | Tip |
|---|---|
| `PV_STS_MAQS_ONLINE` | bool |
| `PV_STS_MAQS_COMM_OK` | bool |
| `PV_STS_MAQS_PACKET_AGE_S` | uint16 |
| `PV_STS_MAQS_LAT_HI` | uint16 |
| `PV_STS_MAQS_LAT_LO` | uint16 |
| `PV_STS_MAQS_LON_HI` | uint16 |
| `PV_STS_MAQS_LON_LO` | uint16 |
| `PV_STS_MAQS_CO_PPM` | uint16 |
| `PV_STS_MAQS_CO2_PPM` | uint16 |
| `PV_STS_MAQS_NO2_PPM` | uint16 |
| `PV_STS_MAQS_SO2_PPM` | uint16 |
| `PV_STS_MAQS_H2S_PPM` | uint16 |
| `PV_STS_MAQS_CH4_PCTLEL` | uint16 |
| `PV_STS_MAQS_O2_PCT` | uint16 |
| `PV_STS_MAQS_PM1_UGM3` | uint16 |
| `PV_STS_MAQS_PM25_UGM3` | uint16 |
| `PV_STS_MAQS_PM10_UGM3` | uint16 |
| `PV_STS_MAQS_TEMP_C` | int16 |
| `PV_STS_MAQS_HUM_PCT` | uint16 |
| `PV_STS_MAQS_BATT_PCT` | uint16 |
| `PV_STS_MAQS_SIGNAL_RSSI` | int16 |
| `PV_STS_MAQS_QUALITY_OK` | bool |
| `PV_STS_MAQS_GAS_ALARM` | bool |
| `PV_STS_MAQS_PM_ALARM` | bool |

## Mobile Station Derived Tags

| Tag | Tip | Aciklama |
|---|---|---|
| `PV_SYS_AIR_QUALITY_DEGRADED` | bool | genel hava kalitesi kotu |
| `PV_SYS_VENT_STRATEGY_FROM_MAQS` | bool | ventilation stratejisi mobil istasyon verisinden etkileniyor |
| `PV_STS_AIR_QUALITY_INDEX` | uint16 | hesaplanan ozet indeks |
| `PV_STS_AIR_QUALITY_SOURCE_SEL` | uint16 | 0=fixed 1=mobile 2=fused |

## Auxiliary Tags

| Tag | Tip |
|---|---|
| `PV_CMD_VMS_01_ENABLE_REQ` | bool |
| `PV_CMD_LCS_01_ENABLE_REQ` | bool |
| `PV_CMD_BAR_01_OPEN_REQ` | bool |
| `PV_CMD_BAR_01_CLOSE_REQ` | bool |
| `PV_CMD_BAR_02_OPEN_REQ` | bool |
| `PV_CMD_BAR_02_CLOSE_REQ` | bool |
| `PV_STS_VMS_01_RUN` | bool |
| `PV_STS_LCS_01_RUN` | bool |
| `PV_STS_BAR_01_FB_OPEN` | bool |
| `PV_STS_BAR_01_FB_CLOSED` | bool |
| `PV_STS_BAR_02_FB_OPEN` | bool |
| `PV_STS_BAR_02_FB_CLOSED` | bool |

## Alarm Tags

| Tag | Tip |
|---|---|
| `PV_ALM_SUM_CRITICAL` | bool |
| `PV_ALM_SUM_MAJOR` | bool |
| `PV_ALM_SUM_MINOR` | bool |
| `PV_ALM_ACTIVE_CNT` | uint16 |
| `PV_ALM_UNACK_CNT` | uint16 |
| `PV_ALM_LAST_CODE` | uint16 |
| `PV_ALM_LAST_SEV` | uint16 |
| `PV_ALM_LAST_TS_HI` | uint16 |
| `PV_ALM_LAST_TS_LO` | uint16 |
| `PV_ALM_BITMAP_W1` | uint16 |
| `PV_ALM_BITMAP_W2` | uint16 |
| `PV_ALM_BITMAP_W3` | uint16 |
| `PV_ALM_BITMAP_W4` | uint16 |

## Demo'yu Guclendirecek Ek Tagler

Bu alanlar "SCADA hissi" verir:

| Tag | Tip | Amac |
|---|---|---|
| `PV_SYS_HEARTBEAT` | uint16 | canli sistem hissi |
| `PV_SYS_EVENT_CNT` | uint32 | event log ozeti |
| `PV_SYS_LAST_EVENT_CODE` | uint16 | son olay |
| `PV_SYS_OPERATOR_LEVEL` | uint16 | role-based UI |
| `PV_SYS_DEMO_MODE` | bool | fuar/demo davranisi |
| `PV_SYS_FORCE_MODE_ACTIVE` | bool | commissioning view |
| `PV_SYS_LAST_MAQS_EVENT_CODE` | uint16 | son mobil istasyon olayi |

## Sonraki Adim

Bu tag database'den sonra:

1. ekran bazli variable matrix
2. popup/dialog command modeli
3. event timeline yapisi

uretilmelidir.

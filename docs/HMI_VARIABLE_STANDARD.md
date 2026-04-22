# HMI Variable Naming Standard

## Durum Ozeti

Bu dokuman, HMI runtime ve PLC arasindaki veri kontratini isimlendirme seviyesinde standartlastirir. Amac, ekipmanlar arasinda tekrar kullanilabilir, ekranlar arasi tutarli ve Modbus mapping'e dogrudan baglanabilir bir isim yapisi kurmaktir.

## Genel Kurallar

- Tum isimler ASCII ve buyuk harf ile yazilir
- Kelime ayirici olarak `_` kullanilir
- Ekipman etiketi daima en basa gelir
- HMI tarafinda iki ana seviye kullanilir:
  - `PV_` = public variable
  - `IV_` = internal variable
- PLC'ye yazilan komut niteligindeki alanlar `CMD_` ile baslar
- PLC'den okunan durum alanlari `STS_` ile baslar

## Ekipman Etiket Standarti

| Ekipman | Tag |
|---|---|
| Jet Fan 1 | `JF_01` |
| Jet Fan 2 | `JF_02` |
| Lighting Group 1 | `LGT_01` |
| Lighting Group 2 | `LGT_02` |
| Lighting Group 3 | `LGT_03` |
| Lighting Group 4 | `LGT_04` |
| Drain Pump | `PMP_01` |
| Main Energy | `PWR_MAIN` |
| UPS | `PWR_UPS` |
| Generator | `PWR_GEN` |
| MCC | `PWR_MCC` |
| Fire Zone 1 | `FIRE_Z1` |
| Fire Zone 2 | `FIRE_Z2` |
| E-Stop | `ESTOP_01` |
| VMS | `VMS_01` |
| LCS | `LCS_01` |
| Barrier 1 | `BAR_01` |
| Barrier 2 | `BAR_02` |

## Public Variable Prefixleri

| Prefix | Anlam |
|---|---|
| `PV_STS_` | PLC'den gelen durum bilgisi |
| `PV_CMD_` | HMI'nin PLC'ye yazdigi komut istegi |
| `PV_SET_` | operatorden gelen ayar/set degeri |
| `PV_ALM_` | alarm ile ilgili public alan |
| `PV_SYS_` | sistem seviyesi alanlar |
| `PV_NAV_` | sayfa/navigasyon alanlari |

## Internal Variable Prefixleri

| Prefix | Anlam |
|---|---|
| `IV_UI_` | lokal UI durumu |
| `IV_POP_` | popup ve dialog kontrolu |
| `IV_FIL_` | filtreleme/siralama |
| `IV_TMP_` | gecici hesaplanan deger |
| `IV_SEL_` | secim state'i |

## Komut Isim Standarti

Komutlar `PV_CMD_<TAG>_<ACTION>` biciminde olmalidir.

Ornekler:

- `PV_CMD_JF_01_START_REQ`
- `PV_CMD_JF_01_STOP_REQ`
- `PV_CMD_JF_01_RESET_REQ`
- `PV_CMD_LGT_01_ON_REQ`
- `PV_CMD_LGT_01_OFF_REQ`
- `PV_CMD_PMP_01_START_REQ`
- `PV_CMD_BAR_01_OPEN_REQ`
- `PV_CMD_BAR_01_CLOSE_REQ`
- `PV_CMD_GLOBAL_ALARM_ACK_REQ`

Not:

- HMI bu alanlari yazar
- PLC komut gecerliligini kendi interlock mantigina gore degerlendirir

## Durum Isim Standarti

Durumlar `PV_STS_<TAG>_<STATE>` biciminde olmalidir.

Standart state alanlari:

- `RUN`
- `FAULT`
- `LOCAL`
- `AUTO`
- `READY`
- `REMOTE`
- `AVAILABLE`
- `FB_ON`
- `FB_OFF`

Ornekler:

- `PV_STS_JF_01_RUN`
- `PV_STS_JF_01_FAULT`
- `PV_STS_JF_01_LOCAL`
- `PV_STS_JF_01_AUTO`
- `PV_STS_JF_01_READY`
- `PV_STS_PMP_01_RUN`
- `PV_STS_LGT_03_FB_ON`

## Register Tabanli Analog / Sayac Alani

Analog ve sayaç alanlari `PV_STS_<TAG>_<VALUE>` ya da `PV_SET_<TAG>_<VALUE>` formunda tutulur.

Ornekler:

- `PV_STS_JF_01_RUNTIME_H`
- `PV_STS_JF_01_STARTS_CNT`
- `PV_STS_PMP_01_RUNTIME_H`
- `PV_STS_TUNNEL_TEMP_C`
- `PV_STS_TUNNEL_CO_PPM`
- `PV_SET_LGT_GLOBAL_MODE`
- `PV_SET_VENT_GLOBAL_MODE`

## Sistem Seviyesi Alanlar

- `PV_SYS_PLC_COMM_OK`
- `PV_SYS_PLC_HEARTBEAT`
- `PV_SYS_MODE_AUTO`
- `PV_SYS_MODE_MANUAL`
- `PV_SYS_ALARM_ACTIVE_CNT`
- `PV_SYS_ALARM_UNACK_CNT`
- `PV_SYS_FIRE_ACTIVE`
- `PV_SYS_ESTOP_ACTIVE`

## Alarm Alanlari

- `PV_ALM_SUM_CRITICAL`
- `PV_ALM_SUM_MAJOR`
- `PV_ALM_SUM_MINOR`
- `PV_ALM_ACTIVE_CNT`
- `PV_ALM_UNACK_CNT`
- `PV_ALM_LAST_CODE`
- `PV_ALM_LAST_SEV`

Ekipman alarm durumu:

- `PV_ALM_JF_01_ACTIVE`
- `PV_ALM_JF_01_UNACK`
- `PV_ALM_PMP_01_ACTIVE`

## Sayfa Bazli Internal Variable Ornekleri

- `IV_UI_ACTIVE_PAGE`
- `IV_UI_PREV_PAGE`
- `IV_UI_POPUP_VISIBLE`
- `IV_UI_SELECTED_EQUIP`
- `IV_UI_ALARM_FILTER_SEV`
- `IV_UI_ALARM_FILTER_ACK`

## Riskler / Acik Noktalar

- Gercek HMI editorunde variable name uzunluk limiti olabilir
- Runtime import/export formatina gore prefix kismi kisaltilabilir
- Son master/slave ve register grouping kararina gore isim-set uyarlamasi gerekebilir

## Sonraki Adim

Bu isim standardi bir sonraki dokuman olan Modbus mapping ile birebir baglanacak.

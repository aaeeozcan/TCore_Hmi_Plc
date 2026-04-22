# HMI Screen Object Draft

## Durum Ozeti

Bu dokuman ilk surum ekranlar icin object list, yerlesim mantigi, variable binding ve navigasyon iliskilerini tanimlar. Koordinatlar 1024x600 layout'a gore taslak olarak verilmis olup HMI editorunde son hizalama gerekecektir.

## Ortak Layout

### Sabit Alanlar

| Alan | x | y | w | h |
|---|---:|---:|---:|---:|
| `TOP_BAR` | 0 | 0 | 1024 | 60 |
| `WORK_AREA` | 0 | 60 | 1024 | 480 |
| `BOTTOM_MENU` | 0 | 540 | 1024 | 60 |

### Ust Bar Standart Objeler

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `LBL_PAGE_TITLE` | Label | 24 | 14 | 280 | 32 | internal |
| `SHP_COMM_STATUS` | Shape | 760 | 15 | 24 | 24 | `PV_SYS_PLC_COMM_OK` |
| `LBL_ALM_ACTIVE_CNT` | Label | 800 | 10 | 90 | 18 | `PV_ALM_ACTIVE_CNT` |
| `LBL_ALM_UNACK_CNT` | Label | 800 | 30 | 90 | 18 | `PV_ALM_UNACK_CNT` |
| `SHP_FIRE_SUMMARY` | Shape | 905 | 15 | 36 | 24 | `PV_SYS_FIRE_ACTIVE` |
| `SHP_ESTOP_SUMMARY` | Shape | 952 | 15 | 36 | 24 | `PV_SYS_ESTOP_ACTIVE` |

### Alt Menu Standart Objeler

| Object | Type | x | y | w | h | Target |
|---|---|---:|---:|---:|---:|---|
| `BTN_NAV_DASH` | Button | 10 | 548 | 120 | 42 | Dashboard |
| `BTN_NAV_MIMIC` | Button | 140 | 548 | 120 | 42 | Mimic |
| `BTN_NAV_VENT` | Button | 270 | 548 | 120 | 42 | Ventilation |
| `BTN_NAV_LIGHT` | Button | 400 | 548 | 120 | 42 | Lighting |
| `BTN_NAV_ENERGY` | Button | 530 | 548 | 120 | 42 | Energy |
| `BTN_NAV_ALARM` | Button | 660 | 548 | 120 | 42 | Alarm |
| `BTN_NAV_AUX` | Button | 790 | 548 | 110 | 42 | Auxiliary |
| `BTN_NAV_FIRE` | Button | 910 | 548 | 104 | 42 | Fire & Safety |

## Dashboard

### Object List

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `GRP_DASH_MIMIC_OVERVIEW` | Shape | 16 | 76 | 470 | 330 | none |
| `GRP_DASH_ENERGY` | Shape | 504 | 76 | 504 | 116 | none |
| `GRP_DASH_VENT_SUM` | Shape | 504 | 206 | 244 | 122 | none |
| `GRP_DASH_PUMP_SUM` | Shape | 764 | 206 | 244 | 122 | none |
| `GRP_DASH_SUMMARY` | Shape | 16 | 420 | 992 | 104 | none |
| `LBL_JF_01_RUN` | Label | 522 | 226 | 90 | 20 | `PV_STS_JF_01_RUN` |
| `LBL_JF_02_RUN` | Label | 522 | 252 | 90 | 20 | `PV_STS_JF_02_RUN` |
| `LBL_PMP_01_RUN` | Label | 782 | 226 | 90 | 20 | `PV_STS_PMP_01_RUN` |
| `LBL_MAIN_EN` | Label | 522 | 96 | 100 | 20 | `PV_STS_PWR_MAIN_AVAILABLE` |
| `LBL_UPS` | Label | 642 | 96 | 100 | 20 | `PV_STS_PWR_UPS_AVAILABLE` |
| `LBL_GEN` | Label | 762 | 96 | 100 | 20 | `PV_STS_PWR_GEN_AVAILABLE` |
| `LBL_MCC` | Label | 882 | 96 | 100 | 20 | `PV_STS_PWR_MCC_AVAILABLE` |
| `LBL_SUM_COMM` | Label | 32 | 454 | 120 | 22 | `PV_SYS_PLC_COMM_OK` |
| `LBL_SUM_FIRE` | Label | 180 | 454 | 120 | 22 | `PV_SYS_FIRE_ACTIVE` |
| `LBL_SUM_ESTOP` | Label | 328 | 454 | 120 | 22 | `PV_SYS_ESTOP_ACTIVE` |
| `LBL_SUM_MODE` | Label | 476 | 454 | 120 | 22 | `PV_SYS_MODE_AUTO` |
| `LBL_SUM_ALARM` | Label | 624 | 454 | 140 | 22 | `PV_ALM_ACTIVE_CNT` |

### Durum Davranis Kurali

- enerji kartlari `available` durumuna gore yesil/gri/kirmizi olur
- `PV_SYS_FIRE_ACTIVE` aktifse summary ve mimic overview blink eder
- `PV_SYS_PLC_COMM_OK=0` ise sag kartlar communication degraded olur

## Mimic

### Object List

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `SHP_TUNNEL_BODY` | Shape | 84 | 154 | 700 | 170 | none |
| `IMG_PORTAL_LEFT` | Image | 34 | 154 | 60 | 170 | none |
| `IMG_PORTAL_RIGHT` | Image | 784 | 154 | 60 | 170 | none |
| `SHP_FIRE_Z1` | TransShape | 210 | 124 | 180 | 230 | `PV_SYS_FIRE_ACTIVE` / `FIRE_Z1` |
| `SHP_FIRE_Z2` | TransShape | 470 | 124 | 180 | 230 | `PV_SYS_FIRE_ACTIVE` / `FIRE_Z2` |
| `IMG_JF_01` | Image | 270 | 182 | 80 | 80 | `PV_STS_JF_01_RUN` |
| `IMG_JF_02` | Image | 530 | 182 | 80 | 80 | `PV_STS_JF_02_RUN` |
| `SHP_LGT_01` | Shape | 120 | 332 | 140 | 18 | `PV_STS_LGT_01_FB_ON` |
| `SHP_LGT_02` | Shape | 280 | 332 | 140 | 18 | `PV_STS_LGT_02_FB_ON` |
| `SHP_LGT_03` | Shape | 440 | 332 | 140 | 18 | `PV_STS_LGT_03_FB_ON` |
| `SHP_LGT_04` | Shape | 600 | 332 | 140 | 18 | `PV_STS_LGT_04_FB_ON` |
| `IMG_VMS_01` | Image | 110 | 88 | 60 | 40 | `PV_STS_VMS_01_RUN` |
| `IMG_LCS_01` | Image | 854 | 88 | 60 | 40 | `PV_STS_LCS_01_RUN` |
| `IMG_BAR_01` | Image | 72 | 388 | 56 | 56 | `PV_STS_BAR_01_FB_OPEN` |
| `IMG_BAR_02` | Image | 820 | 388 | 56 | 56 | `PV_STS_BAR_02_FB_OPEN` |
| `IMG_PMP_01` | Image | 860 | 248 | 90 | 70 | `PV_STS_PMP_01_RUN` |
| `LBL_RIGHT_SUMMARY` | Label | 860 | 150 | 130 | 80 | multiple |

### Durum Davranis Kurali

- `fault` varsa ilgili ekipman kirmizi outline ile gosterilir
- `run` varsa yesil
- `local` varsa mavi yerine sari-mavi accent
- `fire zone` aktifse ilgili transshape blink eder
- `e-stop` aktifse tunnel altinda buyuk kirmizi bant gosterilir

## Ventilation

### Object List

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `CARD_JF_01` | Shape | 40 | 100 | 440 | 180 | none |
| `CARD_JF_02` | Shape | 544 | 100 | 440 | 180 | none |
| `BTN_JF_01_START` | Button | 70 | 220 | 100 | 38 | `PV_CMD_JF_01_START_REQ` |
| `BTN_JF_01_STOP` | Button | 180 | 220 | 100 | 38 | `PV_CMD_JF_01_STOP_REQ` |
| `BTN_JF_01_RESET` | Button | 290 | 220 | 100 | 38 | `PV_CMD_JF_01_RESET_REQ` |
| `BTN_JF_02_START` | Button | 574 | 220 | 100 | 38 | `PV_CMD_JF_02_START_REQ` |
| `BTN_JF_02_STOP` | Button | 684 | 220 | 100 | 38 | `PV_CMD_JF_02_STOP_REQ` |
| `BTN_JF_02_RESET` | Button | 794 | 220 | 100 | 38 | `PV_CMD_JF_02_RESET_REQ` |
| `LBL_JF_01_STATE` | Label | 70 | 130 | 220 | 24 | `PV_STS_JF_01_RUN` + status bits |
| `LBL_JF_02_STATE` | Label | 574 | 130 | 220 | 24 | `PV_STS_JF_02_RUN` + status bits |
| `LBL_VENT_MODE` | Label | 40 | 320 | 200 | 24 | `PV_SET_VENT_GLOBAL_MODE` |

### Durum Davranis Kurali

- `local` iken start/stop butonlari disable
- `fault` varsa reset butonu vurgulanir
- `ready=0` ise start butonu gri kalir

## Lighting

### Object List

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `CARD_LGT_01` | Shape | 32 | 96 | 232 | 170 | none |
| `CARD_LGT_02` | Shape | 280 | 96 | 232 | 170 | none |
| `CARD_LGT_03` | Shape | 528 | 96 | 232 | 170 | none |
| `CARD_LGT_04` | Shape | 776 | 96 | 216 | 170 | none |
| `BTN_LGT_01_ON` | Button | 52 | 210 | 74 | 34 | `PV_CMD_LGT_01_ON_REQ` |
| `BTN_LGT_01_OFF` | Button | 136 | 210 | 74 | 34 | `PV_CMD_LGT_01_OFF_REQ` |
| `BTN_LGT_02_ON` | Button | 300 | 210 | 74 | 34 | `PV_CMD_LGT_02_ON_REQ` |
| `BTN_LGT_02_OFF` | Button | 384 | 210 | 74 | 34 | `PV_CMD_LGT_02_OFF_REQ` |
| `BTN_LGT_03_ON` | Button | 548 | 210 | 74 | 34 | `PV_CMD_LGT_03_ON_REQ` |
| `BTN_LGT_03_OFF` | Button | 632 | 210 | 74 | 34 | `PV_CMD_LGT_03_OFF_REQ` |
| `BTN_LGT_04_ON` | Button | 794 | 210 | 74 | 34 | `PV_CMD_LGT_04_ON_REQ` |
| `BTN_LGT_04_OFF` | Button | 878 | 210 | 74 | 34 | `PV_CMD_LGT_04_OFF_REQ` |
| `LBL_LIGHT_MODE` | Label | 32 | 300 | 220 | 24 | `PV_SET_LIGHTING_GLOBAL_MODE` |

## Energy

### Object List

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `CARD_MAIN_ENERGY` | Shape | 52 | 102 | 210 | 180 | `PV_STS_PWR_MAIN_AVAILABLE` |
| `CARD_UPS` | Shape | 290 | 102 | 210 | 180 | `PV_STS_PWR_UPS_AVAILABLE` |
| `CARD_GENERATOR` | Shape | 528 | 102 | 210 | 180 | `PV_STS_PWR_GEN_AVAILABLE` |
| `CARD_MCC` | Shape | 766 | 102 | 210 | 180 | `PV_STS_PWR_MCC_AVAILABLE` |
| `LBL_MAIN_VOLT` | Label | 72 | 220 | 120 | 22 | `PV_STS_PWR_MAIN_VOLT` |
| `LBL_UPS_LOAD` | Label | 310 | 220 | 120 | 22 | `PV_STS_UPS_LOAD_PCT` |

## Alarm

### Object List

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `LBL_ALM_HEADER` | Label | 24 | 76 | 200 | 26 | none |
| `BTN_ALM_ACK_ALL` | Button | 760 | 76 | 110 | 36 | `PV_CMD_GLOBAL_ALARM_ACK_REQ` |
| `BTN_ALM_RESET_ALL` | Button | 886 | 76 | 120 | 36 | `PV_CMD_GLOBAL_ALARM_RESET_REQ` |
| `LBL_ALM_ROW_01` | Label | 24 | 132 | 976 | 28 | `PV_ALM_LAST_CODE` / active list |
| `LBL_ALM_ROW_02` | Label | 24 | 166 | 976 | 28 | active list |
| `LBL_ALM_ROW_03` | Label | 24 | 200 | 976 | 28 | active list |
| `LBL_ALM_ROW_04` | Label | 24 | 234 | 976 | 28 | active list |
| `LBL_ALM_ROW_05` | Label | 24 | 268 | 976 | 28 | active list |
| `LBL_ALM_FOOTER` | Label | 24 | 500 | 400 | 20 | `PV_ALM_ACTIVE_CNT` / `PV_ALM_UNACK_CNT` |

## Navigation Iliskisi

- `Dashboard` -> tum ekranlara ozet gecis
- `Mimic` -> `Ventilation`, `Lighting`, `Energy`, `Auxiliary`, `Fire` hotzone gecisi
- `Alarm` -> ilgili ekipman sayfasina detay gecis

## Riskler / Acik Noktalar

- HMI editorunde pixel snapping ve font metrikleri farkli olabilir
- image asset isimlendirme standardi henuz yazilmadi
- aktif alarm satir listesi runtime kabiliyetine gore custom mekanizma isteyebilir

## Sonraki Adim

Bu object listeleri sonraki turda:

- reusable component listesine
- image asset naming planina
- popup/dialog akislarina

donusturulebilir.

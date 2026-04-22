# HMI Screen Object Draft

## Durum Ozeti

Bu dokuman ilk surum ekranlar icin object list, yerlesim mantigi, variable binding ve navigasyon iliskilerini tanimlar. Revizyon, board'un tum ana fonksiyonlarini HMI'da gorunur ve kullanilabilir kilacak sekilde guncellenmistir.

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
| `SHP_ETH_COMM` | Shape | 742 | 15 | 20 | 24 | `PV_SYS_ETH_COMM_OK` |
| `SHP_RS485_COMM` | Shape | 768 | 15 | 20 | 24 | `PV_SYS_RS485_COMM_OK` |
| `SHP_CAN_COMM` | Shape | 794 | 15 | 20 | 24 | `PV_SYS_CAN_COMM_OK` |
| `LBL_ALM_ACTIVE_CNT` | Label | 828 | 10 | 90 | 18 | `PV_ALM_ACTIVE_CNT` |
| `LBL_ALM_UNACK_CNT` | Label | 828 | 30 | 90 | 18 | `PV_ALM_UNACK_CNT` |
| `SHP_FIRE_SUMMARY` | Shape | 926 | 15 | 36 | 24 | `PV_SYS_FIRE_ACTIVE` |
| `SHP_ESTOP_SUMMARY` | Shape | 970 | 15 | 36 | 24 | `PV_SYS_ESTOP_ACTIVE` |

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

## Dashboard Add-on Mobile Air Panel

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `GRP_DASH_MAQS` | Shape | 504 | 340 | 504 | 72 | none |
| `LBL_MAQS_ONLINE` | Label | 520 | 354 | 90 | 20 | `PV_STS_MAQS_ONLINE` |
| `LBL_MAQS_CO` | Label | 620 | 354 | 80 | 20 | `PV_STS_MAQS_CO_PPM` |
| `LBL_MAQS_PM25` | Label | 710 | 354 | 90 | 20 | `PV_STS_MAQS_PM25_UGM3` |
| `LBL_MAQS_PM10` | Label | 810 | 354 | 90 | 20 | `PV_STS_MAQS_PM10_UGM3` |
| `LBL_MAQS_AGE` | Label | 910 | 354 | 80 | 20 | `PV_STS_MAQS_PACKET_AGE_S` |

## Dashboard

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
| `LBL_SUM_COMM` | Label | 32 | 454 | 120 | 22 | `PV_SYS_ETH_COMM_OK` |
| `LBL_SUM_FIRE` | Label | 180 | 454 | 120 | 22 | `PV_SYS_FIRE_ACTIVE` |
| `LBL_SUM_ESTOP` | Label | 328 | 454 | 120 | 22 | `PV_SYS_ESTOP_ACTIVE` |
| `LBL_SUM_MODE` | Label | 476 | 454 | 120 | 22 | `PV_SYS_MODE_AUTO` |
| `LBL_SUM_ALARM` | Label | 624 | 454 | 140 | 22 | `PV_ALM_ACTIVE_CNT` |
| `LBL_SUM_CO` | Label | 780 | 454 | 90 | 22 | `PV_STS_TUNNEL_CO_PPM` |
| `LBL_SUM_TEMP` | Label | 892 | 454 | 100 | 22 | `PV_STS_TUNNEL_TEMP_C` |

## Mimic

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `SHP_TUNNEL_BODY` | Shape | 84 | 154 | 700 | 170 | none |
| `IMG_PORTAL_LEFT` | Image | 34 | 154 | 60 | 170 | none |
| `IMG_PORTAL_RIGHT` | Image | 784 | 154 | 60 | 170 | none |
| `SHP_FIRE_Z1` | TransShape | 210 | 124 | 180 | 230 | zone-1 fire |
| `SHP_FIRE_Z2` | TransShape | 470 | 124 | 180 | 230 | zone-2 fire |
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
| `LBL_AI_CO` | Label | 860 | 340 | 130 | 20 | `PV_STS_TUNNEL_CO_PPM` |
| `LBL_AI_LEVEL` | Label | 860 | 365 | 130 | 20 | `PV_STS_SUMP_LEVEL_PCT` |
| `LBL_TEMP_TUNNEL` | Label | 860 | 390 | 130 | 20 | `PV_STS_TUNNEL_TEMP_C` |
| `LBL_TEMP_CAB` | Label | 860 | 415 | 130 | 20 | `PV_STS_CABINET_TEMP_C` |

## Ventilation

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
| `LBL_VENT_MODE` | Label | 40 | 320 | 200 | 24 | `PV_SET_VENT_GLOBAL_MODE` |
| `LBL_CO_VALUE` | Label | 40 | 360 | 200 | 24 | `PV_STS_TUNNEL_CO_PPM` |
| `LBL_VIS_VALUE` | Label | 260 | 360 | 200 | 24 | `PV_STS_TUNNEL_NOX_PPM` veya `VISIBILITY` |
| `LBL_AO_01_REF` | Label | 480 | 360 | 200 | 24 | `PV_SET_AO_01_RAW` |
| `LBL_AO_01_FB` | Label | 700 | 360 | 200 | 24 | `PV_STS_AO_01_FB_RAW` |

## Lighting

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
| `LBL_AO_02_REF` | Label | 32 | 344 | 220 | 24 | `PV_SET_AO_02_RAW` |
| `LBL_LIGHT_SCENE_INFO` | Label | 280 | 344 | 320 | 24 | `PV_SET_LIGHT_DIM_REF` |

## Energy

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `CARD_MAIN_ENERGY` | Shape | 52 | 102 | 210 | 180 | `PV_STS_PWR_MAIN_AVAILABLE` |
| `CARD_UPS` | Shape | 290 | 102 | 210 | 180 | `PV_STS_PWR_UPS_AVAILABLE` |
| `CARD_GENERATOR` | Shape | 528 | 102 | 210 | 180 | `PV_STS_PWR_GEN_AVAILABLE` |
| `CARD_MCC` | Shape | 766 | 102 | 210 | 180 | `PV_STS_PWR_MCC_AVAILABLE` |
| `LBL_MAIN_VOLT` | Label | 72 | 220 | 120 | 22 | `PV_STS_PWR_MAIN_VOLT` |
| `LBL_MAIN_CURR` | Label | 72 | 246 | 120 | 22 | `PV_STS_PWR_MAIN_CURR` |
| `LBL_MAIN_PWR` | Label | 72 | 272 | 120 | 22 | `PV_STS_PWR_MAIN_PWR_KW` |
| `LBL_UPS_LOAD` | Label | 310 | 220 | 120 | 22 | `PV_STS_UPS_LOAD_PCT` |
| `LBL_UPS_BATT` | Label | 310 | 246 | 120 | 22 | `PV_STS_UPS_BATT_PCT` |
| `LBL_GEN_FUEL` | Label | 548 | 220 | 120 | 22 | `PV_STS_GEN_FUEL_PCT` |
| `LBL_RS485_STATE` | Label | 766 | 220 | 150 | 22 | `PV_SYS_RS485_COMM_OK` |
| `LBL_CAN_STATE` | Label | 766 | 246 | 150 | 22 | `PV_SYS_CAN_COMM_OK` |

## Alarm

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `LBL_ALM_HEADER` | Label | 24 | 76 | 200 | 26 | none |
| `BTN_ALM_ACK_ALL` | Button | 760 | 76 | 110 | 36 | `PV_CMD_GLOBAL_ALARM_ACK_REQ` |
| `BTN_ALM_RESET_ALL` | Button | 886 | 76 | 120 | 36 | `PV_CMD_GLOBAL_ALARM_RESET_REQ` |
| `LBL_ALM_ROW_01` | Label | 24 | 132 | 976 | 28 | active list |
| `LBL_ALM_ROW_02` | Label | 24 | 166 | 976 | 28 | active list |
| `LBL_ALM_ROW_03` | Label | 24 | 200 | 976 | 28 | active list |
| `LBL_ALM_ROW_04` | Label | 24 | 234 | 976 | 28 | active list |
| `LBL_ALM_ROW_05` | Label | 24 | 268 | 976 | 28 | active list |
| `LBL_ALM_FOOTER` | Label | 24 | 500 | 400 | 20 | `PV_ALM_ACTIVE_CNT` / `PV_ALM_UNACK_CNT` |
| `LBL_ALM_COMM_STATE` | Label | 460 | 500 | 280 | 20 | comm summary |

## Auxiliary Systems

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `CARD_VMS_01` | Shape | 40 | 100 | 220 | 160 | `PV_STS_VMS_01_RUN` |
| `CARD_LCS_01` | Shape | 282 | 100 | 220 | 160 | `PV_STS_LCS_01_RUN` |
| `CARD_BAR_01` | Shape | 524 | 100 | 220 | 160 | `PV_STS_BAR_01_FB_OPEN` |
| `CARD_BAR_02` | Shape | 766 | 100 | 220 | 160 | `PV_STS_BAR_02_FB_OPEN` |
| `BTN_BAR_01_OPEN` | Button | 548 | 210 | 80 | 34 | `PV_CMD_BAR_01_OPEN_REQ` |
| `BTN_BAR_01_CLOSE` | Button | 638 | 210 | 80 | 34 | `PV_CMD_BAR_01_CLOSE_REQ` |
| `BTN_BAR_02_OPEN` | Button | 790 | 210 | 80 | 34 | `PV_CMD_BAR_02_OPEN_REQ` |
| `BTN_BAR_02_CLOSE` | Button | 880 | 210 | 80 | 34 | `PV_CMD_BAR_02_CLOSE_REQ` |

## Fire & Safety

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `CARD_FIRE_Z1` | Shape | 60 | 110 | 280 | 180 | zone-1 fire |
| `CARD_FIRE_Z2` | Shape | 372 | 110 | 280 | 180 | zone-2 fire |
| `CARD_ESTOP` | Shape | 684 | 110 | 280 | 180 | `PV_SYS_ESTOP_ACTIVE` |
| `LBL_TUNNEL_TEMP` | Label | 60 | 320 | 200 | 24 | `PV_STS_TUNNEL_TEMP_C` |
| `LBL_MCC_TEMP` | Label | 280 | 320 | 200 | 24 | `PV_STS_MCC_ROOM_TEMP_C` |
| `LBL_CAB_TEMP` | Label | 500 | 320 | 200 | 24 | `PV_STS_CABINET_TEMP_C` |
| `LBL_FIRE_ACK_HINT` | Label | 60 | 380 | 420 | 24 | alarm ack/reset status |

## Air Quality / Mobile Station

| Object | Type | x | y | w | h | Binding |
|---|---|---:|---:|---:|---:|---|
| `CARD_MAQS_STATUS` | Shape | 32 | 92 | 280 | 180 | `PV_STS_MAQS_ONLINE` |
| `CARD_MAQS_GAS` | Shape | 332 | 92 | 320 | 180 | multiple gas vars |
| `CARD_MAQS_PM` | Shape | 672 | 92 | 320 | 180 | pm vars |
| `LBL_MAQS_LOC` | Label | 52 | 130 | 220 | 22 | lat/lon or zone |
| `LBL_MAQS_AGE` | Label | 52 | 158 | 220 | 22 | `PV_STS_MAQS_PACKET_AGE_S` |
| `LBL_MAQS_RSSI` | Label | 52 | 186 | 220 | 22 | `PV_STS_MAQS_SIGNAL_RSSI` |
| `LBL_MAQS_CO` | Label | 352 | 124 | 120 | 20 | `PV_STS_MAQS_CO_PPM` |
| `LBL_MAQS_CO2` | Label | 352 | 150 | 120 | 20 | `PV_STS_MAQS_CO2_PPM` |
| `LBL_MAQS_NO2` | Label | 352 | 176 | 120 | 20 | `PV_STS_MAQS_NO2_PPM` |
| `LBL_MAQS_SO2` | Label | 492 | 124 | 120 | 20 | `PV_STS_MAQS_SO2_PPM` |
| `LBL_MAQS_H2S` | Label | 492 | 150 | 120 | 20 | `PV_STS_MAQS_H2S_PPM` |
| `LBL_MAQS_CH4` | Label | 492 | 176 | 120 | 20 | `PV_STS_MAQS_CH4_PCTLEL` |
| `LBL_MAQS_PM1` | Label | 692 | 124 | 120 | 20 | `PV_STS_MAQS_PM1_UGM3` |
| `LBL_MAQS_PM25` | Label | 692 | 150 | 120 | 20 | `PV_STS_MAQS_PM25_UGM3` |
| `LBL_MAQS_PM10` | Label | 692 | 176 | 120 | 20 | `PV_STS_MAQS_PM10_UGM3` |
| `LBL_MAQS_AQI` | Label | 832 | 124 | 120 | 20 | `PV_STS_AIR_QUALITY_INDEX` |
| `LBL_MAQS_SRC` | Label | 832 | 150 | 120 | 20 | `PV_STS_AIR_QUALITY_SOURCE_SEL` |
| `LBL_MAQS_ACT` | Label | 32 | 304 | 960 | 28 | ventilation action summary |

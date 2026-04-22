# PLC Signal Matrix

## Durum Ozeti

Bu dokuman, tunnel HMI_PLC sistemi icin fiziksel board I/O'larini ekipman, HMI variable, Modbus adresi ve alarm etkisi ile tek tabloda birlestirir. Bu tablo, PLC kodu ile HMI runtime arasindaki ana saha referansi olarak kullanilmalidir.

## Kullanim Kurali

Her satir su sorulara cevap verir:

- fiziksel kanal nedir
- hangi ekipmana aittir
- PLC mantiginda rolu nedir
- HMI'da hangi variable ile temsil edilir
- Modbus'ta hangi alana baglanir
- alarm veya cause/effect etkisi var midir

## Dijital Giris Matrisi

| Channel | PLC Signal | Ekipman | Anlam | HMI Variable | Modbus | Alarm / Etki |
|---|---|---|---|---|---|---|
| `DI-01` | `ESTOP_FB` | `ESTOP_01` | acil stop loop aktif | `PV_SYS_ESTOP_ACTIVE` | `10005` | `5003 ESTOP_ACTIVE`, tum komutlar inhibit |
| `DI-02` | `FIRE_Z1_FB` | `FIRE_Z1` | yangin zon-1 aktif | `PV_SYS_FIRE_ACTIVE` + zone flag | mapped discrete | `5001 FIRE_Z1_ACTIVE`, mimic blink |
| `DI-03` | `FIRE_Z2_FB` | `FIRE_Z2` | yangin zon-2 aktif | `PV_SYS_FIRE_ACTIVE` + zone flag | mapped discrete | `5002 FIRE_Z2_ACTIVE`, mimic blink |
| `DI-04` | `JF_01_RUN_FB` | `JF_01` | fan-1 run feedback | `PV_STS_JF_01_RUN` | `10101` | fail-to-start mantigi icin geri bildirim |
| `DI-05` | `JF_01_FAULT_FB` | `JF_01` | fan-1 ariza feedback | `PV_STS_JF_01_FAULT` | `10102` | `1002 JF_01_TRIP` |
| `DI-06` | `JF_02_RUN_FB` | `JF_02` | fan-2 run feedback | `PV_STS_JF_02_RUN` | `10111` | fail-to-start mantigi icin geri bildirim |
| `DI-07` | `JF_02_FAULT_FB` | `JF_02` | fan-2 ariza feedback | `PV_STS_JF_02_FAULT` | `10112` | `1012 JF_02_TRIP` |
| `DI-08` | `PMP_01_RUN_FB` | `PMP_01` | pompa run feedback | `PV_STS_PMP_01_RUN` | `10305` | fail-to-start mantigi icin geri bildirim |
| `DI-09` | `PMP_01_FAULT_FB` | `PMP_01` | pompa ariza feedback | `PV_STS_PMP_01_FAULT` | `10306` | `4002 PMP_01_TRIP` |
| `DI-10` | `MAIN_ENERGY_FB` | `PWR_MAIN` | ana enerji mevcut | `PV_STS_PWR_MAIN_AVAILABLE` | `10301` | `3001 MAIN_ENERGY_LOSS` |
| `DI-11` | `UPS_FB` | `PWR_UPS` | UPS healthy/available | `PV_STS_PWR_UPS_AVAILABLE` | `10302` | `3002 UPS_FAULT` |
| `DI-12` | `GEN_FB` | `PWR_GEN` | generator available/run | `PV_STS_PWR_GEN_AVAILABLE` | `10303` | `3003 GENERATOR_FAULT` |
| `DI-13` | `MCC_TRIP_FB` | `PWR_MCC` | MCC fault/trip | `PV_STS_PWR_MCC_AVAILABLE` ters mantik ile | `10304` | `3004 MCC_FAULT` |
| `DI-14` | `BAR_01_OPEN_FB` | `BAR_01` | bariyer-1 acik | `PV_STS_BAR_01_FB_OPEN` | `10309` | auxiliary status |
| `DI-15` | `BAR_02_OPEN_FB` | `BAR_02` | bariyer-2 acik | `PV_STS_BAR_02_FB_OPEN` | `10311` | auxiliary status |
| `DI-16` | `REMOTE_LOCAL_SEL_FB` | `SYSTEM` | panel local/remote secici | `PV_STS_REMOTE_LOCAL_SEL_FB` | `10313` | local mode inhibit |

## Dijital Cikis Matrisi

| Channel | PLC Signal | Ekipman | Anlam | HMI Command Variable | Modbus Coil | Interlock Notu |
|---|---|---|---|---|---|---|
| `DO-01` | `JF_01_START_CMD` | `JF_01` | fan-1 start cikisi | `PV_CMD_JF_01_START_REQ` / `STOP_REQ` | `00011/00012` | fire/local/fault izinli olmali |
| `DO-02` | `JF_02_START_CMD` | `JF_02` | fan-2 start cikisi | `PV_CMD_JF_02_START_REQ` / `STOP_REQ` | `00014/00015` | fire/local/fault izinli olmali |
| `DO-03` | `LGT_01_CMD` | `LGT_01` | aydinlatma grup-1 | `PV_CMD_LGT_01_ON_REQ` / `OFF_REQ` | `00031/00032` | mode mantigina bagli |
| `DO-04` | `LGT_02_CMD` | `LGT_02` | aydinlatma grup-2 | `PV_CMD_LGT_02_ON_REQ` / `OFF_REQ` | `00033/00034` | mode mantigina bagli |
| `DO-05` | `LGT_03_CMD` | `LGT_03` | aydinlatma grup-3 | `PV_CMD_LGT_03_ON_REQ` / `OFF_REQ` | `00035/00036` | mode mantigina bagli |
| `DO-06` | `LGT_04_CMD` | `LGT_04` | aydinlatma grup-4 | `PV_CMD_LGT_04_ON_REQ` / `OFF_REQ` | `00037/00038` | mode mantigina bagli |
| `DO-07` | `PMP_01_START_CMD` | `PMP_01` | pompa start cikisi | `PV_CMD_PMP_01_START_REQ` / `STOP_REQ` | `00051/00052` | level/fault interlock olmali |
| `DO-08` | `HORN_BEACON_CMD` | `SYSTEM` | ortak alarm horn/beacon | `PV_CMD_GLOBAL_HORN_SILENCE_REQ` | `00003` | critical/unacked alarm mantigi |

## Analog Giris Matrisi 4-20mA

| Channel | PLC Signal | Ekipman / Alan | Anlam | HMI Variable | Modbus Register | Alarm / Etki |
|---|---|---|---|---|---|---|
| `AI-01` | `TUNNEL_CO_PPM` | Tunnel process | CO olcumu | `PV_STS_TUNNEL_CO_PPM` | `30004` | `7001 CO_SENSOR_FAIL`, ventilation logic input |
| `AI-02` | `TUNNEL_NOX_PPM` veya `VISIBILITY` | Tunnel process | hava kalitesi / gorus | `PV_STS_TUNNEL_NOX_PPM` veya visibility var | `30005` | `7002 VISIBILITY_SENSOR_FAIL` |
| `AI-03` | `SUMP_LEVEL_PCT` | `PMP_01` | drenaj kuyu seviyesi | `PV_STS_SUMP_LEVEL_PCT` | `30006` | pompa auto start/stop, high level alarm |
| `AI-04` | `POWER_DEMAND_PCT` veya `AIRFLOW_FEEDBACK` | process | yuk/debi feedback | `PV_STS_POWER_DEMAND_PCT` | `30007` | ventilation/energy summary |

## Analog Cikis Matrisi

| Channel | PLC Signal | Ekipman | Anlam | HMI Variable | Modbus Register | Not |
|---|---|---|---|---|---|---|
| `AO-01` | `VENT_REF_0_10V` | ventilation | fan hiz/ref veya ortak havalandirma referansi | `PV_SET_AO_01_RAW`, `PV_SET_JF_01_SPEED_REF`, `PV_SET_JF_02_SPEED_REF` | `40021`, `40023`, `40024` | fanlar VFD ise aktif |
| `AO-02` | `LIGHT_REF_0_10V` | lighting | dim/ref/test output | `PV_SET_AO_02_RAW`, `PV_SET_LIGHT_DIM_REF` | `40022`, `40025` | dimming destegi teyit edilmeli |

## Sicaklik Sensor Matrisi

| Channel | PLC Signal | Alan | Anlam | HMI Variable | Modbus Register | Alarm |
|---|---|---|---|---|---|---|
| `PT100-01` | `TUNNEL_TEMP` | tunnel | tunnel sicakligi | `PV_STS_TUNNEL_TEMP_C` | `30001` | `7003 PT100_01_FAIL` |
| `PT100-02` | `MCC_ROOM_TEMP` | MCC room | pano/oda sicakligi | `PV_STS_MCC_ROOM_TEMP_C` | `30002` | `7004 PT100_02_FAIL` |
| `DS18B20-01` | `CABINET_TEMP` | cabinet | lokal cabinet sicakligi | `PV_STS_CABINET_TEMP_C` | `30003` | `7005 DS18B20_FAIL` |

## Haberlesme Matrisi

| Kanal | Rol | PLC Signal / Variable | HMI Variable | Modbus / Etki | Not |
|---|---|---|---|---|---|
| `Ethernet Modbus` | ana HMI haberlesmesi | PLC slave server | `PV_SYS_ETH_COMM_OK`, `PV_SYS_PLC_COMM_OK` | tum HMI veri kontrati | ana operator hattı |
| `RS485` | enerji analizoru / servis RTU | energy meter data | `PV_SYS_RS485_COMM_OK`, power vars | `30008-30013` ve ilgili health bitleri | Modbus RTU |
| `CANbus` | yardimci cihaz agi | VMS/LCS/BARRIER states | `PV_SYS_CAN_COMM_OK`, aux vars | `10307-10312`, `10408` | cihaz protokolu netlesmeli |
| `I2C` | RTC / local expander | timestamp source | `PV_SYS_RTC_SYNC_OK` | alarm timestamp / event log | ilk baseline icin RTC onerilir |

## Ekipman Bazli Ozet

### JF-01

- Komut: `DO-01`
- Geri bildirim: `DI-04`, `DI-05`
- HMI durumlari: `PV_STS_JF_01_*`
- Komutlari: `PV_CMD_JF_01_*`
- Sayaçlari: `40101`, `40102`
- Alarm: `1001`, `1002`

### JF-02

- Komut: `DO-02`
- Geri bildirim: `DI-06`, `DI-07`
- HMI durumlari: `PV_STS_JF_02_*`
- Komutlari: `PV_CMD_JF_02_*`
- Sayaçlari: `40103`, `40104`
- Alarm: `1011`, `1012`

### Lighting Groups

- Komutlar: `DO-03..DO-06`
- Durumlar: discrete inputs / contactor feedback mantigi ile genisletilebilir
- HMI: `PV_STS_LGT_0X_*`, `PV_CMD_LGT_0X_*`
- Dim/ref: `AO-02`

### PMP-01

- Komut: `DO-07`
- Feedback: `DI-08`, `DI-09`
- Process input: `AI-03`
- HMI: `PV_STS_PMP_01_*`, `PV_CMD_PMP_01_*`

## Riskler / Acik Noktalar

- Lighting geri bildirimi icin fiziksel DI ayrilmadi; bu geri bildirim kontaktor auxiliary ile ayrica tasarlanmak istenirse DI plan revize olabilir
- `BAR_01` ve `BAR_02` sadece open feedback ile temsil edildi; close feedback istenirse DI genisleme gerekir veya CAN tarafindan okunur
- `AI-02` icin visibility mi NOx mi kullanilacagi sahaya gore netlesmeli
- Enerji analizor register haritasi geldiginde `30008-30013` alanlari kesinlestirilmeli

## Sonraki Adim

Bu matrix'ten sonra en dogru teknik adim:

1. `PLC tag database`
2. `page-by-page variable matrix`
3. `alarm list to HMI row binding`
4. `popup / dialog / command confirmation flow`

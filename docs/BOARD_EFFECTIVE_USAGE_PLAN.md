# Tunnel PLC Board Effective Usage Plan

## Durum Ozeti

Bu dokuman, mevcut PLC board'un fiziksel kabiliyetlerini tunnel HMI_PLC demo sistemine uyarlar. Hedef, board uzerindeki tum ana fonksiyonlari etkin ve anlamli sekilde kullanmaktir.

Board kabiliyetleri:

- `16` dijital giris
- `8` dijital cikis
- `2` analog cikis (`12V`)
- `2` PT100 girisi
- `4` adet `4-20mA` analog giris
- `1` adet `DS18B20` dijital sicaklik sensori girisi
- `1` adet `I2C` portu
- `RS485`
- `CANbus`
- `Ethernet Modbus`

## Tasarim Karari

Board'un efektif kullanimi icin haberlesme ve I/O rolleri su sekilde ayrilir:

- `Ethernet Modbus`: HMI ana haberlesme kanali
- `RS485`: saha servis/enerji analizoru/uzak I/O veya ikinci HMI servis portu
- `CANbus`: yardimci akilli saha cihazlari veya uzak genisleme dugumleri
- `DI/DO`: kritik durum ve komut geri bildirimleri
- `AI 4-20mA`: tunnel proses olcumleri
- `AO 12V`: saha referans/surucu set degerleri veya analog izleme cikislari
- `PT100`: tunnel ve MCC/oda sicaklik izleme
- `DS18B20`: cabinet/board lokal sicaklik izleme
- `I2C`: RTC, expansion I/O, environmental sensor ya da lokal servis modulu

## Ekipman Bazli Donanim Dagilimi

### Dijital Girisler

| DI | Sinyal | Amac |
|---|---|---|
| `DI-01` | `ESTOP_FB` | acil stop loop geri bildirimi |
| `DI-02` | `FIRE_Z1_FB` | yangin zon 1 aktif |
| `DI-03` | `FIRE_Z2_FB` | yangin zon 2 aktif |
| `DI-04` | `JF_01_RUN_FB` | fan-1 run feedback |
| `DI-05` | `JF_01_FAULT_FB` | fan-1 ariza feedback |
| `DI-06` | `JF_02_RUN_FB` | fan-2 run feedback |
| `DI-07` | `JF_02_FAULT_FB` | fan-2 ariza feedback |
| `DI-08` | `PMP_01_RUN_FB` | pompa run feedback |
| `DI-09` | `PMP_01_FAULT_FB` | pompa ariza feedback |
| `DI-10` | `MAIN_ENERGY_FB` | ana enerji var |
| `DI-11` | `UPS_FB` | UPS healthy |
| `DI-12` | `GEN_FB` | generator available/run |
| `DI-13` | `MCC_TRIP_FB` | MCC trip/fault |
| `DI-14` | `BAR_01_OPEN_FB` | bariyer-1 acik |
| `DI-15` | `BAR_02_OPEN_FB` | bariyer-2 acik |
| `DI-16` | `REMOTE_LOCAL_SEL_FB` | panel local/remote switch durumu |

### Dijital Cikisler

| DO | Sinyal | Amac |
|---|---|---|
| `DO-01` | `JF_01_START_CMD` | fan-1 start komutu |
| `DO-02` | `JF_02_START_CMD` | fan-2 start komutu |
| `DO-03` | `LGT_01_CMD` | aydinlatma grup-1 |
| `DO-04` | `LGT_02_CMD` | aydinlatma grup-2 |
| `DO-05` | `LGT_03_CMD` | aydinlatma grup-3 |
| `DO-06` | `LGT_04_CMD` | aydinlatma grup-4 |
| `DO-07` | `PMP_01_START_CMD` | drenaj pompasi komutu |
| `DO-08` | `HORN_BEACON_CMD` | ortak alarm horn/beacon |

Not:

- `VMS`, `LCS`, `BARRIER` gibi elemanlar direkt DO yerine tercihen `CANbus`, `RS485` veya `Ethernet` uzerinden akilli cihaz olarak ele alinabilir.
- Boylece 8 adet fiziksel DO daha kritik ekipmanlarda tutulur.

### Analog Girisler 4-20mA

| AI | Sinyal | Tipik Olcum |
|---|---|---|
| `AI-01` | `TUNNEL_CO_PPM` | CO sensor |
| `AI-02` | `TUNNEL_NOX_PPM` veya `VISIBILITY` | hava kalitesi / gorus |
| `AI-03` | `SUMP_LEVEL_PCT` | drenaj seviye transmitter |
| `AI-04` | `POWER_DEMAND_PCT` veya `AIRFLOW_FEEDBACK` | yuk / debi / proses geri bildirimi |

### Analog Cikisler 12V

| AO | Sinyal | Amac |
|---|---|---|
| `AO-01` | `VENT_REF_0_10V` | VFD / fan hiz referansi veya ortak havalandirma referansi |
| `AO-02` | `LIGHT_REF_0_10V` | dimming / analog test / harici izleme referansi |

### Sicaklik Sensorleri

| Kanal | Sinyal | Amac |
|---|---|---|
| `PT100-01` | `TUNNEL_TEMP` | tunnel ic sicaklik |
| `PT100-02` | `MCC_ROOM_TEMP` | pano/oda sicakligi |
| `DS18B20-01` | `CABINET_TEMP` | board/pano lokal sicaklik |

### I2C Portu

Ilk production baseline icin en mantikli kullanim:

- `RTC + event timestamp`

### RS485 Portu

Ilk baseline icin onerilen kullanim:

- `Energy meter / MCC side Modbus RTU`

### CANbus

Ilk baseline icin onerilen kullanim:

- `VMS-01`
- `LCS-01`
- `BARRIER-01`
- `BARRIER-02`

### Ethernet Modbus

Ana HMI haberlesme kanali olarak kullanilacak:

- HMI -> PLC komut ve durum haberlesmesi
- alarm listesi
- runtime counterlar
- analog degerler
- event/timestamp alanlari

## HMI Revizyon Etkisi

Bu board kullanimi nedeniyle HMI tarafinda asagidaki ekran/alanlar guclenir:

- `Dashboard`: analog trend ve process summary eklenir
- `Mimic`: proses olcumleri ve canbus yardimci cihaz durumlari eklenir
- `Ventilation`: `AO-01` referansi / airflow feedback alani eklenir
- `Lighting`: `AO-02` dimming veya analog referans karti eklenir
- `Energy`: RS485 enerji analizoru degerleri eklenir
- `Alarm`: I/O arizalari, sensor fail, comm fail alarm gruplari ayrilir

## Riskler / Acik Noktalar

- Fanlarin hiz kontrollu olup olmadigi teyit edilmezse `AO-01` nihai atamasi degisebilir
- Aydinlatma dimming destek durumu teyit edilmezse `AO-02` rezerv fonksiyon alabilir
- `VMS/LCS/BARRIER` haberlesme protokolu netlesmeden CAN/RS485 dagilimi kesinlestirilmemeli

# Base Tunnel System Architecture

## Durum Ozeti

Bu dokuman, demo sistemin sadece belirli sayida ekipman gosteren bir PoC degil, urunlesebilir bir `Base Tunnel Operations Platform` olarak nasil kurgulanacagini tanimlar.

Hedef:

- profesyonel PLC + SCADA hissi vermek
- sinirli I/O ile bile buyuk sistem mantigi gostermek
- gercek saha cihazlari eklenince kolay genisleyebilmek
- HMI'yi "operator istasyonu" tadinda konumlamak

## Ana Ilke

Sistem ikiye ayrilir:

- `Control Core`: PLC mantigi
- `Operations Shell`: HMI / SCADA benzeri operator kabugu

HMI karar vermez.
PLC ana kontrolu yapar.
HMI sistemin gorunen yuzu olur.

## Katmanli Mimari

### 1. Field Layer

Gercek fiziksel ve haberlesme katmani:

- DI / DO
- AI / AO
- PT100
- DS18B20
- RS485
- CANbus
- Ethernet Modbus
- I2C RTC
- LoRa tabanli mobil hava kalite istasyonu

### 2. Device Abstraction Layer

Her fiziksel veri sinyale donusur:

- `jf_01_run_fb`
- `main_energy_ok`
- `tunnel_co_ppm`
- `cabinet_temp_c`
- `mobile_aqs_pm25`
- `mobile_aqs_pm10`
- `mobile_aqs_co`
- `mobile_aqs_latlon`

Bu katman saha bagimliligini yukaridan ayirir.

### 3. Equipment Function Block Layer

Her ekipman bir standart function block ile temsil edilir:

- `fb_jet_fan`
- `fb_lighting_group`
- `fb_pump`
- `fb_barrier`
- `fb_vms`
- `fb_lcs`
- `fb_power_source`
- `fb_mobile_air_station`
- `fb_alarm_manager`
- `fb_comm_manager`

Her ekipmanin ortak modeli:

- `cmd`
- `sts`
- `mode`
- `perm`
- `interlock`
- `alarm`
- `runtime`
- `starts`

### 4. Tunnel Logic Layer

Sistem seviyesinde mantik:

- fire senaryolari
- e-stop davranisi
- enerji kaynagi ozetleme
- ventilation auto strategy
- lighting mode strategy
- sump/pump strategy
- communication degraded strategy
- mobile air quality guided strategy
- fixed sensor + mobile station data fusion

### 5. Data Contract Layer

HMI ile PLC arasindaki resmi veri kontrati:

- tag database
- Modbus mapping
- alarm code listesi
- event / timestamp modeli
- mobil hava kalite istasyonu veri protokolu

### 6. HMI Operations Layer

Operator tarafinda gosterilen kabuk:

- dashboard
- mimic
- ventilation
- lighting
- energy
- alarm
- auxiliary
- fire & safety
- diagnostic / maintenance

## Demo'yu Ikna Edici Yapan Unsurlar

Bu urunu etkileyici gosteren sey yalnizca ekipman sayisi degil, profesyonel davranis katmanidir.

Mutlaka gosterilmesi gerekenler:

- command request mantigi
- command accepted / rejected feedback
- local/remote inhibit
- auto/manual state
- fail-to-start
- timeout
- alarm ack
- alarm reset
- event timestamp
- communication quality
- degraded mode
- runtime / starts counters
- sensor invalid quality state
- mobil istasyon online/offline durumu
- mobil istasyon konumu ve son paket zamani
- sabit sensor ile mobil sensor karsilastirma gorunumu

## SCADA Tadinda HMI Icin Gerekli Ozellikler

Demo HMI'da asgari su SCADA hissi olmali:

- top bar system health
- quality/status renk kodlari
- comm state gorunurlugu
- alarm summary + unacked summary
- cihaz bazli popup detail
- command confirmation dialog
- event/alarm timestamp
- operator message / action feedback
- maintenance/diagnostic ekranlari
- mobil hava kalite istasyonu paneli
- gaz/PM bazli otomatik aksiyon ozetleri

## Gercek Urun Ornekleri Kullanildiginda

Gercek sample urun eklenince:

- fan kartlari generic ikon yerine urun gorseline gecebilir
- LED projektor / lighting card alanlarina gercek urun gorseli konabilir
- mimic sahnesi "sample product aware" hale getirilebilir
- HMI, "biz bunu sadece cizmedik, gercek urune uyarlayabiliyoruz" mesajini verir

## Onerilen Ekran Aileleri

### Operator Screens

- Dashboard
- Mimic
- Ventilation
- Lighting
- Energy
- Alarm
- Auxiliary
- Fire & Safety
- Air Quality / Mobile Station

### Maintenance Screens

- I/O Diagnostic
- Communication Diagnostic
- Sensor Diagnostic
- Analog I/O Calibration
- Device Detail Popup
- LoRa packet monitor
- RS485 protocol monitor

### Commissioning Screens

- Force/Test Mode
- Register Watch
- Event Timeline
- Alarm Raw Bits
- mobile station simulation

## Mobil Hava Kalite Istasyonu Veri Zinciri

Onerilen veri akisi:

1. Mobil hava kalite istasyonu olcum alir
2. LoRa uzerinden `ADV` board'una belirlenen ozel protokol ile veri basar
3. `ADV`, konum + gaz + PM verilerini toplar
4. `ADV`, bu veriyi `RS485` uzerinden HMI/PLC tarafina standartlastirilmis protokolle aktarir
5. PLC bu veriyi:
   - alarm
   - ventilation strategy
   - operator bilgi paneli
   - event kaydi
   icin kullanir

## Mobil Istasyon Kullanım Amaci

Bu unsur demoyu ciddi sekilde guclendirir cunku:

- sistem sabit sensorlerle sinirli kalmaz
- mobil olcum noktalari ile "dinamik saha gorusu" saglanir
- tunnel icindeki lokal kirletici birikimleri mobil veriyle gosterilebilir
- PLC'nin yalniz izleyen degil, kosullara gore aksiyon alan karar motoru oldugu ispatlanir

## Sonraki Adim

Bu mimariyi somut urun haline tasimak icin:

1. `Project Profile Template`
2. `PLC Tag Database`
3. `Page Variable Matrix`
4. `Popup / Confirmation Model`

hazirlanmalidir.

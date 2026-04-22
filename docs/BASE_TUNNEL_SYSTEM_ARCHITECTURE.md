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

### 2. Device Abstraction Layer

Her fiziksel veri sinyale donusur:

- `jf_01_run_fb`
- `main_energy_ok`
- `tunnel_co_ppm`
- `cabinet_temp_c`

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

### 5. Data Contract Layer

HMI ile PLC arasindaki resmi veri kontrati:

- tag database
- Modbus mapping
- alarm code listesi
- event / timestamp modeli

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

### Maintenance Screens

- I/O Diagnostic
- Communication Diagnostic
- Sensor Diagnostic
- Analog I/O Calibration
- Device Detail Popup

### Commissioning Screens

- Force/Test Mode
- Register Watch
- Event Timeline
- Alarm Raw Bits

## Sonraki Adim

Bu mimariyi somut urun haline tasimak icin:

1. `Project Profile Template`
2. `PLC Tag Database`
3. `Page Variable Matrix`
4. `Popup / Confirmation Model`

hazirlanmalidir.

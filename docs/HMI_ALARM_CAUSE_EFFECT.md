# Alarm and Cause & Effect Draft

## Durum Ozeti

Bu dokuman revize board kaynaklarina gore alarm listesi ve cause & effect mantigini gunceller.

## Severity Standardi

| Kod | Seviye | Renk | Davranis |
|---|---|---|---|
| `1` | Minor | sari | alarm listesine duser, blink yok |
| `2` | Major | sari/kirmizi | belirgin gosterim |
| `3` | Critical | kirmizi | ack edilene kadar blink |
| `4` | Safety Critical | kirmizi | blink + ust bar guvenlik vurgusu |

## Ack / Reset Kurali

- `ack`: operator alarmi gordugunu bildirir
- `reset`: sadece resetlenebilir kosul ortadan kalkmissa kabul edilir
- `latched` alarm kosul kalksa bile reset/ack gerektirebilir
- `fire` ve `e-stop` safety class kabul edilir

## Alarm Listesi

| Code | Name | Severity | Ack | Latch |
|---|---|---:|---|---|
| `1001` | `JF_01_FAIL_TO_START` | 2 | Evet | Evet |
| `1002` | `JF_01_TRIP` | 3 | Evet | Evet |
| `1011` | `JF_02_FAIL_TO_START` | 2 | Evet | Evet |
| `1012` | `JF_02_TRIP` | 3 | Evet | Evet |
| `2001` | `LGT_01_FAULT` | 1 | Evet | Hayir |
| `2002` | `LGT_02_FAULT` | 1 | Evet | Hayir |
| `2003` | `LGT_03_FAULT` | 1 | Evet | Hayir |
| `2004` | `LGT_04_FAULT` | 1 | Evet | Hayir |
| `3001` | `MAIN_ENERGY_LOSS` | 3 | Evet | Evet |
| `3002` | `UPS_FAULT` | 2 | Evet | Evet |
| `3003` | `GENERATOR_FAULT` | 2 | Evet | Evet |
| `3004` | `MCC_FAULT` | 3 | Evet | Evet |
| `4001` | `PMP_01_FAIL_TO_START` | 2 | Evet | Evet |
| `4002` | `PMP_01_TRIP` | 3 | Evet | Evet |
| `5001` | `FIRE_Z1_ACTIVE` | 4 | Evet | Evet |
| `5002` | `FIRE_Z2_ACTIVE` | 4 | Evet | Evet |
| `5003` | `ESTOP_ACTIVE` | 4 | Evet | Evet |
| `6001` | `PLC_COMM_LOSS` | 4 | Hayir | Hayir |
| `6002` | `HMI_COMM_DEGRADED` | 2 | Hayir | Hayir |
| `6003` | `RS485_COMM_FAIL` | 2 | Evet | Hayir |
| `6004` | `CAN_COMM_FAIL` | 2 | Evet | Hayir |
| `7001` | `CO_SENSOR_FAIL` | 2 | Evet | Hayir |
| `7002` | `VISIBILITY_SENSOR_FAIL` | 2 | Evet | Hayir |
| `7003` | `PT100_01_FAIL` | 2 | Evet | Hayir |
| `7004` | `PT100_02_FAIL` | 2 | Evet | Hayir |
| `7005` | `DS18B20_FAIL` | 1 | Evet | Hayir |
| `7006` | `ENERGY_METER_COMM_FAIL` | 2 | Evet | Hayir |
| `7007` | `CAN_AUX_DEVICE_FAIL` | 2 | Evet | Hayir |
| `7008` | `AI_4_20MA_OUT_OF_RANGE` | 2 | Evet | Hayir |
| `7009` | `AO_CHANNEL_FAULT` | 2 | Evet | Hayir |

## Cause & Effect Matrix

| Cause | Effect |
|---|---|
| `FIRE_Z1_ACTIVE` | fire summary aktif, mimic zone-1 blink, ventilation ekraninda ilgili fan stratejisi goster, alarm listesine critical ekle |
| `FIRE_Z2_ACTIVE` | fire summary aktif, mimic zone-2 blink, ventilation ekraninda ilgili fan stratejisi goster, alarm listesine critical ekle |
| `ESTOP_ACTIVE` | tum komut butonlari inhibit gorunsun, ust barda safety aktif gosterilsin, mimicte e-stop alani blink etsin |
| `PLC_COMM_LOSS` | tum ekipman card'lari communication degraded stiline gecsin, komut butonlari disable olsun |
| `RS485_COMM_FAIL` | energy sayfasinda meter verileri invalid gosterilsin |
| `CAN_COMM_FAIL` | VMS/LCS/BARRIER kartlari comm-fault overlay alsin |
| `JF_01_FAIL_TO_START` | `JF_01` status karti warning/fault versin |
| `JF_01_TRIP` | `JF_01` kirmizi fault, reset butonu gorunsun |
| `JF_02_FAIL_TO_START` | `JF_02` status karti uyari gostersin |
| `JF_02_TRIP` | `JF_02` kirmizi fault gostersin |
| `MAIN_ENERGY_LOSS` | enerji ekraninda main kirmizi, UPS/GEN durumu one ciksin |
| `UPS_FAULT` | enerji kartinda warning/fault goster |
| `GENERATOR_FAULT` | generator karti kirmizi goster |
| `PMP_01_TRIP` | pompa ozet karti kirmizi, mimic pompa alaninda fault simgesi goster |
| `CO_SENSOR_FAIL` | ventilation ve dashboard process kartlarinda sensor invalid durum goster |
| `VISIBILITY_SENSOR_FAIL` | ventilation/lighting strateji kartlari degraded modda gosterilir |
| `PT100_01_FAIL` | tunnel temp alani invalid/gray olarak isaretlenir |
| `PT100_02_FAIL` | MCC room temp invalid gosterilir |
| `DS18B20_FAIL` | cabinet temp invalid gosterilir |
| `ENERGY_METER_COMM_FAIL` | energy sayfasi communication degraded moda gecer |
| `CAN_AUX_DEVICE_FAIL` | VMS/LCS/BARRIER ikonlari comm-fault overlay alir |
| `AI_4_20MA_OUT_OF_RANGE` | ilgili proses degeri invalid range alarmi ile gosterilir |
| `AO_CHANNEL_FAULT` | ventilation/lighting analog ref card'i fault border ile gosterilir |

## Local / Remote / Auto / Manual Davranisi

### Local Mod

- HMI komut butonu gorunebilir ama aktif komut veremez
- ekipman karti `LOCAL` vurgusu tasir

### Remote Mod

- HMI komutlari PLC tarafinda degerlendirilebilir
- command butonlari enable olur

### Auto Mod

- ekipman karti `AUTO` rozeti gosterir
- manuel komutlar kismen inhibit olabilir

### Manual Mod

- ekipman tekil operator komutlarina acik olabilir

### Fail-to-Start

- komut verildi, belirlenen surede `run feedback` gelmedi
- alarm major/critical sinifina gore listelenir

## Riskler / Acik Noktalar

- Fan yangin senaryosu yonu ve fail-safe stratejisi henuz sahaya gore teyit edilmedi
- Analog output kullanim amaci saha ekipmanlarina gore teyit edilmelidir

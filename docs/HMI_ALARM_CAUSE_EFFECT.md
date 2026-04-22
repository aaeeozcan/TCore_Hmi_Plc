# Alarm and Cause & Effect Draft

## Durum Ozeti

Bu dokuman ilk alarm listesi ve cause & effect mantigini uretim oncesi taslak olarak cikarir. Nihai saha matrix'i devreye alma ve PLC detay mantigi ile birlikte netlesmelidir.

## Severity Standardi

| Kod | Seviye | Renk | Davranis |
|---|---|---|---|
| `1` | Minor | sari | alarm listesine duser, blink yok |
| `2` | Major | sari/kirmizi | ozet bloklarda belirgin gosterim |
| `3` | Critical | kirmizi | ack edilene kadar blink |
| `4` | Safety Critical | kirmizi | blink + ust bar guvenlik vurgusu |

## Ack / Reset Kurali

- `ack`: operator alarmi gordugunu bildirir
- `reset`: sadece resetlenebilir kosul ortadan kalkmissa kabul edilir
- `latched` alarm kosul kalksa bile reset/ack gerektirebilir
- `fire` ve `e-stop` grubu safety class kabul edilir

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

## Cause & Effect Matrix

| Cause | Effect |
|---|---|
| `FIRE_Z1_ACTIVE` | fire summary aktif, mimic zone-1 blink, ventilation ekraninda ilgili fan stratejisi goster, alarm listesine critical ekle |
| `FIRE_Z2_ACTIVE` | fire summary aktif, mimic zone-2 blink, ventilation ekraninda ilgili fan stratejisi goster, alarm listesine critical ekle |
| `ESTOP_ACTIVE` | tum komut butonlari inhibit gorunsun, ust barda safety aktif gosterilsin, mimicte e-stop alani blink etsin |
| `PLC_COMM_LOSS` | tum ekipman card'lari communication degraded stiline gecsin, komut butonlari disable olsun |
| `JF_01_FAIL_TO_START` | `JF_01` status karti sari/kirmizi ikaz versin, Alarm sayfasi detay gostersin |
| `JF_01_TRIP` | `JF_01` kirmizi fault, reset butonu gorunsun ama PLC iznine bagli olsun |
| `JF_02_FAIL_TO_START` | `JF_02` status karti uyari gostersin |
| `JF_02_TRIP` | `JF_02` kirmizi fault gostersin |
| `MAIN_ENERGY_LOSS` | enerji ekraninda main kirmizi, UPS/GEN durumu one ciksin |
| `UPS_FAULT` | enerji kartinda warning/fault goster |
| `GENERATOR_FAULT` | generator karti kirmizi, ozet panelde major/critical sayimina gir |
| `PMP_01_TRIP` | pompa ozet karti kirmizi, mimic pompa alaninda fault simgesi goster |

## Local / Remote / Auto / Manual Davranisi

### Local Mod

- HMI komut butonu gorunebilir ama aktif komut veremez
- ekipman karti mavi yerine sari/mavi karisik `LOCAL` vurgusu tasir
- `command rejected by local mode` olay kaydi PLC tarafinda uretilmelidir

### Remote Mod

- HMI komutlari PLC tarafinda degerlendirilebilir
- command butonlari enable olur

### Auto Mod

- ekipman karti `AUTO` rozeti gosterir
- manuel komutlar kismen inhibit olabilir

### Manual Mod

- ekipman tekil operator komutlarina acik olabilir
- otomatik strateji aktif gostergesi kapanir

### Fail-to-Start

- komut verildi, belirlenen surede `run feedback` gelmedi
- alarm major/critical sinifina gore listelenir
- reset PLC tarafinda kosul temizlendikten sonra kabul edilmelidir

## Riskler / Acik Noktalar

- Fan yangin senaryosu yonu ve fail-safe stratejisi heniz sahaya gore teyit edilmedi
- Barrier/VMS/LCS icin detay interlock mantigi henuz net degil
- Alarm reset politikasinin operator proseduru ile uyumlu hale getirilmesi gerekecek

## Sonraki Adim

Bu alarm modeli ekran object listelerinde ikon, renk ve blink davranisina baglanacak.

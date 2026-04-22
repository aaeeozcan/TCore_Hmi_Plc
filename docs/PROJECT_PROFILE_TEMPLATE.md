# Project Profile Template

## Durum Ozeti

Bu dokuman, base tunnel sistemin farkli projelere uyarlanmasi icin kullanilacak profil mantigini tanimlar.

Yani sistem:

- tek bir sabit urun cekirdegi
- birden fazla proje profili

ile calisacak sekilde dusunulur.

## Profil Parametreleri

### Genel

- `project_name`
- `site_name`
- `tunnel_name`
- `tunnel_length_m`
- `hmi_resolution`
- `language_set`

### Ekipman Sayilari

- `jet_fan_count`
- `lighting_group_count`
- `pump_count`
- `fire_zone_count`
- `barrier_count`
- `vms_count`
- `lcs_count`

### Kontrol Ozellikleri

- `ventilation_has_speed_ref`
- `lighting_has_dimming`
- `energy_meter_present`
- `generator_present`
- `ups_present`
- `rtc_present`
- `can_aux_devices_present`

### Analog Kanal Atamalari

- `ai01_role`
- `ai02_role`
- `ai03_role`
- `ai04_role`
- `ao01_role`
- `ao02_role`

### Haberlesme Profili

- `hmi_link_type`
- `meter_link_type`
- `aux_device_link_type`
- `remote_service_port_enabled`

## Bu Demo Icin Profil

| Alan | Deger |
|---|---|
| `project_name` | `Tunnel Base Demo` |
| `site_name` | `Demo Site` |
| `tunnel_name` | `Tunnel-01` |
| `jet_fan_count` | `2` |
| `lighting_group_count` | `4` |
| `pump_count` | `1` |
| `fire_zone_count` | `2` |
| `barrier_count` | `2` |
| `vms_count` | `1` |
| `lcs_count` | `1` |
| `ventilation_has_speed_ref` | `true / field confirmation pending` |
| `lighting_has_dimming` | `optional / field confirmation pending` |
| `energy_meter_present` | `true` |
| `generator_present` | `true` |
| `ups_present` | `true` |
| `rtc_present` | `true` |
| `can_aux_devices_present` | `true` |
| `hmi_link_type` | `Ethernet Modbus TCP` |
| `meter_link_type` | `RS485 Modbus RTU` |
| `aux_device_link_type` | `CANbus` |

## Profilin HMI'ya Etkisi

Profil ile:

- gorunmeyen ekipman kartlari gizlenebilir
- alarm listesi filtrelenebilir
- mimic objeleri sayiya gore cizilebilir
- menu kalemleri azaltilip artirilabilir
- comm widget'lari proje tipine gore degisebilir

## Profilin PLC'ye Etkisi

Profil ile:

- aktif edilmeyen function block'lar bypass edilir
- I/O binding farkli projelere yeniden atanir
- alarm listesi parametre bazli acilir/kapanir
- Modbus map'in rezerv bloklari korunarak proje varyanti secilir

## Sonraki Adim

Bu profil mantigina uygun olarak tum sistem degiskenleri tek katalogda toplanmalidir.

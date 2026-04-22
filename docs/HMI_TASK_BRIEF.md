# Tunnel HMI + PLC Control System

## Baslik

Tunnel HMI + PLC control system migration and first production baseline in PlatformIO

## Amaç

Bu dokuman, tunnel/otoyol uygulamasi icin gelistirilecek HMI + PLC sisteminin ilk resmi calisma brief'ini kayda alir. Bu hatta:

- PLC kontrol mantigi tarafinda ana karar mekanizmasi olacak
- HMI yalnizca operator arayuzu gorevini ustlenecek
- HMI ile PLC arasinda Modbus tabanli cift yonlu haberlesme kurulacak
- VS Code + local Codex + PlatformIO ortaminda ilerleme saglanacak

Not: Bu dokuman HMI hattinin resmi referansidir. Gecmis PlatformIO duzenleme detaylari bu brief'in kapsam belirleme asamasinda esas kabul edilmez.

## Sistem Rolu Ayrimi

### PLC

- interlock
- permissive
- timeout
- fail-to-start
- local/remote
- auto/manual
- saha ekipmani kontrol mantigi

### HMI

- durum gosterimi
- komut istegi yazma
- alarm ve olay gosterimi
- operator navigasyonu

HMI dogrudan saha cikisi surmez.

## Calisma Ortami

- IDE: `Visual Studio Code`
- Agent: `Local Codex`
- Build system: `PlatformIO`
- Embedded target: STM32 tabanli mevcut Keil projesi, yeni PlatformIO yapisina tasinacak
- HMI cozumunurlugu: `1024x600`, landscape
- HMI tipi: sinirli runtime mantiginda endustriyel HMI

## HMI Nesne Yaklasimi

- `Label`
- `Button`
- `Image`
- `Shape`
- `TransShape`
- `public variable`
- `internal variable`

## Baz Alinacak Saha Konfigurasyonu

### Havalandirma

- `JF-01`
- `JF-02`

### Aydinlatma

- `LGT-01` giris grubu
- `LGT-02` ic bolge A
- `LGT-03` ic bolge B
- `LGT-04` cikis grubu

### Enerji Sistemi

- `Main Energy`
- `UPS`
- `Generator`
- `MCC`

### Yardimci Sistemler

- `PMP-01`
- `VMS-01`
- `LCS-01`
- `BARRIER-01`
- `BARRIER-02`

### Guvenlik

- `FIRE-Z1`
- `FIRE-Z2`
- `E-STOP`

## Kritik Tasarim Kurallari

- HMI dogrudan cikis surmez
- HMI yalnizca PLC'ye komut istegi yazar
- tum kontrol mantigi PLC icindedir
- HMI yalnizca gosterim + komut + alarm + navigasyon katmanidir
- her ekipman standart bir durum modeli kullanir

## Ekipman Standart Durum Modeli

Her ekipman icin asgari durum alanlari:

- `run`
- `fault`
- `local`
- `auto`
- `ready`
- `runtime`
- `starts_count`

## Alarm Modeli

- severity tabanli
- ack destekli
- latch mantiginda
- blink yalnizca `fire`, `e-stop` ve `unacked critical alarm` icin kullanilir

## Modbus Ayrimi

Onerilen ayrim:

- `coils`: komutlar
- `discrete inputs`: durum bitleri
- `holding registers`: modlar, sayaclar, set degerleri, alarm sayilari
- `input registers`: analog degerler

## UI Tasarim Kurallari

- gri: off / pasif
- yesil: calisiyor / healthy
- sari: warning
- kirmizi: fault / critical
- mavi: auto / remote / ready
- gereksiz animasyon yok
- operasyonel ve sade endustriyel dil kullanilacak

## Ilk Surum Ekranlari

- `Dashboard`
- `Tunnel Mimic`
- `Ventilation`
- `Lighting`
- `Energy`
- `Alarm`
- `Auxiliary Systems`
- `Fire & Safety`

## Minimum Calisan Ilk Ekran Seti

- `Dashboard`
- `Mimic`
- `Ventilation`
- `Lighting`
- `Energy`
- `Alarm`

## 1024x600 Ekran Layout Kurali

Sabit alanlar:

- ust bar
- ana calisma alani
- alt menu bar

## Dashboard Ozet Kurali

- sol: tunnel genel ozet mimic alani
- sag ust: enerji kartlari
- sag orta: fan ve pompa ozetleri
- alt: fire / e-stop / alarm / comm / mode summary
- alt global menu

## Mimic Ozet Kurali

- tunnel govdesi
- sol ve sag portal
- 2 yangin zonu
- 2 fan
- 4 aydinlatma segmenti
- `VMS`, `LCS`, bariyer ikonlari
- pompa alani
- `e-stop` durumu
- sag ozet panel
- hotzone gecis alanlari

## Teslim Fazlari

### Faz-1

- mevcut embedded projeyi analiz et
- migration planini yaz
- `platformio.ini` oner
- klasor yerlesimini oner

### Faz-2

- proje dosyalarini PlatformIO duzenine tasi
- build hatalarini temizle
- derleme altyapisini ayaga kaldir

### Faz-3

- Modbus veri modelini cikar
- equipment/tag/public variable standardini yaz
- alarm ve cause & effect tablolarini uret

### Faz-4

- `Dashboard` ve `Mimic` icin uretim seviyesinde object listeleri olustur
- `Ventilation` ve `Lighting` ekranlarini cikar
- `Energy` ve `Alarm` ekranlarini tamamla

### Faz-5

- proje icinde dokumante et
- gerekirse ayri markdown/spec dosyalari olustur

## Done Criteria

- PlatformIO projesi acilmis ve yapilandirilmis olacak
- Keil ana embedded yapi anlasilmis ve raporlanmis olacak
- gerekli dosyalar PlatformIO duzenine tasinmis olacak
- `platformio.ini` calisir durumda olacak
- HMI ekran mimarisi yazilmis olacak
- Modbus veri modeli tanimlanmis olacak
- alarm listesi ve cause & effect matrisi yazilmis olacak
- asgari ilk 4 ana ekran icin object list ve variable plani cikmis olacak:
  - `Dashboard`
  - `Mimic`
  - `Ventilation`
  - `Lighting`

## Siki Calisma Kurallari

- mevcut davranisi gereksiz yere degistirme
- buyuk refactor oncesi yapiyi analiz et
- once calisir iskelet, sonra iyilestirme
- uydurma dosya ya da uydurma donanim varsayimi yapma
- emin olunmayan noktalar acikca not edilmeli
- build hatalarinda kok neden dosya/dizin/include seviyesinde aranacak

## Bu Repo Icin Mevcut Durum Notu

Bu repository taramasinda:

- ayrik bir HMI proje klasoru bulunmadi
- HMI runtime asset dosyalari bulunmadi
- mevcut firmware tarafinda yalnizca eski bir UART bazli HMI status paketi izi bulundu
- Modbus tarafi su anda taslak/eksik durumdadir

Bu nedenle HMI hatti icin ilk resmi uretim artefakti, once markdown/spec dokumanlari olarak ilerletilecektir.

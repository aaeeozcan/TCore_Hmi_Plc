# HMI Faz-1 Baslangic Notu

## Durum Ozeti

HMI tarafi icin resmi brief alinmis durumda. Repository icinde bugun itibariyla ayrik bir HMI runtime projesi bulunmuyor. Bu nedenle ilk adim, HMI mimarisini ve veri modelini markdown tabanli teknik spesifikasyon olarak cikarmak olacak.

## Yapilan Cikarimlar

- HMI, PLC'den bagimsiz karar katmani olmayacak
- PLC ana kontrol mantigini tasiyacak
- HMI yalnizca komut istegi + gosterim + alarm + navigasyon katmani olacak
- Modbus veri modeli HMI ve PLC arasindaki tek resmi veri kontrati gibi ele alinmali
- İlk ekran seti operasyonel ve sade tutulmali

## Repository Taramasindan Gelen Notlar

- mevcut firmware tarafinda `USART1` Modbus/test hatti olarak isimlenmis
- eski bir `$PLC,...` formatli HMI durum paketi izi var
- fakat HMI proje dosyasi, ekran tanimi veya runtime export yapisi bu repoda henuz yok

## Riskler / Acik Noktalar

- HMI runtime'in dosya formati henuz net degil
- ekran nesne editorunun disa aktarim bicimi bilinmiyor
- Modbus master/slave rolu HMI-PLC arasinda fiilen netlestirilmeli
- alarm acknowledgement/reset davranisinin saha beklentisiyle birebir esitlenmesi gerekecek

## Sonraki Adim

Siradaki mantikli dokuman seti:

1. `HMI variable naming standard`
2. `Modbus mapping draft`
3. `Alarm and cause & effect matrix`
4. `Dashboard` ve `Mimic` object listeleri

## Ilk Is Emri

Ilk teknik uretim ciktisi olarak su dokumanlar yazilacak:

- public/internal/command variable isim standardi
- ekipman durum modeli
- Modbus adresleme plani
- ilk ekranlar icin object listeleri

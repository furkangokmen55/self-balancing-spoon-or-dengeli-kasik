# Kaşık (Kendi Kendini Dengeleyen Platform)

MPU6050 ivmeölçer/jiroskop sensörü ve iki servo motor kullanarak bir platformu (kaşığı) yatay dengede tutmaya çalışan, el titremesi veya sarsıntıyı telafi eden bir denge sistemi. Tümleyici filtre (complementary filter) ile ivmeölçer ve jiroskop verisi birleştirilerek anlık eğim açısı hesaplanır, bu açıya göre iki eksendeki servo motorlar ters yönde hareket ettirilerek platform dengede tutulur.

## Nasıl Çalışır

1. MPU6050 sensöründen ham ivme ve açısal hız verisi okunur.
2. İvmeölçer verisinden anlık eğim açısı (X ve Y ekseni), jiroskop verisinden ise açısal değişim hesaplanır.
3. Tümleyici filtre, iki veriyi ağırlıklandırarak (α = 0.96) gürültüsüz ve gecikmesiz bir açı tahmini üretir.
4. Hesaplanan açı, servo pozisyonuna eşlenir (`map` + `constrain` ile 20°–160° arasında sınırlanır) ve iki servo motor buna göre sürülür, böylece platform yatayda tutulmaya çalışılır.

## Donanım

- ESP8266 / ESP32 (I2C pinleri: SDA=GPIO4 (D2), SCL=GPIO5 (D1))
- MPU6050 ivmeölçer + jiroskop sensörü (I2C adresi: 0x68)
- 2× Servo motor (X ekseni: GPIO14 / D5, Y ekseni: GPIO12 / D6)

## Klasörler

| Klasör | Açıklama |
|---|---|
| `sketch_jun10b` | İlk test sürümü — `Adafruit_MPU6050` kütüphanesiyle sadece sensör okuma ve açı hesaplama yapar, servo sürmez. Sensör kalibrasyonunu/filtre davranışını gözlemlemek için kullanılmış. |
| `sketch_jun17a` | **Son/çalışan sürüm.** Kütüphanesiz, doğrudan I2C register okuması yapar; iki servoyu gerçek zamanlı sürerek platformu aktif olarak dengeler. |

> Not: İki klasörün de kendi `.ino` dosyası olduğu için Arduino IDE'de her biri ayrı proje olarak açılır.

## Kurulum

1. Arduino IDE'de gerekli kütüphaneleri kur:
   - `sketch_jun10b` için: `Adafruit_MPU6050`, `Adafruit_Sensor`
   - `sketch_jun17a` için ek kütüphane gerekmiyor (sadece `Wire` ve `Servo`, ikisi de Arduino ile birlikte gelir)
2. MPU6050'yi I2C pinlerine (SDA/SCL), servoları ilgili GPIO pinlerine bağla.
3. `sketch_jun17a/sketch_jun17a.ino` dosyasını kartına yükle (önerilen, çalışan sürüm).
4. Seri Port / Seri Çizici (Serial Plotter) üzerinden `Aci_X` ve `Aci_Y` değerlerini izleyerek denge davranışını gözlemleyebilirsin.

## Yapılabilecek İyileştirmeler

- Servo pozisyon sınırları (`constrain(20, 160)`) donanımına göre ince ayar gerektirebilir.
- PID kontrol eklenerek tepki hızı/titreşim daha da azaltılabilir.
- `sketch_jun10b` artık işlevsel olarak `sketch_jun17a`'nın içinde olduğu için, geliştirme geçmişini korumak istemiyorsan kaldırılabilir.

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// Filtre Değişkenleri
float aciX = 0, aciY = 0;
unsigned long oncekiZaman = 0;

// Filtre Katsayısı (Yüksek Geçiren Filtre Ağırlığı)
const float alpha = 0.96; 

void setup() {
  Serial.begin(115200);
  
  // ESP8266'nın seri portunun ayağa kalkmasını güvenli bir şekilde bekliyoruz
  delay(500); 

  // I2C pinlerini NodeMCU için tanımlıyoruz (SDA=D2->GPIO4, SCL=D1->GPIO5)
  Wire.begin(4, 5); 

  // ESP8266 uyumlu güvenli sensör başlatma (Adres: 0x68)
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 sensörü bulunamadı! Bağlantıları kontrol edin.");
    while (1) { 
      yield(); // ESP8266 Watchdog reset atmaması için arka plan görevlerini besliyoruz
    }
  }
  
  // Sensör hassasiyet ayarları (Titreme hassasiyeti için ideal ayarlar)
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // İlk dt sıçramasını önlemek için zaman sayacını tam loop başlamadan önce sıfırlıyoruz
  oncekiZaman = millis();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Geçen zamanı hesapla (dt)
  unsigned long simdikiZaman = millis();
  float dt = (simdikiZaman - oncekiZaman) / 1000.0;
  
  // dt'nin hatalı veya 0 çıkmasını engelleyen güvenlik sınırı
  if (dt <= 0.0 || dt > 0.5) dt = 0.01; 
  oncekiZaman = simdikiZaman;

  // 1. İvmeölçer verisinden açı hesaplama (Radyandan Dereceye)
  float ivmeAciX = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  float ivmeAciY = atan2(-a.acceleration.x, sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)) * 180.0 / PI;

  // 2. Jiroskop verisini entegre etme ve Tümleyici Filtre (Complementary Filter) uygulaması
  aciX = alpha * (aciX + (g.gyro.x * 180.0 / PI) * dt) + (1 - alpha) * ivmeAciX;
  aciY = alpha * (aciY + (g.gyro.y * 180.0 / PI) * dt) + (1 - alpha) * ivmeAciY;

  // Seri Çizici (Serial Plotter) için verileri ekrana bas
  Serial.print("Aci_X:");
  Serial.print(aciX);
  Serial.print(",");
  Serial.print("Aci_Y:");
  Serial.println(aciY);

  delay(10); // Yaklaşık 100Hz örnekleme hızı (ESP8266'yı da rahatlatır)
}
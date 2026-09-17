#include <Wire.h>
#include <Servo.h>

// MPU6050 I2C Adresi
const int MPU_addr = 0x68;

// Servo Motor Nesneleri
Servo servoX;
Servo servoY;

// Sensörden ham verileri çeken veri yapısı (Derleyicinin tanıması için en başa aldık)
struct sensors_event_t {
  int16_t accX, accY, accZ;
  int16_t gyrX, gyrY, gyrZ;
};

// Filtre Değişkenleri
float aciX = 0, aciY = 0;
float lowPassAX = 0, lowPassAY = 0, lowPassAZ = 0; 
unsigned long oncekiZaman = 0;
const float alpha = 0.96; 
const float alphaLow = 0.1; 

// Fonksiyonun prototipini derleyiciye önceden bildiriyoruz
void veriOku(sensors_event_t *a, sensors_event_t *g);

void setup() {
  Serial.begin(115200);
  delay(500);

  // I2C Pinlerini Başlat (SDA=D2, SCL=D1)
  Wire.begin(4, 5); 

  // Servoları Pinlere Ata (X=D5, Y=D6)
  servoX.attach(14); 
  servoY.attach(12); 

  // Motorları ilk açılışta tam orta noktaya (90 derece) getir
  servoX.write(90);
  servoY.write(90);
  delay(500);

  // MPU6050 Uyandırma
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  // İlk ivmeölçer değerlerini okuyup filtreyi önceden dolduralım
  sensors_event_t a, g;
  veriOku(&a, &g);
  lowPassAX = a.accX; 
  lowPassAY = a.accY; 
  lowPassAZ = a.accZ;

  Serial.println("Sistem Düzeltildi! 2 Eksen Aktif.");
  oncekiZaman = millis();
}

void veriOku(sensors_event_t *a, sensors_event_t *g) {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  
  // ESP8266 kütüphane çakışmasını önlemek için veri tiplerini açıkça belirtiyoruz (Casting)
  Wire.requestFrom((uint8_t)MPU_addr, (size_t)14, (bool)true);

  a->accX = Wire.read() << 8 | Wire.read();
  a->accY = Wire.read() << 8 | Wire.read();
  a->accZ = Wire.read() << 8 | Wire.read();
  Wire.read() << 8 | Wire.read(); // Sıcaklığı atla
  g->gyrX = Wire.read() << 8 | Wire.read();
  g->gyrY = Wire.read() << 8 | Wire.read();
  g->gyrZ = Wire.read() << 8 | Wire.read();
}

void loop() {
  sensors_event_t raw_a, raw_g;
  veriOku(&raw_a, &raw_g);

  // dt (Geçen Zaman) Hesaplama
  unsigned long simdikiZaman = millis();
  float dt = (simdikiZaman - oncekiZaman) / 1000.0;
  if (dt <= 0.0 || dt > 0.5) dt = 0.01; 
  oncekiZaman = simdikiZaman;

  // İvmeölçer Alçak Geçiren Filtre
  lowPassAX = alphaLow * raw_a.accX + (1 - alphaLow) * lowPassAX;
  lowPassAY = alphaLow * raw_a.accY + (1 - alphaLow) * lowPassAY;
  lowPassAZ = alphaLow * raw_a.accZ + (1 - alphaLow) * lowPassAZ;

  // Ham verileri G kuvvetine çevir
  float ax = lowPassAX / 16384.0;
  float ay = lowPassAY / 16384.0;
  float az = lowPassAZ / 16384.0;

  // 2 Eksen Bağımsız Matematiksel Hesaplama
  float ivmeAciX = atan2(ay, az) * 180.0 / PI;
  float ivmeAciY = atan(-ax / sqrt(ay * ay + az * az)) * 180.0 / PI;

  // Jiroskop Hızları (Derece/Saniye)
  float gx = raw_g.gyrX / 131.0;
  float gy = raw_g.gyrY / 131.0;

  // Tümleyici Filtre (Complementary Filter)
  aciX = alpha * (aciX + gx * dt) + (1 - alpha) * ivmeAciX;
  aciY = alpha * (aciY + gy * dt) + (1 - alpha) * ivmeAciY;

  // --- DENGELEME MANTIĞI ---
  int servoPozisyonX = map(aciX, -60, 60, 180, 0); 
  int servoPozisyonY = map(aciY, -60, 60, 0, 180); 

  // Güvenlik Sınırları (Motorları zorlamamak için kısıtlama)
  servoPozisyonX = constrain(servoPozisyonX, 20, 160);
  servoPozisyonY = constrain(servoPozisyonY, 20, 160);

  // Motorlara Emri Gönder
  servoX.write(servoPozisyonX);
  servoY.write(servoPozisyonY);

  // Takip için Seri Çizici formatı
  Serial.print("Aci_X:"); Serial.print(aciX);
  Serial.print(",Aci_Y:"); Serial.println(aciY);

  delay(15); 
}
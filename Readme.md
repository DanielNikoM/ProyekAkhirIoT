# Proyek Akhir IoT - CCTV ESP32-CAM dengan Joystick Kontrol

Proyek ini adalah implementasi sistem CCTV berbasis IoT yang memanfaatkan **ESP32-CAM** untuk streaming video dan pengambilan gambar serta **ESP32** tambahan untuk mengontrol posisi kamera menggunakan joystick. Sistem ini terintegrasi dengan Firebase dan Telegram untuk mendukung kontrol dan pengiriman data secara real-time.

## Fitur Proyek
1. **Streaming Video**:
   - ESP32-CAM melakukan streaming video secara langsung ke sebuah web server.
2. **Kontrol Kamera**:
   - Kamera dapat digerakkan secara horizontal dan vertikal menggunakan servo yang terhubung dengan joystick ESP32.
   - Posisi servo diperbarui secara real-time dengan membaca data posisi joystick dari Firebase.
3. **Integrasi Telegram**:
   - Mengirimkan foto melalui perintah Telegram.
   - Menyalakan atau mematikan lampu flash kamera melalui perintah Telegram.
4. **Firebase**:
   - Firebase digunakan sebagai database untuk menyimpan nilai joystick (X, Y) dan status tombol.
5. **Joystick**:
   - ESP32 kedua digunakan untuk membaca nilai joystick (X, Y) dan tombol, lalu mengirimkannya ke Firebase.

## Komponen yang Dibutuhkan
- **ESP32-CAM**
- **ESP32**
- **Joystick module** (VRX, VRY, tombol)
- **2 Servo Motor**
- **Koneksi WiFi**
- **Akun Firebase**
- **Bot Telegram**

## Instalasi
1. **Konfigurasi Firebase**:
   - Buat sebuah proyek Firebase baru.
   - Aktifkan Realtime Database dan tambahkan URL dan key ke kode.
2. **Bot Telegram**:
   - Buat bot Telegram menggunakan BotFather dan dapatkan token.
3. **ESP32-CAM**:
   - Pastikan kamera terhubung dengan benar sesuai dengan konfigurasi pin di kode.
   - Upload kode ESP32-CAM menggunakan Arduino IDE.
4. **ESP32 Joystick**:
   - Sambungkan joystick module ke ESP32.
   - Upload kode ESP32 joystick menggunakan Arduino IDE.
5. **Library yang Dibutuhkan**:
   - Install library berikut melalui Arduino IDE:
     - FirebaseESP32
     - UniversalTelegramBot
     - ArduinoJson
     - ESP32Servo

## Struktur Proyek
### ESP32-CAM
ESP32-CAM bertugas untuk:
- Streaming video ke web server.
- Mengambil foto berdasarkan perintah dari Telegram.
- Mengontrol lampu flash kamera.
- Menggerakkan servo berdasarkan nilai dari Firebase.

### ESP32 Joystick
ESP32 dengan joystick digunakan untuk:
- Membaca nilai sumbu X dan Y dari joystick.
- Membaca status tombol.
- Mengirimkan data X, Y, dan status tombol ke Firebase.

## Dokumentasi Kode
Berikut adalah penjelasan baris demi baris untuk kode yang digunakan dalam proyek ini:
- **ESP32 - CAM**
- **ESP32 Joystick**

## Cara Menggunakan
1. **Streaming Video**:
   - Akses `http://<IP_ESP32_CAM>/mjpeg/1` melalui browser untuk melihat streaming video.
2. **Mengambil Foto**:
   - Kirim perintah `/photo` ke bot Telegram.
3. **Menyalakan Lampu Flash**:
   - Kirim perintah `/flash` ke bot Telegram.
4. **Menggerakkan Kamera**:
   - Gerakkan joystick untuk mengontrol posisi kamera.

## Alur Sistem
1. ESP32 joystick membaca data X, Y, dan status tombol.
2. Data joystick dikirim ke Firebase.
3. ESP32-CAM membaca nilai X dan Y dari Firebase.
4. ESP32-CAM memperbarui posisi kamera menggunakan servo.
5. Pengguna dapat mengontrol kamera dan lampu flash melalui Telegram.

## Arsitektur Sistem
```plaintext
[Joystick] -> [ESP32 (Joystick)] -> [Firebase] <- [ESP32-CAM] <- [Telegram]
                                           |
                                     [Web Server]

```

## Flow Chart
![8BJiu.png](https://s6.imgcdn.dev/8BJiu.png)

## Desain Hardware dan Bentuk Alat Fisik
# ESP32 CAM
![unnamed](https://github.com/user-attachments/assets/bfc3e9de-6b4b-4b27-9dab-ec9f5ded8546)
# ESP32 Joystick
![unnamed-1](https://github.com/user-attachments/assets/511c40b5-0b65-4de7-a4b1-a2e6f4af7b7c)



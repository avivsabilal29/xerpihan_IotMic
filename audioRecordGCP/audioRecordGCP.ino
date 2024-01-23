#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h> // Library untuk menyimpan file kedalam memory internal ESP32
#include <FS.h> // Library untuk mengatur format file penyimpanan
#include <Wav.h> // custom file untuk menyimpan buffer
#include <I2S.h> // Library untuk membca buffer microphone 
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/SDHelper.h>
#define WIFI_SSID "TechDrips"
#define WIFI_PASSWORD "TechDrips123456789"
#define STORAGE_BUCKET_ID "web-xerpihan.appspot.com" // ID untuk pengiriman ke server
#define FIREBASE_PROJECT_ID "web-xerpihan" // ID Project di account GCP
#define FIREBASE_CLIENT_EMAIL "web-xerpihan@appspot.gserviceaccount.com" // Email CLient Project yang di GCP for autentikasi
// API untuk menghubungkan server kedalam mikro esp
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCYvv6HXtjobj8O\n4d5fakiStDgwZ+AURtXP9K+fCeat1s349ZiRvU4XsHKBwleSuVWOOR2+3/7Br3W0\nwqoz52FZVbTdQwPCYbdPyDIjzF4nUGA6J6wQXxVnJsuKdNMAHC/6jn9xPuT9S6n1\nETWFzUAN1NZawyT4vHG32/Jo/TzZtNzuFVempt7xak11g5QMKYXKRDnh82yRKImt\np/3Ec8yZ6FNDEFv0IByHWfOE3R9gjvoyR1PDuYX9VjGg/wEkOxD3M7+sTx6OoF3y\ngknynt0F2EmXd5qCUfuOW4SYqk0gVOz6/dyfD2t4ZDUWcyOWTa4hQAW4WgF8W78j\nAi+eVOWzAgMBAAECggEAJT2n3ARteRnsCOpb58KHiDElY/Pnh7oPhkC/ktR8qBSv\nqkvi02GmyRLNkEJ8THsgV2kojfGZ8MohNh2iFr6KlxfU3X9pyro9UdfbFaxNpmMr\n2vHyOxmbo8crKAo8qZG1xu54NvmhJfqNKQF1nbZBsD2ct4tNl9GGHlAjZaKcc+2K\nsGUdLRQlWGFwRwYFbsvKT5GPFROip8BEvWuLRbeBbBOpg6292ZfjxDEFVw2ZPTHf\nhf4Ff8uLdb6vQEujAiVLfiZCSI1lwNsoMMqZ+RvkxcZM1cI/0R9tIxz3PSfDaiiM\nfUsV0juBd/840LO7qRSslNcHVLyXQ1pgtdCpD4BKtQKBgQDTGihEHOeM9Lg9f5oz\n5l6PD4fPE5pRV328+aDh0pwx5JaWouF/YfM/13qJLvT0GXiBQ27cZtyqJek+vK4F\nkFyJ9c4Xjbkvoy9uD5tNK0u1sB4YHNoc4PWBQVoNe78Z3WpRBlAFTYEmBxfX+i/g\nGf2mh4Lr8fp1z8WIecPF+/heLwKBgQC5O4hOymWmI8aJCz1QJSHjCo6ubDiBv7Rb\nSrSoxxu70ME6q20Sn00RTOczim850g6FsDNfX5oJcTCcncJi4UGiYxu+Em4x6uwB\ndGvRUymxG/wMSsP7M3tQpOoJQmi6UO2H4BUKvA26MQA3ZZ3V5LABChymfNwUuSU4\nfhNrvQwzvQKBgQCBGi4l+cdiXQeQFMmpk+mgdE8KRWLK/EbZm5DYioUbvkCaP2LJ\nFI4/LPizjRQhdLJN23+hKwcox6atx5ZlsLPna3xuyy3GBHatSL4ho6jZpKvC82Iw\n5yM9uUnplRR6LP8oLkQh+hRRvWTHoSpGU57ciWN4CLSz+C1GGbGxEzgB4QKBgDyu\nsiMNz2s0QTBYTEfbQlS8+q1Nqt1mzlEyEilQwVcwNn1f96WoJnlKdlu+OF364OMP\nLGVMToxVVpWYYTw/i18FlOsZ4pvb2GFJoKC82+W3w0TLoZTD8twiisFpziFrrcVb\ntckEfoLG1slPMvwFpVtR0r0AHqFzXiw5sWcUUdEJAoGAI651AYNW5SMLIn/lgEsu\nqyRh5RwLC2bLxoRj4wRWDDW5lnkN9P9Fo/F21c3e2NKWRpFs8iAxXwkBKN1R5wGC\nFJwdWfhQEhTDdjRCGFBWeJ8uRZ3Ckvugec9c/wNtUOVxBU8qXLS8N/AMbBxkqDar\nu5oH9OyKochhO9Yhd8lCzDs=\n-----END PRIVATE KEY-----\n";
#define FILE_AUDIO_PATH "/recorder.wav" // Variabel untuk penamaan file yang akan dikirim
#define BUCKET_AUDIO "/AUDIO_XERPIHAN/recorder.wav" //Variabel untuk path pengiriman ke server
#define I2S_MODE I2S_MODE_ADC_BUILT_IN // Mode pembacaan I2S, nanti di library I2S di jelassin
#define LED_TURN_ON 12
#define LED_RECORDING_START 13
#define LED_RECORDING_END 14
#define BUTTON_START_RECORDING 27
bool recordStatus = false;
const int record_time = 20;  // Variabel untuk mengatur lamanya waktu perekaman
const int headerSize = 44; // Jumlah header file, jangan di ubah karena 44 adalah jumlah header file wav
const int waveDataSize = record_time * 88000; // jumlah buffer size dalam file wav yang akan di rekam
const int numCommunicationData = 8000; // jumlah total komunikasi data yang dikirimkan
const int numPartWavData = numCommunicationData / 4; 
byte header[headerSize];
char communicationData[numCommunicationData];
char partWavData[numPartWavData];
File file;
// Konfigurasi sistem ke server gcp
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool taskCompleted = false;
void gcsUploadCallback(UploadStatusInfo info);
// Fungsi untuk mengecek koneksi wifi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}
// fungsi untuk mengecek sistem memori internak
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("LittleFS mounted successfully");
  }
}
void setup()
{
  Serial.begin(115200);
  pinMode(LED_TURN_ON, OUTPUT);
  pinMode(LED_RECORDING_START, OUTPUT);
  pinMode(LED_RECORDING_END, OUTPUT);
  pinMode(BUTTON_START_RECORDING, INPUT);
  digitalWrite(LED_TURN_ON, HIGH);
  digitalWrite(LED_RECORDING_START, LOW);
  digitalWrite(LED_RECORDING_END, HIGH);
  initWiFi();
  initLittleFS();
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  // Perulangan untuk menjalankan recording system
  while (1) {
    if (digitalRead(BUTTON_START_RECORDING) == HIGH) {
      Serial.println("RECORDING START");
      CreateWavHeader(header, waveDataSize); // sebelum memulai proses record kita harus mendefinisikan jumlah file dan headernya dari file wav
      LittleFS.remove(FILE_AUDIO_PATH); // Cleanning memori internal sebelum memasukan file audio baru
      file = LittleFS.open(FILE_AUDIO_PATH, FILE_WRITE); 
      if (!file) return; // Kondisi jika file gagal di buka maka system akan me-restart
      file.write(header, headerSize); // menentukan penulisan buffer di dalam file LittleFs
      I2S_Init(I2S_MODE, I2S_BITS_PER_SAMPLE_32BIT); // Mode penulisan buffer
      // Perulangan untuk penulisan buffer ke dalam file wav
      for (int j = 0; j < waveDataSize / numPartWavData; ++j) {
        digitalWrite(LED_TURN_ON, HIGH);
        digitalWrite(LED_RECORDING_START, HIGH);
        digitalWrite(LED_RECORDING_END, LOW);
        I2S_Read(communicationData, numCommunicationData);
        for (int i = 0; i < numCommunicationData / 8; ++i) {
          digitalWrite(LED_TURN_ON, HIGH);
          digitalWrite(LED_RECORDING_START, HIGH);
          digitalWrite(LED_RECORDING_END, LOW);
          partWavData[2 * i] = communicationData[8 * i + 2];
          partWavData[2 * i + 1] = communicationData[8 * i + 3];
        }
        file.write((const byte*)partWavData, numPartWavData);
      }
      file.close();
      digitalWrite(LED_RECORDING_START, LOW);
      digitalWrite(LED_RECORDING_END, HIGH);
      Serial.println("finish");
      break;
    } else {
      Serial.println("Silahkan tekan tombol strat recording");
      digitalWrite(LED_TURN_ON, HIGH);
      digitalWrite(LED_RECORDING_END, HIGH);
    }
  }
  digitalWrite(LED_TURN_ON, HIGH);
  digitalWrite(LED_RECORDING_END, HIGH);
  // konfigurasi API Google Cloud Platform
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
  config.service_account.data.project_id = FIREBASE_PROJECT_ID;
  config.service_account.data.private_key = PRIVATE_KEY;
  config.token_status_callback = tokenStatusCallback; 
  config.gcs.upload_buffer_size = 2048;
  Firebase.reconnectNetwork(true);
  Firebase.begin(&config, &auth);
}
// fungsi untuk melihat log pengiriman ke server
void gcsUploadCallback(UploadStatusInfo info)
{
  if (info.status == firebase_gcs_upload_status_init)
  {
    Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.fileSize, info.remoteFileName.c_str());
  }
  else if (info.status == firebase_gcs_upload_status_upload)
  {
    Serial.printf("Uploaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
  }
  else if (info.status == firebase_gcs_upload_status_complete)
  {
    Serial.println("Upload completed\n");
    FileMetaInfo meta = fbdo.metaData();
    Serial.printf("Name: %s\n", meta.name.c_str());
    Serial.printf("Bucket: %s\n", meta.bucket.c_str());
    Serial.printf("contentType: %s\n", meta.contentType.c_str());
    Serial.printf("Size: %d\n", meta.size);
    Serial.printf("Generation: %lu\n", meta.generation);
    Serial.printf("ETag: %s\n", meta.etag.c_str());
    Serial.printf("CRC32: %s\n", meta.crc32.c_str());
    Serial.printf("Tokens: %s\n", meta.downloadTokens.c_str());    
    Serial.printf("Download URL: %s\n", fbdo.downloadURL().c_str()); 
  }
  else if (info.status == firebase_gcs_upload_status_error)
  {
    Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
  }
}

void loop()
{
  // Logika untuk pengiriman ke server GCP, Pengiriman ini bersifat sekali, jika mau merekam lagi tombol restart harus di tekan.
  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;
    Serial.println("\nUpload file via Google Cloud Storage JSON API...\n");
    // Parameter - prameter untuk pengiriman ke GCP
    Firebase.GCStorage.upload(&fbdo, STORAGE_BUCKET_ID , FILE_AUDIO_PATH , mem_storage_type_flash , gcs_upload_type_resumable , BUCKET_AUDIO , "audio/wav" , nullptr , nullptr , nullptr , gcsUploadCallback);
  }
}

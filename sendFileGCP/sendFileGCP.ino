#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the SD card interfaces setting and mounting
#include <addons/SDHelper.h>
/* 1. Define the WiFi credentials */
#define WIFI_SSID "TechDrips"
#define WIFI_PASSWORD "TechDrips123456789"

/* 2. Define the Firebase storage bucket ID e.g bucket-name.appspot.com or Google Cloud Storage bucket name */
#define STORAGE_BUCKET_ID "web-xerpihan.appspot.com"

/* 3 The following Service Account credentials required for OAuth2.0 authen in Google Cloud Storage JSON API upload */
#define FIREBASE_PROJECT_ID "web-xerpihan"
#define FIREBASE_CLIENT_EMAIL "web-xerpihan@appspot.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCYvv6HXtjobj8O\n4d5fakiStDgwZ+AURtXP9K+fCeat1s349ZiRvU4XsHKBwleSuVWOOR2+3/7Br3W0\nwqoz52FZVbTdQwPCYbdPyDIjzF4nUGA6J6wQXxVnJsuKdNMAHC/6jn9xPuT9S6n1\nETWFzUAN1NZawyT4vHG32/Jo/TzZtNzuFVempt7xak11g5QMKYXKRDnh82yRKImt\np/3Ec8yZ6FNDEFv0IByHWfOE3R9gjvoyR1PDuYX9VjGg/wEkOxD3M7+sTx6OoF3y\ngknynt0F2EmXd5qCUfuOW4SYqk0gVOz6/dyfD2t4ZDUWcyOWTa4hQAW4WgF8W78j\nAi+eVOWzAgMBAAECggEAJT2n3ARteRnsCOpb58KHiDElY/Pnh7oPhkC/ktR8qBSv\nqkvi02GmyRLNkEJ8THsgV2kojfGZ8MohNh2iFr6KlxfU3X9pyro9UdfbFaxNpmMr\n2vHyOxmbo8crKAo8qZG1xu54NvmhJfqNKQF1nbZBsD2ct4tNl9GGHlAjZaKcc+2K\nsGUdLRQlWGFwRwYFbsvKT5GPFROip8BEvWuLRbeBbBOpg6292ZfjxDEFVw2ZPTHf\nhf4Ff8uLdb6vQEujAiVLfiZCSI1lwNsoMMqZ+RvkxcZM1cI/0R9tIxz3PSfDaiiM\nfUsV0juBd/840LO7qRSslNcHVLyXQ1pgtdCpD4BKtQKBgQDTGihEHOeM9Lg9f5oz\n5l6PD4fPE5pRV328+aDh0pwx5JaWouF/YfM/13qJLvT0GXiBQ27cZtyqJek+vK4F\nkFyJ9c4Xjbkvoy9uD5tNK0u1sB4YHNoc4PWBQVoNe78Z3WpRBlAFTYEmBxfX+i/g\nGf2mh4Lr8fp1z8WIecPF+/heLwKBgQC5O4hOymWmI8aJCz1QJSHjCo6ubDiBv7Rb\nSrSoxxu70ME6q20Sn00RTOczim850g6FsDNfX5oJcTCcncJi4UGiYxu+Em4x6uwB\ndGvRUymxG/wMSsP7M3tQpOoJQmi6UO2H4BUKvA26MQA3ZZ3V5LABChymfNwUuSU4\nfhNrvQwzvQKBgQCBGi4l+cdiXQeQFMmpk+mgdE8KRWLK/EbZm5DYioUbvkCaP2LJ\nFI4/LPizjRQhdLJN23+hKwcox6atx5ZlsLPna3xuyy3GBHatSL4ho6jZpKvC82Iw\n5yM9uUnplRR6LP8oLkQh+hRRvWTHoSpGU57ciWN4CLSz+C1GGbGxEzgB4QKBgDyu\nsiMNz2s0QTBYTEfbQlS8+q1Nqt1mzlEyEilQwVcwNn1f96WoJnlKdlu+OF364OMP\nLGVMToxVVpWYYTw/i18FlOsZ4pvb2GFJoKC82+W3w0TLoZTD8twiisFpziFrrcVb\ntckEfoLG1slPMvwFpVtR0r0AHqFzXiw5sWcUUdEJAoGAI651AYNW5SMLIn/lgEsu\nqyRh5RwLC2bLxoRj4wRWDDW5lnkN9P9Fo/F21c3e2NKWRpFs8iAxXwkBKN1R5wGC\nFJwdWfhQEhTDdjRCGFBWeJ8uRZ3Ckvugec9c/wNtUOVxBU8qXLS8N/AMbBxkqDar\nu5oH9OyKochhO9Yhd8lCzDs=\n-----END PRIVATE KEY-----\n";
#define FILE_AUDIO_PATH "/BISMILLAH_GCP.wav"
#define BUCKET_AUDIO "/AUDIO_XERPIHAN/BISMILLAH_GCP.wav"
// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

// The Google Cloud Storage upload callback function
void gcsUploadCallback(UploadStatusInfo info);


void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

 
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
  initWiFi();
  initLittleFS();
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the Service Account credentials for OAuth2.0 authen */
  config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
  config.service_account.data.project_id = FIREBASE_PROJECT_ID;
  config.service_account.data.private_key = PRIVATE_KEY;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  /* Assign upload buffer size in byte */
  // Data to be uploaded will send as multiple chunks with this size, to compromise between speed and memory used for buffering.
  // The memory from external SRAM/PSRAM will not use in the TCP client internal tx buffer.
  config.gcs.upload_buffer_size = 2048;

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);
}

// The Google Cloud Storage upload callback function
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
    Serial.printf("Tokens: %s\n", meta.downloadTokens.c_str());      // only gcs_upload_type_multipart and gcs_upload_type_resumable upload types.
    Serial.printf("Download URL: %s\n", fbdo.downloadURL().c_str()); // only gcs_upload_type_multipart and gcs_upload_type_resumable upload types.
  }
  else if (info.status == firebase_gcs_upload_status_error)
  {
    Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
  }
}

void loop()
{
  File file = LittleFS.open(FILE_AUDIO_PATH);
  file.close();

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && !taskCompleted)
  {
    taskCompleted = true;

    Serial.println("\nUpload file via Google Cloud Storage JSON API...\n");

    /**
       The following function uses Google Cloud Storage JSON API to upload the file (object).
       The Google Cloud Storage functions required OAuth2.0 authentication.
       The upload types of methods can be selectable.

       The gcs_upload_type_simple upload type is used for small file upload in a single request without metadata.
       gcs_upload_type_multipart upload type is for small file upload in a single reques with metadata.
       gcs_upload_type_resumable upload type is for medium or large file (larger than or equal to 256 256 KiB) upload with metadata and can be resumable.

       The upload with metadata supports allows the library to add the metadata internally for Firebase to request the download access token in Firebase Storage bucket.
       User also can add custom metadata for the uploading file (object).
    */

    // For query parameters description of UploadOptions, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-parameters
    // For request payload properties description of Requestproperties, see https://cloud.google.com/storage/docs/json_api/v1/objects/insert#optional-properties
    // The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    Firebase.GCStorage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase or Google Cloud Storage bucket id */, FILE_AUDIO_PATH /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, gcs_upload_type_resumable /* upload type */, BUCKET_AUDIO /* path of remote file stored in the bucket */, "audio/wav" /* mime type */, nullptr /* UploadOptions data */, nullptr /* Requestproperties data */, nullptr /* UploadStatusInfo data to get the status */, gcsUploadCallback /* callback function */);
  }
}

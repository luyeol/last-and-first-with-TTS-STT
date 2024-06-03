#pragma once

/*
/////////////////실행 방법/////////////////
////터미널 창에 아래와 같이 순서대로 입력////
g++ -o stt STT.cpp -lportaudio -lsndfile -ljsoncpp -lcurl
./stt
//////////////////////////////////////////
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <json/json.h>
#include <locale>
#include <codecvt>
#include <cstdlib>

using namespace std;

const string CLIENT_ID = "rr7p6k5x1g";
const string CLIENT_SECRET = "pJHfODKVDenEtTFlTtOCGPV1sD9Jl2O6fYXYPI7K";
const string LANG = "Kor"; // Language code

size_t WriteCallback_STT(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

wstring STT() {
    system("./record");

    // this_thread::sleep_for(chrono::seconds(10));   //딜레이 10초
    locale::global(locale(""));

    string audioFilePath = "recorded.wav";
    ifstream audioFile(audioFilePath, ios::binary);

    if (!audioFile) {
        cerr << "Error opening file: " << audioFilePath << endl;
    }

    stringstream buffer;
    buffer << audioFile.rdbuf();
    string audioData = buffer.str();

    string apiURL = "https://naveropenapi.apigw.ntruss.com/recog/v1/stt?lang=" + LANG;

    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    wstring utf16Text = L"";

    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("X-NCP-APIGW-API-KEY-ID: " + CLIENT_ID).c_str());
        headers = curl_slist_append(headers, ("X-NCP-APIGW-API-KEY: " + CLIENT_SECRET).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/octet-stream");

        curl_easy_setopt(curl, CURLOPT_URL, apiURL.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, audioData.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, audioData.size());

        string responseString;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_STT);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        else {
            Json::Reader reader;
            Json::Value jsonResponse;
            if (reader.parse(responseString, jsonResponse)) {
                wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
                utf16Text = converter.from_bytes(jsonResponse["text"].asString());
                // wcout << L"Transcribed Text: " << utf16Text << endl;
                return utf16Text;
            }
            else {
                cerr << "Failed to parse JSON response." << endl;
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
    return utf16Text;
}
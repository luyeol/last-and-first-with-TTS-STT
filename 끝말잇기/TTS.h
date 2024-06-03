#pragma once

#include <iostream>
#include <cstdlib>
#include <codecvt>
#include <locale>
#include <string>

using namespace std;

int TTS(wstring text) {
    // wide 문자열을 시스템 명령어에 포함하여 eSpeak 실행
    wstring espeak_command = L"espeak \"" + text + L"\"";
    int result = system(wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(espeak_command).c_str());

    if (result == -1) {
        cerr << "Failed to execute eSpeak" << endl;
        return 1;
    }

    return 0;
}

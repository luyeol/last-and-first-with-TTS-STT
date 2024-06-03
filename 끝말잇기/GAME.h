#pragma once

#include <iostream>
#include <vector>
#include "api.h"
#include "TTS.h"
#include "STT.h"

using namespace std;

// 두음법칙 적용 함수
wchar_t apply_dooeum(wchar_t first_char, wchar_t second_char) {
    if (first_char == L'ㄴ' && (second_char == L'ㅑ' || second_char == L'ㅕ' || second_char == L'ㅛ' || second_char == L'ㅠ' || second_char == L'ㅣ')) {
        return L'ㅇ';
    } else if (first_char == L'ㄹ') {
        return L'ㅇ';
    }
    return first_char;
}

wstring get_first_char(const wstring& word) {
    if (word.empty()) return L"";
    return apply_dooeum(word[0], word[1]);
}

wstring get_last_char(const wstring& word) {
    if (word.empty()) return L"";
    return wstring(1, word.back());
}

bool is_valid_word(const wstring& prev_word, const wstring& current_word) {
    return get_last_char(prev_word) == get_first_char(current_word);
}

void Game_start()
{
    locale::global(locale(""));
    
    // 이미 있는 단어 알기위해 단어목록 저장
    vector<wstring> history;
    bool playing = true;

    locale originalLocale = locale::global(locale(""));

    wstring answord = L"";
    wstring sword = L"";
    wstring query = L"";
    wstring squery = L"";

    cout << "졸음 방지를 위해 끝말잇기를 시작합니다.\n"
        "가장 처음 단어를 제시하면 끝말잇기가 시작됩니다\n"
        "'그만'을 입력하면 게임이 종료되며, '다시'를 입력하여 게임을 다시 시작할 수 있습니다.\n";
    TTS(L"졸음 방지를 위해 끝말잇기를 시작합니다.\n"
        "가장 처음 단어를 제시하면 끝말잇기가 시작됩니다\n"
        "그만을 입력하면 게임이 종료되며, 다시를 입력하여 게임을 다시 시작할 수 있습니다.\n");

    while (playing) 
    {
        bool wordOK = false;    //단어 입력 완료 여부

        while (!wordOK) 
        {
            wordOK = true;
            // wcin >> query;
            query = STT();
            wcout << query << endl;

            if (query == L"그만") 
            {
                playing = false;
                cout << "컴퓨터의 승리!\n";
                TTS(L"컴퓨터의 승리");
                break;
            }
            else if (query == L"다시") 
            {
                history.clear();
                answord = L"";
                cout << "게임을 다시 시작합니다.\n";
                TTS(L"게임을 다시 시작합니다");
                wordOK = false;
            }
            else 
            {
                if (query.empty()) {
                    wordOK = false;
                    if (history.empty()) {
                        cout << "단어를 말하십시오.\n";
                        TTS(L"단어를 말하십시오");
                    }
                }
                else
                {
                    squery = query[0];
                    if (query.size() == 1)
                    {
                        wordOK = false;
                        cout << "적어도 두 글자가 되어야 합니다.\n";
                        TTS(L"적어도 두 글자가 되어야 합니다");
                    }
                    if (find(history.begin(), history.end(), query) != history.end()) 
                    {
                        wordOK = false;
                        cout << "이미 입력한 단어입니다\n";
                        TTS(L"이미 입력한 단어입니다");
                    }
                    if (sword.compare(squery) && sword != L"")
                    {
                        wordOK = false;
                        cout << utf8_encode(sword) << "으로 시작하는 단어를 입력해 주십시오.\n";
                        TTS(sword + L"으로 시작하는 단어를 입력해 주십시오");
                    }
                    if (wordOK) //단어 유효성 여부 확인
                    {
                        wstring ans = checkexists(query);
                        if (ans == L"")
                        {
                            wordOK = false;
                            cout << "유효한 단어를 입력해 주십시오." << endl;
                            TTS(L"유효한 단어를 입력해 주십시오");
                        }
                    }
                }
            }
        }
        history.push_back(query);

        if (playing)
        {
            wchar_t start_c = query[query.length() - 1]; 
            wstring start(1, start_c);

            answord = findword(start + L"*", history);
            if (answord == L"")
            {
                cout << "당신의 승리" << endl;
                TTS(L"당신의 승리");
                break;
            }
            else
            {
                history.push_back(answord);
                sword = answord[answord.length() - 1];
                cout << utf8_encode(answord) << endl;
                TTS(answord);
            }
        }
    }
}
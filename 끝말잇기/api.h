#pragma once
#include <iostream>
#include <vector>
#include <regex>
#include <iomanip>
#include <codecvt>
#include <sstream>
#include <curl/curl.h>
#include <cstdlib> // for rand() and srand()
#include <ctime>   // for time()

using namespace std;

//vector<string> history;

// url 인코딩 함수
string url_encode(const string& value)
{
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (char c : value) {
        if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        }
        else {
            escaped << '%' << uppercase << setw(2) << int(static_cast<unsigned char>(c));
        }
    }

    return escaped.str();
}

// UTF-8 인코딩 함수
string utf8_encode(const wstring& wstr)
{
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wstr);
}

// 디코딩 함수
wstring decodeFromUTF8(const std::string& input) {
    wstring_convert<codecvt_utf8_utf16<wchar_t>> conv;
    return conv.from_bytes(input);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// XML의 문자열 하나 파싱을 위한 함수
string midReturn(const string& xml, const string& tag)
{
    regex reg("<" + tag + ">(.*?)</" + tag + ">");
    smatch match;
    if (regex_search(xml, match, reg)) {
        return match[1];
    }
    return "";
}

// XML의 문자열들을 파싱하는 함수
vector<string> midReturn_all(const string& xml, const string& tag)
{
    vector<string> matches;

    regex reg("<" + tag + ">(.*?)</" + tag + ">");
    
    sregex_iterator begin(xml.begin(), xml.end(), reg);
    sregex_iterator end;

    for (sregex_iterator i = begin; i != end; ++i) 
    {
        smatch match = *i;
        matches.push_back(match[1].str());  // match[1]은 첫 번째 캡처 그룹
    }

    return matches;
}

// 콜백 함수: 데이터를 받아올 때 호출됨
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) 
{
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}


// XML data를 반환하는 함수
string httpGET(string url)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    string data;
    if (curl) 
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

        CURLcode res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
    }
    else
        cout << "init failed" << endl;
    return data;
}

wstring findword(wstring query, vector<wstring> history)
{
    //url encode
    string utf8_encoded = utf8_encode(query);
    string utf8_url_encoded = url_encode(utf8_encoded);
    string url = "https://krdict.korean.go.kr/api/search?key=482E5BFE172C03AD1C4F03C4D2277784&part=word&pos=1&q=" + utf8_url_encoded;

    string response = httpGET(url);
    // 공백을 제거할 정규 표현식
    regex reg("\\s+");

    // XML에서 공백 제거
    response = regex_replace(response, reg, "");

    vector<wstring> ans;

    // 단어 목록을 불러오기
    vector<string> words = midReturn_all(response, "item");
    for (string w : words) {
        // 이미 쓴 단어가 아닐때
        if (find(history.begin(), history.end(), decodeFromUTF8(w)) == history.end())
        {
            // 한글자가 아니고 품사가 명사일때
            string word = midReturn(w, "word");
            wstring pos = decodeFromUTF8(midReturn(w, "pos"));
            //if (word.length() > 1 && pos == L"����" && find(history.begin(), history.end(), word) == history.end()) 
            if (decodeFromUTF8(word).length() > 1 && pos.compare(L"명사") == 0 && find(history.begin(), history.end(), decodeFromUTF8(word)) == history.end())
            {
                ans.push_back(decodeFromUTF8(word));
            }
        }
    }
    if (!ans.empty()) {
        srand(static_cast<unsigned int>(time(nullptr)));
        return ans[rand() % ans.size()];
    }
    else 
    {
        return L"";
    }
}

wstring checkexists(wstring query)
{
    //url encode
    string utf8_encoded = utf8_encode(query);
    string utf8_url_encoded = url_encode(utf8_encoded);
    string url = "https://krdict.korean.go.kr/api/search?key=482E5BFE172C03AD1C4F03C4D2277784&part=word&sort=popular&num=100&pos=1&q=" + utf8_url_encoded;
    string response = httpGET(url);

    wstring ans = L"";
    // 공백을 제거할 정규 표현식
    regex reg("\\s+");

    // XML에서 공백 제거
    response = regex_replace(response, reg, "");

    // 단어 목록을 불러오기
    vector<string> words = midReturn_all(response, "item");
    for (string w : words) 
    {
        string word = midReturn(w, "word");
        //string pos = midReturn(w, "pos");
        wstring pos = decodeFromUTF8(midReturn(w, "pos"));
        // 한글자가 아니고 품사가 명사일때
        if (decodeFromUTF8(word).length() > 1 && pos == L"명사" && word == utf8_encode(query))
        {
            ans = decodeFromUTF8(word);
        }
    }
    return ans;
}
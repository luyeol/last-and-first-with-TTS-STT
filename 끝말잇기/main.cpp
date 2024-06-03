/*
/////////////////실행 방법/////////////////
////터미널 창에 아래와 같이 순서대로 입력////
g++ -o game main.cpp -lespeak -lcurl -lportaudio -lsndfile -ljsoncpp
./game
//////////////////////////////////////////
*/

#include "GAME.h"

int main()
{
    Game_start();
}
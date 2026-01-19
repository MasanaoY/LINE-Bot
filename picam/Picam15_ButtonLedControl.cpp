
#include <iostream>
#include <pigpio.h> // pigpioライブラリのヘッダー
#include <unistd.h> // usleep()のために必要

// GPIOピン番号の定義（BCM番号）
#define LED_BLUE  17
#define LED_RED   27
#define BTN_GREEN 23
#define BTN_RED   24

using namespace std;

int main(){
    
    // pigpioライブラリの初期化
    // gpioInitialise()は正常に初期化すれば0以上を、失敗すれば0未満を返す
    if (gpioInitialise() < 0) {
        cerr << "pigpio の初期化に失敗しました。" << endl;
        return 1;
    }

    // 出力ピンの設定
    gpioSetMode(LED_BLUE, PI_OUTPUT);
    gpioSetMode(LED_RED, PI_OUTPUT);
    
    // 入力ピンの設定
    gpioSetMode(BTN_GREEN, PI_INPUT);
    gpioSetMode(BTN_RED, PI_INPUT);

    cout << "GPIO制御を開始しました。終了は Ctrl + C" << endl;

    // メインループ
    while (true) {
        
        // ボタンの読み取りとLEDの制御
        
        // 緑ボタンが押されているか確認
        if (gpioRead(BTN_GREEN) == PI_LOW) {

            // 緑ボタンが押されたら青LEDを点灯
            gpioWrite(LED_BLUE, PI_HIGH);

        } else {

            // 押されていなければ消灯
            gpioWrite(LED_BLUE, PI_LOW);
        }
        
        // 赤ボタンが押されているか確認
        if (gpioRead(BTN_RED) == PI_LOW) {

            // 赤ボタンが押されたら赤LEDを点滅
            gpioWrite(LED_RED, PI_HIGH);
            usleep(250000); // 250ms待機
            gpioWrite(LED_RED, PI_LOW);
            usleep(250000); // 250ms待機

        } else {

            // 押されていなければ消灯(チカチカしない)
            gpioWrite(LED_RED, PI_LOW);
            usleep(10000); // 少し待機してCPUの負荷を軽減
        }
    }

    // プログラム終了時のクリーンアップ
    gpioTerminate();
    return 0;
}


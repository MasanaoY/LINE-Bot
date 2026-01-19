
#define CPPHTTPLIB_OPENSSL_SUPPORT // APIへの返信時にHTTPSクライアントとして動作するために必要

#include <iostream>
#include <string>
#include <thread>             // スレッドを使うために必要
#include <chrono>             // 処理の待ち時間（sleep）のために使用
#include "httplib.h"        // Webサーバーとクライアント機能
#include <opencv2/opencv.hpp> // OpenCV (画像処理)


// Webサーバーを起動し、画像リクエストを処理する関数
void start_web_server(int port) {
    httplib::Server svr;

    // --- 画像配信のエンドポイント ---
    // ngrokのURL + /image.jpg にアクセスが来たらこの処理が実行される
    svr.Get("/image.jpg", [](const httplib::Request& req, httplib::Response& res) {
        std::string image_path = "/home/pi/image.jpg"; // ラズパイの画像パス
        cv::Mat img = cv::imread(image_path, cv::IMREAD_COLOR); // 画像をカラーとして読み込む

        // 画像がなければ404not foundを返す
        if (img.empty()) {
            res.set_content("Image not found", "text/plain");
            res.status = 404;
            return;
        }

        // JPEG形式にエンコード
        std::vector<uchar> buf;
        std::vector<int> params;
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(95);

        // エンコード失敗時は500 server errorを返す
        if (!cv::imencode(".jpg", img, buf, params)) {
             res.set_content("Image encoding failed", "text/plain");
             res.status = 500;
             return;
        }

        // レスポンスとして画像バイナリを返す
        res.set_content((const char*)buf.data(), buf.size(), "image/jpeg");
        res.status = 200;
        std::cout << "[サーバー] 画像 '/image.jpg' が正常に送信されました。" << std::endl;
    });

    std::cout << "[サーバー] ポートで待機中 " << port << "..." << std::endl;
    // listen() はブロッキング関数（処理がここで止まって待ち受ける）
    svr.listen("0.0.0.0", port);
    // listen() が返ってくるのは通常、サーバー停止時のみ
}



// LINE APIに画像メッセージを送信する関数
void send_line_message() {
    // Webサーバーの起動を少し待つ（起動が完了するのを保証するため）
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // --- 設定値 ---
    const std::string NGROK_URL_BASE = "ここを変更する"; // ngrokで取得したURL
    const std::string USER_ID = "ここを変更する"; // ユーザーID
    const std::string ACCESS_TOKEN = "ここを変更する"; // チャネルアクセストークン

    // httplibのクライアントを初期化 (HTTPS通信)
    httplib::Client cli("https://api.line.me");

    // HTTPSリクエストに必要なヘッダー
    httplib::Headers headers = {
        {"Authorization", "Bearer " + ACCESS_TOKEN}
    };

    // 画像メッセージのJSONペイロードを作成
    std::string originalUrl = "https://" + NGROK_URL_BASE + "/image.jpg";
    std::string previewUrl = originalUrl; // 簡略化のため同じURL

    std::string json_payload = "{\"to\":\"" + USER_ID + "\",\"messages\":[{"
                                "\"type\":\"image\","
                                "\"originalContentUrl\":\"" + originalUrl + "\","
                                "\"previewImageUrl\":\"" + previewUrl + "\""
                                "}]}";

    std::cout << "[クライアント] LINE API に画像メッセージを送信しています..." << std::endl;

    // LINE APIへのPOSTリクエスト実行
    auto res = cli.Post("/v2/bot/message/push", headers, json_payload, "application/json");

    // 結果の確認
    if (res && res->status == 200) {
        std::cout << "[クライアント] LINEメッセージの送信に成功しました！(ステータス: 200)" << std::endl;
    } else {
        std::cout << "[クライアント] LINEメッセージの送信に失敗しました。ステータス: " << (res ? std::to_string(res->status) : "Error") << std::endl;
        if (res) {
            std::cout << "レスポンス内容: " << res->body << std::endl;
        }
    }
}


int main() {
    const int SERVER_PORT = 8080;

    // 1. Webサーバーを別スレッドで起動
    // std::thread::thread(関数名, 引数...) で新しいスレッドが生成され、関数が実行される
    std::thread server_thread(start_web_server, SERVER_PORT);

    // 2. メインスレッドでLINE APIへの送信処理を実行
    // ここで送信処理を実行することで、Webサーバーの待ち受けと同時に通信を開始できる
    send_line_message();

    // サーバーが止まるのを待つ（サーバーはlistenで待機し続けるため、通常は到達しない）
    server_thread.join();

    return 0;
}



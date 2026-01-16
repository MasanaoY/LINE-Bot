#define CPPHTTPLIB_OPENSSL_SUPPORT //cpp-httplibのHTTPS機能を有効にする

#include <iostream>
#include <string>
#include <fstream>
#include "httplib.h"

const std::string CHANNEL_ACCESS_TOKEN = "ここを変更する"; // チャネルアクセストークン 
const std::string USER_ID_TO_SEND = "ここを変更する"; // 送信する先のLINEユーザーID

const std::string LINE_API_BASE_URL = "https://api.line.me";
const std::string LINE_PUSH_MESSAGE_ENDPOINT = "/v2/bot/message/push";


// LINE APIにHTTP POSTリクエストを送信する汎用関数
bool sendLineApiRequest(const std::string& endpoint, const std::string& body) {
    httplib::Client cli(LINE_API_BASE_URL.c_str());
    cli.set_connection_timeout(std::chrono::seconds(5)); // 接続タイムアウト (5秒)
    cli.set_read_timeout(std::chrono::seconds(10));      // 読み込みタイムアウト (10秒)

    httplib::Headers headers = {
        {"Authorization", "Bearer " + CHANNEL_ACCESS_TOKEN}
    };

    // LINE APIへPOSTリクエストを送信
    auto res = cli.Post(endpoint.c_str(), headers, body, "application/json");

    // レスポンスの確認
    if (res) {
        if (res->status == 200) {
            std::cout << "LINE APIへの送信に成功しました！ レスポンス: " << res->body << std::endl;
            return true;
        } else {
            std::cerr << "LINE APIエラーが発生しました。ステータスコード: " << res->status << ", レスポンスボディ: " << res->body << std::endl;
            return false;
        }
    } else {
        std::cerr << "LINE APIへの接続エラーが発生しました: " << httplib::to_string(res.error()) << std::endl;
        return false;
    }
}

// テキストメッセージを送信する関数
bool sendTextMessage(const std::string& to_user_id, const std::string& text) {
    
    // JSON形式のメッセージペイロード(送りたい文字列本体)を作成、生文字列リテラル「R"()"」でエスケープ文字が不要になる
    std::string body = R"({
        "to": ")" + to_user_id + R"(",
        "messages": [
            {
                "type": "text",
                "text": ")" + text + R"("
            }
        ]
    })";
    
    // LINE APIのプッシュメッセージエンドポイントにリクエストを送信
    return sendLineApiRequest(LINE_PUSH_MESSAGE_ENDPOINT, body);
}

// メイン関数
int main() {
    std::cout << "LINEへメッセージを送信します..." << std::endl;

    // テキストメッセージを送信
   std::string message_to_send = "おはようございます。";
    
    if (sendTextMessage(USER_ID_TO_SEND, message_to_send)) {
        std::cout << "メッセージの送信が完了しました。" << std::endl;
    } else {
        std::cerr << "メッセージの送信に失敗しました。" << std::endl;
    }

    return 0;
}




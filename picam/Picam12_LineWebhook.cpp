
#define CPPHTTPLIB_OPENSSL_SUPPORT // APIへの返信時にHTTPSクライアントとして動作するために必要

#include "httplib.h"
#include <iostream>
#include <string>
#include "nlohmann/json.hpp" // nlohmann/jsonを使用

using namespace httplib;
using json = nlohmann::json;

const std::string CHANNEL_ACCESS_TOKEN = "ここを変更する"; // チャンネルアクセストークン 
const int WEBHOOK_PORT = 8080;

// (1) LINE Messaging APIのReply Messageを送信する関数 (クライアント機能)
void send_reply_message(const std::string& reply_token, const std::string& text) {
    
    // HTTPSクライアントとしてLINE APIのホストに接続
    Client cli("https://api.line.me"); 
    
    // 返信用JSONデータの作成
    json reply_json = {
        {"replyToken", reply_token},
        {"messages", {
            {
                {"type", "text"},
                {"text", text}
            }
        }}
    };
    std::string post_data = reply_json.dump();

    // 必須ヘッダー: 認証トークンとContent-Type
    Headers headers = {
        {"Authorization", "Bearer " + CHANNEL_ACCESS_TOKEN}
    };
    
    // Reply APIを呼び出して返信を送信
    auto res = cli.Post("/v2/bot/message/reply", headers, post_data, "application/json");

    if (res && res->status == 200) {
        std::cout << "  > LINEへ返信しました: " << text << std::endl;
    } else {
        std::cerr << "  > 返信に失敗。HTTP ステータス: " << (res ? std::to_string(res->status) : "接続エラー") << std::endl;
        if (res) std::cerr << "  > レスポンス内容: " << res->body << std::endl;
    }
}

// (2) Webhookリクエストを処理するハンドラ関数 (サーバー機能)
void handle_webhook(const Request& req, Response& res) {
    // ----------------------------------------------------
    // 1. リクエストボディの解析
    // ----------------------------------------------------
    json req_json;
    try {
        req_json = json::parse(req.body);
    } catch (json::parse_error& e) {
        std::cerr << "JSON解析エラー: " << e.what() << std::endl;
        res.set_content("Bad Request", "text/plain");
        res.status = 400; 
        return;
    }

    std::cout << "Webhookリクエストを受信しました。処理を開始します。" << std::endl;

    // ----------------------------------------------------
    // 2. メッセージイベントの抽出と処理
    // ----------------------------------------------------
    // リクエストに含まれるすべてのイベントをループ
    for (const auto& event : req_json["events"]) {
        // イベントタイプが「メッセージ」かつ、メッセージタイプが「テキスト」であるか確認
        if (event.contains("type") && event["type"] == "message" && 
            event.contains("message") && event["message"].contains("type") && event["message"]["type"] == "text") {
            
            std::string user_message = event["message"]["text"];
            std::string reply_token  = event["replyToken"];

            std::cout << "  > ユーザーメッセージ: [" << user_message << "]" << std::endl;

            // ------------------------------------------------
            // 3. 応答ロジックの実装
            // ------------------------------------------------
            if (user_message == "おはよう") {
                send_reply_message(reply_token, "おはようございます");
            } else if (user_message == "こんにちは") {
                send_reply_message(reply_token, "こんにちは！");
            } else {
                // その他のメッセージには何も返信しない、またはデフォルトの応答
                std::cout << "  > 定義されていないメッセージのため、返信しません。" << std::endl;
            }
        }
    }

    // LINEプラットフォームに対して、リクエストを正常に受け取ったことを必ず伝える (ステータス200)
    res.set_content("OK", "text/plain");
    res.status = 200;
}

// (3) メイン関数
int main() {
    Server svr;

    // Webhookの受付パスとハンドラを設定
    svr.Post("/webhook", [](const Request& req, Response& res) {
        handle_webhook(req, res);
    });

    std::cout << "===================================================" << std::endl;
    std::cout << "LINE Webhook サーバーをポート " << WEBHOOK_PORT << " で起動します。" << std::endl;
    std::cout << "ngrok で外部に公開し、LINE DevelopersにURLを設定してください。" << std::endl;
    std::cout << "===================================================" << std::endl;
    
    // サーバーを起動し、リクエストを待ち受け
    if (!svr.listen("0.0.0.0", WEBHOOK_PORT)) {
        std::cerr << "サーバーの起動に失敗しました。ポートが使用中かもしれません。" << std::endl;
        return 1;
    }

    return 0;
}



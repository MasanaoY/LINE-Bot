#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {

    // 1. 顔検出器の初期化
    CascadeClassifier face_detector;
    string cascade_path = "haarcascade_frontalface_default.xml"; // ダウンロードしたXMLファイルのパスを指定。実行ファイルと同じディレクトリに置くか、絶対パスを指定してください。
    
    if (!face_detector.load(cascade_path)) {
        cerr << "エラー: 顔カスケードファイルを読み込めませんでした: " << cascade_path << endl;
        cerr << "'haarcascade_frontalface_default.xml' が実行可能ファイルと同じディレクトリにあること、または絶対パスがパスが合っていることを確認してください。" << endl;
        return -1;
    }
    cout << "顔カスケードファイルが正常に読み込まれました。" << endl;

    // 2. VideoCaptureオブジェクトの初期化
    VideoCapture cap(0); 

    if (!cap.isOpened()) {
        cerr << "エラー: カメラを開けませんでした。" << endl;
        return -1;
    }
    
    // 3. 動画設定の取得 (解像度、FPS)
    int frame_width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    Size frame_size(frame_width, frame_height);
    double fps = 30.0; // 録画FPS

    
    // 4. VideoWriterオブジェクトの初期化 (録画ファイル設定)
    VideoWriter writer("faces.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, frame_size);

    if (!writer.isOpened()) {
        cerr << "エラー: Video Writerを開けませんでした。" << endl;
        return -1;
    }
    
    cout << "Video Writerを初期化しました。顔検出機能を使って10秒間録画を開始します..." << endl;

    Mat frame;
    Mat gray_frame; // 顔検出はグレースケール画像で行うため
    vector<Rect> faces; // 検出された顔の矩形を格納するベクトル
    
    int frame_count = 0;
    const int max_frames = static_cast<int>(fps * 10); // 10秒間の録画

   
    // 5. メインループ: フレーム取得 -> 検出 -> 描画 -> 録画
    while (frame_count < max_frames) {
        
        if (!cap.read(frame)) {
            cerr << "警告: ビデオ ストリームからフレームを読み取ることができません。終了します。" << endl;
            break;
        }

        // --- 顔検出処理 ---
        // BGRフレームをグレースケールに変換 (Haar Cascadeはグレースケールで動作)
        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
        // 顔検出の実行
        face_detector.detectMultiScale(gray_frame, faces, 1.1, 7, 0, Size(60, 60)); 
        // パラメータ:ここで検出精度をざっくり調整する
        //  1.1: スケールファクター。画像をどれだけ縮小して検出を行うか (1.05～1.4程度)小さいほど精度が高いが処理速度が長くなる
        //  5: minNeighbors。矩形が何回検出されたら顔とみなすか (3～6程度)高いほど誤検出は減るが、検出漏れが増える可能性がある。
        //  0: flags。古いOpenCVとの互換性用
        //  Size(30, 30): minSize。検出する最小の顔サイズ (小さすぎると誤検出が増える)大きくすると小さすぎるノイズを顔と検出する誤検知が防げる。
      

        // 検出された顔に緑色の枠を描画
        for (const auto& face : faces) {
            rectangle(frame, face, Scalar(0, 255, 0), 2); // 緑色の2ピクセル幅の枠
        }
        
        // 描画されたフレームをファイルに書き込む
        writer.write(frame);
        
        cout << "Recording Frame " << frame_count + 1 << "/" << max_frames 
             << " (顔を検出した数: " << faces.size() << ")" << endl;
        
        frame_count++;
    }

    // 6. ループ終了。オブジェクトの解放
    cap.release();
    writer.release(); 
    
    cout << "録画完了：ファイルはfaces.aviとして保存されました。" << endl;

    return 0;
}




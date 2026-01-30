#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono> // 時間計測用

using namespace std;
using namespace cv;
using namespace std::chrono;

// グローバルでVideoWriterを定義 (録画の開始/停止でオブジェクトを再生成するため)
VideoWriter writer; 

int main() {
    
    // 1. 初期設定と検出器のロード
    
    CascadeClassifier face_detector;
    string cascade_path = "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml"; 
    
    if (!face_detector.load(cascade_path)) {
        cerr << "エラー: 顔カスケードファイルを読み込めませんでした:" << cascade_path << endl;
        cerr << "'haarcascade_frontalface_default.xml' が実行可能ファイルと同じディレクトリにあること、または絶対パスがパスが合っていることを確認してください。" << endl;
        return -1;
    }
    
    // 2. VideoCaptureオブジェクトの初期化 
    string pipeline = "libcamerasrc ! video/x-raw, width=800, height=600, framerate=15/1 ! videoconvert ! videoscale ! appsink";
    VideoCapture cap(pipeline, CAP_GSTREAMER);

    if (!cap.isOpened()) {
        cerr << "エラー: カメラを開けませんでした。" << endl;
        return -1;
    }

    // 3. 動画設定の取得 (解像度、FPS）
    int frame_width = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    Size frame_size(frame_width, frame_height);
    double fps = 15.0; // 録画FPS
    
    // 4. 状態管理変数
    bool is_recording = false;
    // 最後に顔を検知した時刻 (初期値は現在時刻)
    auto last_detection_time = high_resolution_clock::now(); 
    // 録画を継続する時間（秒）
    const seconds RECORD_DURATION(5); 
    
    // 5. メインループ
    Mat frame;
    vector<Rect> current_faces;  
    vector<Rect> last_faces;     
    int frame_count = 0;
    const int detection_interval = 5; // 5フレームに一度だけ検出

    cout << "モニターモードを開始しています。顔検出を待機しています..." << endl;

    while (true) { // 無限ループで監視を続ける
        
        if (!cap.read(frame)) { break; }

        // --- 検出処理の間引き ---
        if (frame_count % detection_interval == 0) {
            
            // 解像度を半分に縮小
            Mat small_frame;
            resize(frame, small_frame, Size(), 0.5, 0.5); 
            
            Mat gray_frame;
            cvtColor(small_frame, gray_frame, COLOR_BGR2GRAY);
            
            // Haar Cascadeのパラメータ設定 
            face_detector.detectMultiScale(gray_frame, current_faces, 1.1, 7, 0, Size(30, 30));
            
            // 検出結果の座標を元画像サイズに戻す
            last_faces.clear();
            for (auto& face : current_faces) {
                face.x *= 2; face.y *= 2; 
                face.width *= 2; face.height *= 2;
                last_faces.push_back(face); 
            }
        } 
        
        //  録画ロジックの核となる部分 
        bool face_detected_this_frame = !last_faces.empty();
        
        // 1. 【検知があった場合】: タイマーをリセットし、録画を開始/継続
        if (face_detected_this_frame) {
            // 最後に検知した時刻を現在時刻に更新（タイマーリセット）
            last_detection_time = high_resolution_clock::now();
            
            if (!is_recording) {
                // 録画が始まっていなければ、新しく開始する
                string filename = "motion_" + to_string(std::time(nullptr)) + ".avi";
                writer.open(filename, VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, frame_size);
                
                if (writer.isOpened()) {
                    is_recording = true;
                    cout << "\n>>> [録画開始] 顔が検出されました。録画先: " << filename << endl;
                }
            }
        }
        
        // 2. 【検知がなかった場合】: タイマーをチェックし、録画を終了
        if (is_recording) {
            auto current_time = high_resolution_clock::now();
            auto time_elapsed = duration_cast<seconds>(current_time - last_detection_time);
            
            if (time_elapsed >= RECORD_DURATION) {
                // 5秒経過したら録画を終了
                writer.release();
                is_recording = false;
                cout << ">>> [記録停止] 前回の検出から5秒が経過しました。" << endl;
            }
        }
        

        // 描画は常に実行
        Scalar color = is_recording ? Scalar(0, 0, 255) : Scalar(0, 255, 0); // 録画中は赤、待機中は緑
        for (const auto& face : last_faces) {
            rectangle(frame, face, color, 2); 
        }
        
        // 録画中の場合、フレームをファイルに書き込む
        if (is_recording) {
            writer.write(frame);
        }
        
        frame_count++;
    }

    // 終了処理（無限ループを`ctr + c`で止めるため実行されません）
    if (writer.isOpened()) { writer.release(); }
    cap.release();
    cout << "プログラムは終了しました。" << endl;

    return 0;
}



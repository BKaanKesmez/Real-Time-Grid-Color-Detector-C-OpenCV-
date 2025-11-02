#include <iostream>     // std::cout, std::cerr
#include <vector>       // std::vector
#include <string>       // std::string
#include <deque>        // std::deque
#include <algorithm>    // std::sort, std::min_element, std::max_element, std::all_of
#include <opencv2/opencv.hpp> // OpenCV ana başlık dosyası

// --- YENİ (ADIM 4) EKLENEN KÜTÜPHANELER ---
#include <fstream>      // Dosyaya yazmak için (std::ofstream)
#include <iomanip>      // Zaman damgası ve metin formatlama için (std::put_time, std::setw)
#include <chrono>       // Zaman damgası için
#include <sstream>      // Zaman damgasını string'e çevirmek için

// =========================
// YARDIMCI 1: order_corners
// (Adım 2'den, değişmedi)
// =========================
std::vector<cv::Point2f> order_corners(const std::vector<cv::Point2f>& corners) {
    std::vector<cv::Point2f> ordered_corners(4);
    std::vector<float> sums, diffs;
    for (const auto& p : corners) {
        sums.push_back(p.x + p.y);
        diffs.push_back(p.y - p.x);
    }
    ordered_corners[0] = corners[std::min_element(sums.begin(), sums.end()) - sums.begin()];
    ordered_corners[2] = corners[std::max_element(sums.begin(), sums.end()) - sums.begin()];
    ordered_corners[1] = corners[std::min_element(diffs.begin(), diffs.end()) - diffs.begin()];
    ordered_corners[3] = corners[std::max_element(diffs.begin(), diffs.end()) - diffs.begin()];
    return ordered_corners;
}

// =========================
// YARDIMCI 2: find_grid_contour
// (Adım 2'den, değişmedi)
// =========================
std::vector<cv::Point2f> find_grid_contour(const cv::Mat& image, cv::Mat& out_debug_thr) {
    float min_area = 2000.0f;
    float approx_coef = 0.02f;
    cv::Mat small_img, gray, blur;
    float scale = 600.0f / image.cols;
    cv::resize(image, small_img, cv::Size(600, (int)(image.rows * scale)), 0, 0, cv::INTER_AREA);
    cv::cvtColor(small_img, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blur, cv::Size(5, 5), 0);
    cv::threshold(blur, out_debug_thr, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(out_debug_thr, out_debug_thr, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), 2);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(out_debug_thr, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return {};
    std::sort(contours.begin(), contours.end(), [](const auto& a, const auto& b) {
        return cv::contourArea(a) > cv::contourArea(b);
        });
    for (const auto& cnt : contours) {
        double area = cv::contourArea(cnt);
        if (area < (min_area * scale * scale)) continue;
        double peri = cv::arcLength(cnt, true);
        std::vector<cv::Point> approx;
        cv::approxPolyDP(cnt, approx, approx_coef * peri, true);
        if (approx.size() == 4 && cv::isContourConvex(approx)) {
            std::vector<cv::Point2f> original_corners;
            for (const auto& p : approx) {
                original_corners.push_back(cv::Point2f(p.x / scale, p.y / scale));
            }
            return order_corners(original_corners);
        }
    }
    return {};
}

// =========================
// YARDIMCI 3: fix_orientation
// (Adım 3'ten, en son denediğimiz haliyle)
// =========================
cv::Mat fix_orientation(const cv::Mat& img) {
    if (img.empty()) {
        return img;
    }
    cv::Mat rotated;
    // Görüntüyü 90 derece sağa (saat yönünde) döndür
    cv::rotate(img, rotated, cv::ROTATE_90_CLOCKWISE);
    return rotated;
}

// =========================
// YARDIMCI 4: classify_color_hsv
// (Adım 3'ten, değişmedi)
// =========================
std::string classify_color_hsv(const cv::Scalar& bgr_color) {
    cv::Mat bgr_mat(1, 1, CV_8UC3);
    bgr_mat.at<cv::Vec3b>(0, 0) = cv::Vec3b(bgr_color[0], bgr_color[1], bgr_color[2]);
    cv::Mat hsv_mat;
    cv::cvtColor(bgr_mat, hsv_mat, cv::COLOR_BGR2HSV);
    cv::Vec3b hsv_pixel = hsv_mat.at<cv::Vec3b>(0, 0);
    int h = hsv_pixel[0], s = hsv_pixel[1], v = hsv_pixel[2];

    if (s < 40) {
        if (v < 50) return "SIYAH";
        if (v > 200) return "BEYAZ";
        return "GRI";
    }
    if (h < 10 || h > 170) return "KIRMIZI";
    else if (h < 25) return "TURUNCU";
    else if (h < 35) return "SARI";
    else if (h < 85) return "YESIL";
    else if (h < 130) return "MAVI";
    else if (h < 160) return "MOR";
    else return "PEMBE";
}

// =========================
// YARDIMCI 5: analyze_grid
// (Adım 3'ten, değişmedi)
// =========================
cv::Mat analyze_grid(const cv::Mat& warped_image,
    std::vector<std::vector<std::string>>& grid_state,
    std::vector<std::vector<std::deque<std::string>>>& grid_temp_readings,
    int stability_frames)
{
    if (warped_image.empty()) return warped_image;

    int rows = grid_state.size();
    int cols = grid_state[0].size();
    int cell_h = warped_image.rows / rows;
    int cell_w = warped_image.cols / cols;
    cv::Mat vis_grid = warped_image.clone();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int x1 = j * cell_w, y1 = i * cell_h;
            int pad = (int)(cell_w * 0.1);
            cv::Rect roi_rect(x1 + pad, y1 + pad, cell_w - 2 * pad, cell_h - 2 * pad);
            roi_rect &= cv::Rect(0, 0, warped_image.cols, warped_image.rows);
            if (roi_rect.width <= 0 || roi_rect.height <= 0) continue;
            cv::Mat cell_roi = warped_image(roi_rect);
            if (cell_roi.empty()) continue;

            cv::Scalar avg_bgr = cv::mean(cell_roi);
            std::string current_color = classify_color_hsv(avg_bgr);
            grid_temp_readings[i][j].push_back(current_color);
            if (grid_temp_readings[i][j].size() > stability_frames) {
                grid_temp_readings[i][j].pop_front();
            }
            bool is_stable = false;
            if (grid_temp_readings[i][j].size() == stability_frames) {
                is_stable = std::all_of(grid_temp_readings[i][j].begin(), grid_temp_readings[i][j].end(),
                    [&](const std::string& c) { return c == current_color; });
            }
            if (is_stable && grid_state[i][j] != current_color) {
                grid_state[i][j] = current_color;
                std::cout << "Guncellendi: Hucure (" << (i + 1) << ", " << (j + 1) << ") -> " << current_color << std::endl;
            }
            std::string stable_color_name = grid_state[i][j];
            cv::Point text_pos(x1 + cell_w / 2 - 15, y1 + cell_h / 2 + 5);
            cv::putText(vis_grid, stable_color_name, text_pos, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 2, cv::LINE_AA);
            cv::putText(vis_grid, stable_color_name, text_pos, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
            cv::rectangle(vis_grid, cv::Point(x1, y1), cv::Point(x1 + cell_w, y1 + cell_h), cv::Scalar(0, 255, 0), 1);
        }
    }
    return vis_grid;
}

// =========================
// YENİ (ADIM 4): YARDIMCI 6: save_grid_to_file
// Grid durumunu metin dosyasına kaydeder
// =========================
void save_grid_to_file(const std::vector<std::vector<std::string>>& grid_state, const std::string& filename = "grid_sonuc.txt") {

    // 1. Dosya akışını aç
    std::ofstream file_stream(filename);
    if (!file_stream.is_open()) {
        std::cerr << "HATA: " << filename << " dosyası yazılamadı!" << std::endl;
        return;
    }

    // 2. Zaman damgasını al ve formatla
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    // std::put_time için C++'ın yerel zaman fonksiyonunu kullan
    std::tm buf;
    localtime_s(&buf, &in_time_t); // Windows için güvenli fonksiyon

    std::stringstream ss;
    ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");

    // 3. Dosyaya başlığı yaz
    file_stream << "Grid Renk Durumu - " << ss.str() << "\n";
    file_stream << std::string(40, '=') << "\n\n";

    // 4. Grid'i (tablo gibi) yazdır
    int rows = grid_state.size();
    if (rows == 0) return;
    int cols = grid_state[0].size();

    for (int i = 0; i < rows; ++i) {
        file_stream << "Satir " << std::setw(2) << (i + 1) << ":  | "; // Satır numarası
        for (int j = 0; j < cols; ++j) {
            // Python'daki "{color:<8} | " formatlaması
            file_stream << std::left << std::setw(8) << grid_state[i][j] << " | ";
        }
        file_stream << "\n";
        // Ayırıcı çizgi
        if (i < rows - 1) {
            file_stream << "---------+" + std::string(cols * (11), '-') + "\n";
        }
    }

    file_stream.close();
    std::cout << "\nGrid durumu başarıyla '" << filename << "' dosyasına kaydedildi." << std::endl;
}


// =========================
// ANA FONKSİYON (main)
// (Adım 4 çağrısı eklendi)
// =========================
int main() {
    // 1. Video Akışı
    std::string video_source = "http://192.168.1.128:4747/video";
    cv::VideoCapture cap(video_source);
    if (!cap.isOpened()) {
        std::cerr << "HATA: Video akışı başlatılamadı: " << video_source << std::endl;
        return -1;
    }
    std::cout << "Video akışına bağlanıldı..." << std::endl;

    // 2. Grid Ayarları ve Durum Değişkenleri
    const int GRID_ROWS = 9;
    const int GRID_COLS = 9;
    const int STABILITY_FRAMES = 5;
    int DST_W = 630;
    int DST_H = 900;
    cv::Size DST_SIZE(DST_W, DST_H);
    std::vector<cv::Point2f> dst_pts = { {0, 0}, { (float)DST_W, 0 }, { (float)DST_W, (float)DST_H }, { 0, (float)DST_H } };

    std::vector<std::vector<std::string>> grid_state(GRID_ROWS, std::vector<std::string>(GRID_COLS, "---"));
    std::vector<std::vector<std::deque<std::string>>> grid_temp_readings(GRID_ROWS, std::vector<std::deque<std::string>>(GRID_COLS));

    // 3. Pencereler (Oryantasyon düzeltmeleriyle)
    std::string WIN_CAM = "C++ Kamera Akisi (Algilama)";
    std::string WIN_GRID = "C++ Grid Analizi (Duzlestirilmis)";
    std::string WIN_DEBUG = "C++ Debug - Threshold";

    cv::namedWindow(WIN_CAM, cv::WINDOW_NORMAL);
    cv::namedWindow(WIN_GRID, cv::WINDOW_NORMAL);
    cv::namedWindow(WIN_DEBUG, cv::WINDOW_NORMAL);

    cv::resizeWindow(WIN_CAM, 600, 800);    // Dikey kamera akışı
    cv::resizeWindow(WIN_GRID, 900, 630);   // Düzeltilmiş dikey grid
    cv::resizeWindow(WIN_DEBUG, 600, 400);

    // 4. Frame Matrisleri
    cv::Mat frame, vis_frame, warp, thr_debug;
    cv::Mat last_good_vis = cv::Mat::zeros(DST_H, DST_W, 3); // Dikey grid için
    // Oryantasyon düzeltmesinden sonra (90 derece dönünce) boyut 900x630 olacak
    last_good_vis = cv::Mat::zeros(DST_H, DST_W, 3);
    // Aslında last_good_vis'in boyutu dönmüş hali olmalı:
    cv::Mat last_good_vis_rotated = cv::Mat::zeros(DST_SIZE.height, DST_SIZE.width, CV_8UC3);


    // 5. Ana Döngü (while True)
    while (true) {
        bool ret = cap.read(frame);
        if (!ret) {
            std::cout << "Video akışı sonlandı." << std::endl;
            break;
        }

        vis_frame = frame.clone();
        cv::Mat grid_visualization;

        std::vector<cv::Point2f> quad_points = find_grid_contour(frame, thr_debug);

        if (!quad_points.empty()) {
            std::vector<std::vector<cv::Point>> poly_lines(1);
            for (const auto& p : quad_points) poly_lines[0].push_back(cv::Point((int)p.x, (int)p.y));
            cv::polylines(vis_frame, poly_lines, true, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);

            cv::Mat M = cv::getPerspectiveTransform(quad_points, dst_pts);
            cv::warpPerspective(frame, warp, M, DST_SIZE);

            cv::Mat oriented_warp = fix_orientation(warp);

            if (!oriented_warp.empty()) {
                grid_visualization = analyze_grid(oriented_warp, grid_state, grid_temp_readings, STABILITY_FRAMES);
                last_good_vis_rotated = grid_visualization.clone();
            }
            else {
                grid_visualization = last_good_vis_rotated;
            }
        }
        else {
            grid_visualization = last_good_vis_rotated;
        }

        // 6. Görüntüleri göster (Oryantasyon düzeltmeleriyle)
        cv::Mat rotated_vis_frame;
        // Buradaki oryantasyon denemesini (COUNTERCLOCKWISE) geri alıyorum, 
        // Kullanıcı "olmadı" demişti. İkisini de CLOCKWISE yapalım.
        cv::rotate(vis_frame, rotated_vis_frame, cv::ROTATE_90_CLOCKWISE);

        cv::imshow(WIN_CAM, rotated_vis_frame);
        cv::imshow(WIN_GRID, grid_visualization);
        cv::imshow(WIN_DEBUG, thr_debug);


        // 7. Çıkış kontrolü
        char key = (char)cv::waitKey(1);
        if (key == 27 || key == 'q') {
            std::cout << "Çıkış tuşuna basıldı. Grid durumu kaydediliyor..." << std::endl;

            // Dosyayı doğrudan Masaüstünüze kaydetmek için:
            // (Not: Windows'ta ters eğik çizgileri çift kullanmanız gerekir \\)
            std::string save_path = "C:\\Users\\bkaan\\OneDrive\\Desktop\\imagepro\\grid_sonucum.txt";

            save_grid_to_file(grid_state, save_path); // <- Mutlak yol
            break;
        }
    }

    // 8. Temizlik
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
#include <iostream>
#include <chrono>
#include <thread>

#include "ascii_media_player.h"

namespace asciimediaplayer {

    void AsciiMediaPlayer::convertImage(const cv::Mat& grayImage) {
        auto factor = std::min<double>(static_cast<double>(width / 2) / grayImage.cols, static_cast<double>(height) / grayImage.rows);
        cv::Mat resized;
        cv::resize(grayImage, resized, cv::Size(), factor, factor, cv::INTER_AREA);
        uint8_t* imgData = resized.data;
        auto rows = resized.rows, cols = resized.cols;
        int step = resized.step;
    
        const char* map = "   ....'''^^^^\"\"\",,,,:::;;;;IIIllll!!!iiii>>><<<<~~~++++___----???]]]][[[[}}}}{{{{1111))))((((||||\\\\\\\\////ttttffffjjjjrrrrxxxxnnnnuuuuvvvvccccczzzzXXXXYYYYUUUUJJJJCCCCLLLLQQQQ0000OOOOZZZZmmmmwwwwqqqqppppddddbbbbkkkkhhhhaaaaoooo****####MMMMWWWW&&&&8888%%%%BBBB$$$$@@@@";

        for (auto i = 0; i < rows; ++i) {
            for (auto j = 0; j < cols; ++j) {
                auto val = imgData[i * step + j];
                auto pixel = map[val];
                screen[i * (2 * cols + 1) + 2 * j] = pixel;
                screen[i * (2 * cols + 1) + 2 * j + 1] = pixel;
            }
            screen[(i + 1) * (2 * cols + 1) - 1] = '\n';
        }
        screen[rows * (2 * cols + 1) + 1] = '\0';
    }

    void AsciiMediaPlayer::showImage(const std::string& path) {
        cv::Mat grayImage = cv::imread(path, cv::IMREAD_GRAYSCALE);
        convertImage(grayImage);
        std::cout << screen << std::flush;
    }

    void AsciiMediaPlayer::showVideo(const std::string& path) {
        cv::VideoCapture cap(path);
        if (!cap.isOpened()) {
            throw PlayerException("showVideo: failed to open the video");
        }

        auto freq = cap.get(cv::CAP_PROP_FPS);
        auto updateTime = (freq > 0) ? static_cast<int>(1000 / freq) : 25;
        cv::Mat frame;

        while(true) {
            auto success = cap.read(frame);

            if (!success || frame.empty()) {
                break;
            }
            cv::Mat grayFrame;
            cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

            convertImage(grayFrame);
            std::cout << screen << std::flush;

            std::this_thread::sleep_for(std::chrono::milliseconds(updateTime));
        }
        cap.release();
    }

} // namespace asciimediaplayer

/*int main(int argc, char* argv[]) {
    cv::VideoCapture cap(argv[1]);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open the video" << std::endl;
        return -1;
    }

    auto freq = cap.get(cv::CAP_PROP_FPS);
    auto updateTime = (freq > 0) ? static_cast<int>(1000 / freq) : 25;
    cv::Mat frame;

    while (true) {
        auto success = cap.read(frame);

        if (!success || frame.empty()) {
            break;
        }
        cv::Mat grayFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        auto asciiout = convertImage(grayFrame, W, H);
        std::cout << asciiout << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(updateTime));
    }
    cap.release();

    cv::Mat grayImage = cv::imread(argv[1], cv::IMREAD_GRAYSCALE);

    auto asciiout = convertImage(grayImage, W, H);
    std::cout << asciiout << std::flush;

    constexpr auto W = 211, H = 56, FREQ = 25;
    constexpr auto RATE = 1000 / FREQ;
    char output[(W + 1) * H + 1];
    std::cout << output << std::endl;
    for (int t = 0u; t < FREQ * 60; ++t) {
        for (auto i = 0u; i < H; ++i) {
            for (auto j = 0u; j < W; ++j) {
                output[(W + 1) * i + j] = (i != 0 && i != H - 1) ? (j != 0 && j != W - 1) ? (i - 1 == t % (H - 2) || j - 1 == t * 2 % (W - 2) || j - 1 == t * 2 % (W - 2) + 1) ? '@' : ' ' : '|' : '=';
            }
            output[(W + 1) * i + W] = '\n';
        }
        output[(W + 1) * H] = '\0';
        std::cout << output << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(RATE));
    }

    return 0;
}*/

#include <opencv2/opencv.hpp>

namespace asciimediaplayer {

    class PlayerException : public std::exception {
      public:
        PlayerException(const std::string& message) : message(std::string("AsciiMediaPlayer::") + message) {}

        const char* what() const noexcept {
            return message.c_str();
        }

      private:
        std::string message;
    };

    class AsciiMediaPlayer {
      public:
        inline AsciiMediaPlayer(int width, int height) : width(width), height(height) {
            if (width < 3 || height < 1) {
                throw PlayerException("AsciiMediaPlayer: screen size must be not less then 3x1");
            }

            try {
                screen = std::string(width * (height + 1) + 1, '\0');
            } catch(...) {
                throw PlayerException(std::string("AsciiMediaPlayer: failed to initialize screen of size ") + std::to_string(width) + 'x' + std::to_string(height));
            }
        }

        void showImage(const std::string& path);

        void showVideo(const std::string& path);

      private:
        void convertImage(const cv::Mat& image);

      private:
        int width, height;
        std::string screen;
    };

} // namespace asciimediaplayer


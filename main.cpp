#include <iostream>

#include "argument_parser.h"
#include "ascii_media_player.h"

using namespace asciimediaplayer;

int main(int argc, const char* argv[]) {
    ArgumentParser parser{};
    parser.addArgument("-w", "--width", ArgumentType::Int, true);
    parser.addArgument("-h", "--height", ArgumentType::Int, true);
    parser.addArgument("-i", "--image", ArgumentType::Void);
    parser.addArgument("-v", "--video", ArgumentType::Void);
    parser.addArgument("-f", "--file", ArgumentType::String, true);
    parser.parse(argc, argv);

    auto width = parser.get<ArgumentType::Int>("-w").second;
    auto height = parser.get<ArgumentType::Int>("-h").second;
    auto path = parser.get<ArgumentType::String>("-f").second;

    auto imageSelected = parser.get<ArgumentType::Void>("-i"), videoSelected = parser.get<ArgumentType::Void>("-v");
    if (imageSelected == videoSelected) {
        std::cout << "Please choose eighter '-i/--image' for image or '-v/--video' for video" << std::endl;
        return 0;
    }

    AsciiMediaPlayer player(width, height);

    if (parser.get<ArgumentType::Void>("-i")) {
        player.showImage(path);
    } else {
        player.showVideo(path);
    }

    return 0;
}

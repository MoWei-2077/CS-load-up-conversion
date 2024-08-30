#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <thread>
#include <map>
#include <sstream>
#include <thread>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unordered_map>
#include <filesystem>


std::unordered_map<std::string, std::unordered_map<int, int>> frequencyLevels;


void readFrequencyTable(const std::string& policy) {
    std::string filePath = "/sys/devices/system/cpu/cpufreq/" + policy + "/scaling_available_frequencies";
    std::ifstream file(filePath);

    std::string line;
    getline(file, line);
    file.close();

    std::vector<int> frequencies;
    size_t pos = 0;
    while ((pos = line.find(' ')) != std::string::npos) {
        std::string freqStr = line.substr(0, pos);
        int freq = stoi(freqStr);
        frequencies.push_back(freq);
        line.erase(0, pos + 1);
    }

    // Determine frequency levels
    int level = 1;
    int step = frequencies.size() / 12;
    for (int i = 0; i < frequencies.size(); i++) {
        frequencyLevels[policy][frequencies[i]] = level;
        if ((i + 1) % step == 0 && level < 12) {
            level++;
        }
    }
}

 void WriteFile(const std::string& filePath, const std::string& content) noexcept {
        int fd = open(filePath.c_str(), O_WRONLY | O_NONBLOCK);

        if (fd < 0) {
            chmod(filePath.c_str(), 0666);
            fd = open(filePath.c_str(), O_WRONLY | O_NONBLOCK);
        }

        if (fd >= 0) {
            write(fd, content.data(), content.size());
            close(fd); 
            chmod(filePath.c_str(), 0444);
        }
    }

void setMaxFrequency(const std::string& policy, int level) {
    for (const auto& pair : frequencyLevels[policy]) {
        if (pair.second == level) {
            std::string maxFreqPath = "/sys/devices/system/cpu/cpufreq/" + policy + "/scaling_max_freq";
            WriteFile(maxFreqPath, std::to_string(pair.first));
            std::cout << "最大频率设置为:" << level << " (" << pair.first / 1000 << " MHz) 设置核心为: " << policy << std::endl;
            break;
        }
    }
}

int main() {
    for (int i = 0; i <= 7; ++i) {
        std::string policy = "policy" + std::to_string(i);
        readFrequencyTable(policy);
            setMaxFrequency(policy, 5);
    }

    return 0;
}

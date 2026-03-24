#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
	os << "[";
	for (size_t i = 0; i < vec.size(); ++i) {
		os << vec[i];
		if (i != vec.size() - 1) os << ", ";
	}
	os << "]";
	return os;
}

inline const char* BaseFileName(const char* path) {
	const char* file = strrchr(path, '\\');
	if (!file) file = strrchr(path, '/');
	return file ? file + 1 : path;
}

#define LOG(msg){ \
	auto now = std::chrono::system_clock::now(); \
	auto time = std::chrono::system_clock::to_time_t(now); \
	std::tm tm; \
	localtime_s(&tm, &time); \
	std::cout << "[" << std::put_time(&tm, "%H:%M:%S") << "]" \
			  << "[" << BaseFileName(__FILE__) << ":" << __LINE__ << "]" \
			  << " " << msg << std::endl; \
}

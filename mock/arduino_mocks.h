// Minimal Arduino mocks for host-side compilation/testing
#ifndef ARDUINO_MOCKS_H
#define ARDUINO_MOCKS_H

#include <string>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

using String = std::string;

inline void pinMode(int, int) {}
inline int digitalRead(int) { return 1; }
inline void digitalWrite(int, int) {}

#define INPUT_PULLUP 0
#define HIGH 1
#define LOW 0

// DAC pin placeholder
#define DAC1 0

// Millis() implementation using steady clock
inline unsigned long millis() {
    using namespace std::chrono;
    static auto start = steady_clock::now();
    auto now = steady_clock::now();
    return (unsigned long)duration_cast<milliseconds>(now - start).count();
}

// Delay helper
inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Minimal Serial mock to capture prints from code that uses Serial
struct SerialMock {
    void begin(int) {}
    template<typename T>
    void print(const T &v) { std::cout << v; }
    template<typename T>
    void println(const T &v) { std::cout << v << std::endl; }
    void println() { std::cout << std::endl; }
};

inline SerialMock Serial;

#endif // ARDUINO_MOCKS_H

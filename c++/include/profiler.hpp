#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <thread>

struct Profiler {
    using Measurements_t = std::map<std::string, std::vector<double>>;
    double cpu_freq;
    
    Profiler() {
        cpu_freq = measure_cpu_frequency();
    }
    Profiler(double cpu_freq_) : cpu_freq(cpu_freq_) {}

    static uint64_t read_cycles() {
    #if defined(__x86_64__) || defined(_M_X64)
        unsigned lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t)hi << 32) | lo;
    #elif defined(__aarch64__)
        uint64_t val;
        asm volatile("mrs %0, cntvct_el0" : "=r"(val));
        return val;
    #else
        // fallback
        return std::chrono::steady_clock::now().time_since_epoch().count();
    #endif
    }

    static double measure_cpu_frequency() {
        auto start_time = std::chrono::steady_clock::now();
        uint64_t start_cycles = read_cycles();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        uint64_t end_cycles = read_cycles();
        auto end_time = std::chrono::steady_clock::now();

        double elapsed_s = std::chrono::duration<double>(end_time - start_time).count();
        double cycles_per_second = (end_cycles - start_cycles) / elapsed_s;

        return cycles_per_second;
    }

    double to_ms(uint64_t cycles) const {
        return cycles * 1e3 / cpu_freq;
    }

    void add_measurement(const std::string& key, uint64_t value) {
        measurements[key].push_back(value);
    }
    void add_counter(const std::string& key, int32_t value) {
        counters[key].push_back(value);
    }

    Measurements_t get_measurements() {
        Measurements_t result;
        auto oo_cpu_freq = 1.0 / cpu_freq;
        for (const auto& kv : measurements) {
            std::vector<double> times;
            times.reserve(kv.second.size());
            for (const auto& cycles : kv.second) {
                times.push_back(cycles * 1e3 * oo_cpu_freq); // Convert to milliseconds
            }
            result[kv.first] = std::move(times);
        }
        for (const auto& kv : counters) {
            result[kv.first] = std::vector<double>(kv.second.begin(), kv.second.end());
        }   
        return result;
    }

    const std::map<std::string, std::vector<int32_t>>& get_counters() const {
        return counters;
    }

    struct Section {
        Profiler &profiler;
        std::string name;
        uint64_t start_cycles;
        Section(Profiler &profiler_, const std::string &name_)
            : profiler(profiler_), name(name_) {
            start_cycles = Profiler::read_cycles();
        }
        ~Section() {
            uint64_t end_cycles = Profiler::read_cycles();
            double elapsed_ms = profiler.to_ms(end_cycles - start_cycles);
            profiler.add_measurement(name, elapsed_ms);
        }
    };

    protected:
    std::map<std::string, std::vector<uint64_t>> measurements;
    std::map<std::string, std::vector<int32_t>> counters;
};

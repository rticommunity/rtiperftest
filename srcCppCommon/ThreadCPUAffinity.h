/*
 * (c) 2005-2023  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef THREADCPUAFFINITY_H
#define THREADCPUAFFINITY_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

class ThreadCPUAffinity {

  private:
    std::vector<int> cores_main;
    std::string cores_main_str;
    std::vector<int> cores_receive;
    std::string cores_receive_str;
    std::vector<int> cores_db;
    std::string cores_db_str;
    std::vector<int> cores_event;
    std::string cores_event_str;
    bool isSet;

    // Helper to parse a single core string, e.g. "8-9,11,13-15"
    static std::vector<int> parse_core_list(const std::string& str) {
        std::vector<int> result;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, ',')) {
            size_t dash = token.find('-');
            if (dash != std::string::npos) {
                int start = std::stoi(token.substr(0, dash));
                int end = std::stoi(token.substr(dash + 1));
                if (start > end) std::swap(start, end);
                for (int i = start; i <= end; ++i) {
                    result.push_back(i);
                }
            } else {
                result.push_back(std::stoi(token));
            }
        }
        return result;
    }

  public:
    ThreadCPUAffinity()
        : isSet(false) {}

    // Getters
    const std::vector<int>& get_cores_main() const { return cores_main; }
    const std::vector<int>& get_cores_receive() const { return cores_receive; }
    const std::vector<int>& get_cores_db() const { return cores_db; }
    const std::vector<int>& get_cores_event() const { return cores_event; }
    const std::string& get_cores_main_str() const { return cores_main_str; }
    const std::string& get_cores_receive_str() const { return cores_receive_str; }
    const std::string& get_cores_db_str() const { return cores_db_str; }
    const std::string& get_cores_event_str() const { return cores_event_str; }

    // Getter for isSet
    bool isInitialized() const { return isSet; }

    bool parse_affinities(const std::string& arg) {
        // Format: "8-9:10:11-12:13-15"
        std::vector<std::string> parts;
        std::stringstream ss(arg);
        std::string item;
        while (std::getline(ss, item, ':')) {
            parts.push_back(item);
        }
        if (parts.size() != 4) {
            fprintf(stderr, "[ThreadCPUAffinity] Failed to parse input for Perftest_ThreadAffinity\n");
            return false;
        }
        try {
            cores_main = parse_core_list(parts[0]);
            cores_main_str = parts[0];
            cores_receive = parse_core_list(parts[1]);
            cores_receive_str = parts[1];
            cores_db = parse_core_list(parts[2]);
            cores_db_str = parts[2];
            cores_event = parse_core_list(parts[3]);
            cores_event_str = parts[3];
            isSet = true;
            return true;
        } catch (std::exception&) {
            fprintf(stderr, "[ThreadCPUAffinity] Exception while parsing core affinities\n");
            return false;
        }
    }

    bool set_main_thread_affinity() {
      #ifdef RTI_LINUX
        if (!cores_main.empty()) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            for (int core : cores_main) {
                CPU_SET(core, &cpuset);
            }
            int error = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
            if (error != 0) {
              fprintf(stderr, "[ThreadCPUAffinity] Failed to set CPU affinity for main thread (error %d)\n", error);
              return false;
            }
        }
        return true;
      #else
        fprintf(stderr, "[ThreadCPUAffinity] CPU affinity is not supported on this platform\n");
        return false;
      #endif
    }

};

#endif // THREADCPUAFFINITY_H

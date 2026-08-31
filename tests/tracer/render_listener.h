// tracer/render_listener.h
#pragma once

#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include "tracer.h"

class RenderTraceListener : public ::testing::EmptyTestEventListener {
public:
    // Called before each individual test starts
    void OnTestStart(const ::testing::TestInfo& info) override {
        trace_reset();
        printf("[tracer] Starting trace for: %s.%s\n",
               info.test_suite_name(), info.name());
    }

    // Called after each individual test ends
    void OnTestEnd(const ::testing::TestInfo& info) override {
        // Resolve output directory from environment, fall back to CWD
        const char* out_dir_env = std::getenv("TRACE_OUTPUT_DIR");
        std::string out_dir = (out_dir_env && out_dir_env[0]) ? out_dir_env : ".";

        // Build a safe filename: "trace__Suite__TestName.json"
        std::string filename = "trace__";
        filename += info.test_suite_name();
        filename += "__";
        filename += info.name();
        filename += ".json";

        // Replace characters that are unsafe in filenames
        for (char& c : filename) {
            if (c == '/' || c == ' ' || c == ':') c = '_';
        }

        std::string full_path = out_dir + "/" + filename;
        trace_dump_json(full_path.c_str());
    }
};

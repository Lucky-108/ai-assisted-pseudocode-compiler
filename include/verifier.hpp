
#ifndef VERIFIER_HPP
#define VERIFIER_HPP

#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>


class CppVerifier {
private:
    std::string runAndCapture(
        const std::string& executablePath
    ) const {
        const std::string command = "\"" + executablePath + "\"";
        FILE* pipe = popen(command.c_str(), "r");

        if (pipe == nullptr) {
            throw std::runtime_error(
                "Could not run generated executable."
            );
        }

        std::array<char, 256> buffer{};
        std::ostringstream output;

        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            output << buffer.data();
        }

        const int status = pclose(pipe);

        if (status != 0) {
            throw std::runtime_error(
                "Generated executable failed during execution."
            );
        }

        return output.str();
    }

    std::string normalizeOutput(const std::string& output) const {
        std::istringstream input(output);
        std::ostringstream normalized;
        std::string line;

        while (input >> line) {
            if (normalized.tellp() > 0) {
                normalized << ' ';
            }
            normalized << line;
        }

        return normalized.str();
    }

public:
    bool compile(
        const std::string& sourcePath,
        const std::string& executablePath
    ) const {
        const std::string command =
            "g++ -std=c++17 -Wall -Wextra -pedantic \"" +
            sourcePath + "\" -o \"" + executablePath + "\"";

        const int result = std::system(command.c_str());

        if (result != 0) {
            std::cerr << "Verification failed: generated C++ "
                      << "did not compile.\n";
            return false;
        }

        return true;
    }

    bool compareOutput(
        const std::string& executablePath,
        const std::string& expectedOutput
    ) const {
        try {
            const std::string actualOutput =
                runAndCapture(executablePath);

            if (normalizeOutput(actualOutput) ==
                normalizeOutput(expectedOutput)) {
                std::cout << "Verification passed.\n";
                return true;
            }

            std::cerr << "Verification failed: output mismatch.\n"
                      << "Expected: " << expectedOutput
                      << "Actual:   " << actualOutput;

            return false;
        } catch (const std::exception& error) {
            std::cerr << "Verification failed: "
                      << error.what() << '\n';
            return false;
        }
    }
};

#endif

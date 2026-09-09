#ifndef GIVM_EXAMPLE_TEST_TOOLS_HPP
#define GIVM_EXAMPLE_TEST_TOOLS_HPP

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace givm::examples
{
namespace detail
{

// Each process owns one directory, and each example truncates the same file.
// Reports use stderr; stdout does not need to be restored to its original target.
class output_capture
{
public:
    output_capture()
    {
        const auto temporary = std::filesystem::temp_directory_path();
        std::random_device random;
        for(int attempt = 0; attempt != 128; ++attempt)
        {
            auto directory = temporary / ("givm-examples-" + std::to_string(random())
                                          + '-' + std::to_string(random()));
            auto file = directory / "stdout.bin";
            if(std::filesystem::create_directory(directory))
            {
                directory_ = std::move(directory);
                file_ = std::move(file);
                return;
            }
        }
        throw std::runtime_error("Cannot create a unique example output directory");
    }

    output_capture(const output_capture&) = delete;
    output_capture& operator=(const output_capture&) = delete;

    ~output_capture()
    {
        // close() reports failures on the ordinary path; this also cleans up
        // when constructing or running the Catch2 session throws.
        try
        {
            close();
        }
        catch(...)
        {}
    }

    void begin()
    {
        std::cout.flush();
        if(std::fflush(stdout) != 0)
        {
            throw std::runtime_error("Cannot flush stdout before an example");
        }
        redirected_ = true;
#ifdef _MSC_VER
        std::FILE* stream = nullptr;
        open_ = ::_wfreopen_s(&stream, file_.c_str(), L"w+b", stdout) == 0;
#elif defined(_WIN32)
        open_ = ::_wfreopen(file_.c_str(), L"w+b", stdout) != nullptr;
#else
        open_ = std::freopen(file_.c_str(), "w+b", stdout) != nullptr;
#endif
        if(!open_)
        {
            // freopen closes the old stream even when opening the new one fails.
            detach_streams();
            throw std::runtime_error("Cannot redirect example stdout");
        }
    }

    std::string read() const
    {
        std::fpos_t end;
        if(std::fgetpos(stdout, &end) != 0)
        {
            throw std::runtime_error("Cannot locate the end of captured example stdout");
        }
        std::string result;
        std::exception_ptr error;
        try
        {
            if(std::fseek(stdout, 0, SEEK_SET) != 0)
            {
                throw std::runtime_error("Cannot rewind captured example stdout");
            }
            std::array<char, 4096> buffer;
            while(const auto count = std::fread(buffer.data(), 1, buffer.size(), stdout))
            {
                result.append(buffer.data(), count);
            }
            if(std::ferror(stdout) || !std::feof(stdout))
            {
                throw std::runtime_error("Error reading captured example stdout");
            }
        }
        catch(...)
        {
            error = std::current_exception();
        }
        // End the input phase before a later cout flush or another example.
        // Reusing this FILE avoids opening the same file twice on Windows.
        std::clearerr(stdout);
        if(std::fsetpos(stdout, &end) != 0)
        {
            throw std::runtime_error("Cannot restore captured example stdout position");
        }
        if(error)
        {
            std::rethrow_exception(error);
        }
        return result;
    }

    void close()
    {
        if(redirected_)
        {
            // ios_base::Init flushes the standard output streams at shutdown.
            // Detach them before closing stdout so those flushes remain valid.
            detach_streams();
            redirected_ = false;
        }
        if(open_)
        {
            open_ = false;
            if(std::fclose(stdout) != 0)
            {
                throw std::runtime_error("Cannot close captured example stdout");
            }
        }
        if(!file_.empty())
        {
            std::filesystem::remove(file_);
            file_.clear();
        }
        if(!directory_.empty())
        {
            std::filesystem::remove(directory_);
            directory_.clear();
        }
    }

private:
    static void detach_streams()
    {
        std::cout.exceptions(std::ios::goodbit);
        std::wcout.exceptions(std::ios::goodbit);
        std::cout.rdbuf(nullptr);
        std::wcout.rdbuf(nullptr);
    }

    std::filesystem::path directory_;
    std::filesystem::path file_;
    bool redirected_ = false;
    bool open_ = false;
};

inline output_capture* current_capture = nullptr;

inline std::string normalize_newlines(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for(std::size_t index = 0; index != text.size(); ++index)
    {
        if(text[index] == '\r' && index + 1 != text.size() && text[index + 1] == '\n')
        {
            ++index;
        }
        result.push_back(text[index]);
    }
    return result;
}

} // namespace detail

inline void check(int (*run)(), std::string_view expected)
{
    if(!detail::current_capture)
    {
        throw std::logic_error("Example checks must run through givm::examples::run_main");
    }

    std::ostringstream previous_format;
    previous_format.copyfmt(std::cout);
    const auto previous_state = std::cout.rdstate();
    auto* const previous_buffer = std::cout.rdbuf();
    detail::current_capture->begin();

    int exit_code = 0;
    std::exception_ptr error;
    try
    {
        exit_code = run();
    }
    catch(...)
    {
        error = std::current_exception();
    }
    try
    {
        std::cout.flush();
        if(!std::cout)
        {
            throw std::runtime_error("Cannot flush captured example stdout");
        }
    }
    catch(...)
    {
        if(!error)
        {
            error = std::current_exception();
        }
    }
    if(std::fflush(stdout) != 0 && !error)
    {
        error = std::make_exception_ptr(
            std::runtime_error("Cannot flush captured example stdout")
        );
    }

    // Restore formatting, exception mask, error bits and buffer before Catch2
    // writes diagnostics (stderr is ordinarily tied to cout).
    std::cout.exceptions(std::ios::goodbit);
    std::cout.rdbuf(previous_buffer);
    std::cout.clear(previous_state);
    std::cout.copyfmt(previous_format);

    const auto actual = detail::normalize_newlines(detail::current_capture->read());
    const auto expected_output = detail::normalize_newlines(expected);
    if(error)
    {
        UNSCOPED_INFO("captured stdout: " << actual);
        std::rethrow_exception(error);
    }
    INFO("captured stdout: " << actual);
    CHECK(exit_code == 0);
    CHECK(actual == expected_output);
}

inline int run_main(int argc, char* argv[])
{
    std::optional<detail::output_capture> capture;
    try
    {
        int result;
        {
            Catch::Session session;
            session.configData().runOrder = Catch::TestRunOrder::Declared;
            const int command_line_result = session.applyCommandLine(argc, argv);
            if(command_line_result != 0)
            {
                return command_line_result;
            }
            auto& config = session.configData();
            if(config.listTests || config.listTags || config.listReporters
               || config.listListeners || config.showHelp || config.libIdentify)
            {
                return session.run();
            }

            if(!config.defaultOutputFilename.empty() && config.defaultOutputFilename != "%stderr")
            {
                throw std::invalid_argument("Example test reports must use stderr");
            }
            for(const auto& reporter : config.reporterSpecifications)
            {
                if(reporter.name() != "console"
                   || (reporter.outputFile() && *reporter.outputFile() != "%stderr"))
                {
                    throw std::invalid_argument(
                        "Example execution supports only the console reporter on stderr"
                    );
                }
            }
            config.defaultOutputFilename = "%stderr";
            config.showInvisibles = true;
            config.allowZeroTests = false;
            for(auto& name : config.testsOrTags)
            {
                if(name.ends_with(".md"))
                {
                    name.resize(name.size() - 3);
                }
            }

            capture.emplace();
            detail::current_capture = &*capture;
            result = session.run();
        }
        detail::current_capture = nullptr;
        capture->close();
        return result;
    }
    catch(const std::exception& error)
    {
        detail::current_capture = nullptr;
        std::fprintf(stderr, "Example runner error: %s\n", error.what());
        return 1;
    }
    catch(...)
    {
        detail::current_capture = nullptr;
        std::fputs("Example runner error: unknown exception\n", stderr);
        return 1;
    }
}

} // namespace givm::examples

#endif // GIVM_EXAMPLE_TEST_TOOLS_HPP

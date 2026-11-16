#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <variant>
#include <vector>

#include "epp/diagnostics/ir_lower.h"
#include "epp/diagnostics/source_analysis.h"
#include "epp/driver/build_arguments.h"
#include "epp/driver_config.h"
#include "epp/ir/lower.h"
#include "epp/version.h"
#if EPP_NATIVE_BUILD_ENABLED
#include "epp/driver/build_executable.h"
#endif
#include "epp/frontend/analyze.h"
#include "epp/source/source_file.h"

namespace {

enum ErrorCode {
  kSourceError = 1,
  kCliUsageError = 2,
  kInternalError = 3,
};

void PrintUsage(std::ostream& output) {
  output
    << "Usage: epp --version\n"
       "       epp check SOURCE.epp\n"
       "       epp build [-o OUTPUT] SOURCE.epp"
    << std::endl;
}

int InputError(std::string_view path, std::string_view message) {
  std::cerr << path << ": error: " << message << std::endl;
  return kCliUsageError;
}

int ProcessSource(
  const std::string& argument,
  const std::optional<std::filesystem::path>& output,
  std::string_view& phase
) {
#if !EPP_NATIVE_BUILD_ENABLED
  (void)output;
#endif
  phase = "input validation";
  const std::filesystem::path path{argument};
  if (path.extension() != ".epp") {
    return InputError(argument, "source file must have '.epp' extension");
  }
  std::error_code error;
  const bool regular = std::filesystem::is_regular_file(path, error);
  if (error) {
    return InputError(argument, "cannot access source file");
  }
  if (!regular) {
    return InputError(argument, "source is not a regular file");
  }

  phase = "source reading";
  const auto read_result = epp::source::ReadSourceFile(path);
  if (const auto* read_error = std::get_if<epp::source::SourceFileReadError>(&read_result)) {
    switch (*read_error) {
      case epp::source::SourceFileReadError::kCannotOpen:
        return InputError(argument, "cannot open source file");
      case epp::source::SourceFileReadError::kCannotRead:
        return InputError(argument, "cannot read source file");
    }
    throw std::logic_error{"unknown source file read error"};
  }
  const auto& source = std::get<epp::source::SourceFile>(read_result);

  phase = "source analysis";
  const auto result = epp::frontend::AnalyzeSource(source);

  phase = "diagnostic conversion";
  const auto diagnostic = epp::diagnostics::ToDiagnostic(result);
  if (diagnostic.has_value()) {
    phase = "diagnostic formatting";
    std::cerr << epp::diagnostics::FormatDiagnostic(*diagnostic, source);
    return kSourceError;
  }
  phase = "IR lowering";
  const auto lowered = epp::ir::LowerToIr(std::get<epp::frontend::AnalyzedSource>(result));
  if (const auto* error = std::get_if<epp::ir::LoweringError>(&lowered)) {
    std::cerr << epp::diagnostics::FormatDiagnostic(epp::diagnostics::ToDiagnostic(*error), source);
    return kSourceError;
  }
#if EPP_NATIVE_BUILD_ENABLED
  if (output.has_value()) {
    phase = "native build";
    const auto error = epp::driver::BuildExecutableWithGnuDriver(
      std::get<epp::ir::Program>(lowered),
      std::filesystem::absolute(path),
      std::filesystem::absolute(*output),
      EPP_LINK_DRIVER,
      EPP_OUTPUT_RUNTIME
    );
    if (error.has_value()) {
      return InputError(argument, error->message);
    }
  }
#endif
  return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
  std::string_view phase = "command line";
  try {
    if (argc == 2 && std::string_view(argv[1]) == "--version") {
      std::cout << epp::VersionText() << std::endl;
      return 0;
    }
    if (argc >= 2 && std::string_view(argv[1]) == "build") {
      std::vector<std::string_view> arguments;
      for (int i = 2; i < argc; ++i) {
        arguments.emplace_back(argv[i]);
      }
      const auto parsed = epp::driver::ParseBuildArguments(arguments);
      if (const auto* error = std::get_if<epp::driver::BuildArgumentsError>(&parsed)) {
        return InputError("epp build", error->message);
      }
#if !EPP_ENABLE_LLVM
      return InputError("epp build", "native build requires EPP_ENABLE_LLVM=ON");
#elif !EPP_NATIVE_BUILD_ENABLED
      return InputError("epp build", "native build requires a native Unix GNU-style compiler driver");
#else
      const auto& build = std::get<epp::driver::BuildArguments>(parsed);
      auto output = std::filesystem::path{build.source_path};
      output.replace_extension(EPP_EXECUTABLE_SUFFIX);
      if (build.output_path.has_value()) {
        output = *build.output_path;
      }
      return ProcessSource(build.source_path, output, phase);
#endif
    }
    if (argc != 3
      || std::string_view(argv[1]) != "check"
      || std::string_view(argv[2]).empty()
      || std::string_view(argv[2]).starts_with('-')) {
      PrintUsage(std::cerr);
      return kCliUsageError;
    }
    return ProcessSource(argv[2], std::nullopt, phase);
  } catch (const std::exception& error) {
    std::cerr << "epp " << epp::Version() << ": internal error during " << phase << ": " << error.what() << std::endl;
  } catch (...) {
    std::cerr << "epp " << epp::Version() << ": internal error during " << phase << ": unknown exception" << std::endl;
  }
  return kInternalError;
}

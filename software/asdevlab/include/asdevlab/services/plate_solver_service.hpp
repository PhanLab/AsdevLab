#pragma once

#include <optional>
#include <string>

namespace asdevlab {

struct SolveResult {
    bool solved = false;
    std::optional<double> target_ra;
    std::optional<double> target_dec;
    std::optional<double> mount_ra;
    std::optional<double> mount_dec;
    std::optional<double> offset;
    std::optional<std::string> warning_type;
    std::optional<std::string> warning_message;
};

class PlateSolverService {
public:
    SolveResult solve_result(const std::string& image_id);
    std::string solve(const std::string& image_id);
    std::string calibrate(const std::string& image_id);
};

} // namespace asdevlab

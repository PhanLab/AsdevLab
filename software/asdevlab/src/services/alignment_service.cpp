#include "asdevlab/services/alignment_service.hpp"

#include <sstream>

namespace asdevlab {

std::string AlignmentService::sync_from_solution(const std::string& solution) {
    std::ostringstream oss;
    oss << "AlignmentService: sync " << solution;
    return oss.str();
}

} // namespace asdevlab

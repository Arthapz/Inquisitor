// Copryright (C) 2023 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

#include "CoreDependencies.hpp"
#include "Inquisitor.hpp"

/////////////////////////////////////
/////////////////////////////////////
auto main(const int argc, const char **argv) -> int {
    try {
        auto inquisitor = Inquisitor {};
        inquisitor.run(argc, argv);
    } catch (const std::exception &e) {
        flog("Unhandled exception, {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

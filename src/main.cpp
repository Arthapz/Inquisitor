// Copryright (C) 2021 Arthur LAURENT <arthur.laurent4@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level of this distribution

/////////// - STL - ///////////
#include <cstdlib>

/////////// - Inquisitor - ///////////
#include "Inquisitor.hpp"
#include "Log.hpp"

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

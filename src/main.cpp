#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"

int main() {
    glm::vec3 vec{5.0f};

    std::cout << glm::to_string(vec) << std::endl;
    return 0;
}

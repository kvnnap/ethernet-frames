//
// Created by kevin on 8/7/16.
//

#ifndef UTIL_STRINGOPERATIONS_H
#define UTIL_STRINGOPERATIONS_H

#include <string>
#include <vector>

namespace Util
{
    namespace StringOperations {
        std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
        std::vector<std::string> split(const std::string &s, char delim);
    }
}


#endif //UTIL_STRINGOPERATIONS_H

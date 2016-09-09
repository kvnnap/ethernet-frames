//
// Created by kevin on 9/9/16.
//

#ifndef NETWORK_DISCOVERY_SETOPERATIONS_H
#define NETWORK_DISCOVERY_SETOPERATIONS_H

#include <set>
#include <vector>

namespace Mathematics {
    namespace SetOperations {
        template <class T>
        bool setsDisjoint(const std::set<T> &a, const std::set<T> &b) {
            using namespace std;
            vector<T> result;
            set_intersection(a.begin(), a.end(), b.begin(), b.end(), back_inserter(result));
            return result.size() == 0;
        }

        // Does set B fully contain set A - Is Set A a subset of set B?
        template <class T>
        bool setSubsetOf(const std::set<T> &a, const std::set<T> &b) {
            using namespace std;
            set<T> result;
            set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(result, result.begin()));
            return result == a;
        }
    }
}


#endif //NETWORK_DISCOVERY_SETOPERATIONS_H

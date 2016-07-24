//
// Created by kevin on 7/24/16.
//

#ifndef NETWORK_DISCOVERY_IFACTORY_H
#define NETWORK_DISCOVERY_IFACTORY_H

#include <memory>

template <class T>
class IFactory {
public:
    virtual ~IFactory() {}
    virtual std::unique_ptr<T> make(const std::string& name) const = 0;
private:

};

#endif //NETWORK_DISCOVERY_IFACTORY_H

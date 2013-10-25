
#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <string>

#include "core/utils/qs_manager.h"


class A {
    public:
        A() {};        
        const std::vector<int>& all() const {
            //std::cout << "call A::all vector" << std::endl;
            return _all;
            };
    //protected:
        std::vector<int> _all;
    };

class AMap {
    public:
        AMap() {};
        const std::map<std::string, int>& all() const {
            return _all;
            };
        std::map<std::string, int> _all;
    };

class B {
    public:
        B() {};
        void all(std::vector<int>& all) const {
            //std::cout << "call B::all vector" << std::endl;
            all = _all;
            };
    //protected:
        std::vector<int> _all;
    };

class BMap {
    public:
        BMap() {};
        void all(std::map<std::string, int>& all) const {
            //std::cout << "call B::all vector" << std::endl;
            all = _all;
            };
        std::map<std::string, int> _all;
    };


// We need a compare operator for int - std::string
template<typename T>
struct convert {
    struct exception : public std::runtime_error {
        exception() : std::runtime_error("Conversion failed") {};
        };

    inline T operator()(const std::string& str) {
        std::istringstream iss(str);
        T obj;
        iss >> std::ws >> obj >> std::ws;

        if (!iss.eof()) throw exception();
        return obj;
        };
    };

namespace citef {
    namespace core {
        namespace qs_manager_implementation {
            // Need to 'get' by id
            bool operator==(const int& lhs, const std::string& rhs) {
                try {
                    return (lhs == convert<int>()(rhs));
                    }
                catch(convert<int>::exception&) {
                    return false;
                    }
                }
            }}}

int main() {
    std::cout << ">>> TEST #1: vector <<<" << std::endl;
    std::cout << "Container with 'const std::vector<int>& all() const'" << std::endl;
    A a;
    citef::core::qs_manager<int, A, std::string> manager_A(a);
    a._all.push_back(0);
    a._all.push_back(1);
    std::cout << "size: " << manager_A.all().size() << std::endl;
    std::cout << "get('1'): " << manager_A.get("1") << std::endl;
    std::cout << std::endl;

    std::cout << "Container with 'void all(std::vector<int>&) const'" << std::endl;
    B b;
    citef::core::qs_manager<int, B, std::string> manager_B(b);
    b._all.push_back(0);
    b._all.push_back(1);
    std::cout << "size: " << manager_B.all().size() << std::endl;
    std::cout << "get('1'): " << manager_B.get("1") << std::endl;
    std::cout << std::endl;

    std::cout << "Container IS a 'std::vector<int>'" << std::endl;
    std::vector<int> c;
    citef::core::qs_manager<int, std::vector<int>, std::string> manager_C(c);
    c.push_back(0);
    c.push_back(1);
    std::cout << "size: " << manager_C.all().size() << std::endl;
    std::cout << "get('1'): " << manager_C.get("1") << std::endl;
    std::cout << std::endl << std::endl;

    std::cout << ">>> TEST #2: map <<<" << std::endl;
    std::cout << "Container with 'const std::map<std::string, int>& all() const'" << std::endl;
    AMap amap;
    citef::core::qs_manager<int, AMap, std::string> manager_AMap(amap);
    amap._all["0"] = 111;
    std::cout << "size: " << manager_AMap.all().size() << std::endl;
    std::cout << "get('0'): " << manager_AMap.get("0") << std::endl;
    std::cout << std::endl;

    std::cout << "Container with 'void all(std::map<std::string, int>&) const'" << std::endl;
    BMap bmap;
    citef::core::qs_manager<int, BMap, std::string> manager_BMap(bmap);
    bmap._all["0"] = 222;
    std::cout << "size: " << manager_BMap.all().size() << std::endl;
    std::cout << "get('0'): " << manager_BMap.get("0") << std::endl;
    std::cout << std::endl;

    std::cout << "Container IS a 'std::map<std::string, int>'" << std::endl;
    std::map<std::string, int> cmap;
    citef::core::qs_manager<int, std::map<std::string, int>, std::string> manager_CMap(cmap);
    cmap["0"] = 333;    
    std::cout << "size: " << manager_CMap.all().size() << std::endl;
    std::cout << "get('0'): " << manager_CMap.get("0") << std::endl;
    std::cout << std::endl;

    getchar();
    return 0;
    }
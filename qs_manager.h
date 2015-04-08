#pragma once

#include <exception>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include "has_member.hpp"

/* Implements (or tries to) an unified way of working with containers of several types: maps, vectors, multimaps,...
    in a way all of them could inherit from the same interface and be treated the same way.
*/

namespace core {
    namespace utils {

        /* Define exceptions for this class
            * object_not_found: no object found
            * multiple_objects_found: multiple objects found
        */
        class object_not_found : public std::runtime_error {
            public:
                object_not_found(const std::string& id) : std::runtime_error(std::string("No object found for id '") + id +"'") {};
            };

        class multiple_objects_found : public std::runtime_error {
            public:
                multiple_objects_found(const std::string& id) : std::runtime_error(std::string("Multiple objects found for id '") + id + "'") {};
            };

        template <class T, class TContainer, class ID> class qs_manager; // forward declaration

        namespace _private {
            /* Some private for easier template implementation on qs_manager.
            */
            template <typename All_Container>
            struct check_all_to_container {
                template <typename T, void (T::*)(typename All_Container&) const = &T::all>
                struct get {};
                };

            template <typename T, typename ContainerType>
            struct has_all_to_container : has_member<T, typename check_all_to_container<ContainerType>> {};
            
            template <typename All_Container>
            struct check_all_as_container {
                template <typename T, const typename All_Container& (T::*)() const = &T::all>
                struct get {};
                };

            template <typename T, typename ContainerType>
            struct has_all_as_container : has_member<T, typename check_all_as_container<ContainerType>> {};
            
            // To implement
            template <class T, class ID, class ClassToTest, typename Enable = void>
            class qs_manager_get_impl {
                public:
                    qs_manager_get_impl<T, ID, ClassToTest, Enable>(const typename ClassToTest& container) {
                        static_assert( false, "<ClassToTest> is not implemented as a valid container for 'qs_manager'");
                        };

                    void all(std::vector<T>& ret) const {
                        static_assert( false, "<ClassToTest> is not implemented as a valid container for 'qs_manager'");
                        };

                    const std::vector<T>& all() const {
                        static_assert( false, "<ClassToTest> is not implemented as a valid container for 'qs_manager'");
                        };

                    const size_t count() const {
                        static_assert( false, "<ClassToTest> is not implemented as a valid container for 'qs_manager'");
                        };

                    const T& get(const ID& id) const {
                        static_assert( false, "<ClassToTest> is not implemented as a valid container for 'qs_manager'");
                        };
                };
           
/**********************
    VECTORS
***********************/
            // Vector implementations
            template <class T, class ID, typename Enable = void>
            class qs_manager_getter_vector {
                public:
                    virtual const std::vector<T>& all() const = 0;
                    const T& get(const ID& id) const {
                        static_assert(false, "To use 'get' on qs_manager with a vector implementation you must define 'bool ::core::qs_manager_implementation::operator==(const T&, const ID&)' function ");
                        }
                };
            
            
            // SFINAE comparison operator for 'bool operator==(const T&, const ID&)'
            struct dummy {};
            void operator==(const dummy&, const dummy&);

            // bool operator==(const int&, const std::string&); // This is detected. ¡namespaces!

            template <typename T, typename ID>
            struct is_operator_available {
                template <typename T1, typename ID1>
                static decltype(::core::utils::qs_manager_implementation::operator==(T1(), ID1())) test(int, int);
                template <typename, typename>
                static void test(...);

                enum { value = !std::is_void<decltype(test<T, ID>(0, 0))>::value};
                };
            
            
            template <class T, class ID>
            class qs_manager_getter_vector<T, ID,
                typename std::enable_if< is_operator_available<T, ID>::value >::type
                > {
                public:
                    virtual const std::vector<T>& all() const = 0;
                    virtual const size_t count() const {
                        return this->all().size();
                        };
                    const T& get(const ID& id) const {
                        // We have to iterate all elements to look for one (check if there are many)
                        const std::vector<T>& all = this->all();
                        std::vector<T>::const_iterator found = all.end(), begin = all.begin(), end = all.end();
                        while (begin != end) {
                            if ( ::core::utils::qs_manager_implementation::operator==(*begin,id)) {
                                if (found != all.end()) {
                                    throw multiple_objects_found(id);
                                    }
                                found = begin;
                                }
                            ++begin;
                            }
                        if (found == all.end()) {
                            throw object_not_found(id);
                            }
                        return *found;
                        };
                };
            

            // - all to vector
            template <class T, class ID, class ClassToTest>
            class qs_manager_get_impl<T, ID, ClassToTest,
                    typename std::enable_if<has_all_to_container<ClassToTest, std::vector<T> >::value>::type
                    > : public qs_manager_getter_vector<T, ID>
                {
                public:
                    qs_manager_get_impl<T, ID, ClassToTest,
                    typename std::enable_if<has_all_to_container<ClassToTest, std::vector<T> >::value>::type
                    >(const typename ClassToTest& container) : _container(container) {                        
                        };

                    void all(std::vector<T>& ret) const {
                        _container.all(ret);
                        };

                    const std::vector<T>& all() const {
                        _container.all(__all); // We have to update this vector each time because it may have been updated.
                        return __all;
                        };

                    //const T& __get(const ID& id) const {};
                protected:
                    const typename ClassToTest& _container;
                private:
                    mutable std::vector<T> __all;
                };

            // - all returns vector.
            template <class T, class ID, class ClassToTest>
            class qs_manager_get_impl<T, ID, ClassToTest,
                    typename std::enable_if<has_all_as_container<ClassToTest, std::vector<T> >::value>::type
                    > : public qs_manager_getter_vector<T, ID>
                {
                public:
                    qs_manager_get_impl<T, ID, ClassToTest,
                     typename std::enable_if<has_all_as_container<ClassToTest, std::vector<T> >::value>::type
                    >(const typename ClassToTest& container) : _container(container) {                        
                        };

                    void all(std::vector<T>& ret) const {
                        ret = _container.all();
                        };

                    const std::vector<T>& all() const {
                        return _container.all();
                        };

                protected:
                    const typename ClassToTest& _container;
                };

            // - container IS a vector
            template <class T, class ID>
            class qs_manager_get_impl<T, ID, typename std::vector<T>, void> : public qs_manager_getter_vector<T, ID> {
                public:
                    qs_manager_get_impl<T, ID, typename std::vector<T>, void>(const typename std::vector<T>& container) : _container(container) {
                        };

                    void all(std::vector<T>& ret) const {
                        ret = _container;
                        };

                    const std::vector<T>& all() const {
                        return _container;
                        };

                protected:
                    const typename std::vector<T>& _container;
                };

/**********************
    MAPS
***********************/
            // Map implementations
            template <class T, class ID>
            class qs_manager_getter_map {
                public:
                    virtual const size_t count() const = 0;

                    const T& get(const ID& id) const {
                        // There cannot be duplicates on maps
                        const std::map<ID, T>& all = this->__get_all();
                        std::map<ID, T>::const_iterator it = all.find(id);
                        if (it == all.end()) {
                            throw object_not_found(id);
                            }
                        return it->second;
                        };
                protected:
                    virtual const std::map<ID, T>& __get_all() const = 0;
                };

            // Credit: http://stackoverflow.com/questions/771453/copy-map-values-to-vector-in-stl
            template <typename tPair>
            struct second_t {
                typename tPair::second_type operator()(const tPair& p) const { return p.second;};
                };
            template <typename tMap>
            second_t<typename tMap::value_type> second(const tMap& m) { return second_t<typename tMap::value_type>();};
                    
            // - all to map
            template <class T, class ID, class ClassToTest>
            class qs_manager_get_impl<T, ID, ClassToTest,
                    typename std::enable_if<has_all_to_container<ClassToTest, std::map<ID, T> >::value>::type
                    > : public qs_manager_getter_map<T, ID>
                {
                public:
                    qs_manager_get_impl<T, ID, ClassToTest,
                    typename std::enable_if<has_all_to_container<ClassToTest, std::map<ID, T> >::value>::type
                    >(const typename ClassToTest& container) : _container(container) {                        
                        };

                    virtual const size_t count() const {
                        std::map<ID, T> map;
                        _container.all(map);
                        return map.size();
                        };

                    void all(std::vector<T>& ret) const {
                        std::map<ID, T> map;
                        _container.all(map);
                        std::transform( map.begin(), map.end(), std::back_inserter(ret), second(map));
                        };

                    const std::vector<T>& all() const {
                        std::map<ID, T> map;
                        _container.all(map); // We have to update this vector each time because it may have been updated.
                        __all.clear();
                        std::transform( map.begin(), map.end(), std::back_inserter(__all), second(map));
                        return __all;
                        };

                private:
                    const std::map<ID, T>& __get_all() const {
                        _container.all(__all_map);
                        return __all_map;
                        };

                protected:
                    const typename ClassToTest& _container;
                private:
                    mutable std::vector<T> __all;
                    mutable std::map<ID, T> __all_map;
                };

            // - all returns map.
            template <class T, class ID, class ClassToTest>
            class qs_manager_get_impl<T, ID, ClassToTest,
                    typename std::enable_if<has_all_as_container<ClassToTest, std::map<ID, T> >::value>::type
                    > : public qs_manager_getter_map<T, ID>
                {
                public:
                    qs_manager_get_impl<T, ID, ClassToTest,
                     typename std::enable_if<has_all_as_container<ClassToTest, std::map<ID, T> >::value>::type
                    >(const typename ClassToTest& container) : _container(container) {                        
                        };

                    virtual const size_t count() const {
                        return _container.all().size();
                        };

                    void all(std::vector<T>& ret) const {
                        const std::map<ID, T>& all = _container.all();
                        std::transform( all.begin(), all.end(), std::back_inserter(ret), second(all));
                        };

                    const std::vector<T>& all() const {
                        const std::map<ID, T>& all = _container.all();
                        __all.clear();
                        std::transform( all.begin(), all.end(), std::back_inserter(__all), second(all));
                        return __all;
                        };

                private:
                    const std::map<ID, T>& __get_all() const {
                        return _container.all();
                        };

                protected:
                    const typename ClassToTest& _container;
                private:
                    mutable std::vector<T> __all;
                };

            // - container IS a map
            template <class T, class ID>
            class qs_manager_get_impl<T, ID, typename std::map<ID, T>, void> : public qs_manager_getter_map<T, ID> {
                public:
                    qs_manager_get_impl<T, ID, typename std::map<ID, T>, void>(const typename std::map<ID, T>& container) : _container(container) {
                        };

                    virtual const size_t count() const {
                        return _container.size();
                        };

                    void all(std::vector<T>& ret) const {
                        std::transform( _container.begin(), _container.end(), std::back_inserter(ret), second(_container));
                        };

                    const std::vector<T>& all() const {
                        __all.clear();
                        std::transform( _container.begin(), _container.end(), std::back_inserter(__all), second(_container));
                        return __all;
                        };

                private:
                    const std::map<ID, T>& __get_all() const {
                        return _container;
                        };

                protected:
                    const typename std::map<ID, T>& _container;
                private:
                    mutable std::vector<T> __all;
                };

/**********************
    MULTIMAPS
***********************/

            }


/**********************
    JUST THE CLASS
***********************/

        template <class T, class TContainer, class ID=std::string>
        class qs_manager : public _private::qs_manager_get_impl<T, ID, TContainer> {
            public:
                //typedef typename core::smart_ptr<T>::shared _t_type_ptr; // Just for reference, use signal_ptr, connection_ptr,... instead
                typedef T _t_type_ptr;
                typedef std::vector<_t_type_ptr> _t_type_ptr_vector;

            public:
                qs_manager(const typename TContainer& container) : _private::qs_manager_get_impl<T, ID, TContainer>(container), _container(container) {};
                /*
                // Methods to retrieve ALL data
                virtual const _t_type_ptr_vector& all() const throw() {
                    #ifdef LOG_ERROR
                        LOG_ERROR("Not implemented");
                        static _t_type_ptr_vector __empty_vector;
                        return __empty_vector;
                    #else
                        throw std::runtime_exception("Not implemneted");
                    #endif
                    };
                */
                //
                virtual void test() const throw() {
                    std::cout << "void qs_manager::test()" << std::endl;
                    //this->get("");
                    //vector = this->all();
                    };
                
                /*
                // Methods to filter data
                virtual _t_type_ptr_vector filter(std::function<bool (const _t_type_ptr& ptr)> filter_func) const {
                    _t_type_ptr_vector results;
                    this->filter(filter_func, results);
                    return results;
                    };

                virtual int filter(std::function<bool (const _t_type_ptr& ptr)> filter_func, _t_type_ptr_vector& results) const {
                    int prev = results.size();
                    const _t_type_ptr_vector& allqs = this->all();
                    std::for_each(allqs.begin(), allqs.end(), [&results, &filter_func](const _t_type_ptr_vector::value_type& item) {
                        if (filter_func(item.second)) {
                            results.insert(item);
                            }
                        });
                    return (results.size()-prev);
                    };

                // Methods to get data
                virtual int get(const ID& id, _t_type_ptr& ptr) const throw() {
                    try {
                        ptr = this->get(id);
                        return 0;
                        }
                    catch (object_not_found&) {
                        return -1;
                        }
                    catch (multiple_objects_found& e) {
                        return -2;
                        }
                    };

                virtual const _t_type_ptr& get(const ID& id) const {
                    return _private::get<T, TContainer, ID>(id, *this);
                    };
                */
            protected:
                const typename TContainer& _container;
            };

        }
    }

                

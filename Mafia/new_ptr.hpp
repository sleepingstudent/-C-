#pragma once
#include <iostream>


template <typename T>
class NewPtr {
    T* ptr;
    size_t* count;
public: 
    NewPtr(T* sharedPtr = nullptr);
    NewPtr(const NewPtr &other);
    NewPtr(NewPtr &&other);
    ~NewPtr();
 
    void reset(T *other = nullptr);
    T* get() const noexcept;
    void swap(NewPtr &other) noexcept;
    size_t use_count();
 
    NewPtr<T>& operator=(const NewPtr &other) noexcept;
    T& operator*();
    T* operator->();
    bool operator==(const NewPtr &other) { return this->get() == other.get();}
};

template <typename T> 
NewPtr<T>::NewPtr(T* sharedPtr) { 
    // std::cout << "*************\n";

    this->ptr = sharedPtr;
    this->count = new size_t;
    *this->count = 1;
}

template <typename T> 
NewPtr<T>::~NewPtr() {
    // std::cout << "--------------\n";
    if (this->count && --*this->count <= 0) { 
        delete this->ptr;
        delete this->count;
 
        this->ptr = nullptr;
        this->count = nullptr;
    }
}

template <typename T> 
NewPtr<T>::NewPtr(const NewPtr<T> &other) { 
    // std::cout << "+++++++++++\n";
 
    this->ptr = other.ptr;
    this->count = other.count;
 
    if (this->count)
        ++*this->count;
}

template <typename T> 
NewPtr<T>::NewPtr(NewPtr<T> &&other) { 
    // std::cout << "/////////\n";
 
    this->ptr = other.ptr;
    this->count = other.count;

    other.ptr = nullptr;
    other.count = nullptr;
}

template <typename T> 
void NewPtr<T>::reset(T *other) {
    // std::cout << "reset()\n";
    if (other == nullptr && this->ptr == nullptr) {
        return;
    }
 
    if (this->ptr && --*this->count <= 0) {
        delete this->ptr;
        delete this->count;
    }
 
    this->ptr = other;
    if (this->ptr) {
        this->count = new size_t;
        *this->count = 1;
    }else {
        this->count = nullptr;
    }
}

template <typename T>
void NewPtr<T>::swap(NewPtr &other) noexcept {
    std::swap(this->ptr, other.ptr);
    std::swap(this->count, other.count);
}

template <typename T>
T* NewPtr<T>::get() const noexcept {
    return ptr;
}

template <typename T>
size_t NewPtr<T>::use_count() {
    return this->count ? *this->count : 0;
}

template <typename T>
NewPtr<T>& NewPtr<T>::operator=(const NewPtr<T> &other) noexcept {
    // std::cout << "------------\n";
    if (this->count != nullptr && --*this->count <= 0) {
        delete this->ptr;
        delete this->count;
    }
 
    this->ptr = other.ptr;
    this->count = other.count;
 
    if (this->count)
        ++*this->count;

    return *this;
}
 
template <typename T> 
T& NewPtr<T>::operator*() { 
    return *this->get(); 
}
 
template <typename T> 
T* NewPtr<T>::operator->() { 
    return this->get(); 
}
#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <iterator>

template <typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
        : buffer_(Allocate(capacity))
        , capacity_(capacity) {
    }

    RawMemory(const RawMemory&) = delete;

    RawMemory(RawMemory&& other) noexcept {
        buffer_ = std::move(other.GetAddress());
        capacity_ = std::move(other.Capacity());
    }

    RawMemory& operator=(const RawMemory& rhs) = delete;

    RawMemory& operator=(RawMemory&& rhs) noexcept {
        buffer_ = std::move(rhs.GetAddress());
        capacity_ = std::move(rhs.Capacity());
        return *this;
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    T* operator+(size_t offset) noexcept {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом вектора
        assert(offset <= capacity_);
        return buffer_+offset;
    }

    const T* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this)+offset;
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T* GetAddress() const noexcept {
        return buffer_;
    }

    T* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n) {
        return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept {
        operator delete(buf);
    }

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
    using iterator = T*;
    using const_iterator = const T*;

    Vector() = default;

    // Создаёт вектор из size элементов
    explicit Vector(size_t size)
        : data_(size)
        , size_(size)
    {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }  

    // Конструктор копирования
    Vector(const Vector& other)
        : data_(other.size_)
        , size_(other.size_)
    {
        std::uninitialized_copy_n(other.begin(), size_, data_.GetAddress());
    }

    // Конструктор перемещения
    Vector(Vector&& other) noexcept {
        Swap(other);
    }

    // Оператор присваивания копированием
    Vector& operator=(const Vector& rhs) {
        if (this != &rhs) {
            if (data_.Capacity() < rhs.size_) {
                Vector rhs_copy(rhs);
                Swap(rhs_copy);
            }
            else {
                if (rhs.size_ < size_) {
                    std::copy(rhs.data_.GetAddress(),
                              rhs.data_.GetAddress()+rhs.size_,
                              data_.GetAddress());

                    std::destroy_n(data_.GetAddress()+rhs.size_, size_-rhs.size_);
                }
                else {
                    std::copy(rhs.data_.GetAddress(),
                              rhs.data_.GetAddress()+size_,
                              data_.GetAddress());

                    std::uninitialized_copy_n(rhs.data_.GetAddress()+size_,
                                              rhs.size_-size_,
                                              data_.GetAddress()+size_);
                }
                size_ = rhs.size_;
            }
        }
        return *this;
    }

    // Оператор присваивания перемещением
    Vector& operator=(Vector&& rhs) noexcept {
        if (this != &rhs) {
            Swap(rhs);
        }
        return *this;
    }

    // Возвращает итератор на начало вектора
    iterator begin() noexcept {
        return iterator{data_.GetAddress()};
    }

    // Возвращает итератор на элемент, следующий за последним
    iterator end() noexcept {
        return iterator{data_.GetAddress()+size_};
    }

    // Возвращает константный итератор на начало вектора
    const_iterator begin() const noexcept {
        return const_iterator{data_.GetAddress()};
    }

    // Возвращает итератор на элемент, следующий за последним
    const_iterator end() const noexcept {
        return const_iterator{data_.GetAddress()+size_};
    }

    // Возвращает константный итератор на начало вектора
    const_iterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    const_iterator cend() const noexcept {
        return end();
    }

    // Обменивает значение с другим вектором
    void Swap(Vector& other) noexcept {
        data_.Swap(other.data_);
        std::swap(size_, other.size_);
    }

    // Возвращает количество элементов в векторе
    size_t Size() const noexcept {
        return size_;
    }

    // Возвращает вместимость вектора
    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    // Сообщает, пустой ли вектор
    bool IsEmpty() const noexcept {
        return (0 == size_);
    }

    // Резервирует место
    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) {
            return;
        }
        RawMemory<T> new_data(new_capacity);

        if constexpr (std::is_nothrow_move_constructible_v<T> ||
                     !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(begin(), size_, new_data.GetAddress());
        }
        else {
            std::uninitialized_copy_n(begin(), size_, new_data.GetAddress());
        }
        std::destroy_n(begin(), size_);
        data_.Swap(new_data);
    }

    // Возвращает ссылку на элемент с индексом index
    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    // Деструктор
    ~Vector() {
        std::destroy_n(begin(), size_);
    }

    // Изменяет размер вектора
    void Resize(size_t new_size) {
        if (new_size < size_) {
            std::destroy_n(begin()+new_size, size_-new_size);
            size_ = new_size;
        }
        else {
            Reserve(new_size);
            std::uninitialized_value_construct_n(end(), new_size-size_);
            size_ = new_size;
        }
    }

    // Добавляет элемент в конец вектора
    void PushBack(const T& value) {
        EmplaceBack(std::forward<const T&>(value));
    }

    // Перемещает элемент в конец вектора
    void PushBack(T&& value) {
        EmplaceBack(std::forward<T&&>(value));
    }

    // Перемещает значения args в конец вектора
    // Возвращает значение
    template<typename... Args>
    T& EmplaceBack(Args&&... args) {
        T* entry = nullptr;
        if (size_ == Capacity()) {
            RawMemory<T> new_data(IsEmpty() ? 1 : size_ * 2);
            entry = new (new_data+size_) T(std::forward<Args>(args)...);
            if constexpr (std::is_nothrow_move_constructible_v<T> ||
                         !std::is_copy_constructible_v<T>) {
                std::uninitialized_move_n(begin(), size_, new_data.GetAddress());
            }
            else {
                std::uninitialized_copy_n(begin(), size_, new_data.GetAddress());
            }
            std::destroy_n(begin(), size_);
            data_.Swap(new_data);
        }
        else {
            entry = new (data_+size_) T(std::forward<Args>(args)...);
        }
        ++size_;
        return *entry;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ > 0) {
            std::destroy_at(end()-1);
            --size_;
        }
    }

    // Перемещает значения args в позицию pos
    // Возвращает итератор
    template<typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args) {
        assert(pos >= begin() && pos <= end());
        size_t shift = pos-begin();
        iterator result = nullptr;
        if (size_ == Capacity()) {
            RawMemory<T> new_data(IsEmpty() ? 1 : size_ * 2);
            result = new (new_data+shift) T(std::forward<Args>(args)...);
            if constexpr (std::is_nothrow_move_constructible_v<T> ||
                         !std::is_copy_constructible_v<T>) {
                std::uninitialized_move_n(begin(),
                                          shift,
                                          new_data.GetAddress());

                std::uninitialized_move_n(begin()+shift,
                                          size_-shift,
                                          new_data.GetAddress()+shift+1);
            }
            else {
                std::uninitialized_copy_n(begin(),
                                          shift,
                                          new_data.GetAddress());

                std::uninitialized_copy_n(begin()+shift,
                                          size_-shift,
                                          new_data.GetAddress()+shift+1);
            }
            std::destroy_n(begin(), size_);
            data_.Swap(new_data);
        }
        else {
            if (size_ != 0) {
                if constexpr (std::is_nothrow_move_constructible_v<T> ||
                             !std::is_copy_constructible_v<T>) {
                    new (data_+size_) T(std::move(*(end()-1)));
                    std::move_backward(begin()+shift, end(), end()+1);
                }
                else {
                    new (data_+size_) T(std::move(*(end()-1)));
                    std::move_backward(begin()+shift, end(), end()+1);
                }
                std::destroy_at(begin()+shift);
            }
            result = new (data_+shift) T(std::forward<Args>(args)...);
        }
        ++size_;
        return result;
    }

    // Удаляет элемент вектора в указанной позиции
    iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>) {
        assert(pos >= begin() && pos < end());
        size_t index = pos-begin();
        std::move(begin()+index+1, end(), begin()+index);
        PopBack();
        return begin()+index;
    }

    // Вставляет значение value в позицию pos
    iterator Insert(const_iterator pos, const T& value) {
        return Emplace(pos, value);
    }

    // Перемещает значение value в позицию pos
    iterator Insert(const_iterator pos, T&& value) {
        return Emplace(pos, std::move(value));
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};

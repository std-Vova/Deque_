#include <iostream>
#include <exception>
#include <utility>
#include <type_traits>
#include <vector>
#include <initializer_list>
#include <iterator>
#include <algorithm>
#include <concepts>
#include "deque.h"
#include <stddef.h>
#include <cstddef>
#include <tuple>

template <typename Allocator>
template <typename T>
void allocator_traits_<Allocator>::destroy(Allocator& alloc, T* ptr) {
	if constexpr (has_method_destroy_v<Allocator, T*>) {
		alloc.destroy(ptr);
	}
	else {
		ptr->~T();
	}
}

template <typename Allocator>
void allocator_traits_<Allocator>::deallocate(Allocator& alloc, allocator_traits_<Allocator>::pointer ptr) {
	if constexpr (has_method_deallocate_v<Allocator, allocator_traits_<Allocator>::pointer>) {
		alloc.deallocate(ptr);
	}
	else {
		operator delete(ptr);
	}
}

template <typename Allocator>
auto allocator_traits_<Allocator>::allocate(Allocator& alloc, std::size_t count) -> allocator_traits_<Allocator>::pointer {
	if constexpr (has_method_allocate_v<Allocator, std::size_t>) {
		return alloc.allocate(count);
	}
	else {
		return ::operator new(sizeof(allocator_traits_<Allocator>::value_type) * count);
	}
}

template <typename T, typename... Args>
T* construct_at(T* ptr, Args&&... args) {
	if constexpr (std::is_array_v<T>) {
		return ::new(ptr) T[1];
	}
	else {
		return ::new(ptr) T(std::forward<Args>(args)...);
	}
}

template <typename Allocator> 
template <typename T, typename... Args>
void allocator_traits_<Allocator>::construct(Allocator& alloc, T* ptr, Args&&... args) {
	if constexpr (has_method_construct_v<Allocator, T*,  Args...>) {
		alloc.construct(ptr, std::forward<Args>(args)...);
	}
	else {
		construct_at(ptr, std::forward<Args>(args)...);
	}
}

template <typename T>
standart_allocator<T>::standart_allocator() = default;

template <typename T> template <typename U>
standart_allocator<T>::standart_allocator(standart_allocator<U>) {};

template <typename T>
T* standart_allocator<T>::allocate(std::size_t count) {
	return reinterpret_cast<T*>(operator new(sizeof(T) * count));
}

template <typename T>
void standart_allocator<T>::deallocate(T* ptr) {
	operator delete (ptr);
}

template <typename T> template <typename U, typename... Args>
void standart_allocator<T>::construct(U* ptr, Args&&... args) {
	new(ptr) U(std::forward<Args>(args)...);
}

template <typename T> template <typename U>
void standart_allocator<T>::destroy(U* ptr) {
	ptr->~U();
}

//ctor_for_allocation_t ctor_for_allocation_tag;

// Ctors base_iterator
template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>::base_iterator(): ptr(nullptr), arr(nullptr), indexes(std::make_pair(0,0)) {}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>::base_iterator(T* ptr, T** arr, std::size_t i, std::size_t j): ptr(ptr), arr(arr), indexes(std::make_pair(i, j)) {}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>::base_iterator(const base_iterator& other) : ptr(other.ptr), arr(other.arr), indexes(other.indexes) {}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>& Deque<T, Allocator>::base_iterator<is_const>::operator=(const base_iterator& other) {
	ptr = other.ptr;
	indexes = other.indexes;
	arr = other.arr;
	return *this;
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>& Deque<T, Allocator>::base_iterator<is_const>::operator++() {
	if (auto& [i, j] = indexes; j == bucket_size - 1) {
		++i;
		j = 0;
		ptr = &arr[i][j];
		return *this;
	}
	else {
		++indexes.second;
		ptr = &arr[indexes.first][indexes.second];
		return *this;
	}
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const> Deque<T, Allocator>::base_iterator<is_const>::operator++(int) {
	base_iterator temp = *this;
	if (auto& [i, j] = indexes; j == bucket_size - 1) {
		++i;
		j = 0;
		ptr = &arr[i][j];
		return temp;
	}
	else {
		++indexes.second;
		ptr = &arr[indexes.first][indexes.second];
		return temp;
	}
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>& Deque<T, Allocator>::base_iterator<is_const>::operator--() {
	if (auto& [i, j] = indexes; j == 0) {
		--i;
		j = bucket_size - 1;
		ptr = &arr[i][j];
		return *this;
	}
	else {
		--indexes.second;
		ptr = &arr[indexes.first][indexes.second];
		return *this;
	}
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const> Deque<T, Allocator>::base_iterator<is_const>::operator--(int) {
	base_iterator temp = *this;
	if (auto& [i, j] = indexes; j == 0) {
		--i;
		j = bucket_size - 1;
		ptr = &arr[i][j];
		return temp;
	}
	else {
		--indexes.second;
		ptr = &arr[indexes.first][indexes.second];
		return temp;
	}
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>::reference Deque<T, Allocator>::base_iterator<is_const>::operator*() const {
	return *ptr;
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>::pointer Deque<T, Allocator>::base_iterator<is_const>::operator->() const {
	return ptr;
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>& Deque<T, Allocator>::base_iterator<is_const>::operator+=(std::size_t count) {
	auto& [i, j] = indexes;
	std::size_t sum = j + count;
	std::size_t odd = sum / bucket_size;
	std::size_t even = sum % bucket_size;
	i += odd;
	j = even;
	ptr = &arr[i][j];
	return *this;
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>& Deque<T, Allocator>::base_iterator<is_const>::operator-=(std::size_t count) {
	auto& [i, j] = indexes;
	if (count < j) {
		j -= count;
		ptr = &arr[i][j];
		return *this;
	}
	else {
		size_t temp_j = bucket_size - j;
		size_t sum = temp_j + count;
		size_t odd = sum / bucket_size;
		size_t even = sum % bucket_size;
		i -= odd;
		j = bucket_size - even;
		ptr = &arr[i][j];
		return *this;
	}
}

// Operators base_iterator
template <typename T, typename Allocator>
template <bool is_const>
bool Deque<T, Allocator>::base_iterator<is_const>::operator==(const base_iterator& other) const {
	return (ptr == other.ptr);
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>::operator Deque<T, Allocator>::const_iterator() const{
	return base_iterator<true>(ptr);
}

template <typename T, typename Allocator>
template <bool is_const>
Deque<T, Allocator>::base_iterator<is_const>::reference Deque<T, Allocator>::base_iterator<is_const>::operator[](std::size_t count) const {
	Deque<T, Allocator>::base_iterator<is_const> temp = *this + count;
	return *temp;
}

// Ctors Deque
template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(ctor_for_allocation_t, std::size_t count): 
		arr(new T* [count](nullptr)), sz(0), cap(count), start(std::make_pair(count / 2, 0)), stop(std::make_pair(count / 2, 0)) {}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque() : Deque(ctor_for_allocation_tag, out_size) {}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> list) :
	Deque(ctor_for_allocation_tag, 
		[](std::size_t list_size) {
			if (std::size_t temp_len = (list_size % bucket_size == 0) ? list_size / bucket_size : list_size / bucket_size + 1; temp_len <= out_size) 
				return out_size; 
			else return temp_len; 
		}(list.size())) {
	std::size_t temp = 0;
	std::size_t k = 0;
	std::size_t index_i = 0;
	std::size_t count = (list.size()  % bucket_size == 0) ? list.size() / bucket_size : list.size() / bucket_size + 1;
	
	try {
		for (; index_i < count; ++index_i) {
			arr[start.first + index_i] = Alloc_traits::allocate(alloc, bucket_size);
			++sz;
			k = 0;
			for (; temp < list.size() && k < bucket_size; ++temp, ++k) {
				Alloc_traits::construct(alloc, &arr[start.first + index_i][k], *(list.begin() + temp));
			}
		}
	}
	catch (...) {
		std::size_t tmp = 0;
		std::size_t j = 0;
		std::size_t tmp_size = size();
		for (std::size_t i = 0; i < sz; ++i) {
			for (; tmp < tmp_size && j < bucket_size; ++tmp, ++tmp_size) {
				Alloc_traits::destroy(alloc, &arr[start.first + i][j]);
			}
			j = 0;
		}
		for (std::size_t i = 0; i < sz; ++i) {
			Alloc_traits::deallocate(alloc, arr[start.first + i]);
		}
		sz = 0;
		delete[] arr;
		throw;
	}
	stop.first += ((index_i == 0) ? index_i : index_i - 1);
	stop.second = k;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque& other): Deque(ctor_for_allocation_tag, other.cap) {
	std::size_t index_i = 0;
	std::size_t index_k = 0;
	std::size_t temp_size = 0;
	
	try {
		for (; index_i < other.sz; ++index_i) {
			arr[start.first + index_i] = Alloc_traits::allocate(alloc, bucket_size);
			++sz;
			index_k = 0;
			for (; temp_size < other.size() && index_k < bucket_size; ++index_k, ++temp_size) {
				Alloc_traits::construct(alloc, &arr[start.first + index_i][index_k],
					other[temp_size]);
			}
		}
	}
	catch (...) {
		std::size_t count = 0;
		std::size_t k = 0;
		for (std::size_t i = 0; i < sz; ++i) {
			for (; count < temp_size && k < bucket_size; ++count, ++k)
				Alloc_traits::destroy(alloc, &arr[start.first + i][k]);
			k = 0;
			Alloc_traits::deallocate(alloc, arr[start.first + i]);
		}
		sz = 0;
		throw;
	}

	stop.first += ((index_i == 0) ? 0 : index_i - 1);
	stop.second = index_k;
	alloc = other.alloc;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque&& other) noexcept : Deque(ctor_for_allocation_tag, other.cap) {
	Allocator newalloc = (Alloc_traits::propagate_on_container_move_assingment::value) ? other.alloc : alloc;

	start = other.start;

	for (std::size_t i = 0; i < other.sz; ++i) {
		arr[start.first + i] = other.arr[start.first + i];
	}
	for (std::size_t i = 0; i < other.sz; ++i) {
		other.arr[other.start.first + i] = nullptr;
	}
	sz = other.sz;
	alloc = newalloc;
	
	stop = other.stop;
	
	other.arr = nullptr;
	other.sz = 0;
	other.cap = 0;
	other.start = std::make_pair(0, 0);
	other.stop = std::make_pair(0, 0);
}

template <typename T, typename Allocator>
template <Input_iterator iter>
Deque<T, Allocator>::Deque(iter first, iter last): Deque(ctor_for_allocation_tag, out_size) {
	for (; first != last; ++first) {
		emplace_back(*first);
	}
}

// operators

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(const Deque& other) {
	if (this == &other)
		return *this;

	Allocator newalloc = (Alloc_traits::propagate_on_container_copy_assingment::value) ? other.alloc : alloc;

	T** newarr = new T*[other.cap](nullptr);

	std::size_t index_i = 0;
	std::size_t index_k = other.start.second;
	std::size_t temp_size = 0;
	start = other.start;
	stop = other.stop;

	try {
		for (; index_i < other.sz; ++index_i) {
			newarr[start.first + index_i] = Alloc_traits::allocate(alloc, bucket_size);
			for (; temp_size < other.size() && index_k < bucket_size; ++temp_size, ++index_k) {
				Alloc_traits::construct(alloc, &newarr[start.first + index_i][index_k], 
						other[temp_size]);
			}
			index_k = 0;
		}
	}
	catch (...) {
		std::size_t tmp_size = 0;
		for (std::size_t i = 0; i < index_i; ++i) {
			for (std::size_t k = 0; tmp_size < temp_size && k < bucket_size; ++tmp_size, ++k) {
				Alloc_traits::destroy(alloc, &newarr[start.first + i][k]);
			}
			Alloc_traits::deallocate(alloc, newarr[i]);
		}

		delete[] newarr;
		throw;
	}

	std::size_t cnt = 0;
	for (std::size_t i = 0; i < sz; ++i) {
		for (std::size_t k = 0; k < bucket_size && cnt < size(); ++k, ++cnt)
			Alloc_traits::destroy(alloc, &arr[start.first + i][k]);
		Alloc_traits::deallocate(alloc, arr[start.first + i]);
	}

	delete[] arr;
	arr = newarr;
	alloc = newalloc;
	sz = other.sz;
	cap = other.cap;
	return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(Deque&& other) noexcept {
	Allocator newalloc = (Alloc_traits::propagate_on_container_move_assingment::value) ? std::move(other.alloc) : alloc;
	if (this == &other)
		return *this;
	
	std::size_t temp_size = 0;
	for (std::size_t i = 0; i < sz; ++i) {
		for (std::size_t k = 0; k < bucket_size && temp_size < size(); ++k, ++temp_size) {
			Alloc_traits::destroy(alloc, &arr[start.first + i][k]);
		}
		Alloc_traits::deallocate(alloc, arr[start.first + i]);
	}
	delete[] arr;
	temp_size = 0;

	arr = new T* [other.cap];

	start = other.start;
	stop = other.stop;

	for (std::size_t i = 0; i < other.sz; ++i) {
		arr[start.first + i] = other.arr[other.start.first + i];
	}
	for (std::size_t i = 0; i < other.sz; ++i) {
		other.arr[other.start.first + i] = nullptr;
	}

	sz = other.sz;
	cap = other.cap;
	alloc = other.alloc;
	other.arr = nullptr;
	other.sz = 0;
	other.cap = 0;
	other.start = std::make_pair(0, 0);
	other.stop = std::make_pair(0, 0);
	return *this;
}

template <typename T, typename Allocator>
std::size_t Deque<T, Allocator>::size() const {
	std::size_t diff = stop.first - start.first;
	std::size_t temp = ((diff == 0) ? 0 : stop.first - start.first - 1) * bucket_size;
	std::size_t result = (diff == 0) ? temp + stop.second : temp + stop.second + (bucket_size - start.second);
	return result;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& value) {
	emplace_back(value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(T&& value) {
	emplace_back(std::move(value));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_back() {
	if (size() == 0)
		return;

	if (auto& [i, j] = stop; j == 0) {
		Alloc_traits::deallocate(alloc, arr[i]);
		j = bucket_size - 1;
		--i;
		--sz;
		Alloc_traits::destroy(alloc, &arr[i][j]);
	}
	else {
		--j;
		Alloc_traits::destroy(alloc, &arr[i][j]);
	}
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::add_bucket_back() {
	if (stop.first == cap - 1) {
		reserve(cap * 2);
	}

	++stop.first;
	stop.second = 0;

	arr[stop.first] = Alloc_traits::allocate(alloc, sizeof(T) * bucket_size);
	++sz;
}

template <typename T, typename Allocator> 
template <typename... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
	if (sz == cap) {
		reserve((cap == 0) ? 1 : cap * 2);
	}

	if (auto& [i, _] = stop; arr[i] == nullptr) {
		arr[i] = Alloc_traits::allocate(alloc, sizeof(T) * bucket_size);
		++sz;
	}
	if (auto& [_, j] = stop;j == bucket_size) {
		add_bucket_back();
	}

	Alloc_traits::construct(alloc, &(arr[stop.first][stop.second]), std::forward<Args>(args)...);
	++stop.second;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::reserve(std::size_t newcap) {
	if (newcap <= cap)
		return;

	T** newarr = new T*[newcap](nullptr);
	std::size_t new_start_i = newcap / 2;

	std::size_t index = 0;

	for (; index < sz; ++index) {
		newarr[new_start_i + index] = arr[start.first + index];
	}

	operator delete (arr);
	arr = newarr;
	cap = newcap;
	start.first = new_start_i;
	stop.first = new_start_i + (index == 0 ? index : index - 1);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::resize(std::size_t newsize, const T& value) {
	if (newsize <= size())
		return;

	std::size_t temp_cap = (newsize % bucket_size == 0) ? newsize / bucket_size : newsize / bucket_size + 1;
	
	if (temp_cap > cap)
		reserve(temp_cap * 2);

	std::size_t temp_size = size();

	std::size_t i = stop.second;
	std::size_t count = 0;
	
	if (arr[start.first] == nullptr) {
		arr[start.first] = Alloc_traits::allocate(alloc, bucket_size); // resize empty deque
		++sz;
	}
	
	while (count < newsize) {
		for (; temp_size < newsize && i < bucket_size; ++i, ++count, ++temp_size) {
			Alloc_traits::construct(alloc, &arr[stop.first][i], value);
		}
		if (temp_size < newsize) {
			i = 0;
			++stop.first;
			++sz;
			arr[stop.first] = Alloc_traits::allocate(alloc, bucket_size);
		}
	}
	stop.second = i;
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::operator[](std::size_t index) {
	std::size_t i = start.first + ((start.second + index) / bucket_size);
	std::size_t j = (start.second + index) - ((i - start.first) * bucket_size);

	return arr[i][j];
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::operator[](std::size_t index) const {
	std::size_t i = start.first + ((start.second + index) / bucket_size);
	std::size_t j = (start.second + index) - ((i - start.first) * bucket_size);

	return arr[i][j];
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::back() {
	return arr[stop.first][stop.second - 1];
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::back() const {
	return arr[stop.first][stop.second - 1];
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::front() {
	return arr[start.first][start.second];
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::front() const {
	return arr[start.first][start.second];
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& value) {
	emplace_front(value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
	if (size() == 0)
		return;

	if (auto& [i, j] = start; j == bucket_size - 1) {
		Alloc_traits::destroy(alloc, &arr[i][j]);
		Alloc_traits::deallocate(alloc, arr[i]);
		--sz;
		++i;
		j = 0;
	}
	else {
		Alloc_traits::destroy(alloc, &arr[i][j]);
		++j;
	}
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(T&& value) {
	emplace_front(std::move(value));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::add_bucket_front() {
	if (start.first == 0) {
		reserve(cap * 2);
	}
	--start.first;
	start.second = bucket_size;

	arr[start.first] = Alloc_traits::allocate(alloc, sizeof(T) * bucket_size);
	++sz;
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
	if (sz == cap) {
		reserve((cap == 0) ? 1 : cap * 2);
	}

	if (auto& [i, _] = start; arr[i] == nullptr) {
		arr[i] = Alloc_traits::allocate(alloc, sizeof(T) * bucket_size);
		++sz;
	}
	if (auto& [_, j] = start; j == 0) {
		add_bucket_front();
	}

	--start.second;
	Alloc_traits::construct(alloc, &(arr[start.first][start.second]), std::forward<Args>(args)...);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::swap(Deque& other) noexcept{
	Allocator newalloc = (Alloc_traits::propagate_on_container_swap::value) ? other.alloc : alloc;
	
	std::swap(arr, other.arr);
	std::swap(sz, other.sz);
	std::swap(cap, other.cap);
	start.swap(other.start);
	stop.swap(other.stop);
	alloc = newalloc;
}

template <typename T, typename Allocator>
std::size_t Deque<T, Allocator>::capacity() const {
	return sz * bucket_size;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::iterator Deque<T, Allocator>::begin() const {
	auto [i, j] = start;
	return { &arr[i][j], arr, i, j};
}

template <typename T, typename Allocator>
Deque<T, Allocator>::iterator Deque<T, Allocator>::end() const {
	auto [i, j] = stop;
	return { &arr[i][j], arr, i, j};
}

template <typename T, typename Allocator>
Deque<T, Allocator>::const_iterator Deque<T, Allocator>::cbegin() const {
	auto [i, j] = start;
	return { &arr[i][j], arr, i, j};
}

template <typename T, typename Allocator>
Deque<T, Allocator>::const_iterator Deque<T, Allocator>::cend() const {
	auto [i, j] = stop;
	return { &arr[i][j], arr, i, j};
}

template <typename T, typename Allocator>
Deque<T, Allocator>::reverse_iterator Deque<T, Allocator>::rbegin() const {
	return std::make_reverse_iterator(end());
}

template <typename T, typename Allocator>
Deque<T, Allocator>::const_reverse_iterator Deque<T, Allocator>::crbegin() const {
	return std::make_reverse_iterator(cend());
}

template <typename T, typename Allocator>
Deque<T, Allocator>::reverse_iterator Deque<T, Allocator>::rend() const {
	return std::make_reverse_iterator(begin());
}

template <typename T, typename Allocator>
Deque<T, Allocator>::const_reverse_iterator Deque<T, Allocator>::crend() const {
	std::make_reverse_iterator(cbegin());
}

template <typename T, typename Allocator>
bool Deque<T, Allocator>::empty() const {
	return (size() == 0);
}

template <typename T, typename Allocator>
template <bool c>
auto Deque<T, Allocator>::base_iterator<c>::operator-(const typename Deque<T, Allocator>::template base_iterator<c>& other) const ->
Deque<T, Allocator>::base_iterator<c>::difference_type {
	auto [i, j] = other.indexes;
	auto [i_, j_] = indexes;
	std::size_t diff = i_ - i;
	std::size_t temp = ((diff == 0) ? 0 : i_ - i - 1) * bucket_size;
	std::size_t result = (diff == 0) ? temp + j_ : temp + j_ + (bucket_size - j);
	return result;
}

template <typename T, typename Allocator>
template <bool c>
auto Deque<T, Allocator>::base_iterator<c>::operator+(int count) const -> Deque<T, Allocator>::base_iterator<c>{
	Deque<T, Allocator>::base_iterator<c> temp = *this;
	temp += count;
	return temp;
}

template <typename T, typename Allocator>
template <bool c>
bool Deque<T, Allocator>::base_iterator<c>::operator<(const typename Deque<T, Allocator>::template base_iterator<c>& other) const {
	return std::tie(indexes.first, indexes.second) < std::tie(other.indexes.first, other.indexes.second);
}

template <typename T, typename Allocator>
template <bool c>
auto Deque<T, Allocator>::base_iterator<c>::operator-(int count) const -> Deque<T, Allocator>::base_iterator<c> {
	Deque<T, Allocator>::base_iterator<c> temp = *this;
	temp -= count;
	return temp;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::~Deque() {
	size_t i = start.first;
	size_t j = start.second;

	for (; i < sz; ++i) {
		for (; j < bucket_size; ++j) {
			Alloc_traits::destroy(alloc, &arr[i][j]);
		}
		j = 0;
		Alloc_traits::deallocate(alloc, arr[i]);
	}
	delete[] arr;
}
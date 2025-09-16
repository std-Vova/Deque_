#pragma once
// type_traits

// get_pointer
namespace detail_5 {
	template <typename Allocator, typename value_type>
		requires requires(Allocator alloc) {
		typename Allocator::pointer;
	}
	typename Allocator::pointer test(void*);

	template <typename Allocator, typename value_type>
	value_type* test(...);
}

template <typename Allocator, typename value_type>
struct get_pointer : std::type_identity<decltype(detail_5::test<Allocator, value_type>(nullptr))> {};

template <typename Allocator, typename value_type>
using get_pointer_t = typename get_pointer<Allocator, value_type>::type;

// get_POCCA
namespace detail_has_POCCA {
	template <typename T>
		requires requires(T a) {
		typename T::propagate_on_container_copy_assignment;
	}
	auto test(void*) -> T::propagate_on_container_copy_assignment;

	template <typename T>
	std::false_type test(...);
}

template <typename T>
struct get_POCCA : decltype(detail_has_POCCA::test<T>(nullptr)) {};

template <typename T>
using get_POCCA_t = typename get_POCCA<T>::type;

// get_POCS
namespace detail_has_POCS {
	template <typename T>
		requires requires(T a) {
		typename T::propagate_on_container_swap;
	}
	auto test(void*) -> T::propagate_on_container_swap;

	template <typename T>
	std::false_type test(...);
}

template <typename T>
struct get_POCS : decltype(detail_has_POCS::test<T>(nullptr)) {};

template <typename T>
using get_POCS_t = typename get_POCS<T>::type;

// get_POCMA
namespace detail_6 {
	template <typename T>
		requires requires (T a) {
		typename T::propagate_on_container_move_assignment;
	}
	auto test(void*) -> T::propagate_on_container_move_assignmet;

	template <typename T>
	std::false_type test(...);
}

template <typename Alloc>
struct get_POCMA : decltype(detail_6::test<Alloc>(nullptr)) {};

template <typename Alloc>
using get_POCMA_t = typename get_POCMA<Alloc>::type;

// has_method_destroy 
namespace detail_4 {
	template <typename T, typename... Args>
	std::true_type test(decltype(std::declval<T>().destroy(std::declval<Args>()...), nullptr));

	template <typename T, typename... Args>
	std::false_type test(...);
}

template <typename T, typename... Args>
struct has_method_destroy : decltype(detail_4::test<T, Args...>(nullptr)) {};

template <typename T, typename... Args>
const bool has_method_destroy_v = has_method_destroy<T, Args...>::value;

//has_method_deallocate
namespace detail_3 {
	template <typename T, typename... Args>
	std::true_type test(decltype(std::declval<T>().deallocate(std::declval<Args>()...), nullptr));

	template <typename T, typename... Args>
	std::false_type test(...);
}

template <typename T, typename... Args>
struct has_method_deallocate : decltype(detail_3::test<T, Args...>(nullptr)) {};

template <typename T, typename... Args>
const bool has_method_deallocate_v = has_method_deallocate<T, Args...>::value;

// has_method_allocate
namespace detail_2 {
	template <typename T, typename... Args>
	std::true_type test(decltype(std::declval<T>().allocate(std::declval<Args>()...), nullptr));

	template <typename T, typename... Args>
	std::false_type test(...);
}

template <typename T, typename... Args>
struct has_method_allocate : decltype(detail_2::test<T, Args...>(nullptr)) {};

template <typename T, typename... Args>
const bool has_method_allocate_v = has_method_allocate<T, Args...>::value;

// has_method_construct
namespace detail {
	template <typename T, typename... Args>
	std::true_type test(decltype(std::declval<T>().construct(std::declval<Args>()...), nullptr));

	template <typename T, typename... Args>
	std::false_type test(...);
}

template <typename T, typename... Args>
struct has_method_construct : decltype(detail::test<T, Args...>(nullptr)) {};

template <typename T, typename... Args>
const bool has_method_construct_v = has_method_construct<T, Args...>::value;

// Standart allocator
template <typename T>
struct standart_allocator {
	standart_allocator();

	template <typename U>
	standart_allocator(standart_allocator<U>);

	using value_type = T;
	using pointer = T*;
	using propagate_on_container_copy_assignment = std::true_type;
	using propagate_on_container_move_assignment = std::true_type;

	T* allocate(std::size_t);
	void deallocate(T*);

	template <typename U, typename... Args>
	void construct(U*, Args&&...);

	template <typename U>
	void destroy(U*);

	template <typename U>
	struct rebind {
		using other = standart_allocator<U>;
	};
};

//Allocator_traits
template <typename Allocator>
struct allocator_traits_ {
	using allocator_type = Allocator;
	using value_type = Allocator::value_type;
	using pointer = get_pointer_t<Allocator, value_type>;

	using propagate_on_container_copy_assingment = get_POCCA_t<Allocator>;
	using propagate_on_container_move_assingment = get_POCMA_t<Allocator>;
	using propagate_on_container_swap = get_POCS_t<Allocator>;

	static pointer allocate(Allocator&, std::size_t);

	static void deallocate(Allocator&, pointer);

	template <typename T, typename... Args>
	static void construct(Allocator&, T*, Args&&...);

	template <typename T>
	static void destroy(Allocator&, T*);
};

// tag for ctor
struct ctor_for_allocation_t {};

// Concept Input_iterator
template <typename T>
concept Input_iterator = requires(T iter, T iter_) {
	typename T::value_type;
	{ *iter } -> std::same_as<typename std::iterator_traits<T>::value_type&>;
	++iter;
	iter != iter_;
};

template <typename T, typename Allocator = standart_allocator<T>>
class Deque {
public:
	T** arr;
	std::size_t sz;
	std::size_t cap;
	std::pair<std::size_t, std::size_t> start;
	std::pair<std::size_t, std::size_t> stop;
	[[no_unique_address]] Allocator alloc;

	using Alloc_traits = allocator_traits_<Allocator>;
	static constexpr std::size_t bucket_size = 32;
	static constexpr std::size_t out_size = 10;
public:
	Deque(ctor_for_allocation_t, std::size_t);

	template <bool is_const>
	struct base_iterator {
		using pointer = std::conditional_t<is_const, const T*, T*>;
		using reference = std::conditional_t<is_const, const T&, T&>;
		using value_type = T;
		using difference_type = int;
		using iterator_category = std::random_access_iterator_tag;

		friend class Deque;

		T* ptr;
		T** arr;
		std::pair<std::size_t, std::size_t> indexes;

		base_iterator();
		base_iterator(T*, T**, std::size_t i, std::size_t j);
		base_iterator(const base_iterator&);

		base_iterator& operator++();
		base_iterator operator++(int);

		base_iterator& operator--();
		base_iterator operator--(int);

		reference operator*() const;
		pointer operator->() const;

		base_iterator& operator+=(std::size_t);
		base_iterator& operator-=(std::size_t);

		reference operator[](std::size_t) const;

		bool operator==(const base_iterator&) const;
		bool operator<(const base_iterator&) const;

		operator base_iterator<true>() const;

		base_iterator& operator=(const base_iterator&);

		difference_type operator-(const base_iterator&) const;
		base_iterator operator-(int) const;
		base_iterator operator+(int) const;
	};

public:
	Deque();
	Deque(const Deque&);
	Deque(Deque&&) noexcept;
	Deque(std::initializer_list<T>);

	template <Input_iterator iter>
	Deque(iter, iter);

	void push_back(const T&);
	void push_back(T&&);

	void push_front(const T&);
	void push_front(T&&);

	void pop_back();
	void pop_front();

	template <typename... Args>
	void emplace_back(Args&&...);

	template <typename... Args>
	void emplace_front(Args&&...);

	void resize(std::size_t, const T&);
	void reserve(std::size_t);

	void swap(Deque&) noexcept;

	T& operator[](std::size_t);
	const T& operator[](std::size_t) const;

	Deque& operator=(const Deque&);
	Deque& operator=(Deque&&) noexcept;

	std::size_t size() const;
	std::size_t capacity() const;

	T& back();
	const T& back() const;

	T& front();
	const T& front() const;

	[[ nodiscard ]] bool empty() const;

	using value_type = T;
	using allocator_type = Allocator;
	using iterator = base_iterator<false>;
	using const_iterator = base_iterator<true>;
	using reverse_iterator = std::reverse_iterator<base_iterator<false>>;
	using const_reverse_iterator = std::reverse_iterator<base_iterator<true>>;
	using difference_type = typename std::iterator_traits<iterator>::difference_type;

	iterator begin() const;
	iterator end() const;

	const_iterator cbegin() const;
	const_iterator cend() const;

	reverse_iterator rbegin() const;
	reverse_iterator rend() const;

	const_reverse_iterator crbegin() const;
	const_reverse_iterator crend() const;

private:
	void add_bucket_front();
	void add_bucket_back();
};
 
// Deduction guides 
template <typename Iter>
Deque(Iter, Iter) -> Deque<typename std::iterator_traits<Iter>::value_type>;



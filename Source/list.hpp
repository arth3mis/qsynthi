// Arthur's list implementation (2022)
// v2: interpolation, more optimisation 
//

#pragma once

#include <vector>
#include <utility>
#include <numeric>
#include <algorithm>
#include <initializer_list>
#include <functional>
#include <ostream>
#include <sstream>
#include <stdexcept>

template<typename T>
class list
{
protected:
	std::vector<T> ls;


	template<typename T, typename U>
	class pair_list
	{
	private:
		const list<std::pair<T, U>> ls;

	public:
		pair_list(list a, list b) 
			: ls{ a.length != b.length ? list<std::pair<T, U>>() : 
                list<size_t>::range(0, a.length).mapTo<std::pair<T, U>>([&a, &b](size_t i) {
						return std::make_pair(a[i], b[i]);
				}) }
		{
			if (a.length() != b.length())
				throw std::runtime_error("Pairing error: list sizes differ");
		}

		pair_list(list a, list b, size_t s) 
			: ls{ list<size_t>::range(0, s).mapTo<std::pair<T, U>>([&a, &b](size_t i) {
						return std::make_pair(a[i], b[i]);
				}) }
		{}

		list<T> map(std::function<T(T, U)> f) const
		{
			std::vector<T> v;
			std::transform(ls.begin(), ls.end(), std::back_inserter(v),
				[&f](std::pair<T, U> p) {
					return f(p.first, p.second);
				});
			return list(v);
		}

		template<typename R>
		list<R> mapTo(std::function<R(T, U)> f) const
		{
			std::vector<R> v;
			std::transform(ls.begin(), ls.end(), std::back_inserter(v),
				[&f](std::pair<T, U> p) {
					return f(p.first, p.second);
				});
			return list<R>(v);
		}
	};


public:
	list()								: ls{ }
	{}
	list(std::initializer_list<T> il)	: ls{ std::vector<T>(il) }
	{}
	list(std::vector<T> v)				: ls{ v }
	{}
	list(size_t size)					: ls{ std::vector<T>(size) }
	{}
	list(size_t size, T value)			: ls{ std::vector<T>(size, value) }
	{}
	list(std::vector<T>::const_iterator it1, std::vector<T>::const_iterator it2)
										: ls{ std::vector<T>(it1, it2) }
	{}
	list(size_t size, std::function<T(size_t)> f)
										: ls{ rangePos(0, size).mapTo<T>([&f](size_t i) { return f(i); }).toVector() }
	{}

	// list containing incremented numbers
	static list<long long> range(long long a, long long b)
	{
		std::vector<long long> v(b - a);
		std::iota(v.begin(), v.end(), a);
		return list<long long>(v);
	}

	static list<size_t> rangePos(size_t a, size_t b)
	{
		std::vector<size_t> v(b - a);
		std::iota(v.begin(), v.end(), a);
		return list<size_t>(v);
	}

	// size queries
	inline size_t length() const { return ls.size(); }
	size_t empty()  const { return ls.size() == 0; }

	// iterators
	inline std::vector<T>::const_iterator begin() const { return ls.begin(); }
	inline std::vector<T>::const_iterator end()   const { return ls.end(); }

	// access function
	inline T get(size_t i) const { return ls[i]; }
	inline T getWrap(long long i) const { return ls[(i + length()) % length()]; }

	// a + b*x
	inline T getLinearInterpolation(float i) const 
	{ 
		return ls[floor(i)] + (ls[(size_t)ceil(i) % length()] - ls[floor(i)]) * (i - floor(i)); 
	}  

	// access operator
	T operator[](size_t index) const { return ls[index]; }

	// change value at index
	list set(size_t i, T value) const
	{
		std::vector<T> v = ls;
		v[i] = value;
		return list(v);
	}

	// add value to list end
	list append(T x) const
	{
		std::vector<T> v = ls;
		v.push_back(x);
		return list(v);
	}

	// add list to list end
	list append(list<T> l) const
	{
		std::vector<T> v = ls;
		v.insert(v.end(), l.begin(), l.end());
		return list(v);
	}

	// append value operator
	list operator+(const T& right) const
	{
		return append(right);
	}

	// append list operator
	list operator+(const list& right) const
	{
		return append(right);
	}

	// moves values through list (end-start/start-end transfer)
	list rotate(long long n=1) const
	{
		n = abs(n) % length() * /*sign function -->*/ (!signbit(n) * 2 - 1);
		std::vector<T> v = ls;
		if (n > 0)
		{
			v.erase(std::prev(v.end(), abs(n)), v.end());
			v.insert(v.begin(), std::prev(end(), abs(n)), end());
		}
		else if (n < 0)
		{
			v.erase(v.begin(), std::next(v.begin(), abs(n)));
			v.insert(v.end(), begin(), std::next(begin(), abs(n)));
		}
		return list(v);
	}

	// only keep first n elements
	list limit(size_t n) const
	{
		return list(begin(), std::next(begin(), n));
	}

	// execute function for each value
	void forEach(std::function<void(T)> f) const
	{
		std::for_each(begin(), end(), f);
	}

	list filter(std::function<bool(T)> f) const
	{
		std::vector<T> v;
		std::for_each(begin(), end(), [&f, &v](T a) mutable { if (f(a)) v.push_back(a); });
		return list(v);
	}

	// map each value to new value
	list map(std::function<T(T)> f) const
	{
		std::vector<T> v(length());
		std::transform(begin(), end(), v.begin(), f);
		return list(v);
	}

	// map each value to new value, also passing corresponding index
	list mapI(std::function<T(size_t, T)> f) const
	{
		std::vector<T> v = ls;
		size_t i = 0;
		std::for_each(v.begin(), v.end(), [&i, &f](T& a) mutable { a = f(i, a); i++; });
		return list(v);
	}

	// map each value to new value with new type
	template<typename U>
	list<U> mapTo(std::function<U(T)> f) const
	{
		std::vector<U> v(length());
		std::transform(begin(), end(), v.begin(), f);
		return list<U>(v);
	}

	// call type constructor with std::vector (needs implementation)
	template<typename U>
	U castTo() const
	{
		return U(*this);
	}

	// cast to std::vector
	std::vector<T> toVector() const
	{
		return ls;
	}

	// connects each value in two lists
	pair_list<T, T> zip(list<T> l) const
	{
		return pair_list<T, T>(*this, l);
	}

	// connects each value in two lists (can differ in type)
	template<typename U>
	pair_list<T, U> zip(list<U> l) const
	{
		return pair_list<T, U>(*this, l);
	}

	// reduces all values to one (starting from first value)
	T reduce(std::function<T(T, T)> f) const
	{
		if (length() == 0)
			throw std::runtime_error("Error: reducing empty list");

		T result = *begin();
		std::for_each(std::next(begin()), end(), [&f, &result](T a) mutable { result = f(result, a); });
		return result;
	}

	// reduces all values to one (starting from identity value)
	T reduce(T identity, std::function<T(T, T)> f) const
	{
		return reduceTo(identity, f);
	}

	// reduces all values to one (starting from identity value)
	template<typename U>
	U reduceTo(U identity, std::function<U(U, T)> f) const
	{
		U result = identity;
		std::for_each(begin(), end(), [&f, &result](T a) mutable { result = f(result, a); });
		return result;
	}

	// comparison
	bool equals(const list other) const
	{
		if (length() != other.length())
			return false;
		return zip(other)
			.mapTo<bool>([](T a, T b) { return a == b; })
			.reduce([](bool a, bool b) { return a && b; });
	}
	bool operator==(const list& right) const
	{
		return equals(right);
	}
	bool operator!=(const list& right) const
	{
		return !equals(right);
	}

	// representation
	std::string toString() const
	{
		std::ostringstream os;
		os << *this;
		return os.str();
	}

	// reassignment
	list& operator=(const list& right)
	{
		if (&right == this) return *this;  // no self-assignment
		ls = right.ls;
		return *this;
	}
};

template<class T>
std::ostream& operator<<(std::ostream& os, const list<T>& l)
{
	os << "[";
	for (size_t i = 0; i < l.length(); i++)
	{
		os << l[i];
		if (i + 1 < l.length())
			os << ",";
	}
	os << "]";
	return os;
}


template<typename T>
class mutable_list : public list<T>
{
private:
	std::vector<T> ls;

public:
	mutable_list() : list<T>()
	{
		ls = list<T>::toVector();
	}
	mutable_list(std::initializer_list<T> il) : list<T>(il)
	{
		ls = list<T>::toVector();
	}
	mutable_list(std::vector<T> v) : list<T>(v)
	{
		ls = list<T>::toVector();
	}
	mutable_list(size_t size) : list<T>(size)
	{
		ls = list<T>::toVector();
	}
	mutable_list(size_t size, T value) : list<T>(size, value)
	{
		ls = list<T>::toVector();
	}
	mutable_list(std::vector<T>::const_iterator it1, std::vector<T>::const_iterator it2)
		: list<T>(it1, it2)
	{
		ls = list<T>::toVector();
	}
	mutable_list(size_t size, std::function<T(size_t)> f)
		: list<T>(size, f)
	{
		ls = list<T>::toVector();
	}

	inline T& get(size_t i) { return ls.at(i); }
	inline T& getWrap(long long i) { return ls.at((i + length()) % length()); }
	T& operator[](size_t index) { return ls[index]; }

	inline size_t length() const { return ls.size(); }

	void forEach(std::function<void(T&)> f)
	{
		for (size_t i = 0; i < length(); i++)
			f(ls.at(i));
	}
};


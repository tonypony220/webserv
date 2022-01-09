#pragma once
#include <algorithm>

template <class T, class Func>
void ffor(T & arr, Func f) {
	std::for_each(arr.begin(), arr.end(), f);
}


//template<class InputIt, class f>
//constexpr f ffor_each(InputIt first, InputIt last, f f)
//{
//	for (; first != last; ++first) {
//		f(*first);
//	}
//	return f; // implicit move since C++11
//}
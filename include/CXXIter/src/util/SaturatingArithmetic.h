#pragma once

/** @private */
namespace CXXIter::util {
	
	//TODO: unit-tests
	
	/** @private */
	template<typename T>
	struct SaturatingArithmetic {
		T value;
		SaturatingArithmetic(T value) : value(value) {}
		T get() const { return value; }

		SaturatingArithmetic<T> operator+(T o) {
			T res = value + o;
			res |= -(res < value);
			return res;
		}
		SaturatingArithmetic<T> operator-(T o) {
			T res = value - o;
			res &= -(res <= value);
			return res;
		}
		SaturatingArithmetic<T> operator/(T o) {
			return value / o;
		}
	};
	
}

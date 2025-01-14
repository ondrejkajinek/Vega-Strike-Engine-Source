/*
 * resource.cpp
 *
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * Roy Falk, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "resource.h"

#include <algorithm>
#include <iostream>

/*
 * Constructors
 */
template<typename T>
Resource<T>::Resource(const T &value, const T &min_value, const T &max_value):
        value_(value),
        min_value_(min_value),
        max_value_(max_value),
        adjusted_max_value_(value),
        no_max_(max_value==-1) {}

/*
 * Methods
 */
template<typename T>
void Resource<T>::Downgrade(const T &value) {
    if(no_max_) {   // Can't downgrade if there's no max
        return;
    }

    adjusted_max_value_ = std::max(min_value_, adjusted_max_value_ - value);
}

template<typename T>
void Resource<T>::DowngradeByPercent(const T &value) {
    if(no_max_) {   // Can't downgrade if there's no max
        return;
    }

    adjusted_max_value_ = std::max(min_value_, adjusted_max_value_ - (max_value_ * value));
}

template<typename T>
T Resource<T>::Percent() const {
    if(no_max_) {   // Can't calculate percent if there's no max
        return -1;
    }

    return value_ / max_value_;
}

template<typename T>
void Resource<T>::ResetMaxValue() {
    if(no_max_) {   // Can't reset max if there's no max
        return;
    }

    adjusted_max_value_ = max_value_;
}

template<typename T>
void Resource<T>::Set(const T &value) {
    value_ = value;
    if(!no_max_) {
        value_ = std::min(max_value_, value_);
    }
    value_ = std::max(min_value_, value_);
}

template<typename T>
void Resource<T>::SetMaxValue(const T &value) {
    if(no_max_) {   // Can't set max if there's no max
        return;
    }

    adjusted_max_value_ = max_value_ = value;
}

template<typename T>
void Resource<T>::Upgrade(const T &value) {
    if(no_max_) {   // Can't upgrade max if there's no max
        return;
    }

    adjusted_max_value_ = std::min(max_value_, adjusted_max_value_ + value);
}

template<typename T>
void Resource<T>::UpgradeByPercent(const T &value) {
    if(no_max_) {   // Can't upgrade max if there's no max
        return;
    }

    adjusted_max_value_ = std::min(max_value_, adjusted_max_value_ + (max_value_ * value));
}

template<typename T>
T Resource<T>::Value() const {
    return value_;
}

template<typename T>
T Resource<T>::MaxValue() const {
    return max_value_;
}

template<typename T>
T Resource<T>::MinValue() const {
    return min_value_;
}

template<typename T>
T Resource<T>::AdjustedValue() const {
    return adjusted_max_value_;
}

template<typename T>
void Resource<T>::Zero() {
    value_ = adjusted_max_value_ = min_value_;
}

/*
 * Overloaded operators
 */

template<typename T>
Resource<T> Resource<T>::operator+=(const T &value) {
    if(!no_max_) {   // Only applicable if there's max
        value_ = std::min(value_ + value, max_value_);
    } else {
        value_ += value;
    }

    return *this;
}

template<typename T>
Resource<T> Resource<T>::operator-=(const T &value) {
    value_ = std::max(value_ - value, min_value_);
    return *this;
}


template<typename T>
Resource<T> Resource<T>::operator+=(T &value) {
    if(!no_max_) {   // Only applicable if there's max
        value_ = std::min(value_ + value, max_value_);
    } else {
        value_ += value;
    }

    return *this;
}

template<typename T>
Resource<T> Resource<T>::operator-=(T &value) {
    value_ = std::max(value_ - value, min_value_);
    return *this;
}

template<typename T>
bool operator==(const Resource<T> &lhs, const T &rhs) {
    return lhs.Value() == rhs;
}

template<typename T>
bool operator==(const T &lhs, const Resource<T> &rhs) {
    return lhs == rhs.Value();
}

template<typename T>
bool operator>(const Resource<T> &lhs, const T &rhs) {
    return lhs.Value() > rhs;
}

template<typename T>
bool operator<(const Resource<T> &lhs, const T &rhs) {
    return lhs.Value() < rhs;
}

template<typename T>
bool operator<=(const Resource<T> &lhs, const T &rhs) {
    return !(lhs.Value() > rhs);
}

template<typename T>
bool operator>=(const Resource<T> &lhs, const T &rhs) {
    return !(lhs.Value() < rhs);
}

template<typename T>
bool operator>(const T &lhs, const Resource<T> &rhs) {
    return lhs > rhs.Value();
}

template<typename T>
bool operator<(const T &lhs, const Resource<T> &rhs) {
    return lhs < rhs.Value();
}

template<typename T>
bool operator<=(const T &lhs, const Resource<T> &rhs) {
    return !(lhs > rhs.Value());
}

template<typename T>
bool operator>=(const T &lhs, const Resource<T> &rhs) {
    return !(lhs < rhs.Value());
}

template<typename T>
T operator/(const Resource<T> &lhs, const T &rhs) {
    return lhs.Value() / rhs;
}

template<typename T>
T operator/(const T &lhs, const Resource<T> &rhs) {
    return lhs / rhs.Value();
}

// Template Instantiation
// Must come after definition in clang!
template
class Resource<float>;

template bool operator==(const Resource<float> &lhs, const float &rhs);
template bool operator>(const Resource<float> &lhs, const float &rhs);
template bool operator<(const Resource<float> &lhs, const float &rhs);
template bool operator<=(const Resource<float> &lhs, const float &rhs);
template bool operator>=(const Resource<float> &lhs, const float &rhs);
template bool operator==(const float &lhs, const Resource<float> &rhs);
template bool operator>(const float &lhs, const Resource<float> &rhs);
template bool operator<(const float &lhs, const Resource<float> &rhs);
template bool operator<=(const float &lhs, const Resource<float> &rhs);
template bool operator>=(const float &lhs, const Resource<float> &rhs);
template float operator/(const Resource<float> &lhs, const float &rhs);
template float operator/(const float &lhs, const Resource<float> &rhs);




template
class Resource<double>;

template bool operator==(const Resource<double> &lhs, const double &rhs);
template bool operator>(const Resource<double> &lhs, const double &rhs);
template bool operator<(const Resource<double> &lhs, const double &rhs);
template bool operator<=(const Resource<double> &lhs, const double &rhs);
template bool operator>=(const Resource<double> &lhs, const double &rhs);
template bool operator==(const double &lhs, const Resource<double> &rhs);
template bool operator>(const double &lhs, const Resource<double> &rhs);
template bool operator<(const double &lhs, const Resource<double> &rhs);
template bool operator<=(const double &lhs, const Resource<double> &rhs);
template bool operator>=(const double &lhs, const Resource<double> &rhs);
template double operator/(const Resource<double> &lhs, const double &rhs);
template double operator/(const double &lhs, const Resource<double> &rhs);

template
class Resource<int>;

template bool operator==(const Resource<int> &lhs, const int &rhs);
template bool operator>(const Resource<int> &lhs, const int &rhs);
template bool operator<(const Resource<int> &lhs, const int &rhs);
template bool operator<=(const Resource<int> &lhs, const int &rhs);
template bool operator>=(const Resource<int> &lhs, const int &rhs);
template bool operator==(const int &lhs, const Resource<int> &rhs);
template bool operator>(const int &lhs, const Resource<int> &rhs);
template bool operator<(const int &lhs, const Resource<int> &rhs);
template bool operator<=(const int &lhs, const Resource<int> &rhs);
template bool operator>=(const int &lhs, const Resource<int> &rhs);
template int operator/(const Resource<int> &lhs, const int &rhs);
template int operator/(const int &lhs, const Resource<int> &rhs);

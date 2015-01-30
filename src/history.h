/*
 
* PROJECT:  AIModule
 
* VERSION:  0.06
 
* LICENSE:  GNU Lesser GPL v3 (../LICENSE.txt)
 
* AUTHOR:  (c) 2015 Eugene Zavidovsky
 
* LINK:  https://github.com/Eug145/TetrAI
 
*
 
*  This program is free software: you can redistribute it and/or modify
 
*  it under the terms of the GNU Lesser General Public License as 
*  published by
 the Free Software Foundation, either version 3 of 
*  the License, or
 (at your option) any later version.
 
*
 
*  This program is distributed in the hope that it will be useful,
 
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
 
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 
*  GNU Lesser General Public License for more details.
 
*
 
*  You should have received a copy of the GNU Lesser General Public
*  License
 along with this program. If not, see:
*  <http://www.gnu.org/licenses/>.
 
*/

#ifndef HISTORY_H
#define HISTORY_H

#include <QtGlobal>
#include <QVarLengthArray>
#include <cstddef>
#include <algorithm>

template <typename T>
class History
{
    int next_index;
    std::size_t size_max;
    QVarLengthArray<T> hi;

public:
    explicit History(std::size_t sz = 0);

    std::size_t size() const;
    std::size_t capacity() const;
    void reserve(std::size_t sz);
    QVarLengthArray<T> & straighten();
    void clear();

    template <typename TT>
    void append(TT && a);
    T & operator[](int t);
    T const & operator[](int t) const;
    T average() const;
};

template <typename T>
History<T>::History(std::size_t sz) :
    next_index {0}, size_max {sz}
{
    hi.reserve(sz);
}

template <typename T>
inline std::size_t History<T>::size() const
{
    return hi.size();
}

template <typename T>
inline std::size_t History<T>::capacity() const
{
    return size_max;
}

template <typename T>
inline void History<T>::reserve(std::size_t sz)
{
    size_max = sz;
    if (hi.size() <= sz) {
        hi.reserve(static_cast<int>(sz));
        return;
    }

    std::size_t diff {next_index - sz};
    if (diff < 0) {
        typename QVarLengthArray<T>::iterator ei {hi.end()};
        typename QVarLengthArray<T>::iterator di {hi.begin() + next_index};
        typename QVarLengthArray<T>::iterator si {ei + diff};
        std::move(si, ei, di);
    } else if (diff > 0) { //TODO if si may be equal ei, remove that diff check
        typename QVarLengthArray<T>::iterator ei {hi.begin() + next_index};
        typename QVarLengthArray<T>::iterator di {hi.begin()};
        next_index = qMin(sz, diff);
        typename QVarLengthArray<T>::iterator si {ei - next_index};
        std::move(si, ei, di);
    }
    hi.resize(static_cast<int>(sz)); hi.squeeze();
}

template <typename T>
QVarLengthArray<T> & History<T>::straighten()
{
    int size {hi.size()};
    if (next_index < size) {
        QVarLengthArray<T> straightened_hi {};
        T const * hi_data {hi.constData()};
        straightened_hi.append(hi_data + next_index, size - next_index);
        straightened_hi.append(hi_data, next_index);
        next_index = size;
        hi = std::move(straightened_hi);
    }
    return hi;
}

template <typename T>
inline void History<T>::clear()
{
    hi.clear(); next_index = 0;
}

template <typename T>
template <typename TT>
inline void History<T>::append(TT && a)
{
    if (hi.size() < size_max) {
        hi.append(std::forward<TT>(a));
        ++next_index;
    } else if (next_index < size_max) {
        hi[next_index] = std::forward<TT>(a);
        ++next_index;
    } else {
        Q_ASSERT(hi.size() > 0);
        hi[0] = std::forward<TT>(a); next_index = 1;
    }
}

template <typename T>
inline T & History<T>::operator[](int t)
{
    int i {next_index - 1 - t};
    if (i < 0) {
        i += hi.size();
        Q_ASSERT(i >= next_index);
    }
    return hi[i];
}

template <typename T>
inline T const & History<T>::operator[](int t) const
{
    return const_cast<History<T> * >(this)->operator[](t);
}

template <typename T>
inline T History<T>::average() const
{
    std::size_t sz {static_cast<std::size_t>(hi.size())};
    if (sz > 0) {
        return std::accumulate(hi.cbegin(), hi.cend(), 0)/sz;
    }
    return 0;
}

#endif // HISTORY_H

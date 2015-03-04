/*
 * PROJECT:  AIModule
 * VERSION:  0.07
 * LICENSE:  GNU Lesser GPL v3 (../LICENSE.txt, ../GPLv3.txt)
 * AUTHOR:  (c) 2015 Eugene Zavidovsky
 * LINK:  https://github.com/Eug145/TetrAI
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License 
 *  along with this program. If not, see: <http://www.gnu.org/licenses/>.
 */

#ifndef AIMODULE_A_INTF_BB_H
#define AIMODULE_A_INTF_BB_H

#include "history.h"
#include "aimodule_c.h"
#include <QVarLengthArray>
#include <QVector>
#include <QtGlobal>
#include <random>
#include <limits>

namespace AIModule {

extern std::default_random_engine reng;

template <typename T>
using Rsl = typename T::ResultType;

template <typename T>
class DaemonScheme
{
public:
    History<DaemonSchemeData<T>> data;
    Node<T> graph[T::graph_size_max];
    qint32 graph_size;

    qint32 strings[T::strings_number];

    qint32 result_inds[T::result_size];
    qint32 memory_inds[T::memory_size];

private:
    QVector<Rsl<T>> extract_result(DaemonSchemeData<T> & hs);
    void extract_memory(qint32 nodes_number);

public:
    DaemonScheme();

    void calculate_string(qint32 g_begin, qint32 g_end,
                          DaemonSchemeData<T> & hs);
    Rsl<T> mask(qint32 result);
    QVector<Rsl<T>> calculate_result(QVector<qint32> const args);

    void transform(DaemonScheme<T> const & sample);

    void update(QVector<qint32> const & args,
                qint32 start_string, qint32 nodes_number);

};

template <>
Rsl<SoulTraits> DaemonScheme<SoulTraits>::mask(qint32 result);
template <>
Rsl<AngelTraits> DaemonScheme<AngelTraits>::mask(qint32 result);

class SoulScheme : protected DaemonScheme<SoulTraits>
{
public:
    template <bool calculation_is_reduced>
    void update(QVector<qint32> const & args);
};

class AngelScheme : protected DaemonScheme<AngelTraits>
{
public:
    void enforce_plan_compliance(unsigned int history_time);
    quint32 calculate_instant(unsigned int history_time);

    template <bool args_are_reduced>
    void update(QVector<qint32> const & args);
};




template <typename T>
using Dmn = typename T::DaemonType;

template <typename T>
using Ptrs = QVarLengthArray<Dmn<T> *, T::cluster_size>;

template <typename T>
class Daemon
{
public:
    int ind;
    double age;

public:
    Daemon();

    void reset_bases(Dmn<T> * cluster);
};

class Soul : public Daemon<SoulTraits>, SoulScheme
{
    double wit_sum;
    double wit_sum_bases[SoulTraits::cluster_size - 1];

public:
    QVector<qint32> suggested_plan;
    quint64 splan_desirability;
    double wit;

private:
    void rebase(int base_index);
    using Daemon<SoulTraits>::reset_bases;

public:
    Soul();

    void evaluate_suggested_plan();
    void acknowledge_wit();

    double get_base_interval();
    void rate(int base_soul_ind, double const base_interval);
    void transform(Soul const & original);

    void update();

    friend void Daemon<SoulTraits>::reset_bases(Soul * cluster);
};

struct PortfolioData
{
    double count;
    double deviations_sum;
    double mean_deviation;
};

class Angel : public Daemon<AngelTraits>, AngelScheme
{
    QVector<unsigned int> projections_instants;

    unsigned int new_prj_count;

    QVector<PortfolioData> portfolio;
    QVector<PortfolioData> portfolio_bases[AngelTraits::cluster_size - 1];

public:
    double portfolio_integral;

private:
    quint32 get_deviation(unsigned int history_time);
    void update_portfolio(unsigned int history_time);

    void get_base_portfolio_integral(int const b_index, int const b_interval);
    void rebase(int base_index);

public:
    Angel();

    void evaluate_projections();

    int get_base_interval();
    void rate(int base_angel_ind, int const base_interval);
    void transform(Angel const & original);

    quint64 get_splan_desirability(QVector<qint32> splan);

    bool originate_projection();
    void update();

    friend void Daemon<AngelTraits>::reset_bases(Angel * cluster);
};

}

#endif // AIMODULE_A_INTF_BB_H

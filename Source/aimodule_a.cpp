/*
 * PROJECT:  AIModule
 * VERSION:  0.06-B001
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

#include "aimodule_a.h"

#include "qdeepcopy.h"
#include "aimodule_b_intf_ab.h"
#include <algorithm>
#include <functional>
#include <numeric>
#include <iterator>
#include <cstdlib>

namespace AIModule {

double const Consts::age_group_tolerance {0.97};
qint32 const Consts::operands_numbers[] {2, 2, 2, 2};

quint32 sensor_score;
QVector<qint32> sensor;

std::default_random_engine reng;
std::uniform_int_distribution<unsigned int> random_u_integer;

Angel angels[AngelTraits::cluster_size];
Ptrs<AngelTraits> angels_ptrs;
Soul souls[SoulTraits::cluster_size];
Ptrs<SoulTraits> souls_ptrs;

QVector<qint32> args_with_plan;
QVector<qint32> args_with_act;
Angel * best_angel;
Soul * best_soul;
QVector<qint32> current_plan;
qint32 current_act;

void init()
{
    sensor.resize(Consts::sensor_size);
    init_daemons_inds<AngelTraits>(angels);
    init_daemons_inds<SoulTraits>(souls);
    angels_ptrs = create_daemons_ptrs<AngelTraits>(&angels[0]);
    souls_ptrs = create_daemons_ptrs<SoulTraits>(&souls[0]);
    current_plan = QVector<qint32>(Consts::planning_interval,
                                   qint32{Consts::any_act});
}

void set_sensor(unsigned int score, int next_figure, bool new_game,
                int player_mode, int human_act, QVector<int> board)
{
    sensor_score = score;
    if (score <= 2147483647) {
        sensor[Consts::score_addr] = score;
    } else {
        sensor[Consts::score_addr] = ~static_cast<qint32>(score&0x7FFFFFFF);
    }
    sensor[Consts::next_figure_addr] = next_figure;
    sensor[Consts::player_mode_addr] = player_mode;
    sensor[Consts::human_act_addr] = human_act;
    if (new_game) {
        sensor[Consts::new_game_addr] = -1;
    } else {
        sensor[Consts::new_game_addr] = 0;
    }
    std::copy(board.cbegin(), board.cend(),
              sensor.begin() + Consts::sensor_a_size);
}

int do_ai_cycle()
{
    evaluate_angels_projections();
    evaluate_angels();

    evaluate_souls_plans();
    evaluate_souls();

    update_current_act();
    update_souls();
    update_angels();

    return current_act;
}

inline void evaluate_angels_projections()
{
    for (Angel & a : angels) {
        a.evaluate_projections();
    }
}

inline void Angel::evaluate_projections()
{
    QVector<unsigned int>::iterator si {projections_instants.begin()};
    QVector<unsigned int>::iterator ei {projections_instants.end()};
    while (si != ei) {
        if ((*si) <= Consts::projecting_interval) {
            enforce_plan_compliance(*si);
            update_portfolio(*si);
            ++(*si); ++si;
        } else {
            si = projections_instants.erase(si);
            ei = projections_instants.end();
        }
    }
}

inline void AngelScheme::enforce_plan_compliance(unsigned int history_time)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    qint32 * const prj_act {&data[history_time].
                            args[Consts::sensor_size - 1 + history_time]};
    if (*prj_act != Consts::any_act && *prj_act != current_act) {
        *prj_act = current_act;
    }
}

inline void Angel::update_portfolio(unsigned int t)
{
    //TODO try to use normalized deviation
    unsigned int dev {get_deviation(t)};
    if (--t >= static_cast<unsigned int>(portfolio.size())) {
        Q_ASSERT(static_cast<unsigned int>(portfolio.size()) == t);
        PortfolioData pr;
        pr.count = 1.0; pr.deviations_sum = pr.mean_deviation = dev;
        portfolio.append(pr);
    } else {
        //TODO update to GCC 4.9 and use lvalue reference instead of pointer
        PortfolioData * const pr {&portfolio[t]};
        //TODO try to avoid overflows here (also in "wit_sum", etc.)
        pr->deviations_sum += dev; ++pr->count;
        //TODO optimize, computing delta and correcting portfolio_integrals here
        pr->mean_deviation = pr->deviations_sum/pr->count;
    }
}

inline quint32 Angel::get_deviation(unsigned int history_time)
{
    quint32 scr {calculate_instant(history_time)};
    return (sensor_score < scr) ? scr - sensor_score : sensor_score - scr;
}

inline quint32 AngelScheme::calculate_instant(unsigned int history_time)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    DaemonSchemeData<AngelTraits> * const hs {&data[history_time]};
    qint32 index_begin {strings[history_time]};
    qint32 index_end {strings[history_time + 1]};
    calculate_string(index_begin, index_end, *hs);
    Q_ASSERT(index_begin < index_end);
    return mask(hs->results.at(index_end - 1));
}

inline void evaluate_angels()
{
    sort_ptrs_by_age<AngelTraits>(angels_ptrs);
    auto portfolio_comparing = [](Angel *& worse_a, Angel *& better_a) {
        return worse_a->portfolio_integral > better_a->portfolio_integral;
    };
    evaluate_in_age_groups<AngelTraits>(angels_ptrs, portfolio_comparing);
    get_best_angel(angels_ptrs);
}

template <typename T>
inline void sort_ptrs_by_age(Ptrs<T> & all_ptrs)
{
    auto age_comparing = [](Dmn<T> * const & a, Dmn<T> * const & b) {
        return a->age < b->age;
    };
    std::sort(all_ptrs.begin(), all_ptrs.end(), age_comparing);
}

template <typename T, typename R>
inline void evaluate_in_age_groups(Ptrs<T> const & all_ptrs_sorted,
                                   R & rating_comparing)
{
    Ptrs<T> age_grouped_ptrs {};
    typename Ptrs<T>::const_iterator si {all_ptrs_sorted.cbegin()};
    typename Ptrs<T>::const_iterator ei {all_ptrs_sorted.cend()};
    while (si != ei ) {
        age_grouped_ptrs.clear(); age_grouped_ptrs.append(*si);
        double group_age {(*si)->age};
        ++si;
        while (si != ei) {
            double current_age {(*si)->age};
            Q_ASSERT(current_age != 0);
            if (group_age/current_age < Consts::age_group_tolerance) {
                break;
            }
            age_grouped_ptrs.append(*si); ++si;
        }
        if (age_grouped_ptrs.size() > 1) {
            transform_daemons<T>(age_grouped_ptrs, rating_comparing);
        }
    }
}

template <typename T, typename R>
inline void transform_daemons(Ptrs<T> & age_grouped_ptrs, R & rating_comparing)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    Dmn<T> * const the_best {take_the_best_daemon<T>(age_grouped_ptrs,
                                                     rating_comparing)};
    for (Dmn<T> * const d : age_grouped_ptrs) {
        d->transform(*the_best);
    }
}

template <typename T, typename R>
inline Dmn<T> * take_the_best_daemon(Ptrs<T> & age_grouped_ptrs,
                                     R & rating_comparing)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    Dmn<T> * const base_d {age_grouped_ptrs.at(0)};
    int const base_daemon_ind {base_d->ind};
    auto const base_interval = base_d->get_base_interval();
    Q_ASSERT(base_interval > 0);
    for (Dmn<T> * const d : age_grouped_ptrs) {
        d->rate(base_daemon_ind, base_interval);
    }
    typename Ptrs<T>::iterator di {std::max_element(age_grouped_ptrs.begin(),
                                   age_grouped_ptrs.end(), rating_comparing)};
    Q_ASSERT(age_grouped_ptrs.size() > 0);
    Dmn<T> * the_best {*di}; age_grouped_ptrs.erase(di);
    return the_best;
}

inline int Angel::get_base_interval()
{
    return portfolio.size();
}

inline void Angel::rate(int base_angel_ind, int const base_interval)
{
    if (base_angel_ind != ind) {
        if (base_angel_ind > ind) { --base_angel_ind; }
        get_base_portfolio_integral(base_angel_ind, base_interval);
    } else {
        auto portfolio_accumulating = [](double v, PortfolioData const & r) {
            return v + r.mean_deviation;
        };
        portfolio_integral = std::accumulate(portfolio.cbegin(),
                                             portfolio.cend(),
                                             0.0, portfolio_accumulating);
    }
}

inline void Angel::get_base_portfolio_integral(int const b_index,
                                               int const b_interval)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    QVector<PortfolioData> * const pb {&portfolio_bases[b_index]};
    Q_ASSERT(pb->size() <= portfolio.size() && b_interval <= portfolio.size());
    portfolio_integral = 0;
    QVector<PortfolioData>::const_iterator si {portfolio.cbegin()};
    QVector<PortfolioData>::const_iterator ti {pb->cbegin()};

    auto portfolio_diff_integrating = [&](int interval) {
        QVector<PortfolioData>::const_iterator zi {si + interval};
        while (si != zi) {
            Q_ASSERT(si->count != ti->count);
            portfolio_integral += (si->deviations_sum - ti->deviations_sum)/
                                  (si->count - ti->count);
            ++ti; ++si;
        }
    };

    if (pb->size() < b_interval) {
        QVector<PortfolioData>::const_iterator ei {si + b_interval};
        portfolio_diff_integrating(pb->size());
        while (si != ei) {
            portfolio_integral += si->mean_deviation; ++si;
        }
    } else {
        portfolio_diff_integrating(b_interval);
    }
}

inline void Angel::transform(Angel const & original)
{
    AngelScheme::transform(original);
    portfolio.clear();
    projections_instants.clear();
    new_prj_count = 0;
    age = 0;
}

inline void get_best_angel(Ptrs<AngelTraits> const & all_ptrs)
{
    auto age_comparing = [](Angel * const & a, Angel * const & b) {
        return a->age < b->age;
    };
    Q_ASSERT(all_ptrs.size() > 0);
    best_angel = *std::max_element(all_ptrs.cbegin(), all_ptrs.cend(),
                                   age_comparing);
}

inline void evaluate_souls_plans()
{
    //TODO in multithread use local QVector (1)
    args_with_plan = create_args(AngelTraits::all_args_size);
    for (Soul & s : souls) {
        s.evaluate_suggested_plan();
    }
}

inline void Soul::evaluate_suggested_plan()
{
    suggested_plan = calculate_result(sensor);
    QVector<qint32>::const_iterator si {current_plan.cbegin()};
    QVector<qint32>::const_iterator ei {current_plan.cend()};
    QVector<qint32>::iterator di {suggested_plan.begin()};
    Q_ASSERT(current_plan.size() == suggested_plan.size());
    while (si != ei) {
        if (*di == Consts::any_act) {
            *di = *si;
        }
        ++di; ++si;
    }
    splan_desirability = best_angel->get_splan_desirability(suggested_plan);
}

inline quint64 Angel::get_splan_desirability(QVector<qint32> splan)
{
    //TODO in multithread use local QVector (2)
    copy_plan_to_args(splan, args_with_plan);
    QVector<quint32> prj {calculate_result(args_with_plan)};
    return std::accumulate(prj.cbegin(), prj.cend(), 0UL,
                           std::plus<quint64>{});
}

inline void evaluate_souls()
{
    get_best_soul(souls_ptrs);
    update_current_plan();
    sort_ptrs_by_age<SoulTraits>(souls_ptrs);
    auto wit_comparing = [](Soul *& worse_s, Soul *& better_s) {
        return worse_s->wit < better_s->wit;
    };
    evaluate_in_age_groups<SoulTraits>(souls_ptrs, wit_comparing);
}

inline void get_best_soul(Ptrs<SoulTraits> const & all_ptrs)
{
    auto splan_comparing = [](Soul * const & worse_s, Soul * const & better_s) {
        return worse_s->splan_desirability < better_s->splan_desirability;
    };
    Q_ASSERT(all_ptrs.size() > 0);
    Ptrs<SoulTraits>::const_iterator di {std::max_element(all_ptrs.cbegin(),
                                             all_ptrs.cend(), splan_comparing)};
    best_soul = *di; best_soul->acknowledge_wit();
}

inline void Soul::acknowledge_wit()
{
    ++wit_sum;
}

inline void update_current_plan()
{
    if (best_angel->get_splan_desirability(current_plan) <
                                                best_soul->splan_desirability) {
        current_plan = qDeepCopy(best_soul->suggested_plan);
    }
}

inline double Soul::get_base_interval()
{
    return age;
}

inline void Soul::rate(int base_soul_ind, double const base_interval)
{
    if (base_soul_ind != ind) {
        if (base_soul_ind > ind) { --base_soul_ind; }
        Q_ASSERT(base_interval > 0);
        wit = (wit_sum - wit_sum_bases[base_soul_ind])/base_interval;
    } else {
        wit = wit_sum/age;
    }
}

inline void Soul::transform(Soul const & original)
{
    SoulScheme::transform(original);
    wit_sum = 0.0;
    age = 0.0;
}

inline void update_souls()
{
    args_with_act = create_args(Consts::sensor_size + 1);
    args_with_act[Consts::sensor_size] = current_act;
    for (Soul & s : souls) {
        s.update();
    }
}

inline void Soul::update()
{
    reset_bases(&souls[0]);
    if (this == best_soul) {
        SoulScheme::update<true>(args_with_act);
    } else {
        SoulScheme::update<false>(args_with_act);
    }
    ++age;
}

inline void Soul::rebase(int base_index)
{
    wit_sum_bases[base_index] = wit_sum;
}

template <bool calculation_is_reduced>
inline void SoulScheme::update(QVector<qint32> const & args)
{
    qint32 ss {(calculation_is_reduced) ? strings[0] : 0};
    DaemonScheme<SoulTraits>::update(args, ss, graph_size);
}

inline void update_angels()
{
    for (Angel & a : angels) {
        a.reset_bases(&angels[0]);
    }
    //TODO in multithread create args_with_plan here (3)
    copy_plan_to_args(current_plan, args_with_plan);
    for (Angel & a : angels) {
        a.update();
    }
}

inline void Angel::rebase(int base_index)
{
    portfolio_bases[base_index] = qDeepCopy(portfolio);
    if (new_prj_count != 0) {
        random_buffer <<= Consts::bits_number_b;
        random_buffer |= new_prj_count; ++buffer_count;
        new_prj_count = 0;
    }
}

inline void Angel::update()
{
    if (originate_projection()) {
        AngelScheme::update<false>(args_with_plan);
    } else {
        AngelScheme::update<true>(args_with_act);
    }
    ++age;
}

template <bool args_are_reduced>
inline void AngelScheme::update(QVector<qint32> const & args)
{
    DaemonScheme<AngelTraits>::update(args, 0, strings[1]);
    if (!args_are_reduced) {
        data[1].results.resize(graph_size);
    }
}

inline bool Angel::originate_projection()
{
    if (new_prj_count > 0) {
        --new_prj_count;
        return false;
    } else {
        if (buffer_count == 0) { refresh_random_buffer(); }

        new_prj_count = random_buffer&~(~0U<<Consts::bits_number_b);
        random_buffer >>= Consts::bits_number_b; --buffer_count;

        projections_instants.append(1);
        return true;
    }
}

inline void Angel::refresh_random_buffer()
{
    random_buffer = random_u_integer(reng);
    buffer_count = Consts::bits_number_a/Consts::bits_number_b;
}

inline void update_current_act()
{
    Q_ASSERT(!current_plan.isEmpty());
    current_act = current_plan.takeFirst();
    current_plan.append(qint32{Consts::any_act});
}

inline Angel::Angel() :
    new_prj_count {0}, buffer_count {0}
{
    //TODO init history and scheme here ....
    //other angels may be not constructed yet, so bases cannot be reset here
    originate_projection();
    QVector<qint32> startup_args (AngelTraits::all_args_size,
                                  qint32{Consts::any_act});
    /* AngelScheme::update<false>(startup_args); */
}

inline Soul::Soul() :
    wit_sum {0.0}
{
    //TODO init history and scheme here ....
    //other souls may be not constructed yet, so bases cannot be reset here
    QVector<qint32> startup_args (SoulTraits::all_args_size,
                                  qint32{Consts::any_act});
    /* SoulScheme::update<false>(startup_args); */
}

template <typename T>
inline Daemon<T>::Daemon() :
    age {1}
{

}

template <typename T>
inline void Daemon<T>::reset_bases(Dmn<T> * si)
{
    if (age == 0) {
        Dmn<T> * ei {si + T::cluster_size};
        while (si != ei) {
            int base_index {ind};
            int current_ind {si->ind};
            if (current_ind != base_index) {
                if (base_index > current_ind) { --base_index; }
                si->rebase(base_index);
            }
            ++si;
        }
    }
}

template <typename T>
inline DaemonScheme<T>::DaemonScheme() :
    data {T::history_capacity}, graph_size {0},
    graph_size_n {T::all_args_size + T::memory_size},
    graph_size_nn {T::result_size + T::memory_size}
{
    Q_ASSERT(T::history_capacity >= 2);
}

template <typename T>
inline void DaemonScheme<T>::calculate_string(qint32 g_begin, qint32 g_end,
                                              DaemonSchemeData<T> & hs)
{
    Q_ASSERT(g_begin < T::strings_number && g_end < T::strings_number &&
             g_begin >= 0 && g_end >= 0 && g_begin <= g_end);
    Node<T> * si {&graph[g_begin]};
    Node<T> * ei {&graph[g_end]};
    while (si != ei) {
        //TODO register storage
        (si->*si->calculate)(hs);
        //TODO operand optimization: int ne {....}; si += ne;
        ++si;
    }
}

template <typename T>
inline QVector<Rsl<T> > DaemonScheme<T>::calculate_result(
                                                     QVector<qint32> const args)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    DaemonSchemeData<T> * const hs {&data[0]};
    hs->args = args;
    calculate_string(0, strings[0], *hs);
    calculate_string(strings[1], strings[T::strings_number - 1], *hs);
    return extract_result(*hs);
}

template <typename T>
inline QVector<Rsl<T> > DaemonScheme<T>::extract_result(
                                                       DaemonSchemeData<T> & hs)
{
    qint32 const * s_data {hs.results.constData()};
    QVector<Rsl<T>> result {};
    for (qint32 ind : result_inds) {
        result.append(mask(s_data[ind]));
    }
    return result;
}

template <>
inline SoulTraits::ResultType DaemonScheme<SoulTraits>::mask(qint32 result)
{
    return result&SoulTraits::result_mask;
}

template <>
inline AngelTraits::ResultType DaemonScheme<AngelTraits>::mask(qint32 result)
{
    return static_cast<AngelTraits::ResultType>(result);
}

template <typename T>
inline void DaemonScheme<T>::update(QVector<qint32> const & args,
                                    qint32 start_string, qint32 nodes_number)
{
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    DaemonSchemeData<T> * const hs {&data[0]};
    hs->args = args; calculate_string(start_string, strings[1], *hs);
    extract_memory(nodes_number);
}

template <typename T>
inline void DaemonScheme<T>::extract_memory(qint32 nodes_number)
{
    using S = DaemonSchemeData<T>;
    data.append(S{nodes_number});
    qint32 * di {&data[0].memory[0]};
    //TODO update to GCC 4.9 and use lvalue reference instead of pointer
    QVarLengthArray<int, 1> const * const s_data {&data[1].results};
    for (qint32 i {0}; i < T::memory_size; ++i) {
        *di = s_data->at(memory_inds[i]);
        ++di;
    }
}

template <typename T>
inline void DaemonScheme<T>::transform(DaemonScheme<T> const & sample)
{
    SchemeFeaturer<T> l {sample, this};
    l.make();
}

template <typename T>
inline DaemonSchemeData<T>::DaemonSchemeData(qint32 sz) :
    results {static_cast<int>(sz)}
{

}

inline QVector<qint32> create_args(qint32 args_size)
{
    QVector<qint32> args {qDeepCopy(sensor)};
    Q_ASSERT(static_cast<qint32>(sensor.size()) <= args_size);
    args.resize(args_size);
    return args;
}

inline void copy_plan_to_args(QVector<qint32> plan, QVector<qint32> & args)
{
    Q_ASSERT(Consts::sensor_size + plan.size() <=
             static_cast<qint32>(args.size()));
    std::copy(plan.cbegin(), plan.cend(),
              args.begin() + Consts::sensor_size);
}

template <typename T>
inline void init_daemons_inds(Dmn<T> daemons[])
{
    for (int i {0}; i < T::cluster_size; ++i) {
        daemons[i].ind = i;
    }
}

template <typename T>
inline Ptrs<T> create_daemons_ptrs(Dmn<T> * si)
{
    Ptrs<T> all_ptrs(T::cluster_size);
    typename Ptrs<T>::iterator di {all_ptrs.begin()};
    Dmn<T> * ei {si + T::cluster_size};
    while (si != ei) {
        *di = si; ++di; ++si;
    }
    return all_ptrs;
}

}

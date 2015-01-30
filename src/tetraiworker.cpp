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

#include "tetraiworker.h"

#include "aimodule.h"
#include "history.h"
#include "qdeepcopy.h"
#include <QtGlobal>
#include <QReadLocker>
#include <QCoreApplication>
#include <QMutexLocker>
#include <QThread>
#include <limits>
#include <algorithm>
#include <numeric>
#include <chrono>

TetraiWorker::TetraiWorker() :
    random_figure_index {0, 6}
{
    mr.score_history.reserve(Tetrai::score_history_size);
    mr.lines_count_history.reserve(Tetrai::lines_count_history_size);
    mr.nx = Tetrai::board_size_x; mr.ny = Tetrai::board_size_y;
    ny_one_third = Tetrai::board_size_y/3;
    ny_two_thirds = (Tetrai::board_size_y<<1)/3;
    mr.board.resize(Tetrai::board_size);
    figures_heap.resize(Tetrai::board_size);
    clump_mask.resize(Tetrai::board_size);
    next_figure = random_figure_index(reng);
}

TetraiWorker::~TetraiWorker()
{

}

void TetraiWorker::create()
{
    QMutexLocker locker {&mr.general};
    prepare_new_game_aux();
    refresh_next_figure_sprite();
    mr.score_history.clear(); mr.lines_count_history.clear();
    mr.info.overall_cycles_count = 0;
    mr.info.games_count = 0;
    mr.info.score_avg = 0;
    mr.info.lines_count_avg = 0;
    //TODO init AIModule here
    AIModule::init();
}

void TetraiWorker::load(QFile & fl)
{
    QMutexLocker locker {&mr.general};
    //TODO load AIModule and TetraiWorker's variables from QFile
}

void TetraiWorker::save(QFile & fl)
{
    QMutexLocker locker {&mr.general};
    //TODO save AIModule and TetraiWorker's variables to QFile
}

void TetraiWorker::prepare_new_game()
{
    QMutexLocker locker {&mr.general};
    prepare_new_game_aux();

    new_game_prepared();
}

inline void TetraiWorker::prepare_new_game_aux()
{
    mr.board.fill(0); figures_heap.fill(0);
    roll_next_figure();
    mr.game_over = false; new_game = true;
    mr.act = PlayerAct::none;
    mr.info.score = 0; mr.info.lines_count = 0;
    occupied_lines = 0;
}

inline void TetraiWorker::roll_next_figure()
{
    figure = next_figure;
    int f = random_figure_index(reng);
    if (next_figure != f) {
        next_figure = f;
        if (mr.speed != SpeedMode::fast) { refresh_next_figure_sprite(); }
    }
    figure_xpos = figure_start_xpos.at(figure) + (mr.nx>>1);
    figure_ypos = figure_start_ypos.at(figure);
    figure_rpos = 0;
    mr.dropping = mr.cleaning = heap_cleaner_dropping = false;
    heap_cleaner_scoreq = 0;
    mcycles_count = 1; mr.void_mcycle = false;
}

inline void TetraiWorker::refresh_next_figure_sprite()
{
    int x {0}, y {0}, xmax {0}, xmin {std::numeric_limits<int>::max()};
    QVector<int> coords {}; int f {(next_figure<<2)};
    QVector<int>::const_iterator si {figures.at(f).cbegin()};
    QVector<int>::const_iterator ei {figures.at(f).cend()};
    while (si != ei) {
        if (*si < 0) {
            ++y; x = 0;
        } else {
            x += *si; ++si;
            if (xmax < x) { xmax = x; }
            if (xmin > x) { xmin = x; }
            coords.append(x); coords.append(y); coords.append(*si);
        }
        ++si;
    }
    mr.next_figure_size.rwidth() = xmax = xmax + 1 - xmin;
    mr.next_figure_size.rheight() = ++y;
    mr.next_figure_sprite.clear(); mr.next_figure_sprite.resize(y*xmax);
    si = coords.cbegin();
    ei = coords.cend();
    while (si != ei) {
        x = *si - xmin; ++si; y = *si; ++si;
        mr.next_figure_sprite[y*xmax + x] = *si; ++si;
    }

    mr.next_figure_is_rolled = true;
}

void TetraiWorker::refresh_state_a()
{
    QMutexLocker locker {&mr.general};
    refresh_board();
    refresh_next_figure_sprite();

    state_refreshed_a();
}

void TetraiWorker::refresh_state_b()
{
    QMutexLocker locker {&mr.general};
    refresh_board();
    refresh_next_figure_sprite();

    state_refreshed_b();
}

void TetraiWorker::update_tetrai_smode()
{
    QMutexLocker locker {&mr.general};
    update_tetrai();

    tetrai_smode_updated();
}

void TetraiWorker::update_tetrai_fmode()
{
    QMutexLocker locker {&mr.general};
    while (mr.speed == SpeedMode::fast) {
        update_tetrai();

        locker.unlock();
        QCoreApplication::processEvents();
        locker.relock();
    }
}

inline void TetraiWorker::update_tetrai()
{
    if (!mr.game_over) {
        increment_cycles_count();
        if (mr.speed != SpeedMode::stop) {
            if (mr.void_mcycle) {
                increment_mcycles_count();
                do_void_mcycle();
            } else {
                increment_mcycles_count();
                do_inter_mcycle();
            }
        }
    }
    if (mr.game_over) {
        increment_gameover_counts();
        if (mr.player == PlayerMode::ai) {
            prepare_new_game_aux();
        } else {
            stop_tetrai(true);
        }
    }
}

inline void TetraiWorker::increment_gameover_counts()
{
    ++mr.info.games_count;
    mr.score_history.append(mr.info.score);
    mr.lines_count_history.append(mr.info.lines_count);
    mr.info.score_avg = mr.score_history.average();
    mr.info.lines_count_avg = mr.lines_count_history.average();
}

void TetraiWorker::stop_tetrai(bool stopsig)
{
    QMutexLocker locker {&mr.general};
    stop_tetrai_aux();

    tetrai_stopped(stopsig);
}

void TetraiWorker::stop_tetrai_blocking()
{
    QMutexLocker locker {&mr.general};
    stop_tetrai_aux();
}

inline void TetraiWorker::stop_tetrai_aux()
{
    if (mr.speed == SpeedMode::fast) {
        refresh_board();
        refresh_next_figure_sprite();
    }
    mr.speed = SpeedMode::stop;
}

inline void TetraiWorker::increment_cycles_count()
{
    if (mr.void_mcycle || mr.cleaning || mr.dropping) { return; }
    if (mr.info.cycles_count_max > 0) {
        if (mr.info.cycles_count < mr.info.cycles_count_max) {
            int cyc {mr.info.cycles_count}, cycm {mr.info.cycles_count_max};
            int p_a {cyc*100/cycm};
            int p_b {++cyc*100/cycm};
            if (p_a != p_b) { cycles_count_incremented(p_b); }
            ++mr.info.cycles_count;
            ++mr.info.overall_cycles_count;
        } else {
            cycles_count_incremented(100);
            stop_tetrai(true);
        }
    }
}

inline void TetraiWorker::increment_mcycles_count()
{
    if (mr.cleaning || mr.dropping) { return; }
    if (mcycles_count >= Tetrai::inter_mcycle_number) {
        mcycles_count = 0;
        mr.void_mcycle = true;
    } else {
        ++mcycles_count;
        mr.void_mcycle = false;
    }
}

inline void TetraiWorker::do_void_mcycle()
{
    if (mr.cleaning) {
        clean_figures_heap();
    } else if (mr.dropping && mr.speed == SpeedMode::fast) {
        while (move_down()) { ; }
    } else {
        move_down();
    }
}

inline void TetraiWorker::do_inter_mcycle()
{
    using namespace std::chrono;
    steady_clock::time_point t_a {steady_clock::now()};
    PlayerAct ai_act {do_ai_cycle()};
    steady_clock::time_point t_b {steady_clock::now()};
    mr.info.ai_module_delay = duration_cast<microseconds>(t_b - t_a).count();
    respite(); perform_player_act(ai_act);
}

inline PlayerAct TetraiWorker::do_ai_cycle()
{
    AIModule::set_sensor(mr.info.score, next_figure, new_game,
               static_cast<int>(mr.player), static_cast<int>(mr.act), mr.board);
    mr.general.unlock();

    // TODO
    /* PlayerAct ai_act {static_cast<PlayerAct>(AIModule::do_ai_cycle())};
    new_game = false; */
    std::uniform_int_distribution<> random_act {0, 5};
    QThread::usleep((random_act(reng)+1)*1000);
    PlayerAct ai_act {static_cast<PlayerAct>(random_act(reng))};

    mr.general.lock();
    return ai_act;
}

inline void TetraiWorker::perform_player_act(PlayerAct ai_act)
{
    if (mr.player == PlayerMode::ai) { mr.act = ai_act; }
    switch (mr.act) {
    case PlayerAct::any:
        break;
    case PlayerAct::none:
        break;
    case PlayerAct::rotate:
        rotate(); break;
    case PlayerAct::left:
        move_left(); break;
    case PlayerAct::right:
        move_right(); break;
    case PlayerAct::down:
        move_down(); break;
    case PlayerAct::drop:
        mr.void_mcycle = mr.dropping = true; move_down();
    }
    mr.act = PlayerAct::none;
}

inline void TetraiWorker::respite()
{
    if (mr.speed == SpeedMode::slow) {
        QThread::usleep(qMax(0L,
                             mr.smode_wait_interval - mr.info.ai_module_delay));
    } else {
        QThread::usleep(mr.info.ai_module_delay*mr.fmode_respite_ratio);
    }
}

inline bool TetraiWorker::rotate()
{
    int r {figure_rpos++};
    figure_rpos &= 3;
    bool c {check_collision()};
    if (c) {
        figure_rpos = r;
    } else if (mr.speed != SpeedMode::fast) {
        refresh_board();
    }
    return !c;
}

inline bool TetraiWorker::move_left()
{
    int x {figure_xpos--};
    bool c {check_collision()};
    if (c) {
        figure_xpos = x;
    } else if (mr.speed != SpeedMode::fast) {
        refresh_board();
    }
    return !c;
}

inline bool TetraiWorker::move_right()
{
    int x {figure_xpos++};
    bool c {check_collision()};
    if (c) {
        figure_xpos = x;
    } else if (mr.speed != SpeedMode::fast) {
        refresh_board();
    }
    return !c;
}

inline bool TetraiWorker::move_down()
{
    int y {figure_ypos++};
    bool c {check_collision()};
    if (c) {
        figure_ypos = y;
        add_figure_to_heap(); add_move_down_score();
        if (check_full_lines()) {
            mr.void_mcycle = mr.cleaning = true;
        } else if (figure_ypos == figure_start_ypos.at(figure)) {
            mr.game_over = true;
        } else {
            roll_next_figure();
        }
    } else if (mr.speed != SpeedMode::fast) {
        refresh_board();
    }
    return !c;
}

inline void TetraiWorker::add_move_down_score()
{
    occupied_lines = count_occupied_lines();
    if (occupied_lines <= ny_one_third) {
        mr.info.score += 2;
    } else if (occupied_lines <= ny_two_thirds) {
        ++mr.info.score;
    }
}

inline int TetraiWorker::count_occupied_lines()
{
    int oc {0};
    for (int i {mr.ny - 1}; i >= 0; --i) {
        int k {i*mr.nx}; bool empty {true};
        for (int j {0}; j < mr.nx; ++j) {
            if (figures_heap.at(k++) != 0) { empty = false; break; }
        }
        if (!empty) {
            ++oc;
        } else {
            break;
        }
    }
    return oc;
}

inline bool TetraiWorker::check_full_lines()
{
    full_lines.clear();
    for (int i {0}; i < mr.ny; ++i) {
        int k {i*mr.nx}; bool full {true};
        for (int j {0}; j < mr.nx; ++j) {
            if (figures_heap.at(k++) == 0) { full = false; break; }
        }
        if (full) { full_lines.append(i); }
    }
    return full_lines.size() > 0;
}

inline void TetraiWorker::clean_figures_heap()
{
    if (!heap_cleaner_dropping) {
        add_lines_score(); clean_full_lines(); find_clumps();
        heap_cleaner_dropping = true;
        if (mr.speed == SpeedMode::slow) {
            mr.board = qDeepCopy(figures_heap);
            return;
        }
    }
    if (mr.speed == SpeedMode::slow) {
        heap_cleaner_dropping = drop_clumps_smode();
        mr.board = qDeepCopy(figures_heap);
    } else {
        heap_cleaner_dropping = drop_clumps_fmode();
    }
    if (!heap_cleaner_dropping && !check_full_lines()) {
        occupied_lines = count_occupied_lines();
        roll_next_figure();
    }
}

inline void TetraiWorker::add_lines_score()
{
    int s {full_lines.size()};
    if (s > 0) {
        mr.info.lines_count += s;
        int k {s + 3 + heap_cleaner_scoreq};
        if (k > 28) { k = 28; }
        mr.info.score += 1<<k; heap_cleaner_scoreq += s<<1;
    }
}

inline void TetraiWorker::clean_full_lines()
{
    for (int line : full_lines) {
        QVector<int>::iterator si {figures_heap.begin() + line*mr.nx};
        QVector<int>::iterator ei {si + mr.nx};
        std::fill(si, ei, 0);
    }
}

inline void TetraiWorker::find_clumps()
{
    clump_mask.fill(0);
    clump_number = 0; clumps.clear(); clumps_borders.clear();
    for (int i {0}; i < figures_heap.size(); ++i) {
        if (figures_heap.at(i) != 0 && clump_mask.at(i) == 0) {
            clumps.append(++clump_number); clumps_borders.append(Borders{});
            mark_clump(i);
        }
    }
}

void TetraiWorker::mark_clump(int curr_i)
{
    clump_mask[curr_i] = clump_number;
    int k {curr_i - mr.nx};
    if (k >= 0 && figures_heap.at(k) != 0) {
        if (clump_mask.at(k) == 0) { mark_clump(k); }
    } else {
        clumps_borders[clump_number - 1].top.append(curr_i);
    }
    k = curr_i + mr.nx;
    if (k < figures_heap.size() && figures_heap.at(k) != 0) {
        if (clump_mask.at(k) == 0) { mark_clump(k); }
    } else {
        clumps_borders[clump_number - 1].bottom.append(curr_i);
    }
    k = curr_i - 1;
    if (curr_i%mr.nx != 0 && figures_heap.at(k) != 0 && clump_mask.at(k) == 0) {
        mark_clump(k);
    }
    k = curr_i + 1;
    if (k%mr.nx != 0 && figures_heap.at(k) != 0 && clump_mask.at(k) == 0) {
        mark_clump(k);
    }
}

inline bool TetraiWorker::drop_clumps_smode()
{
    QVector<int>::iterator si {clumps.begin()};
    QVector<int>::iterator ei {clumps.end()};
    while (si != ei) {
        if (check_clump_collision(*si)) {
            for (int & m : clump_mask) {
                if (m == *si) { m = 0; }
            }
            si = clumps.erase(si);
            ei = clumps.end();
        } else {
            ++si;
        }
    }
    if (clumps.size() == 0) { return false; }
    move_clumps_down();
    return true;
}

inline bool TetraiWorker::drop_clumps_fmode()
{
    while (drop_clumps_smode()) { ; }
    return false;
}

inline bool TetraiWorker::check_clump_collision(int clump_n)
{
    for (int b : clumps_borders.at(clump_n - 1).bottom) {
        int k = b + mr.nx;
        if (k >= figures_heap.size() || figures_heap.at(k) != 0) {
            return true;
        }
    }
    return false;
}

inline void TetraiWorker::move_clumps_down()
{
    int j {figures_heap.size() - 1}, i {j - mr.nx};
    while (i >= 0) {
        if (clump_mask.at(i) != 0) { figures_heap[j] = figures_heap.at(i); }
        --i; --j;
    }
    for (int k : clumps) {
        //TODO update to GCC 4.9 and use lvalue reference instead of pointer
        Borders * const k_border {&clumps_borders[k - 1]};
        for (int & t : k_border->top) {
            figures_heap[t] = 0;
            clump_mask[t] = 0;
            t += mr.nx;
        }
        for (int & b : k_border->bottom) {
            b += mr.nx;
            clump_mask[b] = k;
        }
    }
}

inline bool TetraiWorker::check_collision()
{
    auto checking = [this](int i, QVector<int>::const_iterator) {
        return figures_heap.at(i) == 0;
    };
    return scan_figure(checking);
}

inline void TetraiWorker::add_figure_to_heap()
{
    auto addition = [this](int i, QVector<int>::const_iterator si) {
        figures_heap[i] = *si;
        return true;
    };
    scan_figure(addition);
}

inline void TetraiWorker::refresh_board()
{
    mr.board = qDeepCopy(figures_heap);
    auto refreshment = [this](int i, QVector<int>::const_iterator si) {
        mr.board[i] = *si;
        return true;
    };
    scan_figure(refreshment);
}

template <typename ScanAction>
inline bool TetraiWorker::scan_figure(ScanAction a)
{
    int i {figure_ypos*mr.nx + figure_xpos};
    int f {(figure<<2) + figure_rpos};
    int iy {i}, ix {0};
    QVector<int>::const_iterator si {figures.at(f).cbegin()};
    QVector<int>::const_iterator ei {figures.at(f).cend()};
    while (si != ei) {
        if (*si < 0) {
            iy += mr.nx; i = iy; ix = 0;
        } else {
            ix += *si;
            int x {ix + figure_xpos};
            if (x < 0 || x >= mr.nx) { return true; }
            i += *si; ++si;
            if (i >= mr.board.size() || (i >= 0 && !a(i, si))) { return true; }
        }
        ++si;
    }
    return false;
}

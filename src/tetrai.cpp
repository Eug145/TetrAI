/*
 * PROJECT:  TetrAI
 * VERSION:  0.90
 * LICENSE:  GNU Lesser GPL v3 (../LICENSE.txt)
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
 
#include "tetrai.h"

#include "tetraiworker.h"
#include <QtGlobal>
#include <QTimer>
#include <QMutexLocker>
#include <QKeyEvent>
#include <QThread>

Tetrai::Tetrai(QObject * parent) :
    QObject(parent), tetrai_view {nullptr}, view_board {nullptr},
    next_fig_model {nullptr}, update_timer {new QTimer{this}},
    worker_thread {new QThread{this}}, tetrai_worker {new TetraiWorker{}}
{
    mr = &tetrai_worker->mr; tetrai_worker->moveToThread(worker_thread);

    chips.clear();
    chips.append(QImage{":/chips/0.png"});
    chips.append(QImage{":/chips/1.png"});
    chips.append(QImage{":/chips/2.png"});
    chips.append(QImage{":/chips/3.png"});
    chips.append(QImage{":/chips/4.png"});
    chips.append(QImage{":/chips/5.png"});
    chips.append(QImage{":/chips/6.png"});
    chips.append(QImage{":/chips/7.png"});

    set_slow_mode_interval(1000);
    set_fast_mode_interval(1000);
    update_timer->disconnect();
    update_timer->stop(); update_timer->setSingleShot(true);
    connect(update_timer, &QTimer::timeout, this, &Tetrai::update_tetrai_view);

    connect(this, &Tetrai::new_game_requested,
                                tetrai_worker, &TetraiWorker::prepare_new_game);
    connect(tetrai_worker, &TetraiWorker::new_game_prepared,
                                               this, &Tetrai::start_tetrai_aux);
    connect(this, &Tetrai::refresh_requested_a,
                                 tetrai_worker, &TetraiWorker::refresh_state_a);
    connect(tetrai_worker, &TetraiWorker::state_refreshed_a,
                                               this, &Tetrai::start_tetrai_aux);

    connect(this, &Tetrai::tetrai_smode_update_requested,
                             tetrai_worker, &TetraiWorker::update_tetrai_smode);
    connect(tetrai_worker, &TetraiWorker::tetrai_smode_updated,
                                       this, &Tetrai::update_tetrai_view_smode);
    connect(this, &Tetrai::tetrai_fmode_update_requested,
                             tetrai_worker, &TetraiWorker::update_tetrai_fmode);
    connect(this, &Tetrai::refresh_requested_b,
                                 tetrai_worker, &TetraiWorker::refresh_state_b);
    connect(tetrai_worker, &TetraiWorker::state_refreshed_b,
                                       this, &Tetrai::update_tetrai_view_fmode);

    connect(this, &Tetrai::tetrai_stop_requested,
                                     tetrai_worker, &TetraiWorker::stop_tetrai);
    connect(tetrai_worker, &TetraiWorker::tetrai_stopped,
                                                this, &Tetrai::stop_tetrai_aux);
    connect(this, &Tetrai::tetrai_blocking_stop_requested,
                             tetrai_worker, &TetraiWorker::stop_tetrai_blocking,
                                                  Qt::BlockingQueuedConnection);

    connect(this, &Tetrai::create_new_training_requested,
                                           tetrai_worker, &TetraiWorker::create,
                                                  Qt::BlockingQueuedConnection);
    connect(this, &Tetrai::load_training_requested,
                                             tetrai_worker, &TetraiWorker::load,
                                                  Qt::BlockingQueuedConnection);
    connect(this, &Tetrai::save_training_requested,
                                             tetrai_worker, &TetraiWorker::save,
                                                  Qt::BlockingQueuedConnection);

    connect(tetrai_worker, &TetraiWorker::cycles_count_incremented,
                                               this, &Tetrai::cycles_performed);

    worker_thread->start(QThread::NormalPriority);
}

Tetrai::~Tetrai()
{
    tetrai_worker->deleteLater();
    worker_thread->quit();
    worker_thread->wait();
}

void Tetrai::set_view(RectBoardView * rbv)
{
    tetrai_view = rbv;
    view_board = tetrai_view->get_view_board();

    tetrai_view->set_chips(&chips);

    QMutexLocker locker {&mr->general};
    *view_board = mr->board;
    QSize vbsz {mr->nx, mr->ny};
    tetrai_view->set_board_size(vbsz);
}

bool Tetrai::eventFilter(QObject *, QEvent * ev)
{
    if (ev->type() != QEvent::KeyPress) { return false; }
    QKeyEvent * key_ev {static_cast<QKeyEvent * >(ev)}; PlayerAct act;
    switch (key_ev->key()) {
    case Qt::Key_Left:
        act = PlayerAct::left; ev->accept(); break;
    case Qt::Key_Right:
        act = PlayerAct::right; ev->accept(); break;
    case Qt::Key_Up:
        act = PlayerAct::rotate; ev->accept(); break;
    case Qt::Key_Down:
        act = PlayerAct::down; ev->accept(); break;
    case Qt::Key_Space:
        act = PlayerAct::drop; ev->accept(); break;
    default:
        return false;
    }

    QMutexLocker locker {&mr->general};
    int t {update_timer->remainingTime()};
    if (mr->player == PlayerMode::ai || mr->speed == SpeedMode::stop || t <= 0
                                          || mr->game_over || mr->void_mcycle) {
        return false;
    }
    mr->act = act; update_timer->stop();
    if (mr->speed == SpeedMode::slow) {
        smode_interval_remains += t;
        tetrai_smode_update_requested();
    } else {
        refresh_requested_b();
    }
    return true;
}

void Tetrai::set_next_figure_model(NextFigureModel * nfm)
{
    next_fig_model = nfm;
    update_next_fig_model();
}

inline void Tetrai::update_next_fig_model()
{
    QMutexLocker locker {&mr->general};
    next_fig_model->update(mr->next_figure_size, mr->next_figure_sprite);
    mr->next_figure_is_rolled = false;
}

void Tetrai::set_slow_mode_interval(int smode_i)
{
    smode_interval = smode_i/(inter_mcycle_number + void_mcycle_number);
}

void Tetrai::set_fast_mode_interval(int fmode_i)
{
    fmode_interval = fmode_i;
}

void Tetrai::set_processing_power(int percent)
{
    QMutexLocker locker {&mr->general};
    float p {static_cast<float>(percent)};
    mr->fmode_respite_ratio = (100 - p)/p;
}

void Tetrai::set_player_mode(bool plm)
{
    QMutexLocker locker {&mr->general};
    if (plm) {
        mr->player = PlayerMode::human;
    } else {
        mr->player = PlayerMode::ai;
    }
}

void Tetrai::set_cycles_count_max(int cycm)
{
    QMutexLocker locker {&mr->general};
    mr->info.cycles_count_max = cycm;
    emit_cycles_performed(mr->info.cycles_count, cycm);
    game_info_changed(mr->info);
}

void Tetrai::reset_cycles_count()
{
    QMutexLocker locker {&mr->general};
    mr->info.cycles_count = 0;
    emit_cycles_performed(0, mr->info.cycles_count_max);
    game_info_changed(mr->info);
}

inline void Tetrai::emit_cycles_performed(int cyc, int cycm)
{
    if (cycm > 0) {
        cycles_performed(qMin(100, qMax(0, cyc*100/cycm)));
    } else {
        cycles_performed(0);
    }
}

int Tetrai::get_xsize()
{
    QMutexLocker locker {&mr->general};
    return mr->nx;
}

int Tetrai::get_ysize()
{
    QMutexLocker locker {&mr->general};
    return mr->ny;
}

QVector<QImage> * Tetrai::get_chips()
{
    return &chips;
}

bool Tetrai::is_game_over()
{
    QMutexLocker locker {&mr->general};
    return mr->game_over;
}

void Tetrai::start_tetrai(SpeedMode spd)
{
    QMutexLocker locker {&mr->general};
    if (spd == SpeedMode::stop || mr->speed == spd) { return; }
    update_timer->stop(); mr->speed = SpeedMode::stop; spe = spd;
    if (mr->info.cycles_count >= mr->info.cycles_count_max) {
        reset_cycles_count();
    }
    if (mr->game_over) {
        smode_interval_remains = 0;
        new_game_requested();
    } else {
        refresh_requested_a();
    }    
}

void Tetrai::start_tetrai_aux()
{
    QMutexLocker locker {&mr->general};
    mr->speed = spe;
    if (mr->speed == SpeedMode::slow) {
        update_tetrai_view_smode();
    } else {
        update_timer->setInterval(fmode_interval);
        update_tetrai_view_fmode();
        tetrai_fmode_update_requested();
    }
}

void Tetrai::update_tetrai_view()
{
    QMutexLocker locker {&mr->general};
    if (mr->speed == SpeedMode::slow) {
        tetrai_smode_update_requested();
    } else if (mr->speed == SpeedMode::fast) {
        refresh_requested_b();
    }
}

void Tetrai::update_tetrai_view_smode()
{
    QMutexLocker locker {&mr->general};
    update_tetrai_view_a();
    int t;
    if (mr->cleaning) {
        t = (smode_interval>>1)*(inter_mcycle_number + void_mcycle_number);
    } else if (mr->dropping) {
        t = smode_interval/dropping_iratio;
    } else if (mr->void_mcycle) {
        t = smode_interval*void_mcycle_number + smode_interval_remains;
        smode_interval_remains = 0;
    } else {
        int d {mr->info.ai_module_delay>>2};
        mr->smode_wait_interval = mr->info.ai_module_delay + d;
        t = smode_interval - static_cast<int>(mr->smode_wait_interval/1000);
        if (t < 0) {
            mr->smode_wait_interval = smode_interval; t = 0;
        }
    }
    update_timer->setInterval(t);
    update_timer->start();
}

void Tetrai::update_tetrai_view_fmode()
{
    update_tetrai_view_a();
    update_timer->setInterval(fmode_interval);
    update_timer->start();
}

void Tetrai::update_tetrai_view_a()
{
    QMutexLocker locker {&mr->general};

    if (mr->next_figure_is_rolled && next_fig_model != nullptr) {
        update_next_fig_model();
    }

    game_info_changed(mr->info);

    if (tetrai_view == nullptr) { return; }
    QRegion rg {}; int i {0};
    QVector<int>::const_iterator ti {view_board->cbegin()};
    QVector<int>::const_iterator si {mr->board.cbegin()};
    QVector<int>::const_iterator ei {mr->board.cend()};
    while (si != ei) {
        if (*ti != *si) { rg += tetrai_view->get_chip_rect(i); }
        ++i; ++si; ++ti;
    }
    *view_board = mr->board; tetrai_view->update(rg);
}

void Tetrai::stop_tetrai(bool stopsig)
{
    tetrai_stop_requested(stopsig);
}

void Tetrai::stop_tetrai_blocking(bool stopsig)
{
    tetrai_blocking_stop_requested();
    stop_tetrai_aux(stopsig);
}

void Tetrai::stop_tetrai_aux(bool stopsig)
{
    update_timer->stop();
    QMutexLocker locker {&mr->general};
    update_tetrai_view_a();
    if (stopsig) { game_stopped(); }
}

void Tetrai::create()
{
    QMutexLocker locker {&mr->general};
    if (mr->speed != SpeedMode::stop || update_timer->isActive()) { return; }
    smode_interval_remains = 0;
    reset_cycles_count();

    locker.unlock();
    create_new_training_requested();
    locker.relock();

    update_tetrai_view_a();
}

bool Tetrai::load(QFile & fl)
{
    QMutexLocker locker {&mr->general};
    if (mr->speed != SpeedMode::stop || update_timer->isActive()) {
        return false;
    }
    //TODO load smode_interval_remains, cycles_count, smode_interval and
    //     TetraiWorker's variables (by load_training_requested(fl)) from QFile
    //Note: do not forget to unlock mr->general before calling another thread
    return true;
}

void Tetrai::save(QFile & fl)
{
    QMutexLocker locker {&mr->general};
    if (mr->speed != SpeedMode::stop || update_timer->isActive()) { return; }
    //TODO save smode_interval_remains, cycles_count, smode_interval and
    //     TetraiWorker's variables (by save_training_requested(fl)) to QFile
    //Note: do not forget to unlock mr->general before calling another thread
}




NextFigureModel::NextFigureModel(Tetrai * gm) :
    QObject(gm), game {gm}, next_figure_view {nullptr}, next_figure_size {0, 0}
{
    game->set_next_figure_model(this);
}

NextFigureModel::~NextFigureModel()
{

}

void NextFigureModel::set_view(RectBoardView * rbv)
{
    next_figure_view = rbv;

    QVector<QImage> * chi {game->get_chips()};
    next_figure_view->set_chips(chi);
    next_figure_view->set_board_size(next_figure_size);

    chip_size.rwidth() = chi->at(0).width();
    chip_size.rheight() = chi->at(0).height();

    view_board = next_figure_view->get_view_board();
    update_view();
}

void NextFigureModel::update(QSize nfs, QVector<int> fig_spr)
{
    next_figure_size = nfs;
    next_figure_sprite = fig_spr;
    update_view();
}

inline void NextFigureModel::update_view()
{
    if (next_figure_view == nullptr) { return; }
    next_figure_view->set_board_size(next_figure_size);
    *view_board = next_figure_sprite;
    next_figure_view->update();
}

QSize NextFigureModel::get_largest_sprite_size()
{
    return QSize{Tetrai::largest_sprite_nx*chip_size.width(),
                                  Tetrai::largest_sprite_ny*chip_size.height()};
}

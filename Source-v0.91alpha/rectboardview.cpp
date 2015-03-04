/*
 * PROJECT:  TetrAI
 * VERSION:  0.91alpha
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
 
#include "rectboardview.h"

#include "tetrai.h"
#include "qdeepcopy.h"
#include <QPainter>
#include <QRegion>
#include <QResizeEvent>
#include <QPaintEvent>

RectBoardView::RectBoardView(RectBoardModel * mdl, QWidget * parent) :
    QWidget(parent), zeros_rendering {true}, board_origin {0, 0}
{
    mdl->set_view(this); model = mdl;
    scaled_chips = qDeepCopy(*chips); adjustSize();
}

RectBoardView::~RectBoardView()
{

}

void RectBoardView::resizeEvent(QResizeEvent * ev)
{
    QSize current_widget_size {ev->size()};
    QSize current_board_size {board_native_size.scaled(current_widget_size,
                                                          Qt::KeepAspectRatio)};
    int nx {n_size.width()}, ny {n_size.height()};
    int qx {current_board_size.width()/nx};
    int rx {current_board_size.width()%nx};
    int qy {current_board_size.height()/ny};
    int ry {current_board_size.height()%ny};
    board_origin.setX((current_widget_size.width() + rx
                                              - current_board_size.width())>>1);
    board_origin.setY((current_widget_size.height() + ry
                                             - current_board_size.height())>>1);
    current_chips_size = QSize{qx, qy};
    QVector<QImage>::iterator di {scaled_chips.begin()};
    QVector<QImage>::const_iterator si {chips->begin()};
    QVector<QImage>::const_iterator ei {chips->end()};
    while (si != ei) {
        *di = si->scaled(current_chips_size);
        ++si; ++di;
    }
}

void RectBoardView::paintEvent(QPaintEvent * ev)
{
    QWidget::paintEvent(ev);
    QPainter p(this);
    p.setClipRegion(ev->region());
    QPoint dp = board_origin;
    int i {0}, nx {n_size.width()}, ny {n_size.height()};
    for (int y {0}; y < ny; ++y) {
        for (int x {0}; x < nx; ++x) {
            int c {board.at(i++)};
            if (c != 0 || zeros_rendering) {
                p.drawImage(dp, scaled_chips.at(c));
            }
            dp.rx() += current_chips_size.width();
        }
        dp.rx() = board_origin.x();
        dp.ry() += current_chips_size.height();
    }
}

QSize RectBoardView::sizeHint() const
{
    return board_native_size;
}

void RectBoardView::set_board_size(QSize ns)
{
    n_size = ns;
    if (n_size.height() <= 0 || n_size.width() <= 0) {
        board_native_size = QSize{0, 0};
    } else {
        board_native_size = QSize{chips->at(0).width()*n_size.width(),
                                         chips->at(0).height()*n_size.height()};
    }
    updateGeometry();
}

void RectBoardView::set_chips(QVector<QImage> * ch)
{
     chips = ch;
}

QRect RectBoardView::get_chip_rect(int chip_num)
{
    int nx {n_size.width()};
    int y {chip_num/nx}, x {chip_num%nx};
    QPoint po {x*current_chips_size.width(), y*current_chips_size.height()};
    po += board_origin;
    return QRect{po, current_chips_size};
}

QVector<int> * RectBoardView::get_view_board()
{
    return &board;
}

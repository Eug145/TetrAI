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

#ifndef RECTBOARDVIEW_H
#define RECTBOARDVIEW_H

#include <QWidget>
#include <QVector>
#include <QImage>
#include <QSize>
#include <QPoint>
#include <QRect>

class Tetrai;
class QResizeEvent;
class QPaintEvent;
class RectBoardView;

class RectBoardModel
{
public:
    virtual void set_view(RectBoardView * bwidget) =0;
};




class RectBoardView : public QWidget
{
    Q_OBJECT

public:
    bool zeros_rendering;

private:
    RectBoardModel * model;
    QVector<QImage> * chips;

    QSize n_size;
    QVector<int> board;

    QSize board_native_size;
    QSize current_chips_size;
    QVector<QImage> scaled_chips;
    QPoint board_origin;

public:
    RectBoardView(RectBoardModel * mdl, QWidget * parent = 0);
    ~RectBoardView();

    void resizeEvent(QResizeEvent * ev) override;
    void paintEvent(QPaintEvent * ev) override;
    QSize sizeHint() const override;

    void set_board_size(QSize ns);
    void set_chips(QVector<QImage> * ch);
    QRect get_chip_rect(int chip_num);
    QVector<int> * get_view_board();
};

#endif // RECTBOARDVIEW_H

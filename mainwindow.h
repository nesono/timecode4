/************************************************************************\

                   Copyright 2009, Jochen Issing

    This is free software; you can redistribute it and/or modify it
    under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation; either version 2.1 of
    the License, or (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this software; if not, write to the Free
    Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
    02110-1301 USA, or see the FSF site: http://www.fsf.org.

\************************************************************************/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QString>
#include <QSettings>

#include "ui_mainwindow.h"
#include "ctimecodeitemgroup.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
  ~MainWindow();

  //! to enable stop flag and cancel button
  void enableStopFlag();
  //! function doing the actual processing (timecode insertion)
  void processImages();
  //! function to setup the preview
  void setupPreview();

public slots:
  //! function-slot to browse for the input directory
  void browseInputDir();
  //! function slot to browse for the output directory
  void browseOutputDir();
  //! function to change the font
  void changeFont();
  //! change text color
  void changeColor();
  //! change frame color
  void changeFrameColor();
  //! to update the text position
  void updateTextPosition();
  //! to set text position (from external)
  void setPositionInfo( qreal x, qreal y );
  //! function to process all images
  void process();
  //! functin to cancel a running job
  void setStopFlag();

private:
  //! the userinterface, created by uic
  Ui::MainWindowClass ui;
  //! the graphics scene for the preview
  QGraphicsScene *m_scene;
  //! the pixmap in preview
  QGraphicsPixmapItem *m_pixmap;
  //! the text for the counter in the preview
  QGraphicsSimpleTextItem *m_text;
  //! the preview rectangle
  QGraphicsPathItem *m_rectangle;
  //! group to apply movements to (text + rectangle)
  CTimecodeItemGroup *m_group;
  //! the status bar of the main window
  QStatusBar *m_statusBar;
  //! the stop flag for cancelling jobs
  bool m_stopflag;
  //! to remember settings from previous session
  QSettings m_settings;
};

#endif // MAINWINDOW_H

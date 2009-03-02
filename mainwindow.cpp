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


#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QFontDialog>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QColorDialog>
#include <QApplication>
#include <QProgressBar>

#include <sstream>

#include "mainwindow.h"

#define RECTALPHA 102

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
  : QMainWindow(parent, flags), m_scene( NULL ), m_pixmap( NULL ),
  m_text( NULL ), m_rectangle( NULL ), m_group( NULL ), m_stopflag( false ), m_settings( "nesono.com", "timecode" )
{
  ui.setupUi(this);

  // create a status bar

  m_statusBar = new QStatusBar( this );
  setStatusBar( m_statusBar );

  // show something in the status bar
  m_statusBar->showMessage( "welcome to timecode :)", 2000 );

  // get default application font
  QFont font = QApplication::font();
  font.setStyleHint( QFont::Courier );

  // set font name
  ui.ui_font_name->setText( font.family() );
  ui.ui_font_name->setFont( font );

  // set background role of color widget
  ui.ui_color->setBackgroundRole( QPalette::Base );
  ui.ui_color->setAutoFillBackground( true );
  // set background role of color widget
  ui.ui_frame_color->setBackgroundRole( QPalette::Base );
  ui.ui_frame_color->setAutoFillBackground( true );

  // load settings
  ui.ui_input_dir->setText( m_settings.value( "inputdir", ui.ui_input_dir->text() ).toString() );
  ui.ui_output_dir->setText( m_settings.value( "outputdir", ui.ui_output_dir->text() ).toString() );

  ui.ui_pos_x->setValue( m_settings.value( "pos_x", ui.ui_pos_x->value() ).toUInt() );
  ui.ui_pos_y->setValue( m_settings.value( "pos_y", ui.ui_pos_y->value() ).toUInt() );
  ui.ui_framerate->setValue( m_settings.value( "fps", ui.ui_framerate->value() ).toDouble() );

  // get defaut font color
  QPalette palette = ui.ui_color->palette();
  QColor color = m_settings.value( "font_color", palette.color( QPalette::Base ) ).value<QColor>();
  palette.setColor( QPalette::Base, color );
  ui.ui_color->setPalette( palette );

  // get defaut frame color
  palette = ui.ui_frame_color->palette();
  color = m_settings.value( "frame_color", palette.color( QPalette::Base ) ).value<QColor>();
  palette.setColor( QPalette::Base, color );
  ui.ui_frame_color->setPalette( palette );

  // get font
  ui.ui_font_name->setFont( m_settings.value( "font_name", ui.ui_font_name->font() ).value<QFont>() );
  ui.ui_font_name->setText( ui.ui_font_name->font().family() );

  // setup preview
  setupPreview();
}

MainWindow::~MainWindow()
{
  setStopFlag();
  // save settings
  m_settings.setValue( "inputdir", ui.ui_input_dir->text() );
  m_settings.setValue( "outputdir", ui.ui_output_dir->text() );

  m_settings.setValue( "pos_x", ui.ui_pos_x->value() );
  m_settings.setValue( "pos_y", ui.ui_pos_y->value() );
  m_settings.setValue( "fps", ui.ui_framerate->value() );

  // set defaut font color
  QPalette palette = ui.ui_color->palette();
  m_settings.setValue( "font_color", palette.color( QPalette::Base ) );

  // set defaut frame color
  palette = ui.ui_frame_color->palette();
  m_settings.setValue( "frame_color", palette.color( QPalette::Base ) );

  // set font
  m_settings.setValue( "font_name", ui.ui_font_name->font() );

  // delete pixmap/rectangle/text
  if( m_rectangle != NULL )
    delete m_rectangle;
  if( m_pixmap != NULL )
    delete m_pixmap;
  if( m_text != NULL )
    delete m_text;
}

// to enable stop flag and cancel button
void MainWindow::enableStopFlag()
{
  m_stopflag = false;
  ui.ui_cancel_button->setEnabled( true );
}

// function doing the actual processing (timecode insertion)
void MainWindow::processImages()
{
  m_statusBar->showMessage( "starting image processing..." );
  QApplication::processEvents();

  // check all images in current diretory
  QDir dir( ui.ui_input_dir->text() );
  dir.setFilter( QDir::Files );
  dir.setSorting( QDir::Name );

  // get the file info list from directory
  const QFileInfoList list = dir.entryInfoList();

  // go through file list
  QFileInfoList::const_iterator it = list.begin();

  QPixmap pixmap;
  // go through file list until an image has been found...
  enableStopFlag();
  // disable group being movable
  m_group->setFlag( QGraphicsItem::ItemIsMovable, false );

  while( pixmap.isNull() )
  {
    if( it == list.end() )
    {
      m_statusBar->showMessage( "no pictures found in input directory" );
      // reset the stop flag
      setStopFlag();
      // re-make group movable
      m_group->setFlag( QGraphicsItem::ItemIsMovable );
      QApplication::processEvents();
      // reset the stop flag
      return;
    }

    if( m_stopflag )
    {
      m_statusBar->showMessage("Job cancelled" );
      // reset the stop flag
      setStopFlag();
      // re-make group movable
      m_group->setFlag( QGraphicsItem::ItemIsMovable );
      return;
    }

    // get first picture width/height
    pixmap = QPixmap ( it->absoluteFilePath() );

    // check if we just opened a pixmap
    if( pixmap.isNull() )
    {
      // step ahead if no pixmap is opened
      it++;
      continue;
    }
  }
  // get size of first picture
  QSize picSize = pixmap.size();

  // time info variables
  unsigned int seqNo = 1;
  unsigned int hour, min, secs;
  unsigned int x, y, w, v;
  float        framerate;

  QFont font = ui.ui_font_name->font();
  unsigned int fontSize = (font.pixelSize() == -1 ? font.pointSize() : font.pixelSize());
  x = ui.ui_pos_x->value() + fontSize/4;
  y = ui.ui_pos_y->value() + fontSize + fontSize/16;

  // specify position of rounded rectangle
  v = ui.ui_pos_x->value();
  w = ui.ui_pos_y->value();

  unsigned int width, height;
  width  = m_text->boundingRect().width() + fontSize/2;
  height = m_text->boundingRect().height();

  qreal radius = fontSize/5.0;

  // get framerate from spin box
  framerate = ui.ui_framerate->value();

  // remember old pixmap
  QPixmap old_pixmap = m_pixmap->pixmap();
  // unset showing text
  m_text->hide();
  // hide rectangle
  m_rectangle->hide();

  // needed for progress bar
  unsigned int progress = 0;
  // the progress bar to show progress :)
  QProgressBar *progressBar = new QProgressBar();
  progressBar->setRange( 0, list.size() );
  // insert the progress bar
  m_statusBar->addPermanentWidget( progressBar );
  progressBar->show();

  for ( ; it != list.end(); it++, progress++ )
  {
    if( m_stopflag )
    {
      // remove progress bar
      m_statusBar->removeWidget( progressBar );
      delete progressBar;
      // reshow old pixmap
      m_pixmap->setPixmap( old_pixmap );
      // reshow text
      m_text->show();
      // reshow rectangle
      m_rectangle->show();
      // show status bar message
      m_statusBar->showMessage("Job cancelled" );

      // re-make group movable
      m_group->setFlag( QGraphicsItem::ItemIsMovable );

      return;
    }

    // apply progress bar value
    progressBar->setValue( progress );

    // calculate timecode
    QPixmap pixmap( it->absoluteFilePath() );

    // show status bar message
    m_statusBar->showMessage( QString( "processing: ") + it->baseName() );
    QApplication::processEvents();

    // get number of overall seconds:
    secs = static_cast<unsigned int>( seqNo / framerate );
    // get remaining frames in second
    unsigned int frame = static_cast<unsigned int>( seqNo - (secs*framerate ) );
    // get overall hours and subtract seconds in the hours from the secs
    hour = static_cast<unsigned int>( secs / 3600 );
    secs -= hour * 3600;
    // get overall minutes and subtract them from secs
    min = static_cast<unsigned int>( secs / 60 );
    secs -= min * 60;

    // create painter object
    QPainter painter( &pixmap );

    // get brush from painter
    QBrush brush = painter.brush();
    // get color from frame color widget
    QColor color = ui.ui_frame_color->palette().color( QPalette::Base );
    // set alpha value (to 40%)
    color.setAlpha( RECTALPHA );
    // set brushes color
    brush.setColor( color );
    // set to solid
    brush.setStyle( Qt::SolidPattern );
    // reapply brush
    painter.setBrush( brush );
    // reset pen
    painter.setPen( Qt::NoPen );

    // draw rounded rectangle
    painter.drawRoundedRect( v, w, width, height, radius, radius );

    // get font from font-widget and set it
    painter.setFont(  font );

    // get color from color widget and set it
    painter.setPen( ui.ui_color->palette().color( QPalette::Base ) );

    // create a string for the timecode
    std::stringstream tcstring;
    tcstring.fill( '0' );
    tcstring.width( 2 );
    tcstring << hour << ":";
    tcstring.width( 2 );
    tcstring << min << ":";
    tcstring.width( 2 );
    tcstring << secs << ".";
    tcstring.width( 2 );
    tcstring << frame;
    painter.drawText( x, y, tcstring.str().c_str()
                      /*QString( "%1:%2:%3.%4" ).arg( hour,2 ).arg( min,2 ).arg( secs,2 ).arg( frame,2 )*/ );

    pixmap.save( ui.ui_output_dir->text() +"/" + it->baseName() + ".png", "PNG", 100 );

    // show preview (every second)
    if( seqNo % static_cast<int>( framerate ) == 1 )
      m_pixmap->setPixmap( pixmap );

    // advance sequence (frame) number
    seqNo++;
  }

  // remove progress bar
  m_statusBar->removeWidget( progressBar );
  delete progressBar;

  // reset the stop flag
  setStopFlag();
  // re-make group movable
  m_group->setFlag( QGraphicsItem::ItemIsMovable );

  // reshow old pixmap
  m_pixmap->setPixmap( old_pixmap );
  // reshow text
  m_text->show();
  // reshow rectangle
  m_rectangle->show();
  // show status bar message
  m_statusBar->showMessage( "processing finished" );
}

// function to setup preview
void MainWindow::setupPreview()
{
  // check all images in current diretory
  QDir dir( ui.ui_input_dir->text() );
  dir.setFilter( QDir::Files );
  dir.setSorting( QDir::Name );

  // get the file info list from directory
  const QFileInfoList list = dir.entryInfoList();

  // go through file list
  QFileInfoList::const_iterator it = list.begin();

  QPixmap pixmap;
  // go through file list until an image has been found...
  while( pixmap.isNull() && it != list.end() )
  {
    if( m_stopflag )
    {
      m_statusBar->showMessage("Job cancelled", 5000 );
      return;
    }

    // get first picture width/height
    pixmap = QPixmap ( it->absoluteFilePath() );

    // check if we just opened a pixmap
    if( pixmap.isNull() )
    {
      // step ahead if no pixmap is opened
      it++;
      continue;
    }

    // get size of first picture
    QSize picSize = pixmap.size();

    // set dimensions
    ui.ui_pos_x->setMaximum( picSize.width() );
    ui.ui_pos_y->setMaximum( picSize.height() );

    // show pixmap
    if( m_scene == NULL )
      m_scene = new QGraphicsScene( pixmap.rect(), this );
    else
      m_scene->setSceneRect( pixmap.rect() );

    // check pixmap
    if( m_pixmap == NULL )
      m_pixmap = m_scene->addPixmap( pixmap );
    else
      m_pixmap->setPixmap( pixmap );


    // show text
    if( m_text == NULL )
      m_text = m_scene->addSimpleText( "03:22:43.04", ui.ui_font_name->font() );
    else
      m_text->setFont( ui.ui_font_name->font() );

    // helper variables
    int x, y;
    // more helper variables
    QFont font = ui.ui_font_name->font();
    unsigned int fontSize = (font.pixelSize() == -1 ? font.pointSize() : font.pixelSize());

    // calc text position
    x = fontSize/4;
    y = fontSize/16;

    // move text regarding position
    m_text->setPos( x, y );

    // specify width and height of rounded rectangle
    unsigned int width, height;
    width  = m_text->boundingRect().width() + fontSize/2;
    height = m_text->boundingRect().height();

    // show rectangle
    if( m_rectangle == NULL )
    {
      qreal radius = fontSize/5.0;
      QPainterPath path;
      path.addRoundedRect( 0, 0, width, height, radius, radius );
      m_rectangle = m_scene->addPath( path );
    }

    // create group
    if( m_group == NULL )
    {
      m_group = new CTimecodeItemGroup();
      m_scene->addItem( m_group );
      // connect signal of new position
      QObject::connect( m_group, SIGNAL( positionChanged(qreal,qreal) ), this, SLOT( setPositionInfo(qreal,qreal) ) );
    }

    // make timecode rectangle and text movable
    m_group->addToGroup( m_rectangle );
    m_group->addToGroup( m_text );

    // make group movable
    m_group->setFlag( QGraphicsItem::ItemIsMovable );
    // move text regarding position
    m_group->setPos( ui.ui_pos_x->value(), ui.ui_pos_y->value() );

    // apply z value(s)
    //m_text->setZValue( 3 );
    //m_rectangle->setZValue( 2 );
    m_group->setZValue( 2 );
    m_pixmap->setZValue( 1 );

    // apply text color
    QBrush brush = m_text->brush();
    brush.setColor( ui.ui_color->palette().color( QPalette::Base ) );
    m_text->setBrush( brush );

    // apply rectangle color
    QColor color = ui.ui_frame_color->palette().color( QPalette::Base );
    color.setAlpha( RECTALPHA );
    brush.setColor( color );
    // apply brush
    m_rectangle->setBrush( brush );
    // disable rectangle contour
    m_rectangle->setPen( Qt::NoPen );

    ui.ui_preview->setScene( m_scene );
  }
}

// browse for input directory
void MainWindow::browseInputDir()
{
  // create a file dialog and set mode to directory
  QString dir_str = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                      "",
                                                      QFileDialog::ShowDirsOnly );
  // check if user clicked OK
  if( dir_str != "" )
  {
    // set input directory text
    ui.ui_input_dir->setText( dir_str );
    // setup preview
    setupPreview();
  }
}

// browse for output directory
void MainWindow::browseOutputDir()
{
  // create a file dialog and set mode to dir only
  QString dir_str = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                      "",
                                                      QFileDialog::ShowDirsOnly );
  // apply user specified directory
  ui.ui_output_dir->setText( dir_str );

}

// function to change the font
void MainWindow::changeFont()
{
  // open a font dialog
  QFont font = QFontDialog::getFont( 0, ui.ui_font_name->font() );
  ui.ui_font_name->setText( font.family() );
  ui.ui_font_name->setFont( font );

  // set text item to font and repaint
  if( m_text )
  {
    m_text->setFont( font );
    m_scene->update();

    // need font size soon...
    QFont font = ui.ui_font_name->font();
    unsigned int fontSize = (font.pixelSize() == -1 ? font.pointSize() : font.pixelSize());

    // specify width and height of rounded rectangle
    unsigned int width, height;
    width  = m_text->boundingRect().width() + fontSize/2;
    height = m_text->boundingRect().height();

    qreal radius = fontSize/5.0;
    QPainterPath path;
    path.addRoundedRect( 0, 0, width, height, radius, radius );
    m_rectangle->setPath( path );
  }
}

// change color
void MainWindow::changeColor()
{
  // get palette
  QPalette palette = ui.ui_color->palette();
  // get user specified color
  QColor color = QColorDialog::getColor( palette.color( QPalette::Base ), this );
  palette.setColor( QPalette::Base, color );
  ui.ui_color->setPalette( palette );

  if( m_text )
  {
    QBrush brush = m_text->brush();
    brush.setColor( color );
    m_text->setBrush( brush );
    m_scene->update();
  }
}

// change frame color
void MainWindow::changeFrameColor()
{
  // get palette
  QPalette palette = ui.ui_frame_color->palette();
  // get user specified color
  QColor color = QColorDialog::getColor( palette.color( QPalette::Base ), this );
  palette.setColor( QPalette::Base, color );
  ui.ui_frame_color->setPalette( palette );

  // apply color to rectangle
  color.setAlpha( RECTALPHA );
  QBrush brush = m_rectangle->brush();
  brush.setColor( color );
  // apply brush
  m_rectangle->setBrush( brush );
}

// to update the text position
void MainWindow::updateTextPosition()
{
  // check, if we have a text placed at all
  if( m_text && m_rectangle )
  {
    // helper variables
    int x, y;
    x = ui.ui_pos_x->value();
    y = ui.ui_pos_y->value();

    // move text regarding position
    m_group->setPos( x, y );

    // update scene...
    m_scene->update();
  }
}

// to set text position (from external)
void MainWindow::setPositionInfo( qreal x, qreal y )
{
  ui.ui_pos_x->setValue( x );
  ui.ui_pos_y->setValue( y );
}

// function to process all images
void MainWindow::process()
{
  // check whether input directory is set
  if( ui.ui_input_dir->text().isEmpty() )
  {
    QMessageBox::critical( this, "Processing Error", "Please specify an input directory", QMessageBox::Ok, QMessageBox::Cancel );
    return;
  }
  // check whether output directory is set
  if( ui.ui_output_dir->text().isEmpty() )
  {
    QMessageBox::critical( this, "Processing Error", "Please specify an output directory", QMessageBox::Ok, QMessageBox::Cancel );
    return;
  }
  // check framerate
  if( ui.ui_framerate->value() <= 0.0 )
  {
    QMessageBox::critical( this, "Processing Error", "Please specify valid framerate", QMessageBox::Ok, QMessageBox::Cancel );
    return;
  }

  // process images
  processImages();
}

// functin to cancel a running job
void MainWindow::setStopFlag()
{
  m_stopflag = true;
  ui.ui_cancel_button->setEnabled( false );
  QApplication::processEvents();
}

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


#include <QRectF>
#include <QPointF>
#include <QGraphicsScene>

#include <iostream>
#include "ctimecodeitemgroup.h"

using namespace std;

CTimecodeItemGroup::CTimecodeItemGroup()
{
}

QVariant CTimecodeItemGroup::itemChange( GraphicsItemChange change, const QVariant & value )
{
  // apply change in items position to main window using signal and slots
  if( change == ItemPositionChange && scene() )
  {
    // value is the new position.
    QPointF newPos = value.toPointF();
    QRectF rect = scene()->sceneRect();

    //std::cout << "new pos x=" << newPos.x() << " y=" << newPos.y() << std::endl;

    // Keep the item inside the scene rect.
    newPos.setX( qMin(rect.right(), qMax(newPos.x(), rect.left())) );
    newPos.setY( qMin(rect.bottom(), qMax(newPos.y(), rect.top())) );

    //std::cout << "    cor x=" << newPos.x() << " y=" << newPos.y() << std::endl;

    emit positionChanged( newPos.x(), newPos.y() );
    return newPos;
  }

  // return base classes result
  return QGraphicsItemGroup::itemChange( change, value );
}

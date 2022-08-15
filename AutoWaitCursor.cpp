/*
 * (c) Copyright 2004 - 2015 Blue Pearl Software Inc.
 * All rights reserved.
 *
 * This source code belongs to Blue Pearl Software Inc.
 * It is considered trade secret and confidential, and is not to be used
 * by parties who have not received written authorization
 * from Blue Pearl Software Inc.
 *
 * Only authorized users are allowed to use, copy and modify
 * this software provided that the above copyright notice
 * remains in all copies of this software.
 *
 *
*/
#include "AutoWaitCursor.h"
#include <QApplication>

namespace NUIUtils
{

CAutoWaitCursor::CAutoWaitCursor( QObject * revertOnShowWidget ) :
	QObject( NULL ),
    fRestoreOnOpenWidget( revertOnShowWidget )
{
    QCursor * cursor = QApplication::overrideCursor();
	QApplication::setOverrideCursor( Qt::BusyCursor );
	if ( !cursor )
        qApp->processEvents();
    if ( revertOnShowWidget )
    {
        revertOnShowWidget->installEventFilter( this );
    }
}

bool CAutoWaitCursor::eventFilter(QObject *obj, QEvent *event)
{
	if ( obj == fRestoreOnOpenWidget ) 
	{
		if (event->type() == QEvent::Show ) 
		{
            restore();
		}
	}
	return QObject::eventFilter(obj, event);
}

CAutoWaitCursor::~CAutoWaitCursor()
{
	restore();
}

void CAutoWaitCursor::restore()
{
	QApplication::restoreOverrideCursor();
	qApp->processEvents();
}

bool CAutoWaitCursor::active()
{
    return QApplication::overrideCursor() != NULL;
}



}


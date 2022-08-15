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
#include "ButtonEnabler.h"

#include <QAbstractItemView>
#include <QAbstractButton>
#include <QLineEdit>

namespace NUIUtils
{

CButtonEnabler::CButtonEnabler( QAbstractItemView * view, QAbstractButton * btn, QObject * parent ) :
	QObject( parent ),
	fButton( btn )
{
	if ( parent == NULL )
		setParent( btn );
	if ( view->selectionModel() )
		connect( view->selectionModel(), SIGNAL( selectionChanged( const QItemSelection & , const QItemSelection  & ) ), this, SLOT( slotChanged( const QItemSelection  & , const QItemSelection & ) ) );
	btn->setEnabled( false );
    if ( view->selectionModel() )
        slotChanged( view->selectionModel()->selection(), QItemSelection() );
}

CButtonEnabler::CButtonEnabler( QLineEdit * le, QAbstractButton * btn, QObject * parent ) :
	QObject( parent ),
	fButton( btn )
{
	if ( parent == NULL )
		setParent( btn );
	connect( le, SIGNAL( textChanged( const QString &) ), this, SLOT( slotChanged( const QString  &  ) ) );
	slotChanged( le->text() );
}

void CButtonEnabler::slotChanged( const QItemSelection  & selected, const QItemSelection  & )
{
	bool enabled = selected.count() && selected.first().isValid();
	fButton->setEnabled( enabled );
}

void CButtonEnabler::slotChanged( const QString  & txt )
{
    fButton->setEnabled( !txt.isEmpty() );
}
}


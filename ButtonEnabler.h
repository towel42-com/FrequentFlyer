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
#ifndef __SHARED_UIUTILS_BUTTONENABLER_H
#define __SHARED_UIUTILS_BUTTONENABLER_H

#include <QObject>
class QAbstractItemView;
class QAbstractButton;
class QItemSelection;
class QLineEdit;

namespace NUIUtils
{
class CButtonEnabler : public QObject
{
Q_OBJECT;
public:
	CButtonEnabler( QAbstractItemView * view, QAbstractButton * btn, QObject * parent=NULL );
	CButtonEnabler( QLineEdit * le, QAbstractButton * btn, QObject * parent=NULL );
private slots:
	void slotChanged( const QItemSelection  & selected, const QItemSelection  & );
    void slotChanged( const QString & changed );
private:
	QAbstractButton * fButton;
};
}
#endif


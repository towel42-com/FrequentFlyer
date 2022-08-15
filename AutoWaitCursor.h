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
#ifndef __AUTOWAITCURSOR_H
#define __AUTOWAITCURSOR_H
#include <QObject>
namespace NUIUtils
{

class CAutoWaitCursor : public QObject // allows for delete later
{
public:
	CAutoWaitCursor( QObject * revertOnOpen = NULL );
	~CAutoWaitCursor();

	void restore();

    bool eventFilter( QObject *, QEvent * );
    static bool active();
private:
    QObject * fRestoreOnOpenWidget;
};

}
#endif


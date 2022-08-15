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
#include "Delegates.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QEvent>
#include <QDate>
#include <QDateEdit>

#include <limits>

namespace NUtils
{
CDateDelegate::CDateDelegate( QObject * parent ) :
	QItemDelegate( parent )
{
}

QWidget * CDateDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/ ) const
{
    QDateEdit * editor = new QDateEdit( parent );
    editor->setCalendarPopup( true );
    editor->setDate( QDate::currentDate() );
    return editor;
}

void CDateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QDateEdit *edit = qobject_cast<QDateEdit *>( editor );
    if ( edit )
    {
        edit->setDate( index.data( Qt::EditRole ).toDate() );
    }
}

void CDateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDateEdit *edit = qobject_cast<QDateEdit *>( editor );
    if ( edit )
    {
        QDate date = edit->date();
        model->setData( index, date );
    }
}

void CDateDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry( option.rect );
}

CActionDelegate::CActionDelegate( QObject * parent ) :
	QItemDelegate( parent )
{
}

//"CASE WHEN Status=0 THEN 'Flight' WHEN Status=1 THEN 'Credit' ELSE 'Redemption' END AS Status"
QWidget * CActionDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/ ) const
{
    QComboBox * editor = new QComboBox( parent );
    editor->addItems( QStringList() << "Flight" << "Credit" << "Redemption" );
    editor->setEditable( true );
    return editor;
}

void CActionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *edit = qobject_cast<QComboBox *>( editor );
    if ( edit )
    {
        edit->setCurrentIndex( edit->findText( index.data( Qt::EditRole ).toString() ) );
    }
}

void CActionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *edit = qobject_cast<QComboBox *>( editor );
    if ( edit )
    {
        model->setData( index, edit->currentText() );
    }
}

void CActionDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry( option.rect );
}

CMilesDelegate::CMilesDelegate( QObject * parent ) :
	QItemDelegate( parent )
{
}

//"CASE WHEN Status=0 THEN 'Flight' WHEN Status=1 THEN 'Credit' ELSE 'Redemption' END AS Status"
QWidget * CMilesDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & index ) const
{
    QLineEdit * editor = new QLineEdit( parent );

    const QAbstractItemModel * model = index.model();
    QString curr = model->index( index.row(), 3 ).data().toString();
    QIntValidator * validator = new QIntValidator( editor );
    if ( curr == "Flight" )
        validator->setRange( 0, 32767 );
    else
        validator->setRange( INT_MIN, INT_MAX );

    editor->setValidator( validator );
    return editor;
}

void CMilesDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>( editor );
    if ( edit )
    {
        edit->setText( index.data( Qt::EditRole ).toString() );
    }
}

void CMilesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>( editor );
    if ( edit )
    {
        model->setData( index, edit->text() );
    }
}

void CMilesDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry( option.rect );
}
}


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
#ifndef __DELEGATES_H
#define __DELEGATES_H

#include <QItemDelegate>
typedef QPair< int, int > TIntPair;
Q_DECLARE_METATYPE( TIntPair )

namespace NUtils
{
class CDateDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    CDateDelegate( QObject * parent );

    QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */ ) const;
};

class CActionDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    CActionDelegate( QObject * parent );

    QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */ ) const;
};

class CMilesDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    CMilesDelegate( QObject * parent );

    QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */ ) const;
};}

#endif


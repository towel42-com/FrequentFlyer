#include "runCmd.h"
#include "AutoWaitCursor.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool runCmd( QSqlQuery & query )
{
    NUIUtils::CAutoWaitCursor awc;
	if ( !query.exec() )
	{
		qDebug() << query.lastError().driverText();
		qDebug() << query.lastError().databaseText();
		Q_ASSERT( 0 );
		return false;
	}

	return true;
}
bool runCmd( const QString & cmd, const QList< QVariantList > & params )
{
    NUIUtils::CAutoWaitCursor awc;
    QSqlQuery query;
	query.clear();
	if ( !query.prepare( cmd ) )
	{
		qDebug() << query.lastError().driverText();
		qDebug() << query.lastError().databaseText();
		Q_ASSERT( 0 );
		return false;
	}

	for( int ii = 0; ii < params.count(); ++ii )
		query.addBindValue( params[ ii ] );
	
    if ( !query.execBatch() )
	{
		qDebug() << query.lastError().driverText();
		qDebug() << query.lastError().databaseText();
		Q_ASSERT( 0 );
		return false;
	}
	return true;
}

bool runCmd( QSqlQuery & query, const QString & cmd, const QList< QVariant > & params )
{
    NUIUtils::CAutoWaitCursor awc;
	query.clear();
	if ( !query.prepare( cmd ) )
	{
		qDebug() << query.lastError().driverText();
		qDebug() << query.lastError().databaseText();
		Q_ASSERT( 0 );
		return false;
	}

	for( int ii = 0; ii < params.count(); ++ii )
		query.bindValue( ii, params[ ii ] );
	
    return runCmd( query );
}
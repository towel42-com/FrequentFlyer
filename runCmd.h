#ifndef __RUNCMD_H
#define __RUNCMD_H

class QSqlQuery;
class QString;
class QVariant;
template < typename T > class QList;
typedef QList<QVariant> QVariantList;

bool runCmd( QSqlQuery & query );
bool runCmd( const QString & cmd, const QList< QVariantList > & params );
bool runCmd( QSqlQuery & query, const QString & cmd, const QList< QVariant > & params );

#endif
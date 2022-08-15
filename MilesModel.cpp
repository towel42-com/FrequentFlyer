#include "MilesModel.h"
#include "AutoWaitCursor.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QDebug>


CMilesModel::CMilesModel( QObject * parent ) :
    QSqlTableModel( parent ),
    fMVPOn( 30, 20000 ),
    fMVPGoldOn( 60, 40000 ),
    fMVP75kOn( 90, 75000 )
{
    computeValues( false );
    connect( this, SIGNAL( dataChanged( const QModelIndex &, const QModelIndex &  ) ), this, SLOT( slotDataChanged() ) );
}

QString SMVPStatusRemain::status() const
{
    QString date = fDate.toString();
    if( fDate.isValid() )
    {
        if( QDate::currentDate() >= fDate )
            return QObject::tr( "Made IT!" );
        return QObject::tr( "Miles: %1 Segments: %2 %3" ).arg( fNumMiles ).arg( fNumSegs ).arg( fDate.toString() );
    }
    return "Unknown";
}

QString SMVPStatusRemain::requirements() const
{
    if( fDate.isValid() )
    {
        if( QDate::currentDate() >= fDate )
            return QObject::tr( "Made IT!" );

        QDate lastDayOfYear = QDate( QDate::currentDate().year(), 12, 31 );
        int numWeeksInYear = lastDayOfYear.weekNumber();
        int currWeek = QDate::currentDate().weekNumber();
        int weeksRemaining = numWeeksInYear - currWeek;

        double segsPerWeek = (1.0*fNumSegs) / weeksRemaining;
        double milesPerWeek = (1.0*fNumMiles) / weeksRemaining;
 
        return QObject::tr( "Segs: %1 Miles: %2" ).arg( segsPerWeek, 4, 'f', 1 ).arg( milesPerWeek );
    }
    return "Unknown";
}

void SMVPStatusRemain::findMVP( int segsThisYear, int milesThisYear, double segsPerWeek, double milesPerWeek )
{
    fNumSegs = fSegmentsNeeded - segsThisYear;
    fNumMiles = fMilesNeeded - milesThisYear;
    if ( fNumSegs <= 0 )
    {
        fDate = QDate::currentDate().addDays( -1 );
        return;
    }
    if ( fNumMiles <= 0 )
    {
        fDate = QDate::currentDate().addDays( -1 );
        return;
    }

    double numWeeksNeededBySeg = segsPerWeek ? (1.0*fNumSegs)/segsPerWeek : -1;
    double numWeeksNeededByMiles = milesPerWeek ? (1.0*fNumMiles)/milesPerWeek : -1;
    double weeksNeeded = std::min( numWeeksNeededBySeg, numWeeksNeededByMiles );
    int daysNeeded = static_cast< int >( weeksNeeded * 7 );
    if ( daysNeeded != -7 ) // at zero
        fDate = QDate::currentDate().addDays( daysNeeded );
    else
        fDate = QDate();
}

void CMilesModel::slotDataChanged()
{
    computeValues( true );
}

void CMilesModel::computeValues( bool aOK )
{
    NUIUtils::CAutoWaitCursor awc;
    fSegmentsThisYear = 0;
    fActualMilesThisYear = 0;
    fBonusMilesThisYear = 0;
    fStatusMilesThisYear = 0;
    fTotalMilesThisYear = 0;
    fTotalSegments = 0;
    fAvailableMiles = 0;
    fTotalFlightMiles = 0;
    fSegmentsPerWeek = 0;
    fMilesPerWeek = 0;
    if ( !aOK )
    {
        emit sigUpdateValues();
        return;
    }
    while( canFetchMore() )
        fetchMore();
    int rowCount = this->rowCount();
    //SELECT "SegmentID", "SegmentDate", "ActivityType", "Status", "Miles", "Bonus", "Total" FROM Segments
    int currYear = QDate::currentDate().year();
    for( int ii = 0; ii < rowCount; ++ii )
    {
        QSqlRecord record = this->record( ii );
        if ( record.isEmpty() )
            continue;
        bool isFlight = record.value( eIsFlight ).toBool();
        QString status = record.value( eStatus ).toString();
        bool isPureFlght =  status == "Flight";

        fAvailableMiles += record.value( eTotal ).toInt();

        if ( isFlight )
        {
            if ( isPureFlght && ( record.value( eDate ).toDate().year() == currYear ) )
            {
                fSegmentsThisYear++;
                fActualMilesThisYear += record.value( eMiles ).toInt();
                fBonusMilesThisYear += record.value( eBonus ).toInt();
                fTotalMilesThisYear += record.value( eTotal ).toInt();
                fStatusMilesThisYear += record.value( eStatusMiles ).toInt();
            }
            if ( isPureFlght )
                fTotalSegments++;
            fTotalFlightMiles += record.value( eMiles ).toInt();
        }
    }
    fSegmentsPerWeek = 0;
    fMilesPerWeek = 0;
    int weekNumber = QDate::currentDate().weekNumber();
    if ( weekNumber )
    {
        fSegmentsPerWeek = 1.0*fSegmentsThisYear / (1.0*weekNumber);
        fMilesPerWeek = 1.0*fActualMilesThisYear / (1.0*weekNumber);
    }
    else
    {
        fMilesPerWeek = 0;
        fSegmentsPerWeek = 0;
    }
    fMVPOn.findMVP( fSegmentsThisYear, fStatusMilesThisYear, fSegmentsPerWeek, fMilesPerWeek );
    fMVPGoldOn.findMVP( fSegmentsThisYear, fStatusMilesThisYear, fSegmentsPerWeek, fMilesPerWeek );
    fMVP75kOn.findMVP( fSegmentsThisYear, fStatusMilesThisYear, fSegmentsPerWeek, fMilesPerWeek );
    emit sigUpdateValues();
}

// helpers for building SQL expressions
class Sql
{
public:
    // SQL keywords
    inline const static QLatin1String as() { return QLatin1String("AS"); }
    inline const static QLatin1String asc() { return QLatin1String("ASC"); }
    inline const static QLatin1String comma() { return QLatin1String(","); }
    inline const static QLatin1String desc() { return QLatin1String("DESC"); }
    inline const static QLatin1String eq() { return QLatin1String("="); }
    // "and" is a C++ keyword
    inline const static QLatin1String et() { return QLatin1String("AND"); }
    inline const static QLatin1String from() { return QLatin1String("FROM"); }
    inline const static QLatin1String leftJoin() { return QLatin1String("LEFT JOIN"); }
    inline const static QLatin1String on() { return QLatin1String("ON"); }
    inline const static QLatin1String orderBy() { return QLatin1String("ORDER BY"); }
    inline const static QLatin1String parenClose() { return QLatin1String(")"); }
    inline const static QLatin1String parenOpen() { return QLatin1String("("); }
    inline const static QLatin1String select() { return QLatin1String("SELECT"); }
    inline const static QLatin1String sp() { return QLatin1String(" "); }
    inline const static QLatin1String where() { return QLatin1String("WHERE"); }

    // Build expressions based on key words
    inline const static QString as(const QString &a, const QString &b) { return b.isEmpty() ? a : concat(concat(a, as()), b); }
    inline const static QString asc(const QString &s) { return concat(s, asc()); }
    inline const static QString comma(const QString &a, const QString &b) { return a.isEmpty() ? b : b.isEmpty() ? a : QString(a).append(comma()).append(b); }
    inline const static QString concat(const QString &a, const QString &b) { return a.isEmpty() ? b : b.isEmpty() ? a : QString(a).append(sp()).append(b); }
    inline const static QString desc(const QString &s) { return concat(s, desc()); }
    inline const static QString eq(const QString &a, const QString &b) { return QString(a).append(eq()).append(b); }
    inline const static QString et(const QString &a, const QString &b) { return a.isEmpty() ? b : b.isEmpty() ? a : concat(concat(a, et()), b); }
    inline const static QString from(const QString &s) { return concat(from(), s); }
    inline const static QString leftJoin(const QString &s) { return concat(leftJoin(), s); }
    inline const static QString on(const QString &s) { return concat(on(), s); }
    inline const static QString orderBy(const QString &s) { return s.isEmpty() ? s : concat(orderBy(), s); }
    inline const static QString paren(const QString &s) { return s.isEmpty() ? s : parenOpen() + s + parenClose(); }
    inline const static QString select(const QString &s) { return concat(select(), s); }
    inline const static QString where(const QString &s) { return s.isEmpty() ? s : concat(where(), s); }
};

QString CMilesModel::selectStatement() const
{
    if( tableName().isEmpty() )
    {
        const_cast< CMilesModel * >( this )->setLastError( QSqlError( QLatin1String( "No table name given" ), QString(), QSqlError::StatementError ) );
        return QString();
    }
    if( record().isEmpty() )
    {
        const_cast< CMilesModel * >( this )->setLastError( QSqlError( QLatin1String( "Unable to find table " ) + tableName(), QString(), QSqlError::StatementError ) );
        return QString();
    }

    QString stmt = database().driver()->sqlStatement( QSqlDriver::SelectStatement, tableName(), record(), false );
    if( stmt.isEmpty() )
    {
        const_cast< CMilesModel * >( this )->setLastError( QSqlError( QLatin1String( "Unable to select fields from table " ) + tableName(), QString(), QSqlError::StatementError ) );
        return stmt;
    }
    ////SELECT "SegmentID", "SegmentDate", "ActivityType", "Status", "Miles", "Bonus", "Total" FROM Segments
    //stmt.replace( "\"Status\"", "CASE WHEN Status=0 THEN 'Flight' WHEN Status=1 THEN 'Credit' ELSE 'Redemption' END AS Status" );
    return Sql::concat( Sql::concat( stmt, Sql::where( filter() ) ), orderByClause() );
}

void CMilesModel::setFilter()
{
    if ( !fShowFlightsOnly && !fShowThisYearOnly )
    {
        QSqlTableModel::setFilter( QString() );
        return;
    }

    QStringList filters;
    if ( fShowThisYearOnly )
    {
        auto year = QDate::currentDate().year();
        filters << Sql::paren( QString( "strftime( '%Y', SegmentDate )='%1' " ).arg ( year ) );
    }
    if ( fShowFlightsOnly )
    {
        filters << Sql::paren( QString( "IsFlight <> 0" ) );
    }

    QString filter= Sql::paren( filters.join( " AND " ) );
    QSqlTableModel::setFilter( filter );
}

bool CMilesModel::select()
{
    NUIUtils::CAutoWaitCursor awc;
    bool retVal = QSqlTableModel::select();
    computeValues( retVal );
    return retVal;
}

QString CMilesModel::mvpON() const
{
    return fMVPOn.status();
}

QString CMilesModel::mvpGoldON() const
{
    return fMVPGoldOn.status();
}

QString CMilesModel::mvp75kON() const
{
    return fMVP75kOn.status();
}

QString CMilesModel::mvpReq() const
{
    return fMVPOn.requirements();
}

QString CMilesModel::goldReq() const
{
    return fMVPGoldOn.requirements();
}

QString CMilesModel::_75kReq() const
{
    return fMVP75kOn.requirements();
}

Qt::ItemFlags CMilesModel::flags( const QModelIndex & index ) const
{
    Qt::ItemFlags retVal = QSqlTableModel::flags( index );
    return retVal;
}

void CMilesModel::setShowFlightsOnly( bool value )
{
    fShowFlightsOnly = value;
    setFilter();
}

void CMilesModel::setShowThisYearOnly( bool value )
{
    fShowThisYearOnly = value;
    setFilter();
}


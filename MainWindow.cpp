#include "MainWindow.h"
#include "MilesModel.h"
#include "AutoWaitCursor.h"
#include "AddSegment.h"
#include "runCmd.h"
#include "ui_MainWindow.h"
#include "ButtonEnabler.h"

#include <QFileDialog>
#include <QSqlTableModel>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QDebug>
#include <QDate>
#include <QLocale>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include "Delegates.h"


CMainWindow::CMainWindow(QWidget *parent)
	: QMainWindow(parent),
	fImpl( new Ui::CMainWindow )
{
	fImpl->setupUi(this);
    setWindowIcon( QIcon( ":/resources/airplane.png" ) );
    fModel = nullptr;
	setAttribute(Qt::WA_DeleteOnClose);

    fImpl->refresh->setEnabled( false );
    connect( fImpl->refresh, &QPushButton::clicked, this, &CMainWindow::slotRefresh );
    connect( fImpl->importSegments, &QPushButton::clicked, this, &CMainWindow::slotImportSegments );
    connect( fImpl->deleteSegment, &QPushButton::clicked, this, &CMainWindow::slotDelSegment );
    connect( fImpl->addSegments, &QPushButton::clicked, this, &CMainWindow::slotAddSegments );
    connect( fImpl->onlyFlights, &QPushButton::clicked, this, &CMainWindow::slotOnlyShowFlights );
    connect( fImpl->onlyThisYear, &QPushButton::clicked, this, &CMainWindow::slotOnlyShowThisYear );
    initDatabase();
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::buildFlights()
{
    QSqlQuery query( "SELECT ActivityType, Miles, Bonus, Total FROM Segments WHERE Status=0 ORDER BY SegmentID DESC" );
    if ( !runCmd( query ) )
        return;

    QSet< QString > handled;
    QRegExp regEx( "Flight \\d+ ([A-Z]{3,4}) ([A-Z]{3,4})" );

    QVariantList fromApts;
    QVariantList toApts;
    QVariantList miles;
    QVariantList bonus;
    QVariantList total;

    while( query.next() )
    {
        int pos = 0;
        QString flight = query.value( pos++ ).toString();
        if ( regEx.indexIn( flight, 0 ) == -1 )
            continue;
        QString from = regEx.cap( 1 );
        QString to = regEx.cap( 2 );

        QString key = QString( "%1-%2" ).arg( from ).arg( to );
        if ( handled.contains( key ) )
            continue;
        handled.insert( key );

        fromApts << from;
        toApts << to;
        miles << query.value( pos++ ).toInt();
        bonus << query.value( pos++ ).toInt();
        total << query.value( pos++ ).toInt();
    }

    runCmd(
        "INSERT INTO Flights "
        "("
        "FromApt, "
        "ToApt, "
        "Miles, "
        "Bonus, "
        "Total "
        ") VALUES ("
        "?, ?, ?, ?, ?"
        ")",
        QList< QVariantList >()
        << fromApts
        << toApts
        << miles
        << bonus
        << total
        );

}


void CMainWindow::initDatabase()
{
    QDir dataDir = QDir( QCoreApplication::applicationDirPath() );
    QString envDir = qgetenv( "FLIGHTSDIR" );
    if ( !envDir.isEmpty() )
        dataDir = QDir( envDir );
    if ( !dataDir.exists() && !dataDir.mkpath( "." ) )
    {
        QMessageBox::warning( this, tr( "Could not create data directory" ), tr( "Could not create data directory : %1" ).arg( dataDir.absolutePath( ) ) );
        return;
    }

    QString dbPath = dataDir.absoluteFilePath( "flights.db" );
    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setDatabaseName( dbPath );
    if( !db.open() )
    {
        QMessageBox::warning( this, tr( "Could not open database" ), tr( "Could not database: %1" ).arg( dbPath ) );
        return;
    }

    QSqlQuery query( 
        "CREATE TABLE IF NOT EXISTS Segments( "
            "SegmentID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "SegmentDate TEXT NOT NULL, "
            "ActivityType TEXT NOT NULL, "
            "Status TEXT NOT NULL, " 
            "Miles INTEGER, "
            "Bonus INTEGER, "
            "StatusMiles INTEGER, "
            "Total INTEGER, "
            "IsFlight INTEGER " 
            ")" );

    if ( !runCmd( query ) )
        return;

    query = QSqlQuery( "CREATE INDEX IF NOT EXISTS IDX_SEGMENTDATE ON Segments( SegmentDate )" );
    if ( !runCmd( query ) )
        return;

    query = QSqlQuery( 
        "CREATE TABLE IF NOT EXISTS Flights( "
            "FlightID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "FROMAPT TEXT NOT NULL, "
            "TOAPT TEXT NOT NULL, "
            "Miles INTEGER, "
            "Bonus INTEGER, "
            "Total INTEGER "
            ")" );

    if ( !runCmd( query ) )
        return;

    query = QSqlQuery( "CREATE INDEX IF NOT EXISTS IDX_FLIGHT_FROM ON Flights( FromAPT )" );
    if ( !runCmd( query ) )
        return;

    query = QSqlQuery( "SELECT COUNT(*) FROM Flights" );
    if ( !runCmd( query ) )
        return;
    if ( !query.next() || query.value( 0 ).toInt() == 0 )
    {
        buildFlights();
    }

    fModel = new CMilesModel( this );
    //fFilterModel = new CFilterModel( this );
    //fFilterModel->setSourceModel( fModel );
    connect( fModel, &CMilesModel::sigUpdateValues, this, &CMainWindow::slotValuesChanged );
    fImpl->segmentsView->setModel( fModel );
    new NUIUtils::CButtonEnabler( fImpl->segmentsView, fImpl->deleteSegment );

    fModel->setTable( "Segments" );
    fModel->setEditStrategy( QSqlTableModel::OnFieldChange );

    fModel->setHeaderData( eID, Qt::Horizontal, tr( "ID" ) );
    fModel->setHeaderData( eDate, Qt::Horizontal, tr( "Date" ) );
    fModel->setHeaderData( eActivity, Qt::Horizontal, tr( "Activity" ) );
    fModel->setHeaderData( eStatus, Qt::Horizontal, tr( "Action" ) );
    fModel->setHeaderData( eMiles, Qt::Horizontal, tr( "Miles" ) );
    fModel->setHeaderData( eBonus, Qt::Horizontal, tr( "Bonus Miles" ) );
    fModel->setHeaderData( eTotal, Qt::Horizontal, tr( "Total" ) );
    fModel->setHeaderData( eStatusMiles, Qt::Horizontal, tr( "Status Miles" ) );
    fModel->setHeaderData( eIsFlight, Qt::Horizontal, tr( "Is Flight" ) );

    fImpl->segmentsView->hideColumn( eID );
    fImpl->segmentsView->hideColumn( eIsFlight );
    fImpl->segmentsView->setItemDelegateForColumn( eDate, new NUtils::CDateDelegate( this ) );
    fImpl->segmentsView->setItemDelegateForColumn( eStatus, new NUtils::CActionDelegate( this ) );
    fImpl->segmentsView->setItemDelegateForColumn( eMiles, new NUtils::CMilesDelegate( this ) );
    fImpl->segmentsView->setItemDelegateForColumn( eBonus, new NUtils::CMilesDelegate( this ) );
    fImpl->segmentsView->setItemDelegateForColumn( eTotal, new NUtils::CMilesDelegate( this ) );
    
    fModel->setSort( eDate, Qt::DescendingOrder );
    fImpl->segmentsView->horizontalHeader()->setSortIndicator( eDate, Qt::DescendingOrder );
}

void CMainWindow::slotImportSegments()
{
    QString csvFile = QFileDialog::getOpenFileName( this, tr( "Select Alaska Airlines CSV File" ), QString(), tr( "CSV Files (*.csv);;All Files (*.*)" ) );
    if ( csvFile.isEmpty() )
        return;

    QFile fi( csvFile );
    if ( !fi.open( QFile::ReadOnly ) )
    {
        QMessageBox::warning( this, tr( "Could not open file" ), tr( "Could not open: %1" ).arg( csvFile ) );
        return;
    }
    QTextStream ts( &fi );

    QList< QVariant > dates;
    QList< QVariant > activityTypes;
    QList< QVariant > statuses;
    QList< QVariant > flightMiles;
    QList< QVariant > bonusMiles;
    QList< QVariant > totalMiles;
    
    bool firstLine = true;
    while ( !ts.atEnd() )
    {
        QString curr = ts.readLine();
        QStringList currLine = curr.split( ',' );
        if ( currLine.size() != 6 )
            continue;
        if ( firstLine )
        {
            firstLine = false;
            continue;
        }
        int pos = 0;
        QDate date = QDate::fromString( currLine[ pos++ ].trimmed(), "M/d/yyyy" );
        Q_ASSERT( date.isValid() );
        QString activityType = currLine[ pos++ ].trimmed();
        QString statusType = currLine[ pos++ ].trimmed();
        bool isFlight = activityType.contains( "Flight", Qt::CaseInsensitive );
        bool isCredit = activityType.contains( "hertz", Qt::CaseInsensitive ) || activityType.contains( "compensation", Qt::CaseInsensitive ) || activityType.contains( "Update total", Qt::CaseInsensitive ) || activityType.contains( "Premium Bonus", Qt::CaseInsensitive ) || statusType.contains( "redeposited", Qt::CaseInsensitive );
        bool isRedeem = statusType.contains( "redeemed", Qt::CaseInsensitive );
        QString status;
        if ( isFlight )
            status = "Flight";
        else if ( isCredit )
            status = "Credit";
        else if ( isRedeem )
            status = "Redemption";
        else
        {
            status = "Credit";
            Q_ASSERT( 0 );
        }
        QString miles = currLine[ pos++ ].trimmed();
        QString bonus = currLine[ pos++ ].trimmed();
        QString total = currLine[ pos++ ].trimmed();

        dates << date;
        activityTypes << activityType;
        statuses << status;
        flightMiles << miles;
        bonusMiles << bonus;
        totalMiles << total;
    }

    Q_ASSERT( dates.size() == statuses.size() );
    Q_ASSERT( dates.size() == activityTypes.size() );
    Q_ASSERT( dates.size() == flightMiles.size() );
    Q_ASSERT( dates.size() == bonusMiles.size() );
    Q_ASSERT( dates.size() == totalMiles.size() );

    if ( !runCmd( 
        "INSERT INTO Segments "
        "("
        "SegmentDate, "
        "ActivityType, "
        "Status, "
        "Miles, "
        "Bonus, "
        "Total "
        ") VALUES ("
        "?, ?, ?, ?, ?, ?"
        ")",
        QList< QVariantList >() 
        << dates
        << activityTypes
        << statuses
        << flightMiles
        << bonusMiles
        << totalMiles
        )
        )
    {
        QMessageBox::warning( this, tr( "Error Importing" ), tr( "Could not import: %1" ).arg( csvFile ) );
    }
    slotRefresh();
}

void CMainWindow::slotValuesChanged()
{
    QLocale locale;
    fImpl->segmentsThisYear->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->segmentsThisYear() ) ) );
    fImpl->actualMilesThisYear->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->actualMilesThisYear() ) ) );
    fImpl->bonusMilesThisYear->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->bonusMilesThisYear() ) ) );
    fImpl->totalMilesThisYear->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->totalMilesThisYear() ) ) );
    fImpl->statusMilesThisYear->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->statusMilesThisYear() ) ) );
    fImpl->totalSegments->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->totalSegments() ) ) );
    fImpl->totalFlightMiles->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->totalFlightMiles() ) ) );
    fImpl->availableMiles->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->availableMiles() ) ) );

    fImpl->segmentsPerWeek->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->segmentsPerWeek(), 'f', 1 ) ) );
    fImpl->milesPerWeek->setText( QString( "<b>%1</b>" ).arg( locale.toString( fModel->milesPerWeek(), 'f', 1 ) ) );
    fImpl->mvpON->setText( QString( "<b>%1</b>" ).arg( fModel->mvpON() ) );
    fImpl->mvpGoldON->setText( QString( "<b>%1</b>" ).arg( fModel->mvpGoldON() ) );
    fImpl->mvp75kON->setText( QString( "<b>%1</b>" ).arg( fModel->mvp75kON() ) );

    fImpl->mvpReq->setText( QString( "<b>%1</b>" ).arg( fModel->mvpReq() ) );
    fImpl->goldReq->setText( QString( "<b>%1</b>" ).arg( fModel->goldReq() ) );
    fImpl->_75kReq->setText( QString( "<b>%1</b>" ).arg( fModel->_75kReq() ) );
    fImpl->refresh->setEnabled( true );
}

void CMainWindow::slotDelSegment()
{
    QItemSelectionModel * model = fImpl->segmentsView->selectionModel();
    if ( !model )
        return;

    QModelIndexList indexes = model->selectedIndexes();
    if ( indexes.isEmpty() )
        return;

    fModel->removeRows( indexes.first().row(), 1 );
    slotRefresh();
}

void CMainWindow::slotAddSegments()
{
    CAddSegment dlg;
    if ( dlg.exec() == QDialog::Accepted )
    {
        QSqlRecord record = fModel->record();
        dlg.fillRecord( record );
        fModel->insertRecord( 0, record );
        slotRefresh();
    }
}

void CMainWindow::slotOnlyShowFlights()
{
    fModel->setShowFlightsOnly( fImpl->onlyFlights->isChecked() );
    fModel->select();
}

void CMainWindow::slotOnlyShowThisYear()
{
    fModel->setShowThisYearOnly( fImpl->onlyThisYear->isChecked() );
    fModel->select();
}

void CMainWindow::slotRefresh()
{
    fModel->select();
}

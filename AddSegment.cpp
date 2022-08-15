#include "AddSegment.h"
#include "AutoWaitCursor.h"
#include "MilesModel.h"

#include "ui_AddSegment.h"
#include "runCmd.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QSettings>
#include <QPushButton>

QDate CAddSegment::sLastDate;
CAddSegment::CAddSegment(QWidget *parent)
	: QDialog(parent),
	fImpl( new Ui::CAddSegment )
{
	fImpl->setupUi(this);
    connect( fImpl->flight, SIGNAL( clicked() ), this, SLOT( slotTypeChanged() ) );
    connect( fImpl->adjustment, SIGNAL( clicked() ), this, SLOT( slotTypeChanged() ) );

    connect( fImpl->from, SIGNAL( currentIndexChanged( const QString & ) ), this, SLOT( slotFromChanged( const QString & ) ) );
    connect( fImpl->from, SIGNAL( editTextChanged( const QString & ) ), this, SLOT( slotFromChanged( const QString & ) ) );

    connect( fImpl->to, SIGNAL( currentIndexChanged( const QString & ) ), this, SLOT( slotToChanged( const QString & ) ) );
    connect( fImpl->to, SIGNAL( editTextChanged( const QString & ) ), this, SLOT( slotToChanged( const QString & ) ) );
    connect( fImpl->flightStatus, SIGNAL( textChanged( const QString & ) ), this, SLOT( slotStatusChanged() ) );

    fImpl->flightActual->setValidator( new QIntValidator( 0, 999999, this ) );
    fImpl->flightBonus->setValidator( new QIntValidator( 0, 999999, this ) );
    fImpl->flightStatus->setValidator( new QIntValidator( 0, 999999, this ) );
    fImpl->flightTotal->setValidator( new QIntValidator( 0, 999999, this ) );

    fImpl->flightMiles->setValidator( new QIntValidator( 0, 999999, this ) );
    fImpl->bonusMiles->setValidator( new QIntValidator( 0, 999999, this ) );
    fImpl->adjStatusMiles->setValidator( new QIntValidator( 0, 999999, this ) );
    fImpl->totalMiles->setValidator( new QIntValidator( 0, 999999, this ) );

    fImpl->from->addItems( loadFrom() );
    if ( !sLastDate.isValid() )
        sLastDate = QDate::currentDate();
    fImpl->date->setDate( sLastDate );

    QSettings settings;
    int val = settings.value( "SegmentType", 0 ).toInt();
    if ( val == 0 )
        fImpl->flight->setChecked( true );
    else 
        fImpl->adjustment->setChecked( true );
    
    if ( settings.contains( "FromAirport" ) )
        fImpl->from->setCurrentIndex( fImpl->from->findText( settings.value( "FromAirport" ).toString() ) );
    if ( settings.contains( "ToAirport" ) )
        fImpl->to->setCurrentIndex( fImpl->to->findText( settings.value( "ToAirport" ).toString() ) );
    slotTypeChanged();
    slotStatusChanged();
}

CAddSegment::~CAddSegment()
{
}

void CAddSegment::accept()
{
    QSettings settings;
    int val = 0;
    if ( fImpl->flight->isChecked() )
    {
        settings.setValue( "FromAirport", fImpl->from->currentText() );
        settings.setValue( "ToAirport", fImpl->to->currentText() );
    }
    else if ( fImpl->adjustment->isChecked() )
        val = 1;
    settings.setValue( "SegmentType", val );
    sLastDate = fImpl->date->date();
    QDialog::accept();
}

void CAddSegment::slotTypeChanged()
{
    fImpl->flightGroupBox->setEnabled( fImpl->flight->isChecked() );
    fImpl->adjustmentGroupBox->setEnabled( fImpl->adjustment->isChecked() );
}

void CAddSegment::slotStatusChanged()
{
    //fImpl->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( !fImpl->flightStatus->text().isEmpty() );
}


QStringList CAddSegment::loadFrom()
{
    QSqlQuery query;
    if ( !runCmd( query, "SELECT DISTINCT(FROMAPT) FROM Flights ORDER BY FROMAPT ASC", QVariantList() ) )
        return QStringList();

    QStringList retVal;
    while( query.next() )
    {
        retVal << query.value( 0 ).toString();
    }
    return retVal;
}

QStringList CAddSegment::getToForFrom( const QString & from )
{
    QSqlQuery query;
    if ( !runCmd( query, "SELECT DISTINCT(TOAPT) FROM Flights WHERE FROMAPT=? ORDER BY TOAPT ASC", QVariantList() << from ) )
        return QStringList();

    QStringList retVal;
    while( query.next() )
    {
        retVal << query.value( 0 ).toString();
    }
    return retVal;
}

void CAddSegment::slotFromChanged( const QString & from )
{
    fImpl->to->clear();
    fImpl->flightActual->setText( QString() );
    fImpl->flightBonus->setText( QString() );
    fImpl->flightTotal->setText( QString() );
    fImpl->to->addItems( getToForFrom( from ) );
}

void CAddSegment::slotToChanged( const QString & to )
{
    fImpl->flightActual->setText( QString() );
    fImpl->flightBonus->setText( QString() );
    fImpl->flightTotal->setText( QString() );

    QSqlQuery query;
    if ( !runCmd( query, "SELECT Miles, Bonus, Total FROM Flights WHERE FROMAPT=? AND TOAPT=?", QVariantList() << fImpl->from->currentText() << to ) )
        return;

    QStringList retVal;
    if ( !query.next() )
        return;

    int pos =0;
    fImpl->flightActual->setText( query.value( pos++ ).toString() );
    fImpl->flightBonus->setText( query.value( pos++ ).toString() );
    fImpl->flightTotal->setText( query.value( pos++ ).toString() );
}

QString CAddSegment::getActivityInfo() const
{
    if ( fImpl->flight->isChecked() )
        return QString( "Flight FROM %1 to %2" ).arg( fImpl->from->currentText() ).arg( fImpl->to->currentText() );
    else if ( fImpl->adjustment->isChecked() )
    {
        QString retVal = fImpl->activityDescription->text();
        if ( retVal.isEmpty() )
            retVal = QString( "Miles adjustment" );
        return retVal;
    }
    else
        return QString();
}

QString CAddSegment::getStatus() const
{
    if ( fImpl->flight->isChecked() )
        return "Flight";
    else if ( fImpl->adjustment->isChecked() )
    {
        QString retVal = fImpl->status->text();
        if ( retVal.isEmpty() )
            retVal = "Adjustment";
        return retVal;
    }
    else
        return 0;
}

int CAddSegment::getMiles() const
{
    if ( fImpl->flight->isChecked() )
        return fImpl->flightActual->text().toInt();
    else if ( fImpl->adjustment->isChecked() )
        return fImpl->flightMiles->text().toInt();
    return 0;
}

int CAddSegment::getBonus() const
{
    if ( fImpl->flight->isChecked() )
        return fImpl->flightBonus->text().toInt();
    else if ( fImpl->adjustment->isChecked() )
        return fImpl->bonusMiles->text().toInt();
    return 0;
}

int CAddSegment::getTotal() const
{
    if ( fImpl->flight->isChecked() )
        return fImpl->flightTotal->text().toInt();
    else if ( fImpl->adjustment->isChecked() )
        return fImpl->totalMiles->text().toInt();
    return 0;
}

int CAddSegment::getStatusMiles() const
{
    if ( fImpl->flight->isChecked() )
        return fImpl->flightStatus->text().toInt();
    else if ( fImpl->adjustment->isChecked() )
        return fImpl->adjStatusMiles->text().toInt();
    return 0;
}

void CAddSegment::fillRecord( QSqlRecord & record )
{
    record.setValue( eDate, fImpl->date->date() );
    record.setValue( eActivity, getActivityInfo() );
    record.setValue( eStatus, getStatus() );
    record.setValue( eMiles, getMiles() );
    record.setValue( eBonus, getBonus() );
    record.setValue( eTotal, getTotal() );
    record.setValue( eStatusMiles, getStatusMiles() );
    record.setValue( eIsFlight, fImpl->flight->isChecked() );
}


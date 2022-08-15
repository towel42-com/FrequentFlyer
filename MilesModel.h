#ifndef MILESMODEL_H
#define MILESMODEL_H

#include <QDate>
#include <QSqlTableModel>
#include <QSortFilterProxyModel>
enum EColumnIndex
{
    eID=0,
    eDate=1,
    eActivity=2,
    eStatus=3,
    eMiles=4,
    eBonus=5,
    eStatusMiles=6,
    eTotal=7,
    eIsFlight=8
};

struct SMVPStatusRemain
{
    SMVPStatusRemain() = default;

    SMVPStatusRemain( int segs, int miles ) :
        fSegmentsNeeded( segs ),
        fMilesNeeded( miles )
    {
    }

    QString status() const;
    QString requirements() const;
    void findMVP( int segsThisYear, int milesThisYear, double segsPerWeek, double milesPerWeek );

    int fSegmentsNeeded{0};
    int fMilesNeeded{0};
    QDate fDate{};
    int fNumSegs{0};
    int fNumMiles{0};
};

class CMilesModel : public QSqlTableModel
{
Q_OBJECT;
public:
    CMilesModel( QObject * parent );
    QString selectStatement() const override;

    int segmentsThisYear() const{ return fSegmentsThisYear; }
    int actualMilesThisYear() const{ return fActualMilesThisYear; }
    int bonusMilesThisYear() const{ return fBonusMilesThisYear; }
    int statusMilesThisYear() const{ return fStatusMilesThisYear; }
    int totalMilesThisYear() const{ return fTotalMilesThisYear; }
    int totalSegments() const{ return fTotalSegments; }
    int totalFlightMiles() const{ return fTotalFlightMiles; }
    int availableMiles() const{ return fAvailableMiles; }

    double segmentsPerWeek() const{ return fSegmentsPerWeek; }
    double milesPerWeek() const{ return fMilesPerWeek; }

    QString mvpON() const;
    QString mvpGoldON() const;
    QString mvp75kON() const;

    QString mvpReq() const;
    QString goldReq() const;
    QString _75kReq() const;

    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;

    void setShowFlightsOnly( bool value );
    void setShowThisYearOnly( bool value );

public slots:
    bool select();
    void slotDataChanged();

signals:
    void sigUpdateValues();
private:
    void computeValues( bool aOK );
    void setFilter();
    int fSegmentsThisYear;
    int fActualMilesThisYear;
    int fBonusMilesThisYear;
    int fStatusMilesThisYear;
    int fTotalMilesThisYear;
    int fTotalSegments;
    int fTotalFlightMiles;
    int fAvailableMiles;
    double fSegmentsPerWeek;
    double fMilesPerWeek;
    SMVPStatusRemain fMVPOn;
    SMVPStatusRemain fMVPGoldOn;
    SMVPStatusRemain fMVP75kOn;
    bool fShowFlightsOnly{ false };
    bool fShowThisYearOnly{ false };
};

#endif 

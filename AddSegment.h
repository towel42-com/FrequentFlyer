#ifndef CAddSegment_H
#define CAddSegment_H

#include <QDialog>
#include <memory>
class QSqlRecord;
class CMilesModel;
namespace Ui{ class CAddSegment; }
class CAddSegment : public QDialog
{
	Q_OBJECT
public:
	CAddSegment(QWidget *parent = 0);
	~CAddSegment();


public slots:
    virtual void accept();
    void slotTypeChanged();
    void slotFromChanged( const QString & from );
    void slotToChanged( const QString & to );

    void fillRecord( QSqlRecord & record );
    void slotStatusChanged();
private:
    QString getActivityInfo() const;
    QString getStatus() const;
    int getMiles() const;
    int getBonus() const;
    int getTotal() const;
    int getStatusMiles() const;
    QStringList loadFrom();
    QStringList getToForFrom( const QString & from );
	std::unique_ptr< Ui::CAddSegment > fImpl;
    static QDate sLastDate;
};

#endif 

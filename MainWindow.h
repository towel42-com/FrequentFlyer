#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>
#include <QDate>
#include <memory>
class QSplashScreen;
class CMilesModel;
class CFilterModel;
namespace Ui{ class CMainWindow; }
class CMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	CMainWindow(QWidget *parent = 0);
	~CMainWindow();

public slots:
    void slotImportSegments();
    void slotAddSegments();
    void slotValuesChanged();
    void slotRefresh();
    void slotDelSegment();
    void slotOnlyShowFlights();
    void slotOnlyShowThisYear();
private:
    void buildFlights();
    void initDatabase();
    CMilesModel * fModel{ nullptr };
    CFilterModel * fFilterModel{ nullptr };
	std::unique_ptr< Ui::CMainWindow > fImpl;
};

#endif // ORDERPROCESSOR_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTimer>

#include "credentials.h"
#include "qdatetime.h"

#define GUI_SW_VERSION_MAJOR    "1"
#define GUI_SW_VERSION_MINOR_1  "0"
#define GUI_SW_VERSION_MINOR_2  "0"

typedef enum
{
    PAG_GENERATOR = 0,
    PAG_EDITOR = 1,
    PAG_NUMEL,
} enPages;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void timerUpdate(void);
    void onStatusBarClicked(void);
    void onEnableEditor(void);
    void onSearch(void);
    void on_pbnAddDelete_clicked();
    void on_lneNewTopic_textChanged();
    void on_pbnRestart_clicked();
    void on_pbnDeleteAll_clicked();
    void on_pbnGenerate_clicked();

private:
    void initFiles(void);
    void initGUI(void);
    void refreshGUI(void);
    bool searchTopicInFile(QFile *file, const QString &topic);
    void deleteTopicFromFile(const QString &topic);
    void addTopicToFile(QFile *file, const QString &topic);
    QString getRandomUnusedTopic(void);
    void checkPermissionForPicking(void);

    Ui::MainWindow *ui;
    Credentials *credentials;
    QTimer *timer;
    QFile *fileTopicList;
    QFile *fileTopicUsed;
    QDateTime timeLastPicking;
    bool bSearched;
};
#endif // MAINWINDOW_H

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clickablelabel.h"
#include "TChronoMeter.h"

#include <QDir>
#include <QTime>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QRandomGenerator>

#define TIMER_TICK          (1000)
#define TIMEOUT_PICKING     (20 * 3600)

const QString strStyleRed    = "background-color: rgb(130, 40, 40);";
const QString strStyleGreen  = "background-color: rgb(50, 130, 50);";

const QString strTimeSinceLastPicking = "Last picking - ";
const QString strTimeRemaining = "Time remaining - ";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString strAppName = "DailyTopic - v%1.%2.%3";
    strAppName.replace("%1", QString(GUI_SW_VERSION_MAJOR));
    strAppName.replace("%2", QString(GUI_SW_VERSION_MINOR_1));
    strAppName.replace("%3", QString(GUI_SW_VERSION_MINOR_2));
    this->setWindowTitle(strAppName);

    ClickableLabel *lblStatus = new ClickableLabel(this);
    lblStatus->setText("Ownership:  Romina Calini");
    lblStatus->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lblStatus->setCursor(Qt::PointingHandCursor);
    ui->statusbar->addPermanentWidget(lblStatus);
    connect(lblStatus, &ClickableLabel::clicked, this, &MainWindow::onStatusBarClicked);

    bSearched = false;
    connect(ui->lneNewTopic, &QLineEdit::returnPressed, this, &MainWindow::onSearch);

    timer = new QTimer(this);
    connect( timer, SIGNAL( timeout() ), this, SLOT( timerUpdate() ) );
    timer->start(TIMER_TICK);

    initFiles();
    initGUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerUpdate(void)
{
    static int iTimeOld = QTime::currentTime().msecsSinceStartOfDay();
    int  iTimeNow = QTime::currentTime().msecsSinceStartOfDay();
    int iDelta = iTimeNow - iTimeOld;
    tClock.IRQ_update(iDelta);

    checkPermissionForPicking();

    iTimeOld = iTimeNow;
}

void MainWindow::checkPermissionForPicking(void)
{
    QDateTime timeNow = QDateTime::currentDateTime();
    if ( timeLastPicking.isValid() )
    {
        int elapsedSeconds = timeLastPicking.secsTo(timeNow);
        float progress = ( (float)elapsedSeconds / (float)TIMEOUT_PICKING ) * 100.0f;
        int progressValue = static_cast<int>(progress);
        if (progressValue < 0)
        {
            progressValue = 0;
        }
        else if (progressValue > 100)
        {
            progressValue = 100;
        }
        ui->pgbNextPicking->setValue(progressValue);
        int remainingSeconds = TIMEOUT_PICKING - elapsedSeconds;
        if (remainingSeconds < 0)
        {
            remainingSeconds = 0;
        }
        QTime timeRemaining(0, 0);
        timeRemaining = timeRemaining.addSecs(remainingSeconds);
        ui->lblTimeRemaining->setText( strTimeRemaining + timeRemaining.toString() );
        if (remainingSeconds == 0)
        {
            ui->lblTopic->setText("---");
            ui->pbnGenerate->setEnabled(true);
        }
        else
        {
            ui->pbnGenerate->setEnabled(false);
        }
    }
    else
    {
        ui->lblTopic->setText("---");
        ui->pbnGenerate->setEnabled(true);
    }
}

void MainWindow::initFiles(void)
{
    QString strFileName_TopicList("Topics/Topics_List.txt");
    QString strFileName_TopicUsed("Topics/Topics_Used.txt");

    if ( !QDir("Topics").exists() )
    {
        QDir().mkdir("Topics");
    }

    fileTopicList = new QFile(strFileName_TopicList);
    fileTopicUsed = new QFile(strFileName_TopicUsed);

    if ( !fileTopicList->open(QIODevice::Append | QIODevice::Text) )
    {
        QMessageBox::critical( this, "Error", QString("Unable to open the file: %1").arg(strFileName_TopicList) );
    }

    if ( !fileTopicUsed->open(QIODevice::Append | QIODevice::Text) )
    {
        QMessageBox::critical( this, "Error", QString("Unable to open the file: %1").arg(strFileName_TopicUsed) );
    }

    fileTopicList->close();
    fileTopicUsed->close();
}

void MainWindow::initGUI(void)
{
    if ( fileTopicUsed->isOpen() )
    {
        fileTopicUsed->close();
    }
    fileTopicUsed->open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(fileTopicUsed);
    QString lastTime = "...";
    QString lastTopic = "---";
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        QStringList parts = line.split("-");
        lastTime = parts[0].trimmed();
        lastTopic = parts[1].trimmed();
    }
    timeLastPicking = QDateTime::fromString(lastTime, "dd/MM/yyyy HH:mm:ss");
    ui->lblTimeLastPicking->setText(strTimeSinceLastPicking + lastTime);
    ui->lblTopic->setText(lastTopic);
    ui->tbWidget->setCurrentIndex(PAG_GENERATOR);
    ui->tbWidget->setTabEnabled(PAG_EDITOR, false);
    ui->lneNewTopic->setPlaceholderText("Insert here the topic...");
    ui->pgbNextPicking->hide();
    refreshGUI();
}

void MainWindow::refreshGUI(void)
{
    if ( fileTopicList->isOpen() )
    {
        fileTopicList->close();
    }
    fileTopicList->open(QIODevice::ReadOnly | QIODevice::Text);
    ui->pteTopicList->setPlainText(fileTopicList->readAll());
    fileTopicList->close();
}

void MainWindow::onStatusBarClicked(void)
{
    credentials = new Credentials(this);
    connect( credentials, SIGNAL( enableEditor() ), this, SLOT( onEnableEditor() ) );
    credentials->open(); // New window is modal, so it's impossible to interact with the main window until the new one is closed
}

void MainWindow::onEnableEditor(void)
{
    ui->tbWidget->setTabEnabled(PAG_EDITOR, true);
}

void MainWindow::onSearch(void)
{
    /// Need to search the topic inside the list
    if ( !bSearched )
    {
        QString strTopic = ui->lneNewTopic->text().trimmed();
        if ( strTopic.isEmpty() )
        {
            QMessageBox::warning(this, "Warning", QString("Insert a valid topic"));
        }
        else
        {
            if ( searchTopicInFile(fileTopicList, strTopic) )
            {
                ui->pbnAddDelete->setText("Delete");
                ui->pbnAddDelete->setStyleSheet(strStyleRed);
                qDebug() << "Topic found:" << strTopic;
            }
            else
            {
                ui->pbnAddDelete->setText("Add");
                ui->pbnAddDelete->setStyleSheet(strStyleGreen);
                qDebug() << "Topic NOT found:" << strTopic;
            }
        }
        bSearched = true;
    }
    /// Add or delete the topic from the list
    else
    {
        on_pbnAddDelete_clicked();
    }
}

void MainWindow::on_pbnAddDelete_clicked()
{
    QString strTopic = ui->lneNewTopic->text().trimmed();
    QString strOperation = ui->pbnAddDelete->text();

    if ( fileTopicList->isOpen() )
    {
        fileTopicList->close();
    }
    fileTopicList->open(QIODevice::Append | QIODevice::Text);
    if ( fileTopicList->isOpen() )
    {
        if (strOperation == "Delete")
        {
            deleteTopicFromFile(strTopic);
            qDebug() << "Topic deleted:" << strTopic;
        }
        else if (strOperation == "Add")
        {
            addTopicToFile(fileTopicList, strTopic);
            qDebug() << "Topic added:" << strTopic;
        }
        else
        {
            QMessageBox::critical(this, "Error", QString("Unkown operation"));
        }
    }
    else
    {
        QMessageBox::critical( this, "Error", QString("Unable to open the file: %1").arg(fileTopicList->fileName() ) );
    }
    fileTopicList->close();
    ui->lneNewTopic->setText("");
    refreshGUI();
}

bool MainWindow::searchTopicInFile(QFile *file, const QString &topic)
{
    bool bTopicFound = false;
    if ( file->isOpen() )
    {
        file->close();
    }
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(file);
    // in.setCodec("UTF-8");
    QString strFile = in.readAll();
    if ( strFile.contains(topic, Qt::CaseInsensitive) )
    {
        bTopicFound = true;
    }
    file->close();
    return (bTopicFound);
}

void MainWindow::deleteTopicFromFile(const QString &topic)
{
    if ( fileTopicList->isOpen() )
    {
        fileTopicList->close();
    }
    fileTopicList->open(QIODevice::ReadOnly | QIODevice::Text);

    QStringList lines;
    QTextStream in(fileTopicList);
    while ( !in.atEnd() )
    {
        QString line = in.readLine();
        if ( !line.contains(topic, Qt::CaseInsensitive) )
        {
            lines << line;
        }
        else
        {
            qDebug() << "Topic to delete:" << line;
        }
    }
    fileTopicList->close();

    if ( fileTopicList->isOpen() )
    {
        fileTopicList->close();
    }
    fileTopicList->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);

    QTextStream out(fileTopicList);
    for (const QString &line : lines)
    {
        out << line << "\n";
    }
    out.flush();
    fileTopicList->close();
}

void MainWindow::addTopicToFile(QFile *file, const QString &topic)
{
    QTextStream out(file);
    // out.setCodec("UTF-8");
    out << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss - ");
    out << topic << "\n";
    out.flush();
    qDebug() << "Topic to add:" << topic;
}

void MainWindow::on_lneNewTopic_textChanged()
{
    ui->pbnAddDelete->setText("---");
    ui->pbnAddDelete->setStyleSheet("");
    bSearched = false;
}

void MainWindow::on_pbnRestart_clicked()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Do you wanna restart from the beginning?");
    msgBox.setInformativeText("The topics will always be the same");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    int iReturn = msgBox.exec();
    switch (iReturn)
    {
    case QMessageBox::Yes:
        if ( fileTopicUsed->isOpen() )
        {
            fileTopicUsed->close();
        }
        fileTopicUsed->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        fileTopicUsed->close();
        break;
    case QMessageBox::Cancel:
    default:
        break;
    }
    ui->lblTopic->setText("---");
    ui->lblTimeLastPicking->setText(strTimeSinceLastPicking + "...");
}

void MainWindow::on_pbnDeleteAll_clicked()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText("Are you sure you want to delete all the topics?");
    msgBox.setInformativeText("This operation cannot be undone");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    int iReturn = msgBox.exec();
    switch (iReturn)
    {
    case QMessageBox::Yes:
        if ( fileTopicList->isOpen() )
        {
            fileTopicList->close();
        }
        fileTopicList->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        fileTopicList->close();
        if ( fileTopicUsed->isOpen() )
        {
            fileTopicUsed->close();
        }
        fileTopicUsed->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        fileTopicUsed->close();
        break;
    case QMessageBox::Cancel:
    default:
        break;
    }
    refreshGUI();
}

void MainWindow::on_pbnGenerate_clicked()
{
    int index = -1;
    QString strTopicUsed = "---";
    if ( fileTopicList->isOpen() )
    {
        fileTopicList->close();
    }
    fileTopicList->open(QIODevice::ReadOnly | QIODevice::Text);
    QStringList lines;
    QTextStream in(fileTopicList);
    while ( !in.atEnd() )
    {
        lines << in.readLine().trimmed();
    }
    fileTopicList->close();
    if ( lines.isEmpty() )
    {
        QMessageBox::warning(this, "Warning", "The topic list file is empty");
    }
    else
    {
        strTopicUsed = getRandomUnusedTopic();
        if ( strTopicUsed.isEmpty() )
        {
            QMessageBox::information(this, "Information", "All topics have been used");
            on_pbnRestart_clicked();
        }
        else
        {
            ui->lblTopic->setText(strTopicUsed);
            qDebug() << "Topic picked:" << strTopicUsed;
            if ( fileTopicUsed->isOpen() )
            {
                fileTopicUsed->close();
            }
            fileTopicUsed->open(QIODevice::Append | QIODevice::Text);
            addTopicToFile(fileTopicUsed, strTopicUsed);
            fileTopicUsed->close();
            timeLastPicking = QDateTime::currentDateTime();
            ui->lblTimeLastPicking->setText(strTimeSinceLastPicking + timeLastPicking.toString("dd/MM/yyyy HH:mm:ss") );
        }
    }
}

QString MainWindow::getRandomUnusedTopic(void)
{
    QString strUnusedTopic = "";

    // 1. Read all topics
    if ( fileTopicList->isOpen() )
    {
        fileTopicList->close();
    }
    fileTopicList->open(QIODevice::ReadOnly | QIODevice::Text);

    QStringList allTopics;
    QTextStream inAll(fileTopicList);
    while (!inAll.atEnd())
    {
        allTopics << inAll.readLine().split("-")[1].trimmed();
    }
    fileTopicList->close();

    // 2. Read used topics
    if ( !allTopics.isEmpty() )
    {
        if ( fileTopicUsed->isOpen() )
        {
            fileTopicUsed->close();
        }
        fileTopicUsed->open(QIODevice::ReadOnly | QIODevice::Text);

        QSet<QString> usedSet;
        QTextStream inUsed(fileTopicUsed);
        while ( !inUsed.atEnd() )
        {
            usedSet.insert( inUsed.readLine().split("-")[1].trimmed().toLower() );
        }
        fileTopicUsed->close();

        // 3. Filter unused topics
        QStringList unusedTopics;
        for (const QString &topic : allTopics)
        {
            if ( !usedSet.contains( topic.toLower() ) )
            {
                unusedTopics << topic;
            }
        }

        // 4. Pick a random unused topic
        if ( !unusedTopics.isEmpty() )
        {
            int index = QRandomGenerator::global()->bounded( unusedTopics.size() );
            strUnusedTopic = unusedTopics.at(index);
        }
    }
    return (strUnusedTopic);
}

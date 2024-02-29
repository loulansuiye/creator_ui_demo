#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QDir>


QT_BEGIN_NAMESPACE


namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void  download(const QString &url, const QString &fileid, const QString &filepath);
    void  upload(const QString &url, const QString &filepath);
    bool  request(const QString &url);
    std::string  get_file_md5(std::string filepath);
private slots:
    bool network_request(QString &str_url);
    void onFinished();
    void onReadyRead();

    void on_le_ip_textChanged(const QString &arg1);
    void on_btn_upload_clicked();
    void on_btn_getdevinfo_clicked();
    void on_btn_start_clicked();

private:
    Ui::MainWindow *ui;
    QString getfilepath();
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_reply;
    QFile* m_file;
    QString g_ip,g_port;
};
#endif // MAINWINDOW_H

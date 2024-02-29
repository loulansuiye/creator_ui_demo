#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QString>
#include<QFileDialog>
#include<QDebug>
#include<QMessageBox>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include<QUrl>
#include"md5.h"
#include<iostream>
#include<fstream>

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_networkManager = new QNetworkAccessManager(this);
    m_reply = nullptr;
    m_file = nullptr;
    g_port="8080";
    g_ip="192.168.42.1";
    connect(m_reply, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getfilepath(){
    QString fileName = QFileDialog::getOpenFileName(this,QStringLiteral("文件对话框！"),".",QStringLiteral("gcode file(*.gc *.gcode *.wws)"));
    if (!fileName.isEmpty())
    {
        qDebug() << "Selected file: " << fileName;
        ui->le_filepath->setText(fileName);
    }    else{
        QMessageBox::warning(this,u8"文件夹路径有误",u8"请确认路径");
    }

    return fileName;
}

//"http://example.com/submit"
bool MainWindow::network_request(QString &str_url){
    QUrl url(str_url); // 设置请求的URL
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

}

bool MainWindow::request(const QString &url){
    return true;
}

void MainWindow::upload(const QString &url, const QString &filepath)
{
    qDebug()<<"[upload file]"<<url<<QFileInfo(filepath).fileName();
    QFile *file=new QFile(filepath);
    if(!file->open(QIODevice::ReadOnly)){
        file->deleteLater();
        qDebug()<<"open file error";
        return;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Content-Type","multipart/form-data");

    //QHttpMultiPart需要在请求完成后释放
    QHttpMultiPart *multi_part = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart file_part;
    file_part.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant(QString("form-data; name=\"myfile\"; filename=\"%1\";")
                                     .arg(QFileInfo(filepath).fileName())));
    //part.header加上这句flask.request.files才能拿到form-data的信息
    //注意不是request的header
    file_part.setRawHeader("Content-Type", "multipart/form-data");
    file_part.setBodyDevice(file);
    file->setParent(multi_part);
    multi_part->append(file_part);

    QNetworkReply *reply = m_networkManager->post(request,multi_part);
    multi_part->setParent(reply); //在删除reply时一并释放

    //因为是测试所以同步等待
    QEventLoop eventLoop;
    //上传进度
    connect(reply, &QNetworkReply::uploadProgress,
            this, [this](qint64 bytesSent, qint64 bytesTotal){
                qDebug()<<"[upload file] bytesSend"<<bytesSent<<"bytesTotal"<<bytesTotal;
            });
    //结束退出事件循环
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();

    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"reply"<<status_code<<QString(reply->readAll());
    qDebug()<<"[upload file] finished";
}

std::string MainWindow::get_file_md5(std::string filepath)
{
    printf("file path:%s\n",filepath.c_str());
    MD5_CTX ctx;
    md5_init(&ctx);
    std::ifstream infile;
    infile.open(filepath.c_str(), ios::in | ios::binary);
    uint8_t data;
    uint8_t md5_buff[16];
    while (infile.read((char *)&data, sizeof(data)))
    {
        md5_update(&ctx, &data, sizeof(data));
    }
    infile.clear();
    infile.seekg(0, ios::beg);

    md5_final(&ctx, md5_buff);
    char str_md5_buff[4];
    std::string str_md5 = {""}; //= str_md5_buff;
    for (int i = 0; i < sizeof(md5_buff); i++)
    {
        sprintf(str_md5_buff, "%.2X", md5_buff[i]);
        //printf("%.2X", md5_buff[i]);
        str_md5 += str_md5_buff;
    }

    infile.close();
    fflush(stdout);
    return str_md5;
}



//网络响应结束
void MainWindow::onFinished()
{
    QFileInfo fileInfo;
    fileInfo.setFile(m_file->fileName());

    m_file->close();
    delete m_file;
    m_file = nullptr;

    m_reply->deleteLater();
    m_reply = nullptr;
}

//读取下载的数据
void MainWindow::onReadyRead()
{
    m_file->write(m_reply->readAll());   //将返回的数据进行读取，写入到临时文件中
    qDebug("result:%s\n",m_file->readAll());
}



void MainWindow::on_le_ip_textChanged(const QString &arg1)
{
    g_ip = arg1;
}

void MainWindow::on_btn_upload_clicked()
{
    QString file = getfilepath();
    ui->le_filepath->setText(file);
    QString str_path=QLatin1String("/process/upload?");
    std::string str_tmp_md5 = get_file_md5(file.toStdString());
    QString str_md5 =str_tmp_md5.c_str();
    if(!g_ip.isEmpty() && !g_port.isEmpty()){
        QString url = "http://"+g_ip+":"+g_port+str_path+"md5="+str_md5;
        upload(url,file);
    }else{
        qDebug("ip or port is empty!\n");
    }

}


void MainWindow::on_btn_getdevinfo_clicked()
{
    QNetworkRequest request_devinfo;
    QString str_devinfo = QLatin1String("/device/info");
    if(!g_ip.isEmpty() && !g_port.isEmpty()){
        QString str_url = "http://"+g_ip+":"+g_port+str_devinfo;
        QUrl url =QUrl(str_url);
        request_devinfo.setUrl(url);
        request_devinfo.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json;charset=utf-8"));
        QString token = "xxxxxxxxxxxxxxxxxxxxxxxx";
        QString token_headerData = token;
        request_devinfo.setRawHeader("token",token_headerData.toLocal8Bit());
        QNetworkReply *reply = m_networkManager->get(request_devinfo);    //get请求头
        //开启事件循环，直到请求完成
        QEventLoop loop;
        connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        loop.exec();
        int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray ret_data = reply->readAll();
        qDebug()<<"reply"<<status_code<<"\n"<<QString(ret_data);
        qDebug()<<"get devinfo finished";
        ui->textEdit->setText(QString(ret_data));
    }else{
        qDebug("ip or port is empty!\n");
    }
}


void MainWindow::on_btn_start_clicked()
{
    QNetworkRequest req_start;
    QString str_start = QLatin1String("/process/start");
    if(!g_ip.isEmpty() && !g_port.isEmpty()){
        QString str_url = "http://"+g_ip+":"+g_port+str_start;
        QUrl url =QUrl(str_url);
        req_start.setUrl(url);
        req_start.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json;charset=utf-8"));
        QString token = "xxxxxxxxxxxxxxxxxxxxxxxx";
        QString token_headerData = token;
        req_start.setRawHeader("token",token_headerData.toLocal8Bit());
        QNetworkReply *reply = m_networkManager->get(req_start);

        QEventLoop loop;
        connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
        loop.exec();
        int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray ret_data = reply->readAll();
        qDebug()<<"reply"<<status_code<<"\n"<<QString(ret_data);
        qDebug()<<"get devinfo finished";
        ui->textEdit->setText(QString(ret_data));

    }else{
        qDebug("ip or port is empty!\n");
    }
}


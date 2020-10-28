#ifndef DATABASEMNG_H
#define DATABASEMNG_H

#include <QObject>
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QDebug>

class FaceSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public :
    FaceSqlTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase())
            : QSqlTableModel(parent, db) {;}
    QVariant data(const QModelIndex &idx, int role) const
    {
        if(role == Qt::BackgroundColorRole)
        {
            QSqlRecord rec = record(idx.row());
            if(!rec.value("checkst").toString().compare("1"))
                return QVariant(QColor(Qt::red));
        }
        return QSqlTableModel::data(idx, role);
    }
};

class AuthPlanViewModel : public QStandardItemModel
{
    Q_OBJECT
public:
    AuthPlanViewModel(QObject *parent = nullptr, QString modeType = QString())
            : QStandardItemModel(parent) { type = modeType;}

    QVariant data(const QModelIndex &index, int role) const
    {
        if(role == Qt::TextAlignmentRole)
        {
        }
        else if(role == Qt::BackgroundColorRole)
        {
            bool bSonNode = index.parent().isValid();
            QStandardItem *sItem = bSonNode ? itemFromIndex(index.parent())->child(index.row(), 4):
                                              itemFromIndex(this->index(index.row(), 3));
            if(sItem != nullptr && !sItem->text().isEmpty())
            {
                if(sItem->text().compare("*") == 0)
                    return QVariant(QColor(Qt::red));
            }
        }
        return QStandardItemModel::data(index, role);
    }

    QString getModeType() { return type;}

private:
    QString type;
};


class AttendRecordItemModel : public QStandardItemModel
{
    Q_OBJECT
public :
    AttendRecordItemModel(QObject *parent = nullptr)
        : QStandardItemModel(parent)
    {
        QStringList header;
        header << "rid" << "userid" << "deviceid" << "personId" << "groupId" << "name"
               << "age" << "gender" << "phone"  << "nation" << "country" << "birth" << "address" << "email" << "certtype"
                 << "certid" << "timestamp" << "imgpath";
        setHorizontalHeaderLabels(header);
    }
};

class AuthGroupListModel : public QStandardItemModel
{
public:
    AuthGroupListModel(QObject *parent = nullptr)
        : QStandardItemModel(parent)
    {
        QStringList header;
        header << "id" << "name";
        setHorizontalHeaderLabels(header);
    }
};


class BlackWhiteListModel : public QStandardItemModel
{
    Q_OBJECT
public :
    BlackWhiteListModel(QObject *parent = nullptr)
        : QStandardItemModel(parent)
    {
        QStringList header;
        header << "id" << "name";
        setHorizontalHeaderLabels(header);
    }
};


class DataBaseMng
{
public:
    DataBaseMng();

    FaceSqlTableModel* getTableModel(QString tableName);
    FaceSqlTableModel* getTableModel();
    FaceSqlTableModel* getSrchTableModel(QString tableName, QString filter);

    FaceSqlTableModel *tmodel;
    FaceSqlTableModel *tModel, *srchModel;

private:
    QSqlDatabase database;
    QMap<QString, QSqlTableModel*> tableModelList;

protected:
    bool createDB(QString dbname, QString dir = QString());
    void close(QString dbname);
    bool hasTable(QString tableName);
    bool createTable(QString tableName, QString strs);
    QSqlRecord readQRecord(QString tableName, int row);
    bool writeQRecord(QString tableName, QSqlRecord record);
    bool delRecord(QString tableName, int row);
    bool modRecord(QString tableName, int row, QSqlRecord record);

    bool setCurrentTable(QString tableName, QString filter);
    bool delCRecord(int row);
    bool delCRecord1(int row);
    bool modCRecord(int row, QSqlRecord record);
    bool writeCRecord(QSqlRecord record);
    bool writeCRecord1(QSqlRecord record, int count = 1);
    bool writeSubmitAll();
    QSqlRecord readCRecord(int row);
};

class AQIDataBaseMng : public DataBaseMng
{
   public :
    AQIDataBaseMng();
    bool addRecord(QString time, QString a18, QString a19);
    bool setCurrFilter(QString filter);

    QSqlTableModel *getRecordTable(QString filter);
};

class StaffDataBaseMng : public DataBaseMng
{

public:
    StaffDataBaseMng();
    bool addPerson1(QString name, QString age, QString id, QString gender, QString phNum,
                   QString grp, QString address, QString mail, QString imgPath, QString workid,
                    QString cardNum, QString password, int authType, int roleType, int cnt);
    bool submitAll();
    bool addPerson(QString name, QString age, QString id, QString gender, QString phNum,
                   QString grp, QString address, QString mail, QString imgPath, QString workid,
                   QString cardNum, QString password, int authType, int roleType);
    bool delPerson(int row);
    bool modPerson(int row, QString name, QString age, QString id, QString gender, QString phNum,
                   QString grp, QString address, QString mail, QString imgPath, QString userId,
                   QString cardNum, QString password, int authType, int roleType);

    bool setCurrFilter(QString filter);
    FaceSqlTableModel *getStaffListTable();
    FaceSqlTableModel *getStaffSrchTable(QString filter);
};

class DevListDatabaseMng : public DataBaseMng
{
public :
    DevListDatabaseMng();
    bool addDev(QString chn, QString ip, QString type, QString pos, QString mac,
                QString chipid, QString sdk, QString name);
    bool delDev(int idx);
    bool modDev(int idx, QString chn, QString type, QString pos);

    bool setDevStmType(int index, int type);

    FaceSqlTableModel *setCurrFilter(QString filter);
    FaceSqlTableModel *getDevListTable();
    FaceSqlTableModel *getDevSrchTable(QString filter);
    QList<QMap<QString,QString>> getDevList();
    QList<QMap<QString,QString>> getDevList1();
    QMap<QString, QMap<QString,QString>> getDevsMap();
};

class AttendRecordDBMng : public DataBaseMng
{
public:
    AttendRecordDBMng(QString dbFileName, QString dir);

    QSqlTableModel *getRecordListTable();
    QSqlTableModel *getRecordSrchTable(QString filter);

    bool addAttendRecord1(int recordId, QString name, QString age, QString gender, QString phone,
                          QString certid, QString timestamp, QString userid,  int cnt);

    bool addAttendRecordsFromItemModel(AttendRecordItemModel *model);
    bool submitAll();
    bool setCurrFilter(QString fliter);
};

class AuthGroupDBMng : public DataBaseMng
{
public:
    AuthGroupDBMng();

    QSqlTableModel *getAuthGroupListTable();

    bool addAuthGroup(QString version,QString groupName,
                      QString planInJson, QString pplInGrp, QString devInGrp);
    bool delAuthGroup(int row);
    bool modAuthGroup(int row, QString version, QString groupName,
                      QString planInJson, QString pplInGrp, QString devInGrp);

    bool submitAll();
};

#endif // DATABASEMNG_H

#include "databasemng.h"
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>

DataBaseMng::DataBaseMng()
{

}

bool DataBaseMng::createDB(QString dbname, QString dir)
{
    QString fullName = QCoreApplication::applicationDirPath()
            .append(QString("/%1").arg(dir))
            .append(QString("/%1.db").arg(dbname));
//    if(!QSqlDatabase::contains(dbname))
    qDebug()<<"db full name" << fullName;
    if(!QSqlDatabase::contains(fullName))
    {
//        database = QSqlDatabase::addDatabase("QSQLITE", dbname);
        database = QSqlDatabase::addDatabase("QSQLITE", fullName);
        database.setDatabaseName(QCoreApplication::applicationDirPath()
                                 .append(QString("/%1").arg(dir))
                                 .append(QString("/%1.db").arg(dbname)));
    }
    else
    {
//        database = QSqlDatabase::database(dbname);
        database = QSqlDatabase::database(fullName);
    }

    bool ret = database.open();
    qDebug()<<"db open" << ret;
    if(ret)
    {
        tmodel = new FaceSqlTableModel(nullptr, database);
        srchModel = new FaceSqlTableModel(nullptr, database);
        tModel = new FaceSqlTableModel(nullptr, database);
    }

    return ret;
}

bool DataBaseMng::setCurrentTable(QString tableName, QString filter)
{
    tmodel->setTable(tableName);
    tModel->setTable(tableName);
    if(!filter.isEmpty())
    {
        tModel->setFilter(filter);
    }
    tModel->select();
}

bool DataBaseMng::delCRecord(int row)
{
    bool res ;
    tModel->removeRow(row);
    res = tModel->submitAll();
    if(res)
        res = tModel->select();
    return res;
}

bool DataBaseMng::delCRecord1(int row)
{
    bool res;
    res = tModel->removeRow(row);
    return res;
}

bool DataBaseMng::modCRecord(int row, QSqlRecord record)
{
    bool res;
    res = tModel->setRecord(row, record);
    if(res)
    {
        res = tModel->submitAll();
    }
    return res;
}

bool DataBaseMng::writeCRecord1(QSqlRecord record, int count)
{
    bool res;
    static int rowCnt = 0;
    static int rowNum = 0;
    static int curRow = 0;
    int cnt = -1;

    if(rowNum == 0)
    {
        tmodel->select();
        while(tmodel->canFetchMore())
            tmodel->fetchMore();
        cnt = tmodel->rowCount();
        qDebug()<<"row count" << cnt;
        curRow = cnt;
        rowNum = tmodel->record(cnt-1).value(0).toUInt() + 1;

        tmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
        tmodel->database().transaction();
        res = tmodel->insertRows(cnt, count);
        qDebug()<<res;
    }
    record.setValue(0, rowNum);
    rowCnt += 1;

    for(int i = 0; i < record.count(); i ++)
    {
        if(i == 0)
        {
            tmodel->setData(tmodel->index(curRow, i), record.value(i).toUInt());
        }
        else {
            tmodel->setData(tmodel->index(curRow, i), record.value(i).toString());
        }
    }

    rowNum ++;
    curRow ++;

    if(rowCnt == count)
    {
        rowNum = 0;
        rowCnt = 0;
        curRow = 0;
    }
    return res;
}

bool DataBaseMng::writeSubmitAll()
{
    bool res ;
    res = tmodel->submitAll();
    if(res)
        tmodel->database().commit();
    tModel->select();
}

bool DataBaseMng::writeCRecord(QSqlRecord record)
{
    int cnt = -1;
    tmodel->select();
    while(tmodel->canFetchMore())
        tmodel->fetchMore();
    cnt = tmodel->rowCount();
    record.setValue(0, tmodel->record(cnt-1).value(0).toUInt() + 1);
    qDebug()<<record.value(0).toInt();
    tmodel->insertRecord(cnt, record);
    tModel->select();
    return tmodel->submitAll();
}


QSqlRecord DataBaseMng::readQRecord(QString tableName, int row)
{
    if(tmodel->tableName().compare(tableName))
    {
        tmodel->setTable(tableName);
        tmodel->select();
    }
    return tmodel->record(row);
}

bool DataBaseMng::writeQRecord(QString tableName, QSqlRecord record)
{
    int cnt = -1;
    if(tmodel->tableName().compare(tableName))
    {
        tmodel->setTable(tableName);
        tmodel->select();
    }

    cnt = tmodel->rowCount();
    record.setValue(0, tmodel->record(cnt-1).value(0).toUInt() + 1);
    tmodel->insertRecord(cnt, record);
    return tmodel->submitAll();
}


FaceSqlTableModel *DataBaseMng::getTableModel(QString tableName)
{
    tmodel->setTable(tableName);
    tmodel->select();
    return tmodel;
}

FaceSqlTableModel* DataBaseMng::getTableModel()
{
    return tModel;
}

FaceSqlTableModel* DataBaseMng::getSrchTableModel(QString tableName, QString filter)
{
    srchModel->setTable(tableName);
    srchModel->setFilter(filter);
    srchModel->select();

    return srchModel;
}


bool DataBaseMng::hasTable(QString tableName)
{
    return database.tables().contains(tableName);
}

bool DataBaseMng::createTable(QString tableName, QString strs)
{
    bool res;
    if(database.tables().contains(tableName))
        return true;

    QSqlQuery query(database);
    res = query.exec(QString("create table %1(%2)").arg(tableName).arg(strs));
    return res;
}

bool DataBaseMng::delRecord(QString tableName, int idx)
{
    bool res ;
    if(tmodel->tableName().compare(tableName))
    {
        tmodel->setTable(tableName);
        tmodel->select();
    }
    tmodel->removeRow(idx);

    res = tmodel->submitAll();
    if(res)
        tmodel->select();

    return res;
}

bool DataBaseMng::modRecord(QString tableName, int row, QSqlRecord record)
{
    bool res;
    if(tmodel->tableName().compare(tableName))
    {
        tmodel->setTable(tableName);
        tmodel->select();
    }
    res = tmodel->setRecord(row, record);
    if(res)
    {

     res = tmodel->submitAll();
    }

    return res;
}

void DataBaseMng::close(QString dbname)
{
    database.close();
}


DevListDatabaseMng::DevListDatabaseMng()
{
     if(createDB("devdb"))
    {
       createTable("devlist", "id int primary key, chn text, ip text, type text, pos text, \
                               mac text, chipid text, sdk text, name text, facenum text, stm int");
                   setCurrentTable("devlist", QString());
    }
}

bool DevListDatabaseMng::addDev(QString chn, QString ip, QString type, QString pos, QString mac,
                             QString chipid, QString sdk, QString name)
{
    QSqlRecord record = tmodel->record();
    record.setValue(1, chn);
    record.setValue(2,  ip);
    record.setValue(3,  type);
    record.setValue(4,  pos);
    record.setValue(5,  mac);
    record.setValue(6,  chipid);
    record.setValue(7,  sdk);
    record.setValue(8,  name);
    record.setValue(10, 0);
    return writeQRecord("devlist", record);
}

bool DevListDatabaseMng::delDev(int idx)
{
    return delRecord("devlist", idx);
}

bool DevListDatabaseMng::setDevStmType(int idx, int type)
{
    QSqlRecord record = tmodel->record(idx);
    record.setValue(10, type);

    return modRecord("devlist", idx, record);
}


bool DevListDatabaseMng::modDev(int idx, QString chn, QString type, QString pos)
{
    QSqlRecord record = tmodel->record(idx);
    record.setValue("name", chn);
    record.setValue("pos", pos);
    record.setValue("type", type);

    return modRecord("devlist", idx, record);
}



FaceSqlTableModel *DevListDatabaseMng::getDevListTable()
{
    return getTableModel("devlist");
}

FaceSqlTableModel *DevListDatabaseMng::getDevSrchTable(QString filter)
{
    return getSrchTableModel("devlist", filter);
}

QList<QMap<QString,QString>> DevListDatabaseMng::getDevList()
{
    QList<QMap<QString,QString>> devList;
    QStringList sdkList;
    QString sdk,app;

    tmodel->select();
    int cnt = tmodel->rowCount();
    for(int i =0 ; i< cnt; i ++)
    {
        QMap<QString, QString> dev;
        dev["index"] = tmodel->record(i).value(0).toString();
        dev["chn"] = tmodel->record(i).value(1).toString();
        dev["ip"] = tmodel->record(i).value(2).toString();
        dev["type"] = tmodel->record(i).value(3).toString();
        dev["pos"]  = tmodel->record(i).value(4).toString();
        dev["stm"]  = tmodel->record(i).value("stm").toString();
        dev["name"] = tmodel->record(i).value("name").toString();
        dev["chipid"] = tmodel->record(i).value("chipid").toString();
        dev["playStatus"] = "连接中";
        sdkList = tmodel->record(i).value(7).toString().split("-");
        if(sdkList.size() >= 2)
        {
            sdk = sdkList[0];
            app = sdkList[1];
        }
        else if(sdkList.size() == 1)
        {
           sdk = sdkList[0];
           app = QString();
        }
        else
        {
            sdk = QString();
            app = QString();
        }
        dev["app"] = app;
        dev["sdk"] = sdk;

        devList.append(dev);
    }

    return devList;
}

QList<QMap<QString,QString>> DevListDatabaseMng::getDevList1()
{
    QList<QMap<QString,QString>> devList;
    QStringList sdkList;
    QString sdk,app;

    tmodel->select();
    int cnt = tmodel->rowCount();
    for(int i =0 ; i< cnt; i ++)
    {
        QMap<QString, QString> dev;
        dev["name"] = tmodel->record(i).value("name").toString();
        sdkList = tmodel->record(i).value(7).toString().split("-");
        if(sdkList.size() >= 2)
        {
            sdk = sdkList[0];
            app = sdkList[1];
        }
        else if(sdkList.size() == 1)
        {
           sdk = sdkList[0];
           app = QString();
        }
        else
        {
            sdk = QString();
            app = QString();
        }

        devList.append(dev);
    }

    return devList;
}


QMap<QString,QMap<QString,QString>> DevListDatabaseMng::getDevsMap()
{
     QMap<QString, QMap<QString,QString>> devsMap;

    tmodel->select();
    int cnt = tmodel->rowCount();
    for(int i =0 ; i< cnt; i ++)
    {
        QMap<QString, QString> dev;
        dev["index"] = tmodel->record(i).value(0).toString();
        dev["chn"] = tmodel->record(i).value(1).toString();
        dev["ip"] = tmodel->record(i).value(2).toString();
        dev["type"] = tmodel->record(i).value(3).toString();
        dev["pos"]  = tmodel->record(i).value(4).toString();
        devsMap[dev["ip"]] = dev;
    }

    return devsMap;
}

AQIDataBaseMng::AQIDataBaseMng()
{
   if(createDB("aqidb"))
   {
       createTable("aqirecords", "id int primary key, timestamp text, a18 text, a19 text");
       setCurrentTable("aqirecords", QString());
   }
}

bool AQIDataBaseMng::addRecord(QString timestamp, QString a18, QString a19)
{
    QSqlRecord record = getTableModel()->record();
    record.setValue("timestamp", timestamp);
    record.setValue("a18", a18);
    record.setValue("a19", a19);
    return writeCRecord(record);
}

bool AQIDataBaseMng::setCurrFilter(QString filter)
{
     setCurrentTable("aqidb", filter);
}

QSqlTableModel *AQIDataBaseMng::getRecordTable(QString filter)
{
    return getSrchTableModel("aqirecords", filter);
}


StaffDataBaseMng::StaffDataBaseMng()
{
    if(createDB("staffdb"))
    {
        createTable("stafflist", "id int primary key, checkst text, workid text, name text, age text, gender text, \
                    phnum text, grp text, address text, mail text, pid text, personid text, imgpath text, \
                    cardNum text, password text, authType int, roleType int");
        setCurrentTable("stafflist", QString());
    }
}

bool StaffDataBaseMng::addPerson1(QString name, QString age, QString id, QString gender, QString phNum,
                                  QString grp, QString address, QString mail, QString imgPath, QString workid,
                                  QString cardNum, QString pwd, int authType, int roleType, int cnt)
{
//    qDebug()<<"to add";
    QFileInfo fileinfo(imgPath);
    QSqlRecord record = getTableModel()->record();
    record.setValue("name", name);
    record.setValue("age", age);
    record.setValue("pid", id);
    record.setValue("gender", gender);
    record.setValue("phnum", phNum);
    record.setValue("grp", grp);
    record.setValue("address", address);
    record.setValue("mail", mail);
    record.setValue("imgpath", imgPath);
    record.setValue("workid", workid);
    record.setValue("cardNum", cardNum);
    record.setValue("password", pwd);
//    record.setValue("authType", QVariant(authType));
//    record.setValue("roleType", QVariant(roleType));

    if(!fileinfo.exists())
        record.setValue("checkst", "1");
    else {
        record.setValue("checkst", "0");
    }

    return writeCRecord1(record, cnt);
}

bool StaffDataBaseMng::submitAll()
{
    return writeSubmitAll();
}


bool StaffDataBaseMng::addPerson(QString name, QString age, QString id, QString gender, QString phNum,
                                 QString grp, QString address, QString mail, QString imgPath, QString workid,
                                 QString cardNum, QString pwd, int authType, int roleType)
{
    QSqlRecord record = getTableModel()->record();
    record.setValue("name", name);
    record.setValue("age", age);
    record.setValue("pid", id);
    record.setValue("gender", gender);
    record.setValue("phnum", phNum);
    record.setValue("grp", grp);
    record.setValue("address", address);
    record.setValue("mail", mail);
    record.setValue("imgpath", imgPath);
    record.setValue("checkst", QString());
    record.setValue("workid", workid);
    record.setValue("cardNum", cardNum);
    record.setValue("password", pwd);
    record.setValue("authType", authType);
    record.setValue("roleType", roleType);

    return writeCRecord(record);
}

bool StaffDataBaseMng::delPerson(int row)
{
    return delCRecord1(row);
}

bool StaffDataBaseMng::setCurrFilter(QString filter)
{
    setCurrentTable("stafflist", filter);
}

bool StaffDataBaseMng::modPerson(int row, QString name, QString age, QString id, QString gender, QString phNum,
                                 QString grp, QString address, QString mail, QString imgPath, QString userId,
                                 QString cardNum, QString pwd, int authType, int roleType)
{
    QSqlRecord record = getTableModel()->record(row);
    record.setValue("name", name);
    record.setValue("age", age);
    record.setValue("pid", id);
    record.setValue("gender", gender);
    record.setValue("phNum", phNum);
    record.setValue("grp", grp);
    record.setValue("address", address);
    record.setValue("mail", mail);
    record.setValue("checkst", "0");
    record.setValue("workid", userId);
    if(!imgPath.isEmpty())
        record.setValue("imgpath", imgPath);
    record.setValue("cardNum", cardNum);
    record.setValue("password", pwd);
    record.setValue("authType", authType);
    record.setValue("roleType", roleType);

    return modCRecord(row,  record);
}

FaceSqlTableModel *StaffDataBaseMng::getStaffListTable()
{
    return getTableModel("stafflist");
}

FaceSqlTableModel *StaffDataBaseMng::getStaffSrchTable(QString filter)
{
    return getSrchTableModel("stafflist", filter);
}


AttendRecordDBMng::AttendRecordDBMng(QString dbFileName, QString dir)
{
    if(createDB(dbFileName, dir))
    {
        createTable("attend_record", "id int primary key, rid int, userid text, deviceid text, personId text, groupId text, name text,\
                    age text, gender text, phone text, nation text, country text, birth text, address text, email text, certtype text,\
                    certid text, timestamp datetime, imgpath text");
        setCurrentTable("attend_record", QString());
    }
}

bool AttendRecordDBMng::addAttendRecord1(int recordId, QString name, QString age, QString gender,
                                    QString phone, QString certid, QString timestamp, QString userId, int cnt)
{
    QSqlRecord record = getTableModel()->record();
//    QSqlRecord record;
    record.setValue("rid", recordId);
    record.setValue("name",name);
    record.setValue("age", age);
    record.setValue("gender",gender);
    record.setValue("phone",phone);
    record.setValue("certid",certid);
    record.setValue("timestamp",QDateTime::fromString(timestamp, "yyyy-MM-dd HH:mm:ss"));
    record.setValue("userid", userId);
    return writeCRecord1(record, cnt);
}

bool AttendRecordDBMng::submitAll()
{
    return writeSubmitAll();
}

bool AttendRecordDBMng::addAttendRecordsFromItemModel(AttendRecordItemModel *model)
{
    /*
    int count = model->rowCount();
    for(int i = 0; i < count;  i ++)
    {
        addAttendRecord1(model->item(i, AT_DB_RID_IDX)->text().toUInt(),
                         model->item(i, AT_DB_NAME_IDX)->text(),
                         model->item(i, AT_DB_AGE_IDX)->text(),
                         model->item(i, AT_DB_GENDER_IDX)->text(),
                         model->item(i, AT_DB_PHONE_IDX)->text(),
                         model->item(i, AT_DB_CERT_IDX)->text(),
                         model->item(i, AT_DB_TIME_IDX)->text(),
                         model->item(i, AT_DB_USRID_IDX)->text(), count);
        qApp->processEvents();
    }
    */
    submitAll();
}

QSqlTableModel *AttendRecordDBMng::getRecordListTable()
{
    return getTableModel("attend_record");
}

QSqlTableModel *AttendRecordDBMng::getRecordSrchTable(QString filter)
{
    return getSrchTableModel("attend_record", filter);
}

bool AttendRecordDBMng::setCurrFilter(QString filter)
{
    return setCurrentTable("attend_record", filter);
}

AuthGroupDBMng::AuthGroupDBMng()
{
    if(createDB("authgrp"))
    {
        createTable("authgrp", "groupId int primary key, name text, version text, plan text, devlist text, ppl text");
        setCurrentTable("authgrp", QString());
    }
}

bool AuthGroupDBMng::addAuthGroup(QString version, QString groupName, QString planInJson,
                                  QString pplInGrp, QString devInGrp)
{
   QSqlRecord record = getTableModel()->record();
   record.setValue("name", groupName);
   record.setValue("version", version);
   record.setValue("plan", planInJson);
   record.setValue("devlist", devInGrp);
   record.setValue("ppl", pplInGrp);

   return writeCRecord(record);
}

bool AuthGroupDBMng::delAuthGroup(int row)
{
    return delCRecord1(row);
}

bool AuthGroupDBMng::modAuthGroup(int row, QString version, QString groupName, QString planInJson,
                                  QString pplInGrp, QString devInGrp)
{
    QSqlRecord record = getTableModel()->record(row);
   record.setValue("name", groupName);
   record.setValue("version", version);
   record.setValue("plan", planInJson);
   record.setValue("devlist", devInGrp);
   record.setValue("ppl", pplInGrp);

   return modCRecord(row, record);
}

bool AuthGroupDBMng::submitAll()
{
    return writeSubmitAll();
}

#ifndef DATABASE_H
#define DATABASE_H

#include <qstringlist.h>
#include <QtSql>
#include <mainwindow.h>

namespace db_ops{

int create_recipes_table(QSqlDatabase *db);

QStringList scan_recipes_folder();

int insert_recipe_in_db(QString title, QString img_path, QString json_path, QString html_path, QString category, QSqlDatabase *db);

int update_database(QSqlDatabase *db);

int clean_database(QSqlDatabase *db);

}

#endif // DATABASE_H

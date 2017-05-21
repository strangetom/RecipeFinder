#include <database.h>
#include <iostream>

#include <glob.h>
#include <qstringlist.h>
#include <QtSql>
#include <QFileInfo>
#include <mainwindow.h>

int db_ops::create_recipes_table(QSqlDatabase *db)
{
    QString sql = "CREATE TABLE RECIPES("  \
                "TITLE TEXT PRIMARY KEY      NOT NULL," \
                "IMG_PATH            TEXT    NOT NULL," \
                "FILE_PATH           TEXT    NOT NULL," \
                "CATEGORY            TEXT    NOT NULL);";

    db->open();
    QSqlQuery query;
    bool result = query.exec(sql);
    db->close();
    if(result){
        std::cout << "[INFO] Table created successfully" << std::endl;
        return 0;
    }else{
        std::cerr << "[WARNING] SQL error: " << query.lastError().text().toStdString() << std::endl;
        return 1;
    }
}

QStringList db_ops::scan_recipes_folder()
{
    const std::string& pattern = "*/*.md";
    glob_t glob_result;
    glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
    QStringList recipesList;
    for(unsigned int i=0; i<glob_result.gl_pathc; ++i){
        recipesList << QString(glob_result.gl_pathv[i]);
    }
    globfree(&glob_result);
    return recipesList;
}

int db_ops::insert_recipe_in_db(QString title, QString img_path, QString file_path, QString category, QSqlDatabase *db)
{
    db->open();
    QSqlQuery query;
    query.prepare("INSERT into RECIPES (TITLE, IMG_PATH, FILE_PATH, CATEGORY) VALUES (:title, :img_path, :file_path, :category)");
    query.bindValue(":title", title);
    query.bindValue(":img_path", img_path);
    query.bindValue(":file_path", file_path);
    query.bindValue(":category", category);
    bool result = query.exec();
    db->close();
    if(result){
        return 0;
    }else{
        std::cerr << "[WARNING] Error inserting data for " << title.toStdString() << ": " << query.lastError().text().toStdString() << std::endl;
        return 1;
    }
}

int db_ops::update_database(QSqlDatabase *db)
{
    QStringList recipe_list = scan_recipes_folder();
    std::cout << "[INFO] Updating recipe database..." << std::endl;
    foreach(QString path, recipe_list){
        QString file_path = path;
        QString title = path.split('/')[1].replace(".md", "");
        QString img_path = "Images/" + path.split('/')[1].replace(" ", "_").replace(".md", ".jpg");
        QString category = path.split('/')[0];

        db->open();
        QSqlQuery query = QSqlQuery();
        query.setForwardOnly(true);
        query.prepare("SELECT 1 from RECIPES where title = :title");
        query.bindValue(":title", title);
        query.exec();

        if(query.next()){
            std::cout << "[INFO] Record already exists for " << title.toStdString() << std::endl;
        }else{
            int insert = insert_recipe_in_db(title, img_path, file_path, category, db);
            if(insert){
                std::cerr << "[ERROR] Unable to add recipe to database: " << title.toStdString() << std::endl;
            }else{
                std::cout << "[INFO] Added record for " << title.toStdString() << std::endl;
            }
        }

    }
    db->close();
    return 0;
}

int db_ops::clean_database(QSqlDatabase *db)
{
    std::cout << "[INFO] Cleaning recipe database..." << std::endl;
    db->open();
    QSqlQuery query;
    query.exec("SELECT FILE_PATH from RECIPES");

    while(query.next())
    {
        QString file_path = query.value(0).toString();
        QFileInfo file(file_path);
        if(!file.exists())
        {
            QSqlQuery del = QSqlQuery();
            del.prepare("DELETE from RECIPES where FILE_PATH = :file_path");
            del.bindValue(":file_path", file_path);
            del.exec();
            std::cout << "[INFO] " << file_path.toStdString() << " --> REMOVED" << std::endl;
        }
    }
    db->close();
    return 0;
}

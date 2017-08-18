#include <database.h>
#include <iostream>
#include <fstream>
#include <json.hpp>
#include <glob.h>
#include <qstringlist.h>
#include <QtSql>
#include <QFileInfo>
#include <mainwindow.h>

using json = nlohmann::json;

int db_ops::create_recipes_table(QSqlDatabase *db)
{
    QString sql = "CREATE TABLE RECIPES("  \
                "TITLE TEXT PRIMARY KEY      NOT NULL," \
                "IMG_PATH            TEXT    NOT NULL," \
                "JSON_PATH           TEXT    NOT NULL," \
                "HTML_PATH           TEXT    NOT NULL," \
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
    const std::string& pattern = "json/*.json";
    glob_t glob_result;
    glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
    QStringList recipesList;
    for(unsigned int i=0; i<glob_result.gl_pathc; ++i){
        recipesList << QString(glob_result.gl_pathv[i]);
    }
    globfree(&glob_result);
    return recipesList;
}

int db_ops::insert_recipe_in_db(QString title, QString img_path, QString json_path, QString html_path, QString category, QSqlDatabase *db)
{
    db->open();
    QSqlQuery query;
    query.prepare("INSERT into RECIPES (TITLE, IMG_PATH, JSON_PATH, HTML_PATH, CATEGORY) VALUES (:title, :img_path, :json_path, :html_path, :category)");
    query.bindValue(":title", title);
    query.bindValue(":img_path", img_path);
    query.bindValue(":json_path", json_path);
    query.bindValue(":html_path", html_path);
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
    QFileInfo dbfile("recipes.db");
    if(dbfile.size() < 1){
        create_recipes_table(db);
    }
    int num_inserts = 0;

    QStringList recipe_list = scan_recipes_folder();
    std::cout << "[INFO] Updating recipe database..." << std::endl;
    foreach(QString path, recipe_list){
        // Read file in and parse json
        json j;
        std::ifstream json_file(path.toStdString());
        json_file >> j;
        json_file.close();

        QString json_path = path;
        QString title = QString::fromStdString(j["name"].get<std::string>());
        QString img_path = "images/" + QString::fromStdString(j["image"].get<std::string>());
        QString category = QString::fromStdString(j["category"].get<std::string>());
        QString html_path = path.replace("json", "html");

        db->open();
        QSqlQuery query = QSqlQuery();
        query.setForwardOnly(true);
        query.prepare("SELECT 1 from RECIPES where title = :title");
        query.bindValue(":title", title);
        query.exec();

        if(query.next()){
            std::cout << "[INFO] Record already exists for " << title.toStdString() << std::endl;
        }else{
            int insert = insert_recipe_in_db(title, img_path, json_path, html_path, category, db);
            if(insert){
                std::cerr << "[ERROR] Unable to add recipe to database: " << title.toStdString() << std::endl;
            }else{
                num_inserts++;
                std::cout << "[INFO] Added record for " << title.toStdString() << std::endl;
            }
        }

    }
    db->close();
    return num_inserts;
}

int db_ops::clean_database(QSqlDatabase *db)
{
    std::cout << "[INFO] Cleaning recipe database..." << std::endl;
    int num_removals = 0;
    db->open();
    QSqlQuery query;
    query.exec("SELECT JSON_PATH from RECIPES");


    while(query.next())
    {
        QString json_path = query.value(0).toString();
        QFileInfo file(json_path);
        if(!file.exists())
        {
            QSqlQuery del = QSqlQuery();
            del.prepare("DELETE from RECIPES where JSON_PATH = :json_path");
            del.bindValue(":json_path", json_path);
            del.exec();
            std::cout << "[INFO] " << json_path.toStdString() << " --> REMOVED" << std::endl;
            num_removals++;
        }
    }
    db->close();
    std::cout << "[INFO] Finished cleaning database." << std::endl;
    return num_removals;
}

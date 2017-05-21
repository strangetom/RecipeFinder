#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDir>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QtSql>
#include <map>

class QLabel;
class QPushButton;

class SearchBox: public QLineEdit
{
    Q_OBJECT

public:
    using QLineEdit::QLineEdit;

private:
    void keyPressEvent(QKeyEvent *evt);

signals:
    void inputText(QString searchText);

private slots:
    void recipeFiterChanged(QString newFilter);
};


class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);

private slots:
    void openFile(QListWidgetItem *recipe);
    void updateRecipesDiplay(QString searchText);

private:
    void resizeEvent(QResizeEvent *event);
    void createRecipeList();

    QList<QListWidgetItem*> getRecipeList(QString searchText);
    QList<QListWidgetItem*> getAllRecipes();
    QList<QListWidgetItem*> getMatchingRecipes(QString searchText);
    std::map<double, QString> findMatches(QString text);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    SearchBox *searchBox;
    QDir currentDir;
    QListWidget *recipeList;
    QComboBox *recipeBox;
    QLabel *numResults;
};

#endif

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QDir>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QtSql>
#include <QtWebEngineWidgets>
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


class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);

private slots:
    void openFile(QListWidgetItem *recipe);
    void updateRecipesDiplay(QString searchText);

private:
    void resizeEvent(QResizeEvent *event);
    void populateRecipeBox(QComboBox*);
    void createRecipeListWidget();
    void updateDatabase();
    void cleanDatabase();

    void getAllRecipes();
    void getMatchingRecipes(QString searchText);
    std::map<double, QStringList> findMatches(QString text);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    QMenu *optionsMenu;
    QAction *updateDb;
    QAction *cleanDb;

    QWidget *centralWidget;
    SearchBox *searchBox;
    QDir currentDir;
    QListWidget *recipeList;
    QComboBox *recipeBox;
    QLabel *numResults;

    QWebEngineView *recipeView;
};

#endif

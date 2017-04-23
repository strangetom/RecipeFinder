#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDir>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <map>

class QLabel;
class QPushButton;

class SearchBox: public QLineEdit
{
    Q_OBJECT

public:
    using QLineEdit::QLineEdit;

private:
    std::map<double, QString> findFiles(const QStringList &files, const QString &text);
    void keyPressEvent(QKeyEvent *e);
    QString recipeFiter = QString("All Recipes");

signals:
    void updateMatches(std::map<double, QString> matchedFiles);

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
    void showFiles(const std::map<double, QString> &files);

private:
    void createRecipeList();

    SearchBox *searchBox;
    QDir currentDir;
    QListWidget *recipeList;
    QComboBox *recipeBox;
};

#endif

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDir>
#include <QLineEdit>
#include <QListWidget>
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

signals:
    void updateMatches(std::map<double, QString> matchedFiles);
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
    QLabel *textLabel;
    QDir currentDir;
    QListWidget *recipeList;

};

class keyEscapeReceiver : public QObject
{
    Q_OBJECT
protected:
    bool eventFilter(QObject* obj, QEvent* event);
};

#endif

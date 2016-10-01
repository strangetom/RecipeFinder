#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDir>
#include <QLineEdit>

class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;


class SearchBox: public QLineEdit
{
    Q_OBJECT

public:
    using QLineEdit::QLineEdit;

protected slots:
    //void doSearch();
};


class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = 0);

private slots:
    void find();
    void openFileOfItem(int row, int column);

private:
    QStringList findFiles(const QStringList &files, const QString &text);
    void showFiles(const QStringList &files);
    void createFilesTable();

    SearchBox *searchBox;
    QLabel *textLabel;
    QPushButton *findButton;
    QTableWidget *filesTable;

    QDir currentDir;

};

#endif

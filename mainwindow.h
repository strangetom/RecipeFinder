#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDir>
#include <QLineEdit>
#include <map>

class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;


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
    void openFileOfItem(int row, int column);
    void showFiles(const std::map<double, QString> &files);

private:
    void createFilesTable();

    SearchBox *searchBox;
    QLabel *textLabel;
    QTableWidget *filesTable;
    QDir currentDir;

};

class keyEscapeReceiver : public QObject
{
    Q_OBJECT
protected:
    bool eventFilter(QObject* obj, QEvent* event);
};

#endif

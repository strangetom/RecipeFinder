#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDir>
#include <QComboBox>

class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;


class QComboBoxMod : public QComboBox
{
     Q_OBJECT
public:
    // Inherit constructor from base class
    using QComboBox::QComboBox;

protected:
    void keyPressEvent(QKeyEvent *e);
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
    QComboBoxMod *createComboBox(const QString &text = QString());
    void createFilesTable();

    QComboBoxMod *textComboBox;
    QLabel *textLabel;
    QPushButton *findButton;
    QTableWidget *filesTable;

    QDir currentDir;
};

#endif

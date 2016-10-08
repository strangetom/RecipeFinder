#include <QtWidgets>
#include "mainwindow.h"
#include <fts_fuzzy_match.h>
#include <glob.h>
#include <string>
#include <map>

Window::Window(QWidget *parent) : QWidget(parent)
{
    textLabel = new QLabel(tr("Search:"));
    searchBox = new SearchBox();
    connect(searchBox, SIGNAL(updateMatches(std::map<int, QString>)), this, SLOT(showFiles(std::map<int, QString>)));

    createFilesTable();

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(textLabel, 0, 0);
    mainLayout->addWidget(searchBox, 0, 1, 1, 1);
    mainLayout->addWidget(filesTable, 1, 0, 1, 4);
    setLayout(mainLayout);

    setWindowTitle(tr("Find Recipes"));
    resize(500, 300);
}

// Get list of files according to glob patternn
QStringList globVector(const std::string& pattern){
    glob_t glob_result;
    glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
    QStringList files;
    for(unsigned int i=0; i<glob_result.gl_pathc; ++i){
        files << QString(glob_result.gl_pathv[i]);
    }
    globfree(&glob_result);
    return files;
}

void SearchBox::keyPressEvent(QKeyEvent *evt){

    QLineEdit::keyPressEvent(evt);
    QStringList files = globVector("*/*.md");
    std::map<int, QString> matches;
    if (!text().isEmpty()){
        matches = findFiles(files, text());
    }
    emit updateMatches(matches);
}

std::map<int, QString> SearchBox::findFiles(const QStringList &files, const QString &text)
{
    std::map<int, QString> matchingFiles;

    std::string txtstr = text.toStdString();
    for (int i=0; i<files.size(); ++i){
        int score;
        std::string filestr = files[i].toStdString();
        if (fts::fuzzy_match_score(txtstr.c_str(), filestr.c_str(), score)){
            matchingFiles[score] = files[i];
        }
    }
    return matchingFiles;
}


void Window::showFiles(const std::map<int, QString> &files)
{
    filesTable->setRowCount(0);

    for (auto iter = files.rbegin(); iter != files.rend(); ++iter){
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(iter->second);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *rankItem = new QTableWidgetItem(tr("%1").arg(int(iter->first) ));
        rankItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        fileNameItem->setFlags(rankItem->flags() ^ Qt::ItemIsEditable);

        int row = filesTable->rowCount();
        filesTable->insertRow(row);
        filesTable->setItem(row, 0, fileNameItem);
        filesTable->setItem(row, 1, rankItem);
    }
}

void Window::createFilesTable()
{
    filesTable = new QTableWidget(0, 2);
    filesTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("Recipe") << tr("Rank");
    filesTable->setHorizontalHeaderLabels(labels);
    filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    filesTable->verticalHeader()->hide();
    filesTable->setShowGrid(false);

    connect(filesTable, &QTableWidget::cellActivated,
            this, &Window::openFileOfItem);
}


void Window::openFileOfItem(int row, int /* column */)
{
    QTableWidgetItem *item = filesTable->item(row, 0);

    QDesktopServices::openUrl(QUrl::fromLocalFile(currentDir.absoluteFilePath(item->text())));
}

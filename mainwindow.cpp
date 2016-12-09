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
    connect(searchBox, SIGNAL(updateMatches(std::map<double, QString>)), this, SLOT(showFiles(std::map<double, QString>)));
    createRecipeList();
    connect(searchBox, SIGNAL(returnPressed()), recipeList, SLOT(setFocus()));

    // Event filter to capture Esc key press
    keyEscapeReceiver* key = new keyEscapeReceiver();
    this->installEventFilter(key);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(textLabel, 0, 0);
    mainLayout->addWidget(searchBox, 0, 1, 1, 1);
    mainLayout->addWidget(recipeList, 1, 0, 1, 4);
    setLayout(mainLayout);

    setWindowTitle(tr("Find Recipes"));
    setFixedSize(600, 400);
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
    std::map<double, QString> matches;
    if (!text().isEmpty()){
        matches = findFiles(files, text());
    }
    emit updateMatches(matches);
}

std::map<double, QString> SearchBox::findFiles(const QStringList &files, const QString &text)
{
    std::map<double, QString> matchingFiles;

    std::string txtstr = text.toStdString();
    for (int i=0; i<files.size(); ++i){
        int score;
        std::string filestr = files[i].split('/')[1].toStdString();
        if (fts::fuzzy_match_score(txtstr.c_str(), filestr.c_str(), score)){
            // If a map entry already has the current score, increase score by 0.01.
            double dbscore = (double)score;
            if (matchingFiles.count(dbscore) > 0){
                dbscore += 0.01;
            }
            matchingFiles[dbscore] = files[i];
        }
    }
    return matchingFiles;
}


void Window::showFiles(const std::map<double, QString> &files)
{
    recipeList->clear();
    for (auto iter = files.rbegin(); iter != files.rend(); ++iter){
        QString path_name = iter->second;
        QString img_path = "Images/" + path_name.split('/')[1].replace(" ", "_").replace(".md", ".jpg");

        QListWidgetItem *recipe = new QListWidgetItem;
        recipe->setText(path_name.split('/')[1].replace(".md", ""));
        recipe->setData(Qt::UserRole, path_name);
        QImage *img = new QImage();
        bool loaded = img->load(img_path);
        if (loaded){
            recipe->setIcon(QPixmap::fromImage(*img));
        }
        recipeList->addItem(recipe);
    }
}

void Window::createRecipeList()
{
    recipeList = new QListWidget();
    recipeList->setViewMode(QListView::IconMode);
    recipeList->setIconSize(QSize(267, 150));
    recipeList->setGridSize(QSize(280, 185));
    recipeList->setWordWrap(true);
    recipeList->setTextElideMode(Qt::ElideNone);

    connect(recipeList, &QListWidget::itemDoubleClicked, this, &Window::openFile);
}

void Window::openFile(QListWidgetItem *recipe)
{
    // Read hidden data to find full file path
    QString path = recipe->data(Qt::UserRole).toString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(currentDir.absoluteFilePath(path)));
}

bool keyEscapeReceiver::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type()==QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if ( (key->key()==Qt::Key_Escape) ) {
            //Escape key was pressed
            QApplication::quit();
        } else {
            return QObject::eventFilter(obj, event);
        }
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
    return false;
}

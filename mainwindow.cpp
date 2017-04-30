#include <QtWidgets>
#include "mainwindow.h"
#include <fts_fuzzy_match.h>
#include <glob.h>
#include <string>
#include <map>

Window::Window(QWidget *parent) : QWidget(parent)
{
    searchBox = new SearchBox();
    searchBox->setPlaceholderText("Search for recipes");
    recipeBox = new QComboBox();
    QStringList recipeCategories;
    recipeCategories << "All Recipes" << "Beef" << "Chicken" << "Dessert" << "Lamb" << "Pork" << "Seafood" << "Turkey" << "Veggie";
    recipeBox->addItems(recipeCategories);
    createRecipeList();
    numResults = new QLabel();

    connect(searchBox, SIGNAL(updateMatches(std::map<double, QString>)), this, SLOT(showMatchedFiles(std::map<double, QString>)));
    connect(searchBox, SIGNAL(emptySearch()), this, SLOT(showAllFiles()));
    connect(searchBox, SIGNAL(returnPressed()), recipeList, SLOT(setFocus()));
    connect(recipeBox, SIGNAL(currentTextChanged(QString)), searchBox, SLOT(recipeFiterChanged(QString)));
    connect(recipeBox, SIGNAL(currentTextChanged(QString)), this, SLOT(showAllFiles()));
    // Set layout
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(searchBox, 0, 0, 1, 6);
    mainLayout->addWidget(numResults, 0, 6, 1, 1);
    mainLayout->addWidget(recipeBox, 0, 7, 1, 1);
    mainLayout->addWidget(recipeList, 1, 0, 1, 8);
    setLayout(mainLayout);
    // Set window paramters
    setWindowTitle(tr("Find Recipes"));
    setMinimumSize(600, 400);
    // Populate list on start up
    showAllFiles();
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

void SearchBox::recipeFiterChanged(QString newFilter){
    recipeFiter = newFilter;
}

void SearchBox::keyPressEvent(QKeyEvent *evt){

    // Set pattern based on category selection
    std::string pattern = "*/*.md";
    if(recipeFiter != "All Recipes"){
        pattern = recipeFiter.toStdString() + "/*.md";
    }

    QLineEdit::keyPressEvent(evt);
    QStringList files = globVector(pattern);
    std::map<double, QString> matches;
    if (!text().isEmpty()){
        matches = findFiles(files, text());
        emit updateMatches(matches);
    }else{
        emit emptySearch();
    }

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

void Window::showAllFiles(){
    recipeList->clear();

    std::string pattern = "*/*.md";
    if(recipeBox->currentText() != "All Recipes"){
        pattern = recipeBox->currentText().toStdString() + "/*.md";
    }
    QStringList files = globVector(pattern);
    for (int i=0; i<files.size(); ++i){
        QString path_name = files[i];
        QString img_path = "Images/" + path_name.split('/')[1].replace(" ", "_").replace(".md", ".jpg");

        QListWidgetItem *recipe = new QListWidgetItem;
        recipe->setText(path_name.split('/')[1].replace(".md", ""));
        recipe->setData(Qt::UserRole, path_name);
        QImage *img = new QImage();
        bool loaded = img->load(img_path);
        if (loaded){
            recipe->setIcon(QPixmap::fromImage(*img));
        }else{
            // If image doesn't exist, use placeholder image
            bool loaded = img->load("./Images/Placeholder.jpg");
            if (loaded){
                recipe->setIcon(QPixmap::fromImage(*img));
            }
        }
        recipeList->addItem(recipe);
        recipeList->sortItems();
    }
    QString text = QString("%1 recipes").arg(files.size());
    numResults->setText(text);

}


void Window::showMatchedFiles(const std::map<double, QString> &matchedfiles)
{
    recipeList->clear();
    for (auto iter = matchedfiles.rbegin(); iter != matchedfiles.rend(); ++iter){
        QString path_name = iter->second;
        QString img_path = "Images/" + path_name.split('/')[1].replace(" ", "_").replace(".md", ".jpg");

        QListWidgetItem *recipe = new QListWidgetItem;
        recipe->setText(path_name.split('/')[1].replace(".md", ""));
        recipe->setData(Qt::UserRole, path_name);
        QImage *img = new QImage();
        bool loaded = img->load(img_path);
        if (loaded){
            recipe->setIcon(QPixmap::fromImage(*img));
        }else{
            // If image doesn't exist, use placeholder image
            bool loaded = img->load("./Images/Placeholder.jpg");
            if (loaded){
                recipe->setIcon(QPixmap::fromImage(*img));
            }
        }
        recipeList->addItem(recipe);
    }
    QString text = QString("%1 recipes").arg(matchedfiles.size());
    numResults->setText(text);
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

void Window::resizeEvent(QResizeEvent *event){
    int iconWidth, iconHeight, gridWidth, gridHeight, columns;
    double gridRatio = 280.0/185.0;
    double iconRatio = 267.0/150.0;
    QSize recipeListSize = recipeList->size();

    if(recipeListSize.width()<=578){
        // Set defaults for minimum size
        columns = 2;
        iconWidth = 267;
        iconHeight = 150;
        gridWidth = 280;
        gridHeight = 185;
    }else{
        // Icons should never go larger than default, so set number of columns to round up
        columns = ceil(recipeListSize.width()/280.0);
        // Width of grid is widget_width/columns, with extra width removed to allow for scrollbar
        gridWidth = int(recipeListSize.width()/columns) - ceil(18.0/columns);
        // Calculate other parameters based on ratios of default values.
        gridHeight = int(gridWidth/gridRatio);
        iconWidth = gridWidth - 13;
        iconHeight = int(iconWidth/iconRatio);
    }

    recipeList->setIconSize(QSize(iconWidth, iconHeight));
    recipeList->setGridSize(QSize(gridWidth, gridHeight));
}


#include <QtWidgets>
#include "mainwindow.h"
#include <fts_fuzzy_match.h>
#include <glob.h>
#include <string>
#include <map>
#include <database.h>

Window::Window(QWidget *parent) : QMainWindow(parent)
{
    // Initalise database
    db.setDatabaseName("recipes.db");

    // Initialise widgets
    searchBox = new SearchBox();
    searchBox->setPlaceholderText("Search for recipes");
    recipeBox = new QComboBox();
    QStringList recipeCategories;
    recipeCategories << "All Recipes" << "Beef" << "Chicken" << "Dessert" << "Lamb" << "Pork" << "Seafood" << "Turkey" << "Veggie";
    recipeBox->addItems(recipeCategories);
    createRecipeList();
    numResults = new QLabel();

    // Set layout
    centralWidget = new QWidget();
    QGridLayout *mainLayout = new QGridLayout(centralWidget);
    mainLayout->addWidget(searchBox, 0, 0, 1, 6);
    mainLayout->addWidget(numResults, 0, 6, 1, 1);
    mainLayout->addWidget(recipeBox, 0, 7, 1, 1);
    mainLayout->addWidget(recipeList, 1, 0, 1, 8);
    centralWidget->setLayout(mainLayout);
    centralWidget->show();
    setCentralWidget(centralWidget);

    // Create menus
    optionsMenu = new QMenu("Options");

    updateDb = new QAction("Update database", this);
    connect(updateDb, &QAction::triggered, this, &Window::updateDatabase);
    cleanDb = new QAction("Clean database", this);
    connect(cleanDb, &QAction::triggered, this, &Window::cleanDatabase);
    optionsMenu->addAction(updateDb);
    optionsMenu->addAction(cleanDb);

    menuBar()->addMenu(optionsMenu);

    // Set window paramters
    setWindowTitle(tr("Find Recipes"));
    setMinimumSize(600, 400);

    // Set signals
    connect(recipeList, &QListWidget::itemDoubleClicked, this, &Window::openFile);
    connect(searchBox, SIGNAL(inputText(QString)), this, SLOT(updateRecipesDiplay(QString)));
    connect(searchBox, SIGNAL(returnPressed()), recipeList, SLOT(setFocus()));
    connect(recipeBox, SIGNAL(currentTextChanged(QString)), searchBox, SLOT(recipeFiterChanged(QString)));

    // Populate list on start up
    updateRecipesDiplay("");
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
    // When the recipe filter changes, emit this signal to call updateRecipeDisplay
    emit inputText(text());
}

void SearchBox::keyPressEvent(QKeyEvent *evt){
    QLineEdit::keyPressEvent(evt);
    // When the search changes, emit this signal to call updateRecipeDisplay
    emit inputText(text());
}

void Window::updateRecipesDiplay(QString searchText){
    recipeList->clear();

    QList<QListWidgetItem*> recipes = getRecipeList(searchText);
    for (int i=0; i<recipes.size(); ++i){
        recipeList->addItem(recipes[i]);
    }

    if(searchText.isEmpty()){
        recipeList->sortItems();
    }

    QString text = QString("%1 recipes").arg(recipes.size());
    if (recipes.size() == 1){
        text = "1 recipe";
    }
    numResults->setText(text);
}

QList<QListWidgetItem*> Window::getRecipeList(QString searchText){
    QList<QListWidgetItem*> recipes;
    if (searchText.isEmpty()) {
        recipes = getAllRecipes();
    }else{
        recipes = getMatchingRecipes(searchText);
    }
    return recipes;
}

QList<QListWidgetItem*> Window::getAllRecipes(){
    QList<QListWidgetItem*> recipes;

    // Open database and execute query
    db.open();
    // Prepare query based on filter
    QSqlQuery query = QSqlQuery();
    if(recipeBox->currentText() != "All Recipes"){
        QString category = recipeBox->currentText();
        query.prepare("select TITLE, IMG_PATH, FILE_PATH from RECIPES where CATEGORY = :category");
        query.bindValue(":category", category);
        query.setForwardOnly(true);
    }else{
        query.prepare("select TITLE, IMG_PATH, FILE_PATH from RECIPES");
        query.setForwardOnly(true);
    }
    // Execute query
    query.exec();

    while(query.next()){
        // Extract info from query results
        QString title = query.value(0).toString();
        QString img_path = query.value(1).toString();
        QString file_path = query.value(2).toString();

        // Create QListWidgetItems
        QListWidgetItem *recipe = new QListWidgetItem;
        recipe->setText(title);
        recipe->setData(Qt::UserRole, file_path);

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
        recipes.append(recipe);
    }
    db.close();
    return recipes;
}


QList<QListWidgetItem*> Window::getMatchingRecipes(QString searchText){
    QList<QListWidgetItem*> recipes;

    // Get matching recipes and their scores. The QStringList contains title, img_path, file_path in order.
    std::map<double, QStringList> matchingRecipes = findMatches(searchText);
    // Build QListWidgetItems and add to QList
    // By default the map should be in ascending order, so use reverse iterator to get highest matches first
    for (auto iter = matchingRecipes.rbegin(); iter != matchingRecipes.rend(); ++iter){

        QString title = iter->second[0];
        QString img_path = iter->second[1];
        QString file_path = iter->second[2];

        QListWidgetItem *recipe = new QListWidgetItem;
        recipe->setText(title);
        recipe->setData(Qt::UserRole, file_path);

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
        recipes.append(recipe);
    }

    return recipes;
}

std::map<double, QStringList> Window::findMatches(QString text)
{
    // Open database
    db.open();
    // Prepare query based on filter
    QSqlQuery query = QSqlQuery();
    if(recipeBox->currentText() != "All Recipes"){
        QString category = recipeBox->currentText();
        query.prepare("select TITLE, IMG_PATH, FILE_PATH from RECIPES where CATEGORY = :category");
        query.bindValue(":category", category);
        query.setForwardOnly(true);
    }else{
        query.prepare("select TITLE, IMG_PATH, FILE_PATH from RECIPES");
        query.setForwardOnly(true);
    }
    // Execute query
    query.exec();

    // Get matching files and their scores
    std::map<double, QStringList> matchingFiles;
    std::string txtstr = text.toStdString();
    while(query.next()){
        int score;
        QString title = query.value(0).toString();
        QString img_path = query.value(1).toString();
        QString file_path = query.value(2).toString();

        std::string titlestr = title.toStdString();
        if (fts::fuzzy_match_score(txtstr.c_str(), titlestr.c_str(), score)){
            // If a map entry already has the current score, increase score by 0.01.
            double dbscore = (double)score;
            if (matchingFiles.count(dbscore) > 0){
                dbscore += 0.01;
            }
            matchingFiles[dbscore] = QStringList() << title << img_path << file_path;
        }
    }
    return matchingFiles;
}

void Window::createRecipeList()
{
    recipeList = new QListWidget();
    recipeList->setViewMode(QListView::IconMode);
    recipeList->setIconSize(QSize(267, 150));
    recipeList->setGridSize(QSize(280, 185));
    recipeList->setWordWrap(true);
    recipeList->setTextElideMode(Qt::ElideNone);
}

void Window::openFile(QListWidgetItem *recipe)
{
    // Read hidden data to find full file path
    QString path = recipe->data(Qt::UserRole).toString();
    QDesktopServices::openUrl(QUrl::fromLocalFile(currentDir.absoluteFilePath(path)));
}

void Window::updateDatabase(){
    db_ops::update_database(&db);
}

void Window::cleanDatabase(){
    db_ops::clean_database(&db);
}

void Window::resizeEvent(QResizeEvent *event){
    int iconWidth, iconHeight, gridWidth, gridHeight, columns;
    double gridRatio = 280.0/185.0;
    double iconRatio = 267.0/150.0;
    QSize recipeListSize = recipeList->size();

    if(recipeListSize.width()<=587){
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


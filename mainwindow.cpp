#define FTS_FUZZY_MATCH_IMPLEMENTATION
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
    searchBox->setClearButtonEnabled(true);
    recipeBox = new QComboBox();
    populateRecipeBox(recipeBox);
    createRecipeListWidget();
    numResults = new QLabel();
    recipeView = new QWebEngineView();

    // Set layout
    centralWidget = new QWidget();
    QGridLayout *sidebar = new QGridLayout();
    sidebar->addWidget(searchBox, 0, 0, 1, 2);
    sidebar->addWidget(numResults, 1, 0, 1, 1);
    sidebar->addWidget(recipeBox, 1, 1, 1, 1);
    sidebar->addWidget(recipeList, 2, 0, 1, 2);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addLayout(sidebar, 1);
    mainLayout->addWidget(recipeView, 3);

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
    setMinimumSize(940, 400);

    // Set signals
    connect(recipeList, &QListWidget::itemClicked, this, &Window::openFile);
    connect(searchBox, SIGNAL(inputText(QString)), this, SLOT(updateRecipesDiplay(QString)));
    connect(searchBox, SIGNAL(returnPressed()), recipeList, SLOT(setFocus()));
    connect(recipeBox, SIGNAL(currentTextChanged(QString)), searchBox, SLOT(recipeFiterChanged(QString)));

    // Update on startup
    updateDatabase();
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
    Q_UNUSED(newFilter);
    // When the recipe filter changes, emit this signal to call updateRecipeDisplay
    emit inputText(text());
}

void SearchBox::keyPressEvent(QKeyEvent *evt){
    QLineEdit::keyPressEvent(evt);
    setPlaceholderText("Search for recipes");
    // When the search changes, emit this signal to call updateRecipeDisplay
    emit inputText(text());
}

void Window::populateRecipeBox(QComboBox* box){
    // Open database
    db.open();
    // Prepare query
    QSqlQuery query = QSqlQuery();
    query.prepare("SELECT CATEGORY, COUNT(*) FROM RECIPES GROUP BY CATEGORY ORDER BY CATEGORY");
    query.setForwardOnly(true);
    query.exec();

    box->addItem("All Recipes");
    while(query.next()){
        box->addItem(query.value("CATEGORY").toString());
    }
    db.close();
}

void Window::updateRecipesDiplay(QString searchText){
    recipeList->clear();
    QList<QListWidgetItem*> recipes;

    if (searchText.isEmpty()) {
        recipes = getAllRecipes();
        for (int i=0; i<recipes.size(); ++i){
            recipeList->addItem(recipes[i]);
        }
        recipeList->sortItems();

    }else{
        recipes = getMatchingRecipes(searchText);
        for (int i=0; i<recipes.size(); ++i){
            recipeList->addItem(recipes[i]);
        }
    }

    recipeList->setDragEnabled(false);

    QString text = QString("%1 recipes").arg(recipes.size());
    if (recipes.size() == 1){
        text = "1 recipe";
    }
    numResults->setText(text);
}

QList<QListWidgetItem*> Window::getAllRecipes(){
    QList<QListWidgetItem*> recipes;

    // Open database and execute query
    db.open();
    // Prepare query based on filter
    QSqlQuery query = QSqlQuery();
    if(recipeBox->currentText() != "All Recipes"){
        QString category = recipeBox->currentText();
        query.prepare("SELECT TITLE, IMG_PATH, HTML_PATH FROM RECIPES WHERE CATEGORY = :category");
        query.bindValue(":category", category);
        query.setForwardOnly(true);
    }else{
        query.prepare("SELECT TITLE, IMG_PATH, HTML_PATH FROM RECIPES");
        query.setForwardOnly(true);
    }
    // Execute query
    query.exec();

    while(query.next()){
        // Extract info from query results
        QString title = query.value("TITLE").toString();
        QString img_path = query.value("IMG_PATH").toString();
        QString html_path = query.value("HTML_PATH").toString();

        // Create QListWidgetItems
        QListWidgetItem *recipe = new QListWidgetItem;
        recipe->setText(title);
        recipe->setData(Qt::UserRole, html_path);

        QImage *img = new QImage();
        bool loaded = img->load(img_path);
        if (loaded){
            recipe->setIcon(QPixmap::fromImage(*img));
        }else{
            // If image doesn't exist, use placeholder image
            bool loaded = img->load("./images/Placeholder.jpg");
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
        QString html_path = iter->second[2];

        QListWidgetItem *recipe = new QListWidgetItem;
        recipe->setText(title);
        recipe->setData(Qt::UserRole, html_path);

        QImage *img = new QImage();
        bool loaded = img->load(img_path);
        if (loaded){
            recipe->setIcon(QPixmap::fromImage(*img));
        }else{
            // If image doesn't exist, use placeholder image
            bool loaded = img->load("./images/Placeholder.jpg");
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
        query.prepare("SELECT TITLE, IMG_PATH, HTML_PATH FROM RECIPES WHERE CATEGORY = :category");
        query.bindValue(":category", category);
        query.setForwardOnly(true);
    }else{
        query.prepare("SELECT TITLE, IMG_PATH, HTML_PATH FROM RECIPES");
        query.setForwardOnly(true);
    }
    // Execute query
    query.exec();

    // Get matching files and their scores
    std::map<double, QStringList> matchingFiles;
    std::string txtstr = text.toStdString();
    while(query.next()){
        int score;
        QString title = query.value("TITLE").toString();
        QString img_path = query.value("IMG_PATH").toString().replace("\'", "").replace(",", "");
        QString file_path = query.value("HTML_PATH").toString();

        std::string titlestr = title.toStdString();
        if (fts::fuzzy_match(txtstr.c_str(), titlestr.c_str(), score)){
            // If a map entry already has the current score, increase score by 0.01.
            double dbscore = (double)score;
            if (matchingFiles.count(dbscore) > 0){
                dbscore += 0.01;
            }
            matchingFiles[dbscore] = QStringList() << title << img_path << file_path;
        }
    }
    db.close();
    return matchingFiles;
}

void Window::createRecipeListWidget()
{
    recipeList = new QListWidget();
    recipeList->setViewMode(QListView::IconMode);
    recipeList->setGridSize(QSize(212, int(212/1.51)) );
    recipeList->setIconSize(QSize(199, int(199/1.78)) );
    recipeList->setWordWrap(true);
    recipeList->setTextElideMode(Qt::ElideNone);
    recipeList->horizontalScrollBar()->setEnabled(false);
    recipeList->horizontalScrollBar()->setVisible(false);
}

void Window::openFile(QListWidgetItem *recipe)
{
    // Read hidden data to find full file path
    QString path = recipe->data(Qt::UserRole).toString();
    QString url = "file://" + currentDir.absoluteFilePath(path);
    recipeView->load(url);
}

void Window::updateDatabase(){
    int num_updates = db_ops::update_database(&db);
    QString updated_text;
    if (num_updates == 0){
        updated_text = QString("Search for recipes");
    }else{
        updated_text = QString("Search for recipes - Updated!").arg(num_updates);
    }
    searchBox->setPlaceholderText(updated_text);
    // Repopulate list
    updateRecipesDiplay("");
}

void Window::cleanDatabase(){
    int num_removals = db_ops::clean_database(&db);
    QString cleaned_text;
    if (num_removals == 1){
        cleaned_text = QString("Search for recipes - Removed 1 recipe!");
    }else{
        cleaned_text = QString("Search for recipes - Removed %1 recipes!").arg(num_removals);
    }
    searchBox->setPlaceholderText(cleaned_text);
    // Repopulate list
    updateRecipesDiplay("");
}

void Window::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event);
    int iconWidth, iconHeight, gridWidth, gridHeight;
    double gridRatio = 1.51;
    double iconRatio = 1.78;
    QSize recipeListSize = recipeList->size();

    // Set gridwith based on recipeList size and ensure it doesn't go below minimum value
    gridWidth = qMax(int(recipeListSize.width()) - 17.0, 212.0);
    // Calculate other parameters based on ratios of default values.
    gridHeight = int(gridWidth/gridRatio);
    iconWidth = gridWidth - 13;
    iconHeight = int(iconWidth/iconRatio);

    recipeList->setIconSize(QSize(iconWidth, iconHeight));
    recipeList->setGridSize(QSize(gridWidth, gridHeight));
    recipeList->setUniformItemSizes(true);
}

